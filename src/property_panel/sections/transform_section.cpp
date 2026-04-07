#include "property_panel/sections/transform_section.h"

#include <QLabel>
#include <QVBoxLayout>

#include "property_panel/editors/vector3_editor.h"
#include "property_panel/property_editor_factory.h"
#include "scene/scene_controller.h"

TransformSection::TransformSection(SceneController *sceneController, QWidget *parent)
    : PropertySection(sceneController, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto *title = new QLabel(tr("Transform"), this);
    title->setStyleSheet("font-weight: 600; color: #334155; padding-top: 4px;");
    layout->addWidget(title);

    m_positionEditor = PropertyEditorFactory::createVector3Editor(tr("Position"), this);
    layout->addWidget(m_positionEditor);
    connect(m_positionEditor, &Vector3Editor::valueEdited, this, [this](const QVector3D &value) {
        if (!m_updating && m_currentObject && m_sceneController) {
            m_sceneController->setObjectPosition(m_currentObject, value);
        }
    });

    m_rotationEditor = PropertyEditorFactory::createVector3Editor(tr("Rotation"), this);
    layout->addWidget(m_rotationEditor);
    connect(m_rotationEditor, &Vector3Editor::valueEdited, this, [this](const QVector3D &value) {
        if (!m_updating && m_currentObject && m_sceneController) {
            m_sceneController->setObjectRotation(m_currentObject, value);
        }
    });
}

void TransformSection::setObjects(const QList<SceneObject *> &objects)
{
    Q_UNUSED(objects)
}

void TransformSection::setCurrentObject(SceneObject *object)
{
    m_currentObject = object;
    refreshEditors();
}

void TransformSection::refreshObject(SceneObject *object)
{
    if (object == m_currentObject) {
        refreshEditors();
    }
}

void TransformSection::refreshEditors()
{
    m_updating = true;
    const bool hasObject = m_currentObject != nullptr;
    setVisible(hasObject);
    if (hasObject) {
        m_positionEditor->setValue(m_currentObject->position());
        m_rotationEditor->setValue(m_currentObject->rotation());
    } else {
        m_positionEditor->setValue({});
        m_rotationEditor->setValue({});
    }
    m_updating = false;
}
