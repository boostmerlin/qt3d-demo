#include "property_panel.h"

#include <QComboBox>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QSpinBox>
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

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    auto *wrapperLayout = new QVBoxLayout(this);
    wrapperLayout->setContentsMargins(12, 12, 12, 12);
    wrapperLayout->setSpacing(10);

    auto *title = new QLabel(tr("Properties"), this);
    title->setStyleSheet("font-weight: 600; font-size: 13px; color: #334155;");
    wrapperLayout->addWidget(title);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    wrapperLayout->addWidget(scrollArea, 1);

    auto *scrollContent = new QWidget(scrollArea);
    auto *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(10);
    scrollArea->setWidget(scrollContent);

    m_emptyStateLabel = new QLabel(tr("No object selected"), scrollContent);
    m_emptyStateLabel->setAlignment(Qt::AlignCenter);
    m_emptyStateLabel->setStyleSheet("color: #64748b; padding: 24px 12px;");
    scrollLayout->addWidget(m_emptyStateLabel);

    m_propertyContentWidget = new QWidget(scrollContent);
    scrollLayout->addWidget(m_propertyContentWidget);
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

    m_polygonSectionLabel = createSectionTitle(tr("Polygon"));
    contentLayout->addWidget(m_polygonSectionLabel);
    m_polygonSectionWidget = new QWidget(m_propertyContentWidget);
    auto *polygonSectionLayout = new QVBoxLayout(m_polygonSectionWidget);
    polygonSectionLayout->setContentsMargins(0, 0, 0, 0);
    polygonSectionLayout->setSpacing(8);
    contentLayout->addWidget(m_polygonSectionWidget);

    m_polygonValidationLabel = new QLabel(tr("At least 3 vertices are required to render the polygon."),
                                          m_polygonSectionWidget);
    m_polygonValidationLabel->setStyleSheet("color: #b45309; font-size: 12px;");
    polygonSectionLayout->addWidget(m_polygonValidationLabel);

    m_addPolygonVertexButton = new QPushButton(tr("Add Vertex"), m_polygonSectionWidget);
    m_addPolygonVertexButton->setCursor(Qt::PointingHandCursor);
    polygonSectionLayout->addWidget(m_addPolygonVertexButton);
    connect(m_addPolygonVertexButton, &QPushButton::clicked, this, [this] {
        auto *polygonObject = qobject_cast<PolygonObject *>(m_currentObject.data());
        if (!polygonObject) {
            return;
        }
        polygonObject->addRegularVertex();
    });

    auto *polygonVerticesScrollArea = new QScrollArea(m_polygonSectionWidget);
    polygonVerticesScrollArea->setWidgetResizable(true);
    polygonVerticesScrollArea->setFrameShape(QFrame::NoFrame);
    polygonVerticesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    polygonVerticesScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    polygonVerticesScrollArea->setMinimumHeight(220);
    polygonVerticesScrollArea->setMaximumHeight(280);
    polygonVerticesScrollArea->setStyleSheet(
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
    polygonSectionLayout->addWidget(polygonVerticesScrollArea);

    m_polygonVerticesWidget = new QWidget(m_polygonSectionWidget);
    m_polygonVerticesLayout = new QVBoxLayout(m_polygonVerticesWidget);
    m_polygonVerticesLayout->setContentsMargins(8, 8, 8, 8);
    m_polygonVerticesLayout->setSpacing(10);
    polygonVerticesScrollArea->setWidget(m_polygonVerticesWidget);

    auto *polygonParametersWidget = new QWidget(m_polygonSectionWidget);
    auto *polygonParametersLayout = new QHBoxLayout(polygonParametersWidget);
    polygonParametersLayout->setContentsMargins(0, 0, 0, 0);
    polygonParametersLayout->setSpacing(10);
    polygonSectionLayout->addWidget(polygonParametersWidget);

    auto *heightLabel = new QLabel(tr("Height"), polygonParametersWidget);
    polygonParametersLayout->addWidget(heightLabel);

    m_polygonHeightEdit = new QDoubleSpinBox(polygonParametersWidget);
    m_polygonHeightEdit->setDecimals(3);
    m_polygonHeightEdit->setRange(0.0, 9999.0);
    m_polygonHeightEdit->setSingleStep(0.1);
    m_polygonHeightEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    polygonParametersLayout->addWidget(m_polygonHeightEdit, 1);
    connect(m_polygonHeightEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        if (m_updatingEditors) {
            return;
        }
        auto *polygonObject = qobject_cast<PolygonObject *>(m_currentObject.data());
        if (!polygonObject || !polygonObject->setHeight(float(value))) {
            refreshPolygonEditor();
        }
    });

    auto *ringsLabel = new QLabel(tr("Rings"), polygonParametersWidget);
    polygonParametersLayout->addWidget(ringsLabel);

    m_polygonRingsEdit = new QSpinBox(polygonParametersWidget);
    m_polygonRingsEdit->setRange(0, 64);
    m_polygonRingsEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    polygonParametersLayout->addWidget(m_polygonRingsEdit, 1);
    connect(m_polygonRingsEdit, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
        if (m_updatingEditors) {
            return;
        }
        auto *polygonObject = qobject_cast<PolygonObject *>(m_currentObject.data());
        if (!polygonObject) {
            return;
        }
        if (value == 1) {
            const QSignalBlocker blocker(m_polygonRingsEdit);
            m_polygonRingsEdit->setValue(2);
            value = 2;
        }
        if (!polygonObject->setRings(uint(value))) {
            refreshPolygonEditor();
        }
    });

    m_extrudeSectionLabel = createSectionTitle(tr("Extrude"));
    contentLayout->addWidget(m_extrudeSectionLabel);
    m_extrudeSectionWidget = new QWidget(m_propertyContentWidget);
    auto *extrudeSectionLayout = new QVBoxLayout(m_extrudeSectionWidget);
    extrudeSectionLayout->setContentsMargins(0, 0, 0, 0);
    extrudeSectionLayout->setSpacing(8);
    contentLayout->addWidget(m_extrudeSectionWidget);

    auto *pathSectionLabel = new QLabel(tr("Path"), m_extrudeSectionWidget);
    pathSectionLabel->setStyleSheet("font-weight: 600; color: #475569;");
    extrudeSectionLayout->addWidget(pathSectionLabel);

    auto *extrudePathFormLayout = new QFormLayout();
    extrudePathFormLayout->setContentsMargins(0, 0, 0, 0);
    extrudePathFormLayout->setHorizontalSpacing(10);
    extrudePathFormLayout->setVerticalSpacing(10);
    extrudeSectionLayout->addLayout(extrudePathFormLayout);

    m_extrudePathTypeCombo = new QComboBox(m_extrudeSectionWidget);
    m_extrudePathTypeCombo->addItem(tr("Line"), int(ExtrudeObject::PathType::Line));
    m_extrudePathTypeCombo->addItem(tr("Cubic Bezier"), int(ExtrudeObject::PathType::CubicBezier));
    extrudePathFormLayout->addRow(tr("Path Type"), m_extrudePathTypeCombo);
    connect(m_extrudePathTypeCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (m_updatingEditors || index < 0) {
            return;
        }
        auto *extrudeObject = qobject_cast<ExtrudeObject *>(m_currentObject.data());
        if (!extrudeObject) {
            return;
        }
        if (!extrudeObject->setPathType(ExtrudeObject::PathType(m_extrudePathTypeCombo->itemData(index).toInt()))) {
            refreshExtrudeEditor();
        }
    });

    m_extrudePathDivisionsEdit = new QSpinBox(m_extrudeSectionWidget);
    m_extrudePathDivisionsEdit->setRange(1, 128);
    extrudePathFormLayout->addRow(tr("Divisions"), m_extrudePathDivisionsEdit);
    connect(m_extrudePathDivisionsEdit, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
        if (m_updatingEditors) {
            return;
        }
        auto *extrudeObject = qobject_cast<ExtrudeObject *>(m_currentObject.data());
        if (!extrudeObject || !extrudeObject->setPathDivisions(uint(value))) {
            refreshExtrudeEditor();
        }
    });

    m_extrudePathValidationLabel = new QLabel(
        tr("Path is invalid. Line requires 2 distinct points; Cubic Bezier requires 4 control points."),
        m_extrudeSectionWidget);
    m_extrudePathValidationLabel->setStyleSheet("color: #b45309; font-size: 12px;");
    extrudeSectionLayout->addWidget(m_extrudePathValidationLabel);

    m_extrudePathPointsWidget = new QWidget(m_extrudeSectionWidget);
    m_extrudePathPointsLayout = new QVBoxLayout(m_extrudePathPointsWidget);
    m_extrudePathPointsLayout->setContentsMargins(0, 0, 0, 0);
    m_extrudePathPointsLayout->setSpacing(8);
    extrudeSectionLayout->addWidget(m_extrudePathPointsWidget);

    auto *profileSectionLabel = new QLabel(tr("Profile"), m_extrudeSectionWidget);
    profileSectionLabel->setStyleSheet("font-weight: 600; color: #475569; padding-top: 4px;");
    extrudeSectionLayout->addWidget(profileSectionLabel);

    m_extrudeProfileValidationLabel = new QLabel(
        tr("At least 3 profile vertices are required to render the extrude."),
        m_extrudeSectionWidget);
    m_extrudeProfileValidationLabel->setStyleSheet("color: #b45309; font-size: 12px;");
    extrudeSectionLayout->addWidget(m_extrudeProfileValidationLabel);

    m_addExtrudeProfileVertexButton = new QPushButton(tr("Add Vertex"), m_extrudeSectionWidget);
    m_addExtrudeProfileVertexButton->setCursor(Qt::PointingHandCursor);
    extrudeSectionLayout->addWidget(m_addExtrudeProfileVertexButton);
    connect(m_addExtrudeProfileVertexButton, &QPushButton::clicked, this, [this] {
        auto *extrudeObject = qobject_cast<ExtrudeObject *>(m_currentObject.data());
        if (!extrudeObject) {
            return;
        }
        extrudeObject->addRegularProfileVertex();
    });

    auto *extrudeProfileScrollArea = new QScrollArea(m_extrudeSectionWidget);
    extrudeProfileScrollArea->setWidgetResizable(true);
    extrudeProfileScrollArea->setFrameShape(QFrame::NoFrame);
    extrudeProfileScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    extrudeProfileScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    extrudeProfileScrollArea->setMinimumHeight(180);
    extrudeProfileScrollArea->setMaximumHeight(240);
    extrudeProfileScrollArea->setStyleSheet(
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
    extrudeSectionLayout->addWidget(extrudeProfileScrollArea);

    m_extrudeProfileVerticesWidget = new QWidget(m_extrudeSectionWidget);
    m_extrudeProfileVerticesLayout = new QVBoxLayout(m_extrudeProfileVerticesWidget);
    m_extrudeProfileVerticesLayout->setContentsMargins(8, 8, 8, 8);
    m_extrudeProfileVerticesLayout->setSpacing(10);
    extrudeProfileScrollArea->setWidget(m_extrudeProfileVerticesWidget);

    scrollLayout->addStretch(1);

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

        auto *parentObject = qobject_cast<SceneObject *>(m_parentCombo->currentData().value<QObject *>());
        emit parentChangeRequested(m_currentObject, parentObject);
    });

    m_colorButton = new QPushButton(this);
    m_colorButton->setCursor(Qt::PointingHandCursor);
    objectFormLayout->addRow(tr("Color"), m_colorButton);
    connect(m_colorButton, &QPushButton::clicked, this, &PropertyPanel::chooseColor);

    m_lineTypeLabel = new QLabel(tr("Line Type"), this);
    m_lineTypeCombo = new QComboBox(this);
    m_lineTypeCombo->addItem(tr("Line"), int(LineGeometry::Line));
    m_lineTypeCombo->addItem(tr("Line Segments"), int(LineGeometry::LineSegments));
    m_lineTypeCombo->addItem(tr("Line Loop"), int(LineGeometry::LineLoop));
    m_lineTypeLabel->setVisible(false);
    m_lineTypeCombo->setVisible(false);
    objectFormLayout->addRow(m_lineTypeLabel, m_lineTypeCombo);
    connect(m_lineTypeCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (m_updatingEditors || index < 0) {
            return;
        }
        auto *lineObject = qobject_cast<LineObject *>(m_currentObject.data());
        if (!lineObject) {
            return;
        }
        lineObject->setLineType(LineGeometry::LineType(m_lineTypeCombo->itemData(index).toInt()));
    });

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
    m_polygonSectionLabel->setVisible(false);
    m_polygonSectionWidget->setVisible(false);
    m_extrudeSectionLabel->setVisible(false);
    m_extrudeSectionWidget->setVisible(false);
}

