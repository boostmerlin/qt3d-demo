#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;

class TextEditorRow final : public QWidget
{
    Q_OBJECT

public:
    explicit TextEditorRow(const QString &label, QWidget *parent = nullptr);

    void setText(const QString &text);
    [[nodiscard]] QString text() const;
    void setReadOnly(bool readOnly);
    void setEnabled(bool enabled);
    [[nodiscard]] QLineEdit *lineEdit() const;

signals:
    void editingFinished(const QString &text);

private:
    QLabel *m_label{};
    QLineEdit *m_lineEdit{};
};
