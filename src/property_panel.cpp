#include "property_panel.h"

#include <QComboBox>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

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

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *wrapperLayout = new QVBoxLayout(this);
    wrapperLayout->setContentsMargins(12, 12, 12, 12);
    wrapperLayout->setSpacing(10);

    auto *title = new QLabel(tr("Properties"), this);
    title->setStyleSheet("font-weight: 600; font-size: 13px; color: #334155;");
    wrapperLayout->addWidget(title);

    m_emptyStateLabel = new QLabel(tr("No object selected"), this);
    m_emptyStateLabel->setAlignment(Qt::AlignCenter);
    m_emptyStateLabel->setStyleSheet("color: #64748b; padding: 24px 12px;");
    wrapperLayout->addWidget(m_emptyStateLabel);

    m_propertyContentWidget = new QWidget(this);
    wrapperLayout->addWidget(m_propertyContentWidget);
    auto *contentLayout = new QVBoxLayout(m_propertyContentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(14);

    auto createSectionTitle = [this](const QString &text) {
        auto *label = new QLabel(text, m_propertyContentWidget);
        label->setStyleSheet("font-weight: 600; color: #334155; padding-top: 4px;");
        return label;
    };

    contentLayout->addWidget(createSectionTitle(tr("Object")));
    auto *objectFormLayout = new QFormLayout();
    objectFormLayout->setContentsMargins(0, 0, 0, 0);
    objectFormLayout->setHorizontalSpacing(10);
    objectFormLayout->setVerticalSpacing(10);
    contentLayout->addLayout(objectFormLayout);

    contentLayout->addWidget(createSectionTitle(tr("Transform")));
    auto *transformFormLayout = new QFormLayout();
    transformFormLayout->setContentsMargins(0, 0, 0, 0);
    transformFormLayout->setHorizontalSpacing(10);
    transformFormLayout->setVerticalSpacing(10);
    contentLayout->addLayout(transformFormLayout);

    m_dimensionsSectionLabel = createSectionTitle(tr("Dimensions"));
    contentLayout->addWidget(m_dimensionsSectionLabel);
    auto *dimensionsFormLayout = new QFormLayout();
    dimensionsFormLayout->setContentsMargins(0, 0, 0, 0);
    dimensionsFormLayout->setHorizontalSpacing(10);
    dimensionsFormLayout->setVerticalSpacing(10);
    contentLayout->addLayout(dimensionsFormLayout);
    contentLayout->addStretch(1);
    wrapperLayout->addStretch(1);

    m_nameEdit = new QLineEdit(this);
    objectFormLayout->addRow(tr("Name"), m_nameEdit);
    connect(m_nameEdit, &QLineEdit::editingFinished, this, [this] {
        if (m_updatingEditors || !m_currentObject) {
            return;
        }
        m_currentObject->setName(m_nameEdit->text());
    });

    m_typeEdit = new QLineEdit(this);
    m_typeEdit->setReadOnly(true);
    objectFormLayout->addRow(tr("Type"), m_typeEdit);

    m_parentCombo = new QComboBox(this);
    objectFormLayout->addRow(tr("Parent"), m_parentCombo);
    connect(m_parentCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (m_updatingEditors || index < 0 || !m_currentObject) {
            return;
        }

        auto *parentObject = qobject_cast<PrimitiveObject *>(m_parentCombo->currentData().value<QObject *>());
        emit parentChangeRequested(m_currentObject, parentObject);
    });

    m_colorButton = new QPushButton(this);
    m_colorButton->setCursor(Qt::PointingHandCursor);
    objectFormLayout->addRow(tr("Color"), m_colorButton);
    connect(m_colorButton, &QPushButton::clicked, this, &PropertyPanel::chooseColor);

    auto createVectorRow = [this](QFormLayout *targetLayout,
                                  const QString &label,
                                  std::array<QDoubleSpinBox *, 3> &editors,
                                  auto handler) {
        auto *rowWidget = new QWidget(this);
        auto *outerLayout = new QVBoxLayout(rowWidget);
        outerLayout->setContentsMargins(0, 0, 0, 0);
        outerLayout->setSpacing(6);

        auto *rowLabel = new QLabel(label, rowWidget);
        rowLabel->setStyleSheet("font-weight: 600; color: #475569;");
        outerLayout->addWidget(rowLabel);

        auto *rowLayout = new QHBoxLayout();
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(6);
        const std::array axisLabels = {QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z")};

        for (int i = 0; i < 3; ++i) {
            auto *editor = new QDoubleSpinBox(rowWidget);
            editor->setRange(-9999.0, 9999.0);
            editor->setDecimals(3);
            editor->setSingleStep(0.1);
            editor->setPrefix(axisLabels[i] + QStringLiteral(": "));
            rowLayout->addWidget(editor);
            editors[i] = editor;
            connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), this, handler);
        }

        outerLayout->addLayout(rowLayout);
        targetLayout->addRow(rowWidget);
    };

    createVectorRow(transformFormLayout, tr("Position"), m_positionEdits, [this](double) {
        updatePositionFromEditors();
    });
    createVectorRow(transformFormLayout, tr("Rotation"), m_rotationEdits, [this](double) {
        updateRotationFromEditors();
    });

    for (int i = 0; i < int(m_dimensionEdits.size()); ++i) {
        auto *label = new QLabel(this);
        auto *editor = new QDoubleSpinBox(this);
        label->setVisible(false);
        editor->setVisible(false);
        editor->setDecimals(3);
        editor->setRange(PrimitiveObject::minimumDimensionValue(), 9999.0);
        editor->setSingleStep(0.1);
        connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, i](double) {
            updateDimensionFromEditors(i);
        });
        m_dimensionLabels[i] = label;
        m_dimensionEdits[i] = editor;
        dimensionsFormLayout->addRow(label, editor);
    }

    m_deleteButton = new QPushButton(tr("Delete Object"), this);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setStyleSheet(
        "QPushButton {"
        " background: #fff1f2;"
        " color: #b42318;"
        " border: 1px solid #f5c2c7;"
        " border-radius: 6px;"
        " padding: 8px 12px;"
        " font-weight: 600;"
        "}"
        "QPushButton:hover {"
        " background: #ffe4e6;"
        "}"
        "QPushButton:pressed {"
        " background: #fecdd3;"
        "}"
        "QPushButton:disabled {"
        " color: #b8c0cc;"
        " background: #f8fafc;"
        " border-color: #e2e8f0;"
        "}");
    connect(m_deleteButton, &QPushButton::clicked, this, [this] {
        if (m_currentObject) {
            emit removeRequested(m_currentObject);
        }
    });
    contentLayout->addWidget(m_deleteButton);

    setPropertyEditorsEnabled(false);
    m_propertyContentWidget->setVisible(false);
    m_emptyStateLabel->setVisible(true);
    m_dimensionsSectionLabel->setVisible(false);
}

