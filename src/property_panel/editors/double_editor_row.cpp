#include "property_panel/editors/double_editor_row.h"

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>

DoubleEditorRow::DoubleEditorRow(const QString &label, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_label = new QLabel(label, this);
    m_label->setMinimumWidth(72);
    layout->addWidget(m_label);

    m_spinBox = new QDoubleSpinBox(this);
    m_spinBox->setRange(-9999.0, 9999.0);
    m_spinBox->setDecimals(3);
    m_spinBox->setSingleStep(0.1);
    layout->addWidget(m_spinBox, 1);

    connect(m_spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DoubleEditorRow::valueEdited);
}

void DoubleEditorRow::setLabelText(const QString &text)
{
    m_label->setText(text);
}

void DoubleEditorRow::setValue(double value)
{
    m_spinBox->setValue(value);
}

double DoubleEditorRow::value() const
{
    return m_spinBox->value();
}

void DoubleEditorRow::setRange(double minimum, double maximum)
{
    m_spinBox->setRange(minimum, maximum);
}

void DoubleEditorRow::setSingleStep(double step)
{
    m_spinBox->setSingleStep(step);
}

void DoubleEditorRow::setDecimals(int decimals)
{
    m_spinBox->setDecimals(decimals);
}

void DoubleEditorRow::setPrefix(const QString &prefix)
{
    m_spinBox->setPrefix(prefix);
}

QDoubleSpinBox *DoubleEditorRow::spinBox() const
{
    return m_spinBox;
}