void PropertyPanel::setObjects(const QList<SceneObject *> &objects)
{
    m_objects = objects;
    refreshParentOptions();
}

void PropertyPanel::setCurrentObject(SceneObject *object)
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
        m_typeEdit->setText(object->typeName());
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
    refreshLineTypeEditor();
    refreshDimensionEditors();
    refreshPolygonEditor();
    refreshExtrudeEditor();
    m_updatingEditors = false;
}

void PropertyPanel::refreshObject(SceneObject *object)
{
    if (!object) {
        return;
    }

    m_updatingEditors = true;
    if (m_currentObject == object) {
        m_nameEdit->setText(object->name());
        m_typeEdit->setText(object->typeName());
        const QVector3D position = object->position();
        const QVector3D rotation = object->rotation();
        m_positionEdits[0]->setValue(position.x());
        m_positionEdits[1]->setValue(position.y());
        m_positionEdits[2]->setValue(position.z());
        m_rotationEdits[0]->setValue(rotation.x());
        m_rotationEdits[1]->setValue(rotation.y());
        m_rotationEdits[2]->setValue(rotation.z());
        refreshColorEditor();
        refreshLineTypeEditor();
        refreshDimensionEditors();
        refreshPolygonEditor();
        refreshExtrudeEditor();
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
    m_lineTypeCombo->setEnabled(enabled);
    m_polygonSectionWidget->setEnabled(enabled);
    m_extrudeSectionWidget->setEnabled(enabled);
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

    const auto *parentObject = m_currentObject->parentSceneObject();
    for (int index = 0; index < m_parentCombo->count(); ++index) {
        auto *candidate = qobject_cast<SceneObject *>(m_parentCombo->itemData(index).value<QObject *>());
        if (candidate == parentObject) {
            m_parentCombo->setCurrentIndex(index);
            return;
        }
    }
    m_parentCombo->setCurrentIndex(0);
}

void PropertyPanel::refreshLineTypeEditor()
{
    auto *lineObject = qobject_cast<LineObject *>(m_currentObject.data());
    const bool visible = lineObject != nullptr;
    m_lineTypeLabel->setVisible(visible);
    m_lineTypeCombo->setVisible(visible);
    if (!lineObject) {
        return;
    }

    const QSignalBlocker blocker(m_lineTypeCombo);
    const int value = int(lineObject->lineType());
    for (int index = 0; index < m_lineTypeCombo->count(); ++index) {
        if (m_lineTypeCombo->itemData(index).toInt() == value) {
            m_lineTypeCombo->setCurrentIndex(index);
            return;
        }
    }
}

void PropertyPanel::refreshDimensionEditors()
{
    auto *primitiveObject = qobject_cast<PrimitiveObject *>(m_currentObject.data());
    const int visibleCount = primitiveObject ? primitiveObject->editableDimensionCount() : 0;
    m_dimensionsSectionLabel->setVisible(visibleCount > 0);

    for (int i = 0; i < int(m_dimensionEdits.size()); ++i) {
        const bool visible = primitiveObject && i < visibleCount;
        m_dimensionLabels[i]->setVisible(visible);
        m_dimensionEdits[i]->setVisible(visible);
        if (visible) {
            m_dimensionLabels[i]->setText(primitiveObject->dimensionLabel(i));
            m_dimensionEdits[i]->setRange(primitiveObject->dimensionMinimum(i),
                                          primitiveObject->dimensionMaximum(i));
            m_dimensionEdits[i]->setValue(primitiveObject->dimensionValue(i));
        } else {
            m_dimensionLabels[i]->clear();
            m_dimensionEdits[i]->setValue(PrimitiveObject::minimumDimensionValue());
        }
    }
}

void PropertyPanel::refreshPolygonEditor()
{
    auto *polygonObject = qobject_cast<PolygonObject *>(m_currentObject.data());
    const bool visible = polygonObject != nullptr;
    m_polygonSectionLabel->setVisible(visible);
    m_polygonSectionWidget->setVisible(visible);
    if (!polygonObject) {
        clearLayout(m_polygonVerticesLayout);
        return;
    }

    m_polygonValidationLabel->setVisible(!polygonObject->hasValidPolygon());

    {
        const QSignalBlocker blocker(m_polygonHeightEdit);
        m_polygonHeightEdit->setValue(polygonObject->height());
    }
    {
        const QSignalBlocker blocker(m_polygonRingsEdit);
        m_polygonRingsEdit->setValue(int(polygonObject->rings()));
    }

    rebuildPolygonVertexRows(polygonObject);
}

void PropertyPanel::refreshExtrudeEditor()
{
    auto *extrudeObject = qobject_cast<ExtrudeObject *>(m_currentObject.data());
    const bool visible = extrudeObject != nullptr;
    m_extrudeSectionLabel->setVisible(visible);
    m_extrudeSectionWidget->setVisible(visible);
    if (!extrudeObject) {
        clearLayout(m_extrudePathPointsLayout);
        clearLayout(m_extrudeProfileVerticesLayout);
        return;
    }

    m_extrudePathValidationLabel->setVisible(!extrudeObject->hasValidPath());
    m_extrudeProfileValidationLabel->setVisible(!extrudeObject->hasValidProfile());

    {
        const QSignalBlocker blocker(m_extrudePathTypeCombo);
        const int value = int(extrudeObject->pathType());
        for (int index = 0; index < m_extrudePathTypeCombo->count(); ++index) {
            if (m_extrudePathTypeCombo->itemData(index).toInt() == value) {
                m_extrudePathTypeCombo->setCurrentIndex(index);
                break;
            }
        }
    }
    {
        const QSignalBlocker blocker(m_extrudePathDivisionsEdit);
        m_extrudePathDivisionsEdit->setValue(int(extrudeObject->pathDivisions()));
    }

    rebuildExtrudePathRows(extrudeObject);
    rebuildExtrudeProfileRows(extrudeObject);
}

void PropertyPanel::rebuildPolygonVertexRows(PolygonObject *polygonObject)
{
    Q_ASSERT(polygonObject);
    clearLayout(m_polygonVerticesLayout);

    const auto vertices = polygonObject->vertices();
    for (int index = 0; index < vertices.size(); ++index) {
        auto *rowWidget = new QWidget(m_polygonVerticesWidget);
        rowWidget->setObjectName(QStringLiteral("polygonVertexCard"));
        rowWidget->setStyleSheet(
            "#polygonVertexCard {"
            " background: #f8fbff;"
            " border: 1px solid #dbe5f0;"
            " border-radius: 8px;"
            "}"
            "#polygonVertexCard QPushButton {"
            " min-width: 58px;"
            "}"
            "#polygonVertexCard QPushButton:disabled {"
            " color: #94a3b8;"
            "}");

        auto *rowLayout = new QVBoxLayout(rowWidget);
        rowLayout->setContentsMargins(10, 8, 10, 8);
        rowLayout->setSpacing(6);

        auto createCoordinateEditor = [rowWidget](const QString &axisLabel, double value) {
            auto *editor = new QDoubleSpinBox(rowWidget);
            editor->setRange(-9999.0, 9999.0);
            editor->setDecimals(3);
            editor->setSingleStep(0.1);
            editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
            editor->setPrefix(axisLabel + QStringLiteral(": "));
            editor->setMinimumWidth(96);
            editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            editor->setValue(value);
            return editor;
        };

        auto *coordinatesLayout = new QHBoxLayout();
        coordinatesLayout->setContentsMargins(0, 0, 0, 0);
        coordinatesLayout->setSpacing(8);

        auto *label = new QLabel(tr("V%1").arg(index + 1), rowWidget);
        label->setFixedWidth(28);
        label->setStyleSheet("font-weight: 600; color: #334155;");
        coordinatesLayout->addWidget(label);

        auto *xEdit = createCoordinateEditor(QStringLiteral("X"), vertices[index].x());
        coordinatesLayout->addWidget(xEdit, 1);

        auto *yEdit = createCoordinateEditor(QStringLiteral("Y"), vertices[index].y());
        coordinatesLayout->addWidget(yEdit, 1);

        auto *zEdit = createCoordinateEditor(QStringLiteral("Z"), vertices[index].z());
        coordinatesLayout->addWidget(zEdit, 1);

        rowLayout->addLayout(coordinatesLayout);

        auto *actionLayout = new QHBoxLayout();
        actionLayout->setContentsMargins(0, 0, 0, 0);
        actionLayout->setSpacing(8);
        actionLayout->addStretch(1);

        auto *upButton = new QPushButton(tr("Up"), rowWidget);
        upButton->setEnabled(index > 0);
        actionLayout->addWidget(upButton);

        auto *downButton = new QPushButton(tr("Down"), rowWidget);
        downButton->setEnabled(index + 1 < vertices.size());
        actionLayout->addWidget(downButton);

        auto *removeButton = new QPushButton(tr("Remove"), rowWidget);
        actionLayout->addWidget(removeButton);

        rowLayout->addLayout(actionLayout);

        connect(xEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, index](double value) {
            if (m_updatingEditors) {
                return;
            }
            auto *polygon = qobject_cast<PolygonObject *>(m_currentObject.data());
            if (!polygon) {
                return;
            }
            auto points = polygon->vertices();
            if (index >= points.size()) {
                return;
            }
            points[index].setX(value);
            polygon->setVertices(points);
        });
        connect(yEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, index](double value) {
            if (m_updatingEditors) {
                return;
            }
            auto *polygon = qobject_cast<PolygonObject *>(m_currentObject.data());
            if (!polygon) {
                return;
            }
            auto points = polygon->vertices();
            if (index >= points.size()) {
                return;
            }
            points[index].setY(value);
            polygon->setVertices(points);
        });
        connect(zEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, index](double value) {
            if (m_updatingEditors) {
                return;
            }
            auto *polygon = qobject_cast<PolygonObject *>(m_currentObject.data());
            if (!polygon) {
                return;
            }
            auto points = polygon->vertices();
            if (index >= points.size()) {
                return;
            }
            points[index].setZ(float(value));
            polygon->setVertices(points);
        });
        connect(upButton, &QPushButton::clicked, this, [this, index] {
            auto *polygon = qobject_cast<PolygonObject *>(m_currentObject.data());
            if (polygon) {
                polygon->moveVertex(index, index - 1);
            }
        });
        connect(downButton, &QPushButton::clicked, this, [this, index] {
            auto *polygon = qobject_cast<PolygonObject *>(m_currentObject.data());
            if (polygon) {
                polygon->moveVertex(index, index + 1);
            }
        });
        connect(removeButton, &QPushButton::clicked, this, [this, index] {
            auto *polygon = qobject_cast<PolygonObject *>(m_currentObject.data());
            if (polygon) {
                polygon->removeVertexAt(index);
            }
        });

        m_polygonVerticesLayout->addWidget(rowWidget);
    }
    m_polygonVerticesLayout->addStretch(1);
}

