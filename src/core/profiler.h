#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <stack>
#include <iostream>

namespace util {
    class Profiler {
    public:
        enum class TimeUnit {
            MICROSECONDS,
            MILLISECONDS,
            SECONDS
        };

        using Clock = std::chrono::steady_clock;

        explicit Profiler(std::string name);

        struct SessionData {
            Clock::time_point startTime;
            int64_t totalTime = 0;
            int callCount = 0;
            int64_t minTime = std::numeric_limits<int64_t>::max();
            int64_t maxTime = 0;
            int64_t lastDuration = 0;
            std::string tag;
            std::thread::id threadId;
            std::string parentTag;
        };

        struct ThreadData {
            std::unordered_map<std::string, SessionData> sessions;
            const std::thread::id threadId;
            std::stack<std::pair<std::string, Clock::time_point> > tagStack;
            ThreadData();
            ThreadData(const ThreadData &) = delete;
            ThreadData &operator=(const ThreadData &) = delete;
        };

        static Profiler &instance();

        void begin(std::string_view tag) const;

        void end(std::string_view tag) const;

        void print() const;

        void print(int n) const;

        void print(std::string_view tag) const;

        static void reset();

        void enable(bool enable);

        [[nodiscard]] bool isEnabled() const;

        void setDisplayTimeUnit(const TimeUnit unit) { m_displayUnit = unit; }

        [[nodiscard]] TimeUnit getDisplayTimeUnit() const { return m_displayUnit; }

        [[nodiscard]] static const SessionData *findSession(std::string_view tag);

        void toYaml(std::ostream& out = std::cout) const;
    private:
        [[nodiscard]] std::string formatTime(int64_t duration) const;
        Profiler();
        static constexpr size_t kNumLocks = 16;
        static std::vector<std::mutex> s_bucketMutexes;
        static std::vector<std::unordered_map<std::thread::id, std::shared_ptr<ThreadData> > > s_allThreadDataRegistry;
        static thread_local std::shared_ptr<ThreadData> s_threadData;

        static void setThreadData(const std::shared_ptr<ThreadData> &td);
        static std::vector<SessionData> collectAllSessions();
        static ThreadData &getThreadData();

        std::atomic<bool> m_enable{true};
        TimeUnit m_displayUnit{TimeUnit::MILLISECONDS};
        const std::string m_name;
        const std::thread::id m_mainThreadId;
    };

    class ScopedProfiler {
    public:
        explicit ScopedProfiler(std::string_view tag, bool print = true);

        ~ScopedProfiler();

        ScopedProfiler(const ScopedProfiler &) = delete;

        ScopedProfiler &operator=(const ScopedProfiler &) = delete;

    private:
        bool m_print;
        std::string m_tag;
    };
}

#define CAT(a, b) a##b
#define CCAT(a, b) CAT(a,b)

#ifndef ENABLE_PROFILER
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
#define ENABLE_PROFILER 1
#else
#define ENABLE_PROFILER 0
#endif
#endif

#if ENABLE_PROFILER
#define X_PROFILE(tag, ...) util::ScopedProfiler CCAT(_profiler, __LINE__)(tag, ##__VA_ARGS__)
#define X_PROFILE_FUNC(...) X_PROFILE(__FUNCTION__, ##__VA_ARGS__)
#define X_PROFILE_PRINT(...) util::Profiler::instance().print(__VA_ARGS__)
#define X_PROFILE_BEGIN(tag) util::Profiler::instance().begin(tag)
#define X_PROFILE_END(tag) util::Profiler::instance().end(tag)
#define X_PROFILE_END_PRINT(tag) (util::Profiler::instance().end(tag),X_PROFILE_PRINT(tag))
#define X_PROFILE_RESET() util::Profiler::reset()
#define X_PROFILE_TO_YAML(...) util::Profiler::instance().toYaml(__VA_ARGS__)
#else
#define X_PROFILE(tag, ...) ((void)0)
#define X_PROFILE_FUNC(...) ((void)0)
#define X_PROFILE_PRINT(...) ((void)0)
#define X_PROFILE_BEGIN(tag) ((void)0)
#define X_PROFILE_END(tag) ((void)0)
#define X_PROFILE_END_PRINT(tag) ((void)0)
#define X_PROFILE_RESET() ((void)0)
#define X_PROFILE_TO_YAML(...) ((void)0)
#endif
