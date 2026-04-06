#include "property_panel/editors/int_editor_row.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

IntEditorRow::IntEditorRow(const QString &label, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_label = new QLabel(label, this);
    m_label->setMinimumWidth(72);
    layout->addWidget(m_label);

    m_spinBox = new QSpinBox(this);
    m_spinBox->setRange(-9999, 9999);
    layout->addWidget(m_spinBox, 1);

    connect(m_spinBox, qOverload<int>(&QSpinBox::valueChanged), this, &IntEditorRow::valueEdited);
}

void IntEditorRow::setLabelText(const QString &text)
{
    m_label->setText(text);
}

void IntEditorRow::setValue(int value)
{
    m_spinBox->setValue(value);
}

int IntEditorRow::value() const
{
    return m_spinBox->value();
}

void IntEditorRow::setRange(int minimum, int maximum)
{
    m_spinBox->setRange(minimum, maximum);
}

QSpinBox *IntEditorRow::spinBox() const
{
    return m_spinBox;
}