void PropertyPanel::setObjects(const QList<PrimitiveObject *> &objects)
{
    m_objects = objects;
    refreshParentOptions();
}

void PropertyPanel::setCurrentObject(PrimitiveObject *object)
{
    m_currentObject = object;

    m_updatingEditors = true;
    m_propertyContentWidget->setVisible(object != nullptr);
    m_emptyStateLabel->setVisible(object == nullptr);
    setPropertyEditorsEnabled(object != nullptr);
    if (!object) {
        m_nameEdit->clear();
        m_typeEdit->clear();
        for (auto *editor : m_positionEdits) {
            editor->setValue(0.0);
        }
        for (auto *editor : m_rotationEdits) {
            editor->setValue(0.0);
        }
    } else {
        m_nameEdit->setText(object->name());
        m_typeEdit->setText(PrimitiveObject::displayName(object->primitiveType()));
        const QVector3D position = object->position();
        const QVector3D rotation = object->rotation();
        m_positionEdits[0]->setValue(position.x());
        m_positionEdits[1]->setValue(position.y());
        m_positionEdits[2]->setValue(position.z());
        m_rotationEdits[0]->setValue(rotation.x());
        m_rotationEdits[1]->setValue(rotation.y());
        m_rotationEdits[2]->setValue(rotation.z());
    }
    refreshColorEditor();
    refreshParentOptions();
    refreshDimensionEditors();
    m_updatingEditors = false;
}

void PropertyPanel::refreshObject(PrimitiveObject *object)
{
    if (!object) {
        return;
    }

    m_updatingEditors = true;
    if (m_currentObject == object) {
        m_nameEdit->setText(object->name());
        m_typeEdit->setText(PrimitiveObject::displayName(object->primitiveType()));
        const QVector3D position = object->position();
        const QVector3D rotation = object->rotation();
        m_positionEdits[0]->setValue(position.x());
        m_positionEdits[1]->setValue(position.y());
        m_positionEdits[2]->setValue(position.z());
        m_rotationEdits[0]->setValue(rotation.x());
        m_rotationEdits[1]->setValue(rotation.y());
        m_rotationEdits[2]->setValue(rotation.z());
        refreshColorEditor();
        refreshDimensionEditors();
    }
    refreshParentOptions();
    m_updatingEditors = false;
}

