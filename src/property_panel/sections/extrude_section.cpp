#include "property_panel/sections/extrude_section.h"

#include <QLabel>
#include <QVBoxLayout>

#include "scene/extrude_object.h"
#include "property_panel/editors/double_editor_row.h"
#include "property_panel/editors/enum_editor.h"
#include "property_panel/editors/int_editor_row.h"
#include "property_panel/editors/reorderable_point_list_editor.h"
#include "property_panel/property_editor_factory.h"
#include "scene/scene_controller.h"

ExtrudeSection::ExtrudeSection(SceneController *sceneController, QWidget *parent)
    : PropertySection(sceneController, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto *title = new QLabel(tr("Extrude"), this);
    title->setStyleSheet("font-weight: 600; color: #334155; padding-top: 4px;");
    layout->addWidget(title);

    auto *pathTitle = new QLabel(tr("Path"), this);
    pathTitle->setStyleSheet("font-weight: 600; color: #475569;");
    layout->addWidget(pathTitle);

    m_pathTypeEditor = PropertyEditorFactory::createEnumEditor(tr("Path Type"), this);
    m_pathTypeEditor->setItems({
        {tr("Line"), int(ExtrudeObject::PathType::Line)},
        {tr("Cubic Bezier"), int(ExtrudeObject::PathType::CubicBezier)},
        {tr("Ellipse"), int(ExtrudeObject::PathType::Ellipse)},
        {tr("Arc"), int(ExtrudeObject::PathType::Arc)},
    });
    layout->addWidget(m_pathTypeEditor);
    connect(m_pathTypeEditor, &EnumEditor::valueEdited, this, [this](const QVariant &value) {
        if (m_updating || !m_currentObject || !m_sceneController
            || !m_sceneController->setExtrudePathType(m_currentObject,
                                                      static_cast<ExtrudeObject::PathType>(value.toInt()))) {
            refreshEditors();
        }
    });

    m_divisionsEditor = PropertyEditorFactory::createIntEditor(tr("Divisions"), this);
    m_divisionsEditor->setRange(1, 128);
    layout->addWidget(m_divisionsEditor);
    connect(m_divisionsEditor, &IntEditorRow::valueEdited, this, [this](int value) {
        if (m_updating || !m_currentObject || !m_sceneController
            || !m_sceneController->setExtrudePathDivisions(m_currentObject, uint(value))) {
            refreshEditors();
        }
    });

    m_rotationEditor = PropertyEditorFactory::createDoubleEditor(tr("Rotation"), this);
    m_rotationEditor->setRange(-360.0, 360.0);
    m_rotationEditor->setSingleStep(5.0);
    layout->addWidget(m_rotationEditor);
    connect(m_rotationEditor, &DoubleEditorRow::valueEdited, this, [this](double value) {
        if (m_updating || !m_currentObject || !m_sceneController
            || !m_sceneController->setExtrudeEllipseRotation(m_currentObject, float(value))) {
            refreshEditors();
        }
    });

    m_startAngleEditor = PropertyEditorFactory::createDoubleEditor(tr("Start Angle"), this);
    m_startAngleEditor->setRange(-360.0, 360.0);
    m_startAngleEditor->setSingleStep(5.0);
    layout->addWidget(m_startAngleEditor);
    connect(m_startAngleEditor, &DoubleEditorRow::valueEdited, this, [this](double value) {
        if (m_updating || !m_currentObject || !m_sceneController
            || !m_sceneController->setExtrudePathStartAngle(m_currentObject, float(value))) {
            refreshEditors();
        }
    });

    m_endAngleEditor = PropertyEditorFactory::createDoubleEditor(tr("End Angle"), this);
    m_endAngleEditor->setRange(-360.0, 360.0);
    m_endAngleEditor->setSingleStep(5.0);
    layout->addWidget(m_endAngleEditor);
    connect(m_endAngleEditor, &DoubleEditorRow::valueEdited, this, [this](double value) {
        if (m_updating || !m_currentObject || !m_sceneController
            || !m_sceneController->setExtrudePathEndAngle(m_currentObject, float(value))) {
            refreshEditors();
        }
    });

    m_pathValidationLabel = new QLabel(
        tr("Path is invalid. Line requires 2 distinct points; Cubic Bezier requires 4 control points; Ellipse and Arc require valid radii and different start/end angles."),
        this);
    m_pathValidationLabel->setWordWrap(true);
    m_pathValidationLabel->setStyleSheet("color: #b45309; font-size: 12px;");
    layout->addWidget(m_pathValidationLabel);

    PointListEditorConfig pathConfig;
    pathConfig.dimension = 2;
    pathConfig.allowAdd = false;
    pathConfig.allowReorder = false;
    pathConfig.allowRemove = false;
    pathConfig.addButtonText = tr("Add Point");
    pathConfig.rowLabel = [this](int index) {
        if (!m_currentObject) {
            return tr("P%1").arg(index + 1);
        }
        if (m_currentObject->pathType() == ExtrudeObject::PathType::Ellipse
            || m_currentObject->pathType() == ExtrudeObject::PathType::Arc) {
            return index == 0 ? tr("Center") : tr("Radius");
        }
        return tr("P%1").arg(index + 1);
    };
    pathConfig.axisLabel = [this](int row, int axis) {
        if (!m_currentObject) {
            return axis == 0 ? QStringLiteral("X") : QStringLiteral("Y");
        }
        if (m_currentObject->pathType() == ExtrudeObject::PathType::Arc && row == 1 && axis == 0) {
            return QStringLiteral("R");
        }
        if (m_currentObject->pathType() == ExtrudeObject::PathType::Ellipse && row == 1) {
            return axis == 0 ? QStringLiteral("Major") : QStringLiteral("Minor");
        }
        return axis == 0 ? QStringLiteral("X") : QStringLiteral("Y");
    };
    pathConfig.axisVisible = [this](int row, int axis) {
        if (!m_currentObject) {
            return axis < 2;
        }
        if (m_currentObject->pathType() == ExtrudeObject::PathType::Arc && row == 1 && axis == 1) {
            return false;
        }
        return axis < 2;
    };
    m_pathPointsEditor = PropertyEditorFactory::createPointListEditor(pathConfig, this);
    layout->addWidget(m_pathPointsEditor);
    connect(m_pathPointsEditor, &ReorderablePointListEditor::pointEdited, this,
            [this](int index, int axis, double value) {
                if (m_updating || !m_currentObject) {
                    return;
                }
                auto points = m_currentObject->pathControlPoints();
                if (index < 0 || index >= points.size()) {
                    return;
                }
                if (axis == 0) {
                    points[index].setX(float(value));
                } else if (axis == 1) {
                    points[index].setY(float(value));
                }
                if (m_currentObject->pathType() == ExtrudeObject::PathType::Arc && index == 1) {
                    points[index].setY(0.0f);
                }
                if (!m_sceneController->setExtrudePathControlPoints(m_currentObject, points)) {
                    refreshEditors();
                }
            });

    auto *profileTitle = new QLabel(tr("Profile"), this);
    profileTitle->setStyleSheet("font-weight: 600; color: #475569; padding-top: 4px;");
    layout->addWidget(profileTitle);

    m_profileValidationLabel = new QLabel(tr("At least 3 profile vertices are required to render the extrude."), this);
    m_profileValidationLabel->setStyleSheet("color: #b45309; font-size: 12px;");
    layout->addWidget(m_profileValidationLabel);

    PointListEditorConfig profileConfig;
    profileConfig.dimension = 2;
    profileConfig.allowAdd = true;
    profileConfig.allowReorder = true;
    profileConfig.allowRemove = true;
    profileConfig.addButtonText = tr("Add Vertex");
    profileConfig.rowLabel = [](int index) { return QObject::tr("V%1").arg(index + 1); };
    m_profileVerticesEditor = PropertyEditorFactory::createPointListEditor(profileConfig, this);
    layout->addWidget(m_profileVerticesEditor);
    connect(m_profileVerticesEditor, &ReorderablePointListEditor::addRequested, this, [this] {
        if (m_currentObject && m_sceneController) {
            m_sceneController->addExtrudeRegularProfileVertex(m_currentObject);
        }
    });
    connect(m_profileVerticesEditor, &ReorderablePointListEditor::pointEdited, this,
            [this](int index, int axis, double value) {
                if (m_updating || !m_currentObject) {
                    return;
                }
                auto vertices = m_currentObject->profileVertices();
                if (index < 0 || index >= vertices.size()) {
                    return;
                }
                if (axis == 0) {
                    vertices[index].setX(float(value));
                } else if (axis == 1) {
                    vertices[index].setY(float(value));
                }
                if (!m_sceneController->setExtrudeProfileVertices(m_currentObject, vertices)) {
                    refreshEditors();
                }
            });
    connect(m_profileVerticesEditor, &ReorderablePointListEditor::moveRequested, this, [this](int from, int to) {
        if (m_currentObject && m_sceneController) {
            m_sceneController->moveExtrudeProfileVertex(m_currentObject, from, to);
        }
    });
    connect(m_profileVerticesEditor, &ReorderablePointListEditor::removeRequested, this, [this](int index) {
        if (m_currentObject && m_sceneController) {
            m_sceneController->removeExtrudeProfileVertex(m_currentObject, index);
        }
    });
}

