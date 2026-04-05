//
// Created by merlin
//

#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <QCoreApplication>
#include <QEvent>
#include <QPointer>

class EventNotifier {
public:
    using EventType = int;
    static constexpr EventType All = -1;
    static constexpr EventType None = QEvent::Type::None;
    explicit EventNotifier(size_t offset, EventType autoWatchEventType = None);
    ~EventNotifier();

    // watch event of eventType
    // watch [None] event will unwatch ALL event
    void watchEvent(EventType eventType) const;
    // unwatch event of eventType
    // unwatch [None] event will not change current watch status
    void unwatchEvent(EventType eventType) const;

protected:
    template<typename T>
    constexpr static size_t offset() {
        static_assert(std::is_base_of_v<QObject, T> && std::is_base_of_v<EventNotifier, T>,
                      "T must be both inherited from QObject and EventNotifier");
        return Q_OFFSETOF(T, m_offset);
    }

private:
    const size_t m_offset;
    friend class EventBus;
};

// ReSharper disable once CppUnnamedNamespaceInHeaderFile
namespace {
    struct DummyObject {
    };

    template<typename T>
    using QObjectLike = std::conditional_t<std::is_void_v<T>, QObject, T>;
    template<typename T>
    using NotifierLike = std::conditional_t<std::is_base_of_v<EventNotifier, T>, DummyObject, EventNotifier>;
}

//helper template class auto calculate and init offset
template<typename T>
struct EventNotifierT : EventNotifier {
    explicit EventNotifierT(EventType autoWatchEventType = None) : EventNotifier(offset<T>(), autoWatchEventType) {
    }
};

template<typename T = void, class... Types>
struct WithEventNotifier : QObjectLike<T>, NotifierLike<T>, Types... {
    using QObjectLikeT = QObjectLike<T>;
    static_assert(std::is_base_of_v<QObject, QObjectLikeT>);

    template<typename O>
    explicit WithEventNotifier(O *parent = nullptr) : QObjectLikeT(parent), EventNotifier(sizeof(QObjectLikeT)) {
    }

    template<typename O, typename = std::enable_if_t<std::conjunction_v<std::is_default_constructible<Types>...> > >
    explicit WithEventNotifier(EventNotifier::EventType eventType = EventNotifier::None,
                               O *parent = nullptr) : QObjectLikeT(parent),
                                                      EventNotifier(sizeof(QObjectLikeT),
                                                                    eventType) {
    }

    template<typename... Args>
    explicit WithEventNotifier(const Types &... types, EventNotifier::EventType eventType = EventNotifier::None,
                               Args &&... args) : QObjectLikeT(args...),
                                                  EventNotifier(sizeof(QObjectLikeT), eventType), Types(types)... {
    }
};

class EventBus final {
public:
    /**
    * Sends event event directly to receiver receiver, Returns true if all the event handler handled the event, else false.
    * The event is not deleted when the event has been sent. The normal approach is to create the event on the stack
    * see also QCoreApplication::sendEvent()
    * \code
    * QMouseEvent event(QEvent::MouseButtonPress, pos, 0, 0, 0);
    * EventCenter::sendEvent(&event);
    * \endcode
    */
    static bool sendEvent(QEvent *event);

    /**
     * @see bool sendEvent(QEvent *event)
     */
    static bool sendEvent(const QEvent &event);
    /**
    * Adds the event event, to an event queue and returns immediately.
    * The event must be allocated on the heap since the post event queue will take ownership of the event and delete it once it has been posted. It is not safe to access the event after it has been posted.
    * see also: QCoreApplication::postEvent()
    */
    static void postEvent(QEvent *event, int priority = Qt::NormalEventPriority);
    /**
     *  EventCenter will unmanages the notifier, so it will not receive any event using EventCenter.
     */
    static void unregisterNotifier(const EventNotifier *notifier);
    /**
    * Immediately dispatches all events which have been previously queued with postEvent() and which are for the object receiver and have the event type event_type.
    * Events from the window system are not dispatched by this function, but by processEvents().
    * If receiver is nullptr, the events of event_type are sent for all objects. If event_type is None, all the events are sent for receiver.
    * see also: QCoreApplication::sendPostedEvents()
    */
    static void sendPostedEvents(QObject *receiver = nullptr, int event_type = EventNotifier::None);

