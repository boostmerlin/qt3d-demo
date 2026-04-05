//
// Created by merlin
//

#include <QtGlobal>
#include <QDebug>
#include "crash_handler.h"

#ifdef Q_OS_WIN32
#include "client/windows/handler/exception_handler.h"
using namespace google_breakpad;
class CrashHandlerWindows : public CrashHandler {
public:
    static bool processDumpResults(const wchar_t *dump_path,
                                   const wchar_t *minidump_id,
                                   void *context,
                                   EXCEPTION_POINTERS *exinfo,
                                   MDRawAssertionInfo *assertion,
                                   bool succeeded) {
        qDebug() << "Generation Dump:" << (succeeded ? "Succeeded" : "Failed");
        qDebug() << "Dump path:" << QString::fromWCharArray(dump_path);
        qDebug() << "Minidump ID:" << QString::fromWCharArray(minidump_id);
        return succeeded;
    }

    void init(const QString &reportPath) override {
        m_handler = new ExceptionHandler(
                reportPath.toStdWString(),
                nullptr,
                processDumpResults,
                nullptr,
                ExceptionHandler::HANDLER_EXCEPTION
        );
    }

    void requestDump() override {
        Q_ASSERT(m_handler);
        m_handler->WriteMinidump();
    }

    ~CrashHandlerWindows() {
        delete m_handler;
    }
private:
    ExceptionHandler *m_handler;
};

CrashHandler *CrashHandler::instance() {
    static CrashHandlerWindows instance;
    return &instance;
}

#elif defined(Q_OS_LINUX)
#include "client/linux/handler/exception_handler.h"
using namespace google_breakpad;
class CrashHandlerLinux : public CrashHandler {
    static bool processDumpResults(const google_breakpad::MinidumpDescriptor &descriptor,
                                   void *context,
                                   bool succeeded) {
        qDebug() << "Generation Dump:" << (succeeded ? "Succeeded" : "Failed");
        qDebug() << "Dump path:" << descriptor.path();
        return succeeded;
    }

public:
    void init(const QString &reportPath) override {
        m_handler = new ExceptionHandler(
                google_breakpad::MinidumpDescriptor(reportPath.toStdString()),
                nullptr,
                processDumpResults,
                nullptr,
                true,
                -1
        );
    }

    void requestDump() override {
        Q_ASSERT(m_handler);
        m_handler->WriteMinidump();
    }

    ~CrashHandlerLinux() {
        delete m_handler;
    }
private:
    ExceptionHandler *m_handler{};
};

CrashHandler *CrashHandler::instance() {
    static CrashHandlerLinux instance;
    return &instance;
}
#endif

