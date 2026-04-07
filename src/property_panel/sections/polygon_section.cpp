#include "property_panel/sections/polygon_section.h"

#include <QLabel>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

#include "scene/polygon_object.h"
#include "property_panel/editors/double_editor_row.h"
#include "property_panel/editors/int_editor_row.h"
#include "property_panel/editors/reorderable_point_list_editor.h"
#include "property_panel/property_editor_factory.h"
#include "scene/scene_controller.h"

PolygonSection::PolygonSection(SceneController *sceneController, QWidget *parent)
    : PropertySection(sceneController, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto *title = new QLabel(tr("Polygon"), this);
    title->setStyleSheet("font-weight: 600; color: #334155; padding-top: 4px;");
    layout->addWidget(title);

    m_validationLabel = new QLabel(tr("At least 3 vertices are required to render the polygon."), this);
    m_validationLabel->setStyleSheet("color: #b45309; font-size: 12px;");
    layout->addWidget(m_validationLabel);

    PointListEditorConfig config;
    config.addButtonText = tr("Add Vertex");
    config.dimension = 3;
    config.allowAdd = true;
    config.allowReorder = true;
    config.allowRemove = true;
    config.rowLabel = [](int index) { return QObject::tr("V%1").arg(index + 1); };
    m_verticesEditor = PropertyEditorFactory::createPointListEditor(config, this);
    layout->addWidget(m_verticesEditor);
    connect(m_verticesEditor, &ReorderablePointListEditor::addRequested, this, [this] {
        if (m_currentObject && m_sceneController) {
            m_sceneController->addPolygonRegularVertex(m_currentObject);
        }
    });
    connect(m_verticesEditor, &ReorderablePointListEditor::pointEdited, this,
            [this](int index, int axis, double value) {
                if (m_updating || !m_currentObject) {
                    return;
                }
                auto vertices = m_currentObject->vertices();
                if (index < 0 || index >= vertices.size()) {
                    return;
                }
                if (axis == 0) {
                    vertices[index].setX(float(value));
                } else if (axis == 1) {
                    vertices[index].setY(float(value));
                } else if (axis == 2) {
                    vertices[index].setZ(float(value));
                }
                m_sceneController->setPolygonVertices(m_currentObject, vertices);
            });
    connect(m_verticesEditor, &ReorderablePointListEditor::moveRequested, this, [this](int from, int to) {
        if (m_currentObject && m_sceneController) {
            m_sceneController->movePolygonVertex(m_currentObject, from, to);
        }
    });
    connect(m_verticesEditor, &ReorderablePointListEditor::removeRequested, this, [this](int index) {
        if (m_currentObject && m_sceneController) {
            m_sceneController->removePolygonVertex(m_currentObject, index);
        }
    });

    m_heightEditor = PropertyEditorFactory::createDoubleEditor(tr("Height"), this);
    m_heightEditor->setRange(0.0, 9999.0);
    layout->addWidget(m_heightEditor);
    connect(m_heightEditor, &DoubleEditorRow::valueEdited, this, [this](double value) {
        if (m_updating || !m_currentObject || !m_sceneController
            || !m_sceneController->setPolygonHeight(m_currentObject, float(value))) {
            refreshEditors();
        }
    });

    m_ringsEditor = PropertyEditorFactory::createIntEditor(tr("Rings"), this);
    m_ringsEditor->setRange(0, 64);
    layout->addWidget(m_ringsEditor);
    connect(m_ringsEditor, &IntEditorRow::valueEdited, this, [this](int value) {
        if (m_updating || !m_currentObject || !m_sceneController) {
            return;
        }
        if (value == 1) {
            const QSignalBlocker blocker(m_ringsEditor->spinBox());
            m_ringsEditor->setValue(2);
            value = 2;
        }
        if (!m_sceneController->setPolygonRings(m_currentObject, uint(value))) {
            refreshEditors();
        }
    });
}

void PolygonSection::setObjects(const QList<SceneObject *> &objects)
{
    Q_UNUSED(objects)
}

void PolygonSection::setCurrentObject(SceneObject *object)
{
    m_currentObject = qobject_cast<PolygonObject *>(object);
    refreshEditors();
}

void PolygonSection::refreshObject(SceneObject *object)
{
    if (object == m_currentObject) {
        refreshEditors();
    }
}

void PolygonSection::refreshEditors()
{
    m_updating = true;
    const bool visible = m_currentObject != nullptr;
    setVisible(visible);
    if (!visible) {
        m_verticesEditor->setPoints3D({});
        m_updating = false;
        return;
    }

    m_validationLabel->setVisible(!m_currentObject->hasValidPolygon());
    m_heightEditor->setValue(m_currentObject->height());
    m_ringsEditor->setValue(int(m_currentObject->rings()));
    m_verticesEditor->setPoints3D(m_currentObject->vertices());
    m_updating = false;
}
