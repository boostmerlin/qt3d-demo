//
// Created by merlin
//

#include "event_bus.h"

EventBus EventBus::s_instance;
bool EventBus::disposed = false;

void EventNotifier::watchEvent(EventType eventType) const {
    if (EventBus::disposed) return;
    const QList<int> &events = EventBus::s_instance.findNotifier(this);
    if (eventType == None) {
        //? watch none event on this ? means unwatch any event.
        EventBus::unregisterNotifier(this);
    } else {
        if (eventType == All) {
            EventBus::unregisterNotifier(this);
        }
        if (!events.contains(eventType)) {
            EventBus::s_instance.registerNotifier(this, eventType);
        }
    }
}

void EventNotifier::unwatchEvent(EventType eventType) const {
    if (!EventBus::checkAvailable()) return;
    switch (eventType) {
        case None:
            break;
        case All:
            EventBus::unregisterNotifier(this);
            break;
        default:
            EventBus::s_instance.unregisterNotifier(this, eventType);
            break;
    }
}

EventNotifier::EventNotifier(size_t offset, EventType autoWatchEventType) : m_offset(offset) {
    if (autoWatchEventType != None) {
        watchEvent(autoWatchEventType);
    }
}

EventNotifier::~EventNotifier() {
    if (EventBus::disposed) return;
    EventBus::unregisterNotifier(this);
}

void EventBus::forwardEvent(int eventType, const std::function<void(QObject *)> &handler) const {
    for (auto it = m_notifiers.cbegin(), end = m_notifiers.cend(); it != end; ++it) {
        if (it.value() == EventNotifier::None || eventType == QEvent::Type::None) {
            continue;
        }
        if (it.value() == EventNotifier::All || it.value() == eventType) {
            const auto ob = const_cast<EventNotifier *>(it.key());
            const auto realAddr = reinterpret_cast<char *>(ob) - ob->m_offset;
            handler(static_cast<QObject *>(static_cast<void *>(realAddr)));
        }
    }
}

EventBus::~EventBus() {
    qDebug() << "EventBus::~EventBus()";
    disposed = true;
}


bool EventBus::checkAvailable() {
    return qApp && !disposed;
}

void EventBus::sendPostedEvents(QObject *receiver, int event_type) {
    return QCoreApplication::sendPostedEvents(receiver, event_type);
}

bool EventBus::sendEvent(QEvent *event) {
    if (!checkAvailable()) return false;
    bool ret = true;
    s_instance.forwardEvent(event->type(), [&event, &ret](QObject *object) {
        ret &= qApp->sendEvent(object, event);
    });
    return ret;
}

bool EventBus::sendEvent(const QEvent &event) {
    return sendEvent(const_cast<QEvent *>(&event));
}

void EventBus::postEvent(QEvent *event, int priority) {
    if (checkAvailable()) {
        s_instance.forwardEvent(event->type(), [event, priority](QObject *object) {
            qApp->postEvent(object, event, priority);
        });
    }
}

void EventBus::unregisterNotifier(const EventNotifier *observable) {
    if (disposed) return;
    s_instance.m_notifiers.remove(observable);
}

void EventBus::unregisterNotifier(const EventNotifier *notifier, int eventType) {
    const auto c = m_notifiers.remove(notifier, eventType);
    if (c) {
        qDebug() << "unregisterNotifier of eventType:" << eventType;
    }
}

void EventBus::registerNotifier(const EventNotifier *notifier, int eventType) {
    m_notifiers.insert(notifier, eventType);
}

QList<int> EventBus::findNotifier(const EventNotifier *notifier) const {
    return m_notifiers.values(notifier);
}

SelfRemoveEventFilter::SelfRemoveEventFilter(QObject *filterOwner, bool selfInstall,
                                             QObject *parent) : QObject(parent), m_filterOwner(
                                                                    filterOwner) {
    Q_ASSERT_X(filterOwner, "AutoDisposableEventFilter", "filterOwner is nullptr");
    if (selfInstall) {
        m_filterOwner->installEventFilter(this);
    }
}

SelfRemoveEventFilter::SelfRemoveEventFilter(EventType eventType, bool selfInstall,
                                             QObject *parent) : SelfRemoveEventFilter(
    this, selfInstall, parent) {
    watchEvent(eventType);
}

SelfRemoveEventFilter::SelfRemoveEventFilter(const EventFilterCallable &callable, QObject *filterOwner,
                                             bool selfInstall, QObject *parent) : SelfRemoveEventFilter(
    filterOwner, selfInstall, parent) {
    subscribe(callable);
}

SelfRemoveEventFilter::SelfRemoveEventFilter(const EventFilterCallable &callable, EventType eventType, bool selfInstall,
                                             QObject *parent) : SelfRemoveEventFilter(
    eventType, selfInstall, parent) {
    subscribe(callable);
}

SelfRemoveEventFilter::~SelfRemoveEventFilter() {
    if (m_filterOwner) {
        m_filterOwner->removeEventFilter(this);
    }
    m_eventFilter = nullptr;
}

void SelfRemoveEventFilter::subscribe(const EventFilterCallable &eventFilter) {
    m_eventFilter = eventFilter;
}

void SelfRemoveEventFilter::unsubscribe() {
    subscribe(nullptr);
}

void SelfRemoveEventFilter::setFilterOwner(QObject *filterOwner) {
    if (m_filterOwner) {
        m_filterOwner->removeEventFilter(this);
    }
    m_filterOwner = filterOwner;
    if (m_filterOwner) {
        m_filterOwner->installEventFilter(this);
    }
}

bool SelfRemoveEventFilter::eventFilter(QObject *watched, QEvent *event) {
    if (m_eventFilter) {
        return m_eventFilter(watched, event);
    }
    return QObject::eventFilter(watched, event);
}
