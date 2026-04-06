#pragma once

#include <QWidget>

class QLabel;
class QSpinBox;

class IntEditorRow final : public QWidget
{
    Q_OBJECT

public:
    explicit IntEditorRow(const QString &label, QWidget *parent = nullptr);

    void setLabelText(const QString &text);
    void setValue(int value);
    [[nodiscard]] int value() const;
    void setRange(int minimum, int maximum);
    [[nodiscard]] QSpinBox *spinBox() const;

signals:
    void valueEdited(int value);

private:
    QLabel *m_label{};
    QSpinBox *m_spinBox{};
};
