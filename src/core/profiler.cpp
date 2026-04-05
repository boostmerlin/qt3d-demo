#include <algorithm>
#include <utility>
#include <vector>
#include <iostream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <set>
#include <functional>
#include <iomanip>
#include <limits>
#include <cassert>
#include "profiler.h"


#ifdef _WIN32
#include <windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
void enableConsoleAnsi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return;
    }
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
}
#endif

template<>
struct std::hash<std::pair<std::thread::id, std::string> > {
    size_t operator()(const std::pair<std::thread::id, std::string> &p) const noexcept {
        size_t h1 = std::hash<std::thread::id>{}(p.first);
        size_t h2 = std::hash<std::string>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

namespace util {
    namespace Color {
        auto *Reset = "\033[0m";
        auto *Red = "\033[1;31m";
        auto *Green = "\033[1;32m";
        auto *Yellow = "\033[1;33m";
        auto *Magenta = "\033[1;35m";
    }

    constexpr size_t Profiler::kNumLocks;
    std::vector<std::mutex> Profiler::s_bucketMutexes(Profiler::kNumLocks);
    std::vector<std::unordered_map<std::thread::id, std::shared_ptr<Profiler::ThreadData> > >
    Profiler::s_allThreadDataRegistry(Profiler::kNumLocks);

    void Profiler::setThreadData(const std::shared_ptr<ThreadData> &td) {
        assert(td && "ThreadData pointer is null!");
        auto &tid = td->threadId;
        assert(tid != std::thread::id{} && "Invalid thread id!");
        size_t idx = std::hash<std::thread::id>{}(tid) % kNumLocks;
        std::lock_guard<std::mutex> lock(s_bucketMutexes[idx]);
        s_allThreadDataRegistry[idx][tid] = td;
    }

    std::vector<Profiler::SessionData> Profiler::collectAllSessions() {
        std::vector<SessionData> sessionList;
        for (const auto &seg: s_allThreadDataRegistry) {
            for (const auto &[_, td]: seg) {
                for (const auto &[_, data]: td->sessions) {
                    sessionList.push_back(data);
                }
            }
        }
        return sessionList;
    }

    static double convertTime(int64_t duration, Profiler::TimeUnit unit) {
        if (duration == std::numeric_limits<int64_t>::max()) {
            return 0.0;
        }

        using TimeUnit = Profiler::TimeUnit;
        std::chrono::duration<int64_t, std::micro> d(duration);
        switch (unit) {
            case TimeUnit::MICROSECONDS:
                return d.count();
            case TimeUnit::MILLISECONDS:
                return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
            case TimeUnit::SECONDS:
                return std::chrono::duration_cast<std::chrono::seconds>(d).count();
            default:
                return d.count();
        }
    }

    static std::string getUnitSuffix(Profiler::TimeUnit unit) {
        switch (unit) {
            case Profiler::TimeUnit::MICROSECONDS: return "μs";
            case Profiler::TimeUnit::MILLISECONDS: return "ms";
            case Profiler::TimeUnit::SECONDS: return "s";
            default: return "ms";
        }
    }

    static const char *getColor(double duration) {
        if (duration > 100) {
            return Color::Red;
        } else if (duration > 10) {
            return Color::Yellow;
        } else if (duration >= 0) {
            return Color::Green;
        }
        return Color::Magenta;
    }

    static std::string internalFormatTime(int64_t duration, Profiler::TimeUnit displayUnit) {
        double converted = convertTime(duration, displayUnit);
        const char *color = getColor(convertTime(duration, Profiler::TimeUnit::MILLISECONDS));
        std::ostringstream ss;
        ss << color << std::fixed << std::setprecision(3) << converted << Color::Reset;
        return ss.str();
    }

    std::string Profiler::formatTime(int64_t duration) const {
        return internalFormatTime(duration, m_displayUnit);
    }

    thread_local std::shared_ptr<Profiler::ThreadData> Profiler::s_threadData = nullptr;

    Profiler::ThreadData &Profiler::getThreadData() {
        if (!s_threadData) {
            s_threadData = std::make_shared<ThreadData>();
            setThreadData(s_threadData);
        }
        assert(s_threadData->threadId != std::thread::id{} && "Invalid thread id!");

        return *s_threadData;
    }

    Profiler::Profiler(std::string name) : m_name(std::move(name)), m_mainThreadId(std::this_thread::get_id()) {
#ifdef _WIN32
        enableConsoleAnsi();
#endif
    }

    Profiler::Profiler() : Profiler("[PROFILER]") {
    }

    Profiler::ThreadData::ThreadData(): threadId(std::this_thread::get_id()) {
    }

    Profiler &Profiler::instance() {
        static Profiler instance;
        return instance;
    }

    void Profiler::begin(std::string_view tag) const {
        if (!isEnabled()) {
            return;
        }
        auto &threadData = getThreadData();
        auto stag = std::string(tag);
        auto &session = threadData.sessions[stag];
        if (!threadData.tagStack.empty()) {
            session.parentTag = threadData.tagStack.top().first;
        }
        threadData.tagStack.emplace(stag, Clock::now());
        session.threadId = threadData.threadId;
        session.tag = std::move(stag);
    }

    void Profiler::end(std::string_view tag) const {
        auto now = Clock::now();
        if (!isEnabled()) {
            return;
        }
        auto &threadData = getThreadData();
        if (threadData.tagStack.empty()) {
            return;
        }
        auto [lastTag, startTime] = threadData.tagStack.top();
        threadData.tagStack.pop();
        if (lastTag != tag) {
            return;
        }
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count();
        auto &session = threadData.sessions[lastTag];
        session.callCount++;
        session.totalTime += duration;
        session.minTime = std::min(session.minTime, duration);
        session.maxTime = std::max(session.maxTime, duration);
        session.lastDuration = duration;
    }

    using NodeKey = std::pair<std::thread::id, std::string>;

    static void printNode(const std::string &tag,
                          const std::unordered_map<std::string, std::set<std::string> > &tree,
                          const std::unordered_map<std::string, const util::Profiler::SessionData *> &sessionMap,
                          const std::string &prefix,
                          bool isLast,
                          util::Profiler::TimeUnit displayTimeUnit) {
        if (sessionMap.count(tag) == 0) return;
        const auto *session = sessionMap.at(tag);

        auto catPrefix = [prefix, isLast](const char *b1, const char *b2) {
            return prefix + (isLast ? b1 : b2);
        };
        std::cout << catPrefix("└─", "├─") << session->tag << std::endl;

        auto avg = session->callCount ? session->totalTime / session->callCount : 0;
        std::cout << catPrefix("  ", "│ ") << "☝Total: " << internalFormatTime(session->totalTime, displayTimeUnit)
                << ", Count: " << session->callCount
                << ", Avg: " << internalFormatTime(avg, displayTimeUnit)
                << ", Min: " << internalFormatTime(session->minTime, displayTimeUnit)
                << ", Max: " << internalFormatTime(session->maxTime, displayTimeUnit)
                << std::endl;

        if (tree.count(tag)) {
            const auto &children = tree.at(tag);
            auto end = children.end();
            for (auto it = children.begin(); it != end; ++it) {
                printNode(*it, tree, sessionMap, catPrefix("  ", "│ "), std::next(it) == end, displayTimeUnit);
            }
        }
    }

    void Profiler::print() const {
        const auto header = "\n===============" + m_name + " Summary==============\n";
        std::cout << header;

        std::unordered_map<std::thread::id, std::vector<const SessionData *> > threadGroups;
        auto sessions = collectAllSessions();
        for (const auto &session: sessions) {
            threadGroups[session.threadId].push_back(&session);
        }
        for (const auto &[tid, sessions]: threadGroups) {
            std::cout << (m_mainThreadId == tid ? "[main-" : "[thread-") << tid << "]\n";

            std::unordered_map<std::string, std::set<std::string> > tree;
            std::unordered_map<std::string, const SessionData *> sessionMap;
            std::set<std::string> rootNodes;

            for (auto &session: sessions) {
                tree[session->parentTag].insert(session->tag);
                sessionMap[session->tag] = session;
            }

            for (const auto &[tag, session]: sessionMap) {
                if (session->parentTag.empty() || sessionMap.find(session->parentTag) == sessionMap.end()) {
                    rootNodes.insert(tag);
                }
            }

            if (!rootNodes.empty()) {
                size_t rootIdx = 0;
                for (const auto &rootTag: rootNodes) {
                    bool isLastRoot = (++rootIdx == rootNodes.size());
                    printNode(rootTag, tree, sessionMap, "", isLastRoot, m_displayUnit);
                }
            } else {
                std::cout << "  No root nodes found\n";
            }

            std::cout << "\n";
        }

        std::cout << std::string(header.size() - 2, '=') << std::endl;
    }

    void Profiler::print(int n) const {
        if (!m_enable) {
            return;
        }
        auto sessionList = collectAllSessions();
        if (sessionList.empty()) {
            std::cout << "No profiling data available." << std::endl;
            return;
        }
        std::ranges::sort(sessionList, [](const auto &a, const auto &b) {
            return a.totalTime > b.totalTime;
        });


        std::stringstream ss;
        std::string unitSuffix = getUnitSuffix(m_displayUnit);
        int maxCount = (n > 0 && n < static_cast<int>(sessionList.size())) ? n : static_cast<int>(sessionList.size());
        ss << "\n=============" << m_name << " Top " << maxCount << " Records" << "============\n";
        auto peekLen = formatTime(sessionList[0].totalTime).length();
        const auto size = std::clamp(peekLen, size_t(12), size_t(15));
        const auto tagSize = 28;
        const auto dotSize = size * 5 + tagSize + 1;
        ss << std::left << std::setw(tagSize) << "Tag"
                << std::setw(size) << "ThreadId"
                << std::setw(size - 5) << "Count"
                << std::setw(size) << ("Total(" + unitSuffix + ")")
                << std::setw(size) << ("Avg(" + unitSuffix + ")")
                << std::setw(size) << ("Min(" + unitSuffix + ")")
                << std::setw(size) << ("Max(" + unitSuffix + ")")
                << std::endl;
        ss << std::string(dotSize, '-') << std::endl;

        for (int i = 0; i < maxCount; ++i) {
            const auto &data = sessionList[i];
            const auto total = formatTime(data.totalTime);
            const auto avg = formatTime(data.callCount > 0 ? data.totalTime / data.callCount : 0);
            const auto min = formatTime(data.minTime);
            const auto max = formatTime(data.maxTime);

            ss << std::left << std::setw(tagSize) << data.tag
                    << std::setw(1) << (m_mainThreadId == data.threadId ? "*" : "")
                    << std::setw(size) << data.threadId
                    << std::setw(size - 5) << data.callCount
                    << std::setw(size + 11) << total
                    << std::setw(size + 11) << avg
                    << std::setw(size + 11) << min
                    << std::setw(size + 11) << max
                    << std::endl;
        }
        ss << std::string(dotSize, '-') << std::endl;
        std::cout << ss.str();
    }

    const Profiler::SessionData *Profiler::findSession(std::string_view tag) {
        if (!s_threadData) {
            return nullptr;
        }
        const auto it = s_threadData->sessions.find(std::string(tag));
        if (it != s_threadData->sessions.end()) {
            return &it->second;
        }
        return nullptr;
    }

    void Profiler::print(std::string_view tag) const {
        if (!isEnabled()) {
            return;
        }
        if (const auto s = findSession(tag); s) {
            std::stringstream ss;
            ss << m_name << " " << s->tag << ((m_mainThreadId == s->threadId) ? " [main-" : " [thread-") << s->threadId
                    << "]";
            if (s->callCount == 1) {
                ss << ": " <<
                        formatTime(s->lastDuration) << " " << getUnitSuffix(m_displayUnit);
            } else {
                ss << std::string(10, '>') << "\n"
                        << "Total: " << formatTime(s->totalTime) << " " << getUnitSuffix(m_displayUnit) << "\n"
                        << "Call Count: " << s->callCount << "\n"
                        << "Min: " << formatTime(s->minTime) << " " << getUnitSuffix(m_displayUnit) << "\n"
                        << "Max: " << formatTime(s->maxTime) << " " << getUnitSuffix(m_displayUnit) << std::string(
                            10, '<');
            }
            std::cout << ss.str() << std::endl;
        }
    }

    void Profiler::reset() {
        for (size_t i = 0; i < kNumLocks; ++i) {
            std::lock_guard<std::mutex> lock(s_bucketMutexes[i]);
            s_allThreadDataRegistry[i].clear();
        }
    }

    void Profiler::enable(bool enable) {
        m_enable = enable;
    }

    bool Profiler::isEnabled() const {
        return m_enable;
    }

    static std::string getTimeUnitSuffix(Profiler::TimeUnit unit) {
        switch (unit) {
            case Profiler::TimeUnit::MICROSECONDS: return "μs";
            case Profiler::TimeUnit::MILLISECONDS: return "ms";
            case Profiler::TimeUnit::SECONDS: return "s";
            default: return "ms";
        }
    }

    static void appendYamlNode(std::ostream &out,
                               const Profiler::SessionData *node,
                               const std::unordered_map<std::string, std::set<std::string> > &tree,
                               const std::unordered_map<std::string, const Profiler::SessionData *> &sessionMap,
                               Profiler::TimeUnit displayUnit,
                               const std::string &indent) {
        auto avg = node->callCount ? node->totalTime / node->callCount : 0;
        std::string tag = node->tag;

        out << indent << "- tag: \"" << tag << "\"\n"
                << indent << "  thread_id: " << node->threadId << "\n"
                << indent << "  total_time: " << convertTime(node->totalTime, displayUnit) << "\n"
                << indent << "  call_count: " << node->callCount << "\n"
                << indent << "  avg_time: " << (node->callCount > 0 ? convertTime(avg, displayUnit) : 0.0) << "\n"
                << indent << "  min_time: " << convertTime(node->minTime, displayUnit) << "\n"
                << indent << "  max_time: " << convertTime(node->maxTime, displayUnit) << "\n";

        if (tree.count(node->tag) && !tree.at(node->tag).empty()) {
            out << indent << "  children:\n";
            for (const auto &childTag: tree.at(node->tag)) {
                if (sessionMap.count(childTag)) {
                    appendYamlNode(out, sessionMap.at(childTag), tree, sessionMap, displayUnit, indent + "   ");
                }
            }
        }
    }

    void Profiler::toYaml(std::ostream &out) const {
        auto sessions = collectAllSessions();
        std::unordered_map<std::thread::id, std::vector<const SessionData *> > threadGroups;

        for (const auto &session: sessions) {
            threadGroups[session.threadId].push_back(&session);
        }

        out << "# Profiler Export\n"
                << "name: " << "\"" << m_name << "\"\n"
                << "time_unit: " << getTimeUnitSuffix(m_displayUnit) << "\n"
                << "threads:\n";

        for (const auto &[tid, sessions]: threadGroups) {
            out << "- thread_id: " << tid << "\n"
                    << "  main_thread: " << (tid == m_mainThreadId ? "true" : "false") << "\n";

            std::unordered_map<std::string, const SessionData *> sessionMap;
            std::unordered_map<std::string, std::set<std::string> > tree;
            std::set<std::string> rootNodes;

            for (const auto &session: sessions) {
                sessionMap[session->tag] = session;
                tree[session->parentTag].insert(session->tag);
            }

            for (const auto &[tag, session]: sessionMap) {
                if (session->parentTag.empty() || sessionMap.find(session->parentTag) == sessionMap.end()) {
                    rootNodes.insert(tag);
                }
            }

            out << "  nodes:\n";
            for (const auto &rootTag: rootNodes) {
                if (sessionMap.count(rootTag)) {
                    appendYamlNode(out, sessionMap.at(rootTag), tree, sessionMap, m_displayUnit, "   ");
                }
            }
        }
    }


    ScopedProfiler::ScopedProfiler(std::string_view tag, bool print): m_print(print), m_tag(tag) {
        Profiler::instance().begin(m_tag);
    }

    ScopedProfiler::~ScopedProfiler() {
        Profiler::instance().end(m_tag);
        if (m_print) {
            Profiler::instance().print(m_tag);
        }
    }
}
