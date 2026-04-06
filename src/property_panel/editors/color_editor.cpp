#include "property_panel/editors/color_editor.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace {
QString colorButtonStyle(const QColor &color)
{
    const QString textColor = color.lightnessF() > 0.65 ? QStringLiteral("#0f172a")
                                                        : QStringLiteral("#f8fafc");
    return QStringLiteral(
               "QPushButton {"
               " background: %1;"
               " color: %2;"
               " border: 1px solid #cbd5e1;"
               " border-radius: 6px;"
               " padding: 8px 12px;"
               " font-weight: 600;"
               " text-align: left;"
               "}"
               "QPushButton:hover {"
               " border-color: #94a3b8;"
               "}"
               "QPushButton:disabled {"
               " background: #f8fafc;"
               " color: #94a3b8;"
               " border-color: #e2e8f0;"
               "}")
        .arg(color.name(QColor::HexRgb), textColor);
}
}

ColorEditor::ColorEditor(const QString &label, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    m_label = new QLabel(label, this);
    m_label->setMinimumWidth(72);
    layout->addWidget(m_label);

    m_button = new QPushButton(this);
    m_button->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_button, 1);

    refreshButton();

    connect(m_button, &QPushButton::clicked, this, [this] {
        const QColor chosen = QColorDialog::getColor(m_color, this, m_dialogTitle);
        if (!chosen.isValid() || chosen == m_color) {
            return;
        }
        m_color = chosen;
        refreshButton();
        emit valueEdited(m_color);
    });
}

void ColorEditor::setColor(const QColor &color)
{
    m_color = color;
    refreshButton();
}

QColor ColorEditor::color() const
{
    return m_color;
}

void ColorEditor::setDialogTitle(const QString &title)
{
    m_dialogTitle = title;
}

QPushButton *ColorEditor::button() const
{
    return m_button;
}

void ColorEditor::refreshButton() const {
    m_button->setText(m_color.name(QColor::HexRgb).toUpper());
    m_button->setStyleSheet(colorButtonStyle(m_color));
}