void ExtrudeSection::setObjects(const QList<SceneObject *> &objects)
{
    Q_UNUSED(objects)
}

void ExtrudeSection::setCurrentObject(SceneObject *object)
{
    m_currentObject = qobject_cast<ExtrudeObject *>(object);
    refreshEditors();
}

void ExtrudeSection::refreshObject(SceneObject *object)
{
    if (object == m_currentObject) {
        refreshEditors();
    }
}

void ExtrudeSection::refreshEditors()
{
    m_updating = true;
    const bool visible = m_currentObject != nullptr;
    setVisible(visible);
    if (!visible) {
        m_pathPointsEditor->setPoints2D({});
        m_profileVerticesEditor->setPoints2D({});
        m_updating = false;
        return;
    }

    m_pathValidationLabel->setVisible(!m_currentObject->hasValidPath());
    m_profileValidationLabel->setVisible(!m_currentObject->hasValidProfile());
    m_pathTypeEditor->setCurrentValue(int(m_currentObject->pathType()));
    m_divisionsEditor->setValue(int(m_currentObject->pathDivisions()));
    m_rotationEditor->setValue(m_currentObject->ellipseRotationDegrees());
    m_startAngleEditor->setValue(m_currentObject->pathStartAngleDegrees());
    m_endAngleEditor->setValue(m_currentObject->pathEndAngleDegrees());

    const bool ellipseVisible = m_currentObject->pathType() == ExtrudeObject::PathType::Ellipse;
    const bool angleVisible = ellipseVisible || m_currentObject->pathType() == ExtrudeObject::PathType::Arc;
    m_rotationEditor->setVisible(ellipseVisible);
    m_startAngleEditor->setVisible(angleVisible);
    m_endAngleEditor->setVisible(angleVisible);

    m_pathPointsEditor->setPoints2D(m_currentObject->pathControlPoints());
    m_profileVerticesEditor->setPoints2D(m_currentObject->profileVertices());
    m_updating = false;
}