    static bool checkAvailable();

private:
    friend class EventNotifier;
    void unregisterNotifier(const EventNotifier *notifier, int eventType);
    void registerNotifier(const EventNotifier *notifier, int eventType);
    QList<int> findNotifier(const EventNotifier *notifier) const;
    void forwardEvent(int eventType, const std::function<void(QObject *)> &handler) const;

private:
    ~EventBus();
    static EventBus s_instance;
    static bool disposed;
    QMultiMap<const EventNotifier *, int> m_notifiers;
};

/**
 * an auto removable event filter
 * the self removable event filter which is also an event notifier
 */
class SelfRemoveEventFilter : public QObject, public EventNotifierT<SelfRemoveEventFilter> {
    Q_OBJECT

public:
    using EventFilterCallable = std::function<bool(QObject *, QEvent *)>;
    explicit SelfRemoveEventFilter(QObject *filterOwner, bool selfInstall = true, QObject *parent = nullptr);
    explicit SelfRemoveEventFilter(EventType eventType = None, bool selfInstall = true, QObject *parent = nullptr);
    explicit SelfRemoveEventFilter(const EventFilterCallable &callable, QObject *filterOwner, bool selfInstall = true,
                                   QObject *parent = nullptr);
    explicit SelfRemoveEventFilter(const EventFilterCallable &callable, EventType eventType = None,
                                   bool selfInstall = true, QObject *parent = nullptr);

    ~SelfRemoveEventFilter() override;
    /**
     * if eventFilter is nullptr, means unsubscribe
     *
     * if override bool eventFilter(QObject *watched, QEvent *event) in subclass. the callback will not be invoked
     * handle event in subclass instead.
     * @param eventFilter a callback for this eventFilter
     */
    void subscribe(const EventFilterCallable &eventFilter);
    void unsubscribe();
    void setFilterOwner(QObject *filterOwner);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPointer<QObject> m_filterOwner;
    EventFilterCallable m_eventFilter;
};

template<typename... Args>
class GenericEvent : public QEvent {
    using DataTuple = std::tuple<Args...>;

public:
#define DECL_NAMED_EVENT_TYPE(name) inline static QEvent::Type name = QEvent::Type(QEvent::registerEventType(-1))
#define DECL_EVENT_TYPE DECL_NAMED_EVENT_TYPE(eventType)

    explicit GenericEvent(Type eventType) : QEvent(eventType) {
    }

    explicit GenericEvent(Type eventType, const Args &... args) : QEvent(eventType), m_data(args...) {
    }

    /*
     * 获取第n个数据的引用
     * 从0开始
     */
    template<int N>
    std::tuple_element_t<N, DataTuple> &data() {
        static_assert(N < sizeof...(Args), "N out of range");
        return std::get<N>(m_data);
    }

    DataTuple &data() {
        return m_data;
    }

private:
    DataTuple m_data;
};

#ifdef DOCTEST_LIBRARY_INCLUDED
#define TEST_EVENT_CENTER
#endif

#ifdef TEST_EVENT_CENTER
#include <QTimer>
class TestApplication final : public QCoreApplication {
public:
    TestApplication(const int &argc = 0)
        : QCoreApplication(const_cast<int&>(argc), nullptr) {
    }
    struct ExitApp {
        ~ExitApp() {
            exit();
        }
    };
private:
    bool event(QEvent *event) override {
        qDebug() << "app event:" << event->type();
        return QCoreApplication::event(event);
    }
};

#define BEGIN_TEST_Q_APP { TestApplication _app_;QTimer::singleShot(1, [](){\
    TestApplication::ExitApp _exit_;
#define END_TEST_Q_APP });_app_.exec(); }
#define WAIT_MS(ms) QThread::msleep(ms)
#endif

#endif //EVENT_BUS_H
