//
// Created by ml on 2024/5/24.
//

#include <QThread>
#include <QWidget>
#include <doctest/doctest.h>
#include "core/event_bus.h"
#include "test_helper.h"

using EventVerification = FunctionVerification<int>;

struct X {
    int a;
};

struct Y {
    Y() {}
    Y(X) {
    }
};

class ComplicatedQObject : public QObject {
public:
    ComplicatedQObject(QObject *parent = nullptr) : n(0), r(n) {
    }

    ~ComplicatedQObject() {
    }

    static int si;
    int n;
    int &r;
    int a[2] = {1, 2};
    std::string s, *ps;
    struct NestedS {
        std::string s;
    } d5;

    char bit: 2;
};

// class MyEventHandler2 {
// public:
//     int a,b,c;
// };

class MyEventHandler : public WithEventNotifier<>, public EventVerification {
    Q_OBJECT

private:
    bool event(QEvent *event) override {
        operator()(event->type());
        return QObject::event(event);
    }
};

// class MyEventHandler2 : public QWidget {
//
// };

class MyEventHandler2 : public WithEventNotifier<QObject, EventVerification> {
    Q_OBJECT
public:
    MyEventHandler2() : WithEventNotifier(static_cast<QObject *>(nullptr)) {
    }
private:
    bool event(QEvent *event) override {
        operator()(event->type());
        return QObject::event(event);
    }
};

//for very comlicated inheritance
class MyEventHandler3 : public ComplicatedQObject, public X, public EventNotifierT<MyEventHandler3>,
                        public EventVerification {
    Q_OBJECT

private:
    bool event(QEvent *event) override {
        operator()(event->type());
        return QObject::event(event);
    }
};

class MyEventHandler4 : public WithEventNotifier<ComplicatedQObject, X, Y, EventVerification> {
    Q_OBJECT

public:
    MyEventHandler4(EventType type) : WithEventNotifier(X(), Y(X()), EventVerification(), type,
                                                        static_cast<QObject *>(nullptr)) {
    }
    virtual void f() {}
private:
    bool event(QEvent *event) override {
        operator()(event->type());
        return QObject::event(event);
    }
};

class MyEventHandler5 : public MyEventHandler4 {
    Q_OBJECT
public:
    MyEventHandler5() : MyEventHandler4(None) {
    }
    int a;
};

class CustomEvent : public GenericEvent<int, double*> {
public:
    DECL_EVENT_TYPE;
    CustomEvent(const int &i)
        : GenericEvent<int, double *>(eventType, i, nullptr) {
    }

};

TEST_CASE("eventbus test") {
    SUBCASE("test app setup") {
        CHECK_FALSE(EventBus::checkAvailable());
        BEGIN_TEST_Q_APP
            CHECK(EventBus::checkAvailable());
            MyEventHandler3 handler;
            QObject *ob = &handler;
            CHECK_EQ(ob, &handler);
            QEvent event(QEvent::Type::KeyPress);
            qApp->sendEvent(&handler, &event);
            CHECK_CALLED(handler);
        END_TEST_Q_APP
        //should called
        CHECK(true);
    }
    SUBCASE("test event center send") {
        BEGIN_TEST_Q_APP

            auto testHander = [](auto &handler) {
                QEvent eventMousePress(QEvent::Type::KeyPress);
                EventBus::sendEvent(&eventMousePress);
                CHECK_NOT_CALLED(handler);
                handler.watchEvent(QEvent::Type::KeyPress);
                EventBus::sendEvent(&eventMousePress);
                CHECK_CALLED(handler);
                handler.watchEvent(EventNotifier::All);
                EventBus::sendEvent(&eventMousePress);
                CHECK_CALLED(handler);
                QEvent eventMouseRelease(QEvent::Type::KeyRelease);
                EventBus::sendEvent(&eventMouseRelease);
                CHECK_CALLED(handler);

                handler.unwatchEvent(EventNotifier::All);
                EventBus::sendEvent(&eventMousePress);
                CHECK_NOT_CALLED(handler);
                EventBus::sendEvent(&eventMouseRelease);
                CHECK_NOT_CALLED(handler);
                handler.watchEvent(EventNotifier::All);
                handler.watchEvent(QEvent::Type::None); //== unwatch all
                EventBus::sendEvent(&eventMouseRelease);
                CHECK_NOT_CALLED(handler);

                handler.watchEvent(QEvent::Type::KeyPress);
                handler.watchEvent(QEvent::Type::KeyRelease);
                handler.watchEvent(QEvent::Type::MouseMove);
                QEvent eventMouseMove(QEvent::Type::MouseMove);

                handler.unwatchEvent(EventNotifier::None);
                EventBus::sendEvent(&eventMousePress);
                CHECK_CALLED(handler);
                EventBus::sendEvent(&eventMouseRelease);
                CHECK_CALLED(handler);
                EventBus::sendEvent(&eventMouseMove);
                CHECK_CALLED(handler);

                //unwatch
                handler.unwatchEvent(QEvent::Type::KeyPress);
                EventBus::sendEvent(&eventMousePress);
                CHECK_NOT_CALLED(handler);
                handler.unwatchEvent(EventNotifier::All);
                EventBus::sendEvent(&eventMouseRelease);
                CHECK_NOT_CALLED(handler);
                EventBus::sendEvent(&eventMouseMove);
                CHECK_NOT_CALLED(handler);
            };
            MyEventHandler handler;
            testHander(handler);

            MyEventHandler2 handler2;
            testHander(handler2);

            MyEventHandler3 handler3;
            testHander(handler3);

            MyEventHandler5 handler5;
            testHander(handler5);
        END_TEST_Q_APP
    }
    SUBCASE("test event center post") {
        TestApplication app;
        MyEventHandler4 handler(EventNotifier::All);
        QThread *thread = QThread::create([&]() {
            EventBus::postEvent(new QEvent(QEvent::Type::KeyPress));
            WAIT_MS(10);
            CHECK_CALLED(handler);
            handler.watchEvent(QEvent::Type::None);
            EventBus::postEvent(new QEvent(QEvent::Type::KeyRelease));
            CHECK_NOT_CALLED(handler);
            thread->deleteLater();
            app.quit();
        });
        thread->start();
        app.exec();
    }

    SUBCASE("test auto disable event filter") {
        BEGIN_TEST_Q_APP
        SelfRemoveEventFilter eventFilter(EventNotifier::All);
        int count = 0;
        GenericEvent event(QEvent::Type::Close, &count);

                        eventFilter.subscribe([&](QObject *, QEvent *e) {
                            count++;
                            return true;
                        });
        EventBus::sendEvent(&event);
        CHECK_EQ(count, 1);

        SelfRemoveEventFilter eventFilter2;
        MyEventHandler handler;
        handler.watchEvent(QEvent::Type::KeyPress);
        handler.installEventFilter(&eventFilter2);
        GenericEvent event2(event.type(), 3);

        auto d = event2.data<0>();
        CHECK_EQ(d, 3);
        EventBus::sendEvent(&event2);
        CHECK_EQ(count, 2);
        GenericEvent<QString, int, QEvent*> event3(QEvent::Type::Clipboard, "abc", 3, &event);
        CustomEvent event4(5);
        CHECK_EQ(CustomEvent::eventType, event4.type());

        END_TEST_Q_APP
    }
}

#include "event_bus_test.moc"
