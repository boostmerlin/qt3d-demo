#pragma once

#include <array>

#include <QVector3D>
#include <QWidget>

class QLabel;
class QDoubleSpinBox;

class Vector3Editor final : public QWidget
{
    Q_OBJECT

public:
    explicit Vector3Editor(const QString &label, QWidget *parent = nullptr);

    void setValue(const QVector3D &value);
    [[nodiscard]] QVector3D value() const;
    void setRange(double minimum, double maximum);
    void setSingleStep(double step);
    void setDecimals(int decimals);
    [[nodiscard]] std::array<QDoubleSpinBox *, 3> spinBoxes() const;

signals:
    void valueEdited(const QVector3D &value);

private:
    void emitCurrentValue();

    QLabel *m_label{};
    std::array<QDoubleSpinBox *, 3> m_spinBoxes{};
};
