#include "property_panel/editors/enum_editor.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>

EnumEditor::EnumEditor(const QString &label, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_label = new QLabel(label, this);
    m_label->setMinimumWidth(72);
    layout->addWidget(m_label);

    m_comboBox = new QComboBox(this);
    layout->addWidget(m_comboBox, 1);

    connect(m_comboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index >= 0) {
            emit valueEdited(m_comboBox->itemData(index));
        }
    });
}

void EnumEditor::setItems(const QList<EnumEditorItem> &items)
{
    const QSignalBlocker blocker(m_comboBox);
    m_comboBox->clear();
    for (const auto &item : items) {
        m_comboBox->addItem(item.label, item.value);
    }
}

void EnumEditor::setCurrentValue(const QVariant &value)
{
    const QSignalBlocker blocker(m_comboBox);
    for (int index = 0; index < m_comboBox->count(); ++index) {
        if (m_comboBox->itemData(index) == value) {
            m_comboBox->setCurrentIndex(index);
            return;
        }
    }
    if (m_comboBox->count() > 0) {
        m_comboBox->setCurrentIndex(0);
    }
}

QVariant EnumEditor::currentValue() const
{
    return m_comboBox->currentData();
}

QComboBox *EnumEditor::comboBox() const
{
    return m_comboBox;
}