void PropertyPanel::rebuildExtrudePathRows(ExtrudeObject *extrudeObject)
{
    Q_ASSERT(extrudeObject);
    clearLayout(m_extrudePathPointsLayout);

    const auto points = extrudeObject->pathControlPoints();
    for (int index = 0; index < points.size(); ++index) {
        auto *rowWidget = new QWidget(m_extrudePathPointsWidget);
        auto *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(8);

        auto *label = new QLabel(tr("P%1").arg(index + 1), rowWidget);
        label->setFixedWidth(28);
        label->setStyleSheet("font-weight: 600; color: #334155;");
        rowLayout->addWidget(label);

        auto *xEdit = new QDoubleSpinBox(rowWidget);
        xEdit->setRange(-9999.0, 9999.0);
        xEdit->setDecimals(3);
        xEdit->setSingleStep(0.1);
        xEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
        xEdit->setPrefix(QStringLiteral("X: "));
        xEdit->setValue(points[index].x());
        rowLayout->addWidget(xEdit, 1);

        auto *yEdit = new QDoubleSpinBox(rowWidget);
        yEdit->setRange(-9999.0, 9999.0);
        yEdit->setDecimals(3);
        yEdit->setSingleStep(0.1);
        yEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
        yEdit->setPrefix(QStringLiteral("Y: "));
        yEdit->setValue(points[index].y());
        rowLayout->addWidget(yEdit, 1);

        connect(xEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, index](double value) {
            if (m_updatingEditors) {
                return;
            }
            auto *extrude = qobject_cast<ExtrudeObject *>(m_currentObject.data());
            if (!extrude) {
                return;
            }
            auto currentPoints = extrude->pathControlPoints();
            if (index >= currentPoints.size()) {
                return;
            }
            currentPoints[index].setX(float(value));
            if (!extrude->setPathControlPoints(currentPoints)) {
                refreshExtrudeEditor();
            }
        });
        connect(yEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, index](double value) {
            if (m_updatingEditors) {
                return;
            }
            auto *extrude = qobject_cast<ExtrudeObject *>(m_currentObject.data());
            if (!extrude) {
                return;
            }
            auto currentPoints = extrude->pathControlPoints();
            if (index >= currentPoints.size()) {
                return;
            }
            currentPoints[index].setY(float(value));
            if (!extrude->setPathControlPoints(currentPoints)) {
                refreshExtrudeEditor();
            }
        });

        m_extrudePathPointsLayout->addWidget(rowWidget);
    }
}

