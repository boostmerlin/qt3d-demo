#include "property_panel/editors/text_editor_row.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

TextEditorRow::TextEditorRow(const QString &label, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_label = new QLabel(label, this);
    m_label->setMinimumWidth(72);
    layout->addWidget(m_label);

    m_lineEdit = new QLineEdit(this);
    layout->addWidget(m_lineEdit, 1);

    connect(m_lineEdit, &QLineEdit::editingFinished, this, [this] {
        emit editingFinished(m_lineEdit->text());
    });
}

void TextEditorRow::setText(const QString &text)
{
    m_lineEdit->setText(text);
}

QString TextEditorRow::text() const
{
    return m_lineEdit->text();
}

void TextEditorRow::setReadOnly(bool readOnly)
{
    m_lineEdit->setReadOnly(readOnly);
}

void TextEditorRow::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);
    m_lineEdit->setEnabled(enabled);
}

QLineEdit *TextEditorRow::lineEdit() const
{
    return m_lineEdit;
}
