#include "property_panel/editors/vector3_editor.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QVBoxLayout>

Vector3Editor::Vector3Editor(const QString &label, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    m_label = new QLabel(label, this);
    m_label->setStyleSheet("font-weight: 600; color: #475569;");
    layout->addWidget(m_label);

    auto *rowLayout = new QHBoxLayout();
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(6);
    layout->addLayout(rowLayout);

    const std::array axisLabels = {QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")};
    for (int index = 0; index < 3; ++index) {
        auto *spinBox = new QDoubleSpinBox(this);
        spinBox->setRange(-9999.0, 9999.0);
        spinBox->setDecimals(3);
        spinBox->setSingleStep(0.1);
        spinBox->setPrefix(axisLabels[index] + QStringLiteral(": "));
        rowLayout->addWidget(spinBox);
        m_spinBoxes[index] = spinBox;

        connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
            emitCurrentValue();
        });
    }
}

void Vector3Editor::setValue(const QVector3D &value)
{
    const std::array values = {value.x(), value.y(), value.z()};
    for (int index = 0; index < 3; ++index) {
        const QSignalBlocker blocker(m_spinBoxes[index]);
        m_spinBoxes[index]->setValue(values[index]);
    }
}

QVector3D Vector3Editor::value() const
{
    return QVector3D(float(m_spinBoxes[0]->value()),
                     float(m_spinBoxes[1]->value()),
                     float(m_spinBoxes[2]->value()));
}

void Vector3Editor::setRange(double minimum, double maximum)
{
    for (auto *spinBox : m_spinBoxes) {
        spinBox->setRange(minimum, maximum);
    }
}

void Vector3Editor::setSingleStep(double step)
{
    for (auto *spinBox : m_spinBoxes) {
        spinBox->setSingleStep(step);
    }
}

void Vector3Editor::setDecimals(int decimals)
{
    for (auto *spinBox : m_spinBoxes) {
        spinBox->setDecimals(decimals);
    }
}

std::array<QDoubleSpinBox *, 3> Vector3Editor::spinBoxes() const
{
    return m_spinBoxes;
}

void Vector3Editor::emitCurrentValue()
{
    emit valueEdited(value());
}