void PropertyPanel::rebuildExtrudeProfileRows(ExtrudeObject *extrudeObject)
{
    Q_ASSERT(extrudeObject);
    clearLayout(m_extrudeProfileVerticesLayout);

    const auto vertices = extrudeObject->profileVertices();
    for (int index = 0; index < vertices.size(); ++index) {
        auto *rowWidget = new QWidget(m_extrudeProfileVerticesWidget);
        rowWidget->setObjectName(QStringLiteral("extrudeProfileVertexCard"));
        rowWidget->setStyleSheet(
            "#extrudeProfileVertexCard {"
            " background: #f8fbff;"
            " border: 1px solid #dbe5f0;"
            " border-radius: 8px;"
            "}"
            "#extrudeProfileVertexCard QPushButton {"
            " min-width: 58px;"
            "}"
            "#extrudeProfileVertexCard QPushButton:disabled {"
            " color: #94a3b8;"
            "}");

        auto *rowLayout = new QVBoxLayout(rowWidget);
        rowLayout->setContentsMargins(10, 8, 10, 8);
        rowLayout->setSpacing(6);

        auto createCoordinateEditor = [rowWidget](const QString &axisLabel, double value) {
            auto *editor = new QDoubleSpinBox(rowWidget);
            editor->setRange(-9999.0, 9999.0);
            editor->setDecimals(3);
            editor->setSingleStep(0.1);
            editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
            editor->setPrefix(axisLabel + QStringLiteral(": "));
            editor->setMinimumWidth(96);
            editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            editor->setValue(value);
            return editor;
        };

        auto *coordinatesLayout = new QHBoxLayout();
        coordinatesLayout->setContentsMargins(0, 0, 0, 0);
        coordinatesLayout->setSpacing(8);

        auto *label = new QLabel(tr("V%1").arg(index + 1), rowWidget);
        label->setFixedWidth(28);
        label->setStyleSheet("font-weight: 600; color: #334155;");
        coordinatesLayout->addWidget(label);

        auto *xEdit = createCoordinateEditor(QStringLiteral("X"), vertices[index].x());
        coordinatesLayout->addWidget(xEdit, 1);

        auto *yEdit = createCoordinateEditor(QStringLiteral("Y"), vertices[index].y());
        coordinatesLayout->addWidget(yEdit, 1);

        rowLayout->addLayout(coordinatesLayout);

        auto *actionLayout = new QHBoxLayout();
        actionLayout->setContentsMargins(0, 0, 0, 0);
        actionLayout->setSpacing(8);
        actionLayout->addStretch(1);

        auto *upButton = new QPushButton(tr("Up"), rowWidget);
        upButton->setEnabled(index > 0);
        actionLayout->addWidget(upButton);

        auto *downButton = new QPushButton(tr("Down"), rowWidget);
        downButton->setEnabled(index + 1 < vertices.size());
        actionLayout->addWidget(downButton);

        auto *removeButton = new QPushButton(tr("Remove"), rowWidget);
        actionLayout->addWidget(removeButton);

        rowLayout->addLayout(actionLayout);

        connect(xEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, index](double value) {
            if (m_updatingEditors) {
                return;
            }
            auto *extrude = qobject_cast<ExtrudeObject *>(m_currentObject.data());
            if (!extrude) {
                return;
            }
            auto points = extrude->profileVertices();
            if (index >= points.size()) {
                return;
            }
            points[index].setX(float(value));
            if (!extrude->setProfileVertices(points)) {
                refreshExtrudeEditor();
            }
        });
        connect(yEdit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, index](double value) {
            if (m_updatingEditors) {
                return;
            }
            auto *extrude = qobject_cast<ExtrudeObject *>(m_currentObject.data());
            if (!extrude) {
                return;
            }
            auto points = extrude->profileVertices();
            if (index >= points.size()) {
                return;
            }
            points[index].setY(float(value));
            if (!extrude->setProfileVertices(points)) {
                refreshExtrudeEditor();
            }
        });
        connect(upButton, &QPushButton::clicked, this, [this, index] {
            auto *extrude = qobject_cast<ExtrudeObject *>(m_currentObject.data());
            if (extrude) {
                extrude->moveProfileVertex(index, index - 1);
            }
        });
        connect(downButton, &QPushButton::clicked, this, [this, index] {
            auto *extrude = qobject_cast<ExtrudeObject *>(m_currentObject.data());
            if (extrude) {
                extrude->moveProfileVertex(index, index + 1);
            }
        });
        connect(removeButton, &QPushButton::clicked, this, [this, index] {
            auto *extrude = qobject_cast<ExtrudeObject *>(m_currentObject.data());
            if (extrude) {
                extrude->removeProfileVertexAt(index);
            }
        });

        m_extrudeProfileVerticesLayout->addWidget(rowWidget);
    }
    m_extrudeProfileVerticesLayout->addStretch(1);
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
    if (auto *primitiveObject = qobject_cast<PrimitiveObject *>(m_currentObject.data())) {
        primitiveObject->setDimensionValue(index, float(m_dimensionEdits[index]->value()));
    }
}
