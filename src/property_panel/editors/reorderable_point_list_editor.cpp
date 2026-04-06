#include "property_panel/editors/reorderable_point_list_editor.h"

#include <QAbstractSpinBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace {
void clearLayout(QLayout *layout)
{
    if (!layout) {
        return;
    }

    while (auto *item = layout->takeAt(0)) {
        if (auto *widget = item->widget()) {
            widget->deleteLater();
        }
        if (auto *childLayout = item->layout()) {
            clearLayout(childLayout);
            delete childLayout;
        }
        delete item;
    }
}
}

ReorderablePointListEditor::ReorderablePointListEditor(PointListEditorConfig config, QWidget *parent)
    : QWidget(parent)
    , m_config(std::move(config))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_addButton = new QPushButton(m_config.addButtonText, this);
    m_addButton->setCursor(Qt::PointingHandCursor);
    m_addButton->setVisible(m_config.allowAdd);
    layout->addWidget(m_addButton);
    connect(m_addButton, &QPushButton::clicked, this, &ReorderablePointListEditor::addRequested);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMinimumHeight(180);
    scrollArea->setMaximumHeight(280);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        " background: #f8fbff;"
        " border: 1px solid #dbe5f0;"
        " border-radius: 8px;"
        "}"
        "QScrollBar:vertical {"
        " background: transparent;"
        " width: 10px;"
        " margin: 6px 4px 6px 0;"
        "}"
        "QScrollBar::handle:vertical {"
        " background: #cbd5e1;"
        " border-radius: 5px;"
        " min-height: 28px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical,"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        " background: transparent;"
        " height: 0px;"
        "}");
    layout->addWidget(scrollArea);

    m_rowsWidget = new QWidget(scrollArea);
    m_rowsLayout = new QVBoxLayout(m_rowsWidget);
    m_rowsLayout->setContentsMargins(8, 8, 8, 8);
    m_rowsLayout->setSpacing(10);
    scrollArea->setWidget(m_rowsWidget);
}

void ReorderablePointListEditor::setPoints3D(const QList<QVector3D> &points)
{
    m_points = points;
    rebuild();
}

void ReorderablePointListEditor::setPoints2D(const QList<QVector2D> &points)
{
    m_points.clear();
    for (const auto &point : points) {
        m_points.append(QVector3D(point.x(), point.y(), 0.0f));
    }
    rebuild();
}

void ReorderablePointListEditor::rebuild()
{
    clearLayout(m_rowsLayout);

    for (int index = 0; index < m_points.size(); ++index) {
        auto *rowWidget = new QWidget(m_rowsWidget);
        rowWidget->setObjectName(QStringLiteral("pointListCard"));
        rowWidget->setStyleSheet(
            "#pointListCard {"
            " background: #f8fbff;"
            " border: 1px solid #dbe5f0;"
            " border-radius: 8px;"
            "}"
            "#pointListCard QPushButton {"
            " min-width: 58px;"
            "}"
            "#pointListCard QPushButton:disabled {"
            " color: #94a3b8;"
            "}");

        auto *rowLayout = new QVBoxLayout(rowWidget);
        rowLayout->setContentsMargins(10, 8, 10, 8);
        rowLayout->setSpacing(6);

        auto *coordinatesLayout = new QHBoxLayout();
        coordinatesLayout->setContentsMargins(0, 0, 0, 0);
        coordinatesLayout->setSpacing(8);

        auto *label = new QLabel(rowLabelForIndex(index), rowWidget);
        label->setFixedWidth(52);
        label->setStyleSheet("font-weight: 600; color: #334155;");
        coordinatesLayout->addWidget(label);

        const std::array values = {m_points[index].x(), m_points[index].y(), m_points[index].z()};
        for (int axis = 0; axis < m_config.dimension; ++axis) {
            auto *spinBox = new QDoubleSpinBox(rowWidget);
            spinBox->setRange(-9999.0, 9999.0);
            spinBox->setDecimals(3);
            spinBox->setSingleStep(0.1);
            spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
            spinBox->setPrefix(axisLabelFor(index, axis) + QStringLiteral(": "));
            spinBox->setMinimumWidth(96);
            spinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            spinBox->setValue(values[axis]);
            spinBox->setVisible(isAxisVisible(index, axis));
            spinBox->setEnabled(isAxisVisible(index, axis));
            coordinatesLayout->addWidget(spinBox, isAxisVisible(index, axis) ? 1 : 0);

            connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
                    [this, index, axis](double value) {
                        emit pointEdited(index, axis, value);
                    });
        }

        rowLayout->addLayout(coordinatesLayout);

        if (m_config.allowReorder || m_config.allowRemove) {
            auto *actionsLayout = new QHBoxLayout();
            actionsLayout->setContentsMargins(0, 0, 0, 0);
            actionsLayout->setSpacing(8);
            actionsLayout->addStretch(1);

            if (m_config.allowReorder) {
                auto *upButton = new QPushButton(tr("Up"), rowWidget);
                upButton->setEnabled(index > 0);
                actionsLayout->addWidget(upButton);
                connect(upButton, &QPushButton::clicked, this, [this, index] {
                    emit moveRequested(index, index - 1);
                });

                auto *downButton = new QPushButton(tr("Down"), rowWidget);
                downButton->setEnabled(index + 1 < m_points.size());
                actionsLayout->addWidget(downButton);
                connect(downButton, &QPushButton::clicked, this, [this, index] {
                    emit moveRequested(index, index + 1);
                });
            }

            if (m_config.allowRemove) {
                auto *removeButton = new QPushButton(tr("Remove"), rowWidget);
                actionsLayout->addWidget(removeButton);
                connect(removeButton, &QPushButton::clicked, this, [this, index] {
                    emit removeRequested(index);
                });
            }

            rowLayout->addLayout(actionsLayout);
        }

        m_rowsLayout->addWidget(rowWidget);
    }
    m_rowsLayout->addStretch(1);
}

QString ReorderablePointListEditor::rowLabelForIndex(int index) const
{
    if (m_config.rowLabel) {
        return m_config.rowLabel(index);
    }
    return QStringLiteral("P%1").arg(index + 1);
}

QString ReorderablePointListEditor::axisLabelFor(int row, int axis) const
{
    if (m_config.axisLabel) {
        return m_config.axisLabel(row, axis);
    }
    static const std::array defaults = {QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")};
    return defaults[axis];
}

bool ReorderablePointListEditor::isAxisVisible(int row, int axis) const
{
    if (m_config.axisVisible) {
        return m_config.axisVisible(row, axis);
    }
    return axis < m_config.dimension;
}