void PropertyPanel::setPropertyEditorsEnabled(bool enabled) const
{
    m_nameEdit->setEnabled(enabled);
    m_typeEdit->setEnabled(false);
    m_parentCombo->setEnabled(enabled);
    m_colorButton->setEnabled(enabled);
    m_deleteButton->setEnabled(enabled);
    for (auto *editor : m_positionEdits) {
        editor->setEnabled(enabled);
    }
    for (auto *editor : m_rotationEdits) {
        editor->setEnabled(enabled);
    }
    for (auto *editor : m_dimensionEdits) {
        if (editor) {
            editor->setEnabled(enabled);
        }
    }
}

void PropertyPanel::chooseColor()
{
    if (m_updatingEditors || !m_currentObject) {
        return;
    }

    const QColor color = QColorDialog::getColor(m_currentObject->color(), this, tr("Select Object Color"));
    if (!color.isValid() || color == m_currentObject->color()) {
        return;
    }

    m_currentObject->setColor(color);
}

void PropertyPanel::refreshColorEditor()
{
    if (!m_currentObject) {
        m_colorButton->setText(tr("Choose Color"));
        m_colorButton->setStyleSheet(colorButtonStyle(QColor(QStringLiteral("#e2e8f0"))));
        return;
    }

    const QColor color = m_currentObject->color();
    m_colorButton->setText(color.name(QColor::HexRgb).toUpper());
    m_colorButton->setStyleSheet(colorButtonStyle(color));
}

void PropertyPanel::refreshParentOptions()
{
    const QSignalBlocker blocker(m_parentCombo);

    m_parentCombo->clear();
    m_parentCombo->addItem(tr("<Scene Root>"), QVariant::fromValue(static_cast<QObject *>(nullptr)));

    for (auto *candidate : m_objects) {
        if (!m_currentObject || candidate == m_currentObject || m_currentObject->isAncestorOf(candidate)) {
            continue;
        }
        m_parentCombo->addItem(candidate->name(), QVariant::fromValue(static_cast<QObject *>(candidate)));
    }

    if (!m_currentObject) {
        m_parentCombo->setCurrentIndex(0);
        return;
    }

    const auto *parentObject = m_currentObject->parentPrimitive();
    for (int index = 0; index < m_parentCombo->count(); ++index) {
        auto *candidate = qobject_cast<PrimitiveObject *>(m_parentCombo->itemData(index).value<QObject *>());
        if (candidate == parentObject) {
            m_parentCombo->setCurrentIndex(index);
            return;
        }
    }
    m_parentCombo->setCurrentIndex(0);
}

void PropertyPanel::refreshDimensionEditors()
{
    const int visibleCount = m_currentObject ? m_currentObject->editableDimensionCount() : 0;
    m_dimensionsSectionLabel->setVisible(visibleCount > 0);

    for (int i = 0; i < int(m_dimensionEdits.size()); ++i) {
        const bool visible = m_currentObject && i < visibleCount;
        m_dimensionLabels[i]->setVisible(visible);
        m_dimensionEdits[i]->setVisible(visible);
        if (visible) {
            m_dimensionLabels[i]->setText(m_currentObject->dimensionLabel(i));
            m_dimensionEdits[i]->setValue(m_currentObject->dimensionValue(i));
        } else {
            m_dimensionLabels[i]->clear();
            m_dimensionEdits[i]->setValue(PrimitiveObject::minimumDimensionValue());
        }
    }
}

void PropertyPanel::updatePositionFromEditors() const
{
    if (m_updatingEditors || !m_currentObject) {
        return;
    }
    m_currentObject->setPosition(QVector3D(float(m_positionEdits[0]->value()),
                                           float(m_positionEdits[1]->value()),
                                           float(m_positionEdits[2]->value())));
}

void PropertyPanel::updateRotationFromEditors() const
{
    if (m_updatingEditors || !m_currentObject) {
        return;
    }
    m_currentObject->setRotation(QVector3D(float(m_rotationEdits[0]->value()),
                                           float(m_rotationEdits[1]->value()),
                                           float(m_rotationEdits[2]->value())));
}

void PropertyPanel::updateDimensionFromEditors(int index) const
{
    if (m_updatingEditors || !m_currentObject) {
        return;
    }
    m_currentObject->setDimensionValue(index, float(m_dimensionEdits[index]->value()));
}
