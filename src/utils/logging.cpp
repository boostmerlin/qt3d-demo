//
// Created by merlin
//

#include <QDir>
#include <QCoreApplication>
#include <QMutex>
#include "logging.h"


auto defaultCategory = "default";
auto loggingIni = "qtlogging.ini";
auto defaultMessagePattern =
        "[%{time yyMMdd h:mm:ss.zzz} "
        "%{if-debug}D%{endif}"
        "%{if-info}I%{endif}"
        "%{if-warning}W%{endif}"
        "%{if-critical}C%{endif}"
        "%{if-fatal}F%{endif}:%{category}:%{threadid}] "
        "%{file}:%{line} - %{message}";

Logging Logging::instance;
QtMessageHandler Logging::s_oldHandler;

static const QMap<QtMsgType, const char *> msgTypeNames = {
    {QtDebugMsg, "debug"},
    {QtInfoMsg, "info"},
    {QtWarningMsg, "warning"},
    {QtCriticalMsg, "critical"},
};

//ansi color: https://en.wikipedia.org/wiki/ANSI_escape_code
static QString color(int ansicolor, const QString &msg) {
    return QString("\033[1;%1m%2\033[0m").arg(ansicolor).arg(msg);
}

Logging::~Logging() {
    qInstallMessageHandler(nullptr);
    qDeleteAll(m_messageHandlers);
}

//seems no working?
void Logging::setLogConfigDir(const QString &dir) {
    if (QDir(dir).exists()) {
        qputenv("QT_LOGGING_CONF",
                QDir(dir).absoluteFilePath(loggingIni).toUtf8());
    }
}

Logging &Logging::init(bool defaultHandler) {
    const auto oldHandler = qInstallMessageHandler(customHandler);
    if (defaultHandler) {
        s_oldHandler = oldHandler;
    }
#if defined(QT_DEBUG) && defined(Q_OS_WIN)
    if (QFile file(loggingIni); !file.exists()) {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write("[Rules]\n");
        file.close();
    }
#endif
    setLogPattern(defaultMessagePattern);
    return instance;
}

Logging &Logging::installMessageHandler(MessageHandler *handler) {
    if (!m_messageHandlers.contains(handler)) {
        m_messageHandlers.append(handler);
    }
    return *this;
}

void Logging::setLogPattern(const QString &pattern) {
    qSetMessagePattern(pattern);
}

void Logging::setEnabled(bool enable) {
    setEnabled(defaultCategory, enable);
}

void Logging::setEnabled(QtMsgType msgType, bool enable) {
    setEnabled(defaultCategory, msgType, enable);
}

void Logging::setEnabled(const QString &category, bool enable) {
    for (const auto t: msgTypeNames) {
        instance.m_rules[category + "." + t] = enable;
    }
    updateRules();
}

void Logging::setEnabled(const QLoggingCategory &category, bool enable) {
    setEnabled(LOGGING_CATEGORY(category).categoryName(), enable);
}

void Logging::setEnabled(const QString &category, QtMsgType msgType, bool enable) {
    const QString t = msgTypeNames.value(msgType);
    if (t.isEmpty()) {
        return;
    }
    instance.m_rules[category + "." + t] = enable;
    updateRules();
}

void Logging::setEnabled(const QLoggingCategory &category, QtMsgType msgType, bool enable) {
    setEnabled(LOGGING_CATEGORY(category).categoryName(), msgType, enable);
}

void Logging::updateRules() {
    QString rules;
    for (auto [rule, enable]: instance.m_rules.asKeyValueRange()) {
        rules += QString("%1=%2\n").arg(rule, enable ? "true" : "false");
    }
    QLoggingCategory::setFilterRules(rules);
}

void Logging::customHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    for (auto handler: instance.m_messageHandlers) {
        handler->handle(type, context, msg);
    }
    if (s_oldHandler) {
        (*s_oldHandler)(type, context, msg);
    }
}

void WriterHandler::writeLine(const QString &msg) const {
    if (!m_writer) {
        return;
    }
    *m_writer << msg << Qt::endl;
    if (m_autoFlush) {
        m_writer->flush();
    }
}

void WriterHandler::setAutoFlush(bool autoFlush) {
    m_autoFlush = autoFlush;
}

void WriterHandler::setEncoding(QStringConverter::Encoding encoding) {
    m_encoding = encoding;
    if (m_writer)
        m_writer->setEncoding(m_encoding);
}

void WriterHandler::setWriter(QTextStream *writer) {
    if (writer == m_writer) {
        return;
    }
    m_writer = writer;
    if (m_writer)
        m_writer->setEncoding(m_encoding);
}

ConsoleHandler::ConsoleHandler(bool isStdErr) : m_out(isStdErr ? stderr : stdout) {
    setWriter(&m_out);
}

void ConsoleHandler::handle(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    const auto str = qFormatLogMessage(type, context, msg);
    switch (type) {
        case QtDebugMsg:
            writeLine(color(32, str));
            break;
        case QtWarningMsg:
            writeLine(color(93, str));
            break;
        case QtCriticalMsg:
            writeLine(color(91, str));
            break;
        case QtFatalMsg:
            writeLine(color(96, str));
            abort();
        default:
            writeLine(str);
            break;
    }
}

FileHandler::FileHandler(bool append) : m_appendFile(append) {
}

FileHandler::~FileHandler() {
    closeFile();
}

bool FileHandler::setFile(const QString &fileName) {
    if (m_fileName == fileName) {
        return false;
    }
    m_fileName = fileName;
    return true;
}

void FileHandler::openFile() {
    if (m_fileName.isEmpty()) {
        return;
    }
    const QFileInfo fileInfo(m_fileName);
    if (auto dir = fileInfo.dir(); !dir.exists()) {
        const auto name = dir.dirName();
        dir.cdUp();
        if (!dir.mkdir(name)) {
            qCritical() << "Failed to create directory" << dir.absolutePath();
            return;
        }
    }
    auto mode = QIODevice::WriteOnly | QIODevice::Text;
    mode |= m_appendFile ? QIODevice::Append : QIODevice::Truncate;
    m_file.setFileName(m_fileName);
    if (!m_file.open(mode)) {
        qCritical() << "Failed to open log file" << m_fileName;
        return;
    }
    m_out.setDevice(&m_file);
    setWriter(&m_out);
}

void FileHandler::closeFile() {
    setWriter(nullptr);
    if (m_file.isOpen()) {
        m_file.close();
    }
}

void FileHandler::setLogDir(const QString& logDir) {
    m_logDir = logDir;
}

const QString &FileHandler::logDir() const {
    return m_logDir;
}

void AllLogFileHandler::handle(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    writeLine(qFormatLogMessage(type, context, msg));
}

AllLogFileHandler::AllLogFileHandler(bool append) : FileHandler(append) {
    if (setFile(QString("%1/all.txt").arg(logDir()))) {
        openFile();
    }
}

void TypeLogFileHandler::handle(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QMutexLocker locker(&m_lock);
    auto typeName = msgTypeNames.value(type);
    if (!typeName) {
        return;
    }
    if (setFile(QString("%1/%2.txt").arg(logDir(), typeName))) {
        closeFile();
        openFile();
    }
    writeLine(qFormatLogMessage(type, context, msg));
}
