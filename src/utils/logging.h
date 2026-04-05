//
// Created by merlin
//

#ifndef LOGGING_H
#define LOGGING_H

#include <QtLogging>
#include <QFile>
#include <QList>
#include <QMutex>
#include <QLoggingCategory>

class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    virtual void handle(QtMsgType type, const QMessageLogContext &context, const QString &msg) = 0;
};

class Logging {
public:
    ~Logging();
    static Logging instance;
    static void setLogConfigDir(const QString &dir);
    static Logging &init(bool defaultHandler = false);
    Logging &installMessageHandler(MessageHandler *handler);
    static void setLogPattern(const QString &pattern);
    // Enable or disable default logging for all message types
    static void setEnabled(bool enable);
    // Enable or disable default logging for a specific message type
    static void setEnabled(QtMsgType msgType, bool enable);
    // Enable or disable logging for a specific category for all message types
    static void setEnabled(const QString &category, bool enable);
    // Enable or disable logging for a specific category for all message types
    static void setEnabled(const QLoggingCategory &category, bool enable);
    // Enable or disable logging for a specific category and message type
    static void setEnabled(const QString &category, QtMsgType msgType, bool enable);
    // Enable or disable logging for a specific category and message type
    static void setEnabled(const QLoggingCategory &category, QtMsgType msgType, bool enable);

#define LOGGING_CATEGORY(name) (name)()

private:
    static void updateRules();
    static void customHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static QtMessageHandler s_oldHandler;
    QMap<QString, bool> m_rules;
    QList<MessageHandler *> m_messageHandlers;
};

class WriterHandler : public MessageHandler {
public:
    void writeLine(const QString &msg) const;

    void setAutoFlush(bool autoFlush);

    void setEncoding(QStringConverter::Encoding encoding);

    void setWriter(QTextStream *writer);
private:
    volatile bool m_autoFlush = false;
    QTextStream *m_writer = nullptr;
    QStringConverter::Encoding m_encoding = QStringConverter::System;
};

class ConsoleHandler final : public WriterHandler {
public:
    explicit ConsoleHandler(bool isStdErr = false);
    void handle(QtMsgType type, const QMessageLogContext &context, const QString &msg) override;
private:
    QTextStream m_out;
};

class FileHandler : public WriterHandler {
public:
    static const inline auto defaultLogDir = QStringLiteral("./logs");
    explicit FileHandler(bool append = true);
    ~FileHandler() override;
    bool setFile(const QString& fileName);
    const QString& logDir() const;
    void setLogDir(const QString& logDir);
protected:
    void openFile();
    void closeFile();
private:
    bool m_appendFile;
    QString m_fileName;
    QFile m_file;
    QTextStream m_out;
    QString m_logDir = defaultLogDir;
};

class AllLogFileHandler final : public FileHandler {
public:
    explicit AllLogFileHandler(bool append = true);
    void handle(QtMsgType type, const QMessageLogContext &context, const QString &msg) override;
};

//better used only in Debug
class TypeLogFileHandler final : public FileHandler {
public:
    void handle(QtMsgType type, const QMessageLogContext &context, const QString &msg) override;
private:
    QRecursiveMutex m_lock;
};

#endif //LOGGING_H
