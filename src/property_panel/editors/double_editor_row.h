#pragma once

#include <QWidget>

class QLabel;
class QDoubleSpinBox;

class DoubleEditorRow final : public QWidget
{
    Q_OBJECT

public:
    explicit DoubleEditorRow(const QString &label, QWidget *parent = nullptr);

    void setLabelText(const QString &text);
    void setValue(double value);
    [[nodiscard]] double value() const;
    void setRange(double minimum, double maximum);
    void setSingleStep(double step);
    void setDecimals(int decimals);
    void setPrefix(const QString &prefix);
    [[nodiscard]] QDoubleSpinBox *spinBox() const;

signals:
    void valueEdited(double value);

private:
    QLabel *m_label{};
    QDoubleSpinBox *m_spinBox{};
};
