#include "property_panel/sections/primitive_dimensions_section.h"

#include <QLabel>
#include <QVBoxLayout>

#include "scene/primitive_object.h"
#include "property_panel/editors/double_editor_row.h"
#include "property_panel/property_editor_factory.h"
#include "scene/scene_controller.h"

PrimitiveDimensionsSection::PrimitiveDimensionsSection(SceneController *sceneController, QWidget *parent)
    : PropertySection(sceneController, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto *title = new QLabel(tr("Dimensions"), this);
    title->setStyleSheet("font-weight: 600; color: #334155; padding-top: 4px;");
    layout->addWidget(title);

    for (int index = 0; index < MaxDimensionEditors; ++index) {
        auto *editor = PropertyEditorFactory::createDoubleEditor(QString(), this);
        editor->setVisible(false);
        layout->addWidget(editor);
        m_dimensionEditors[index] = editor;
        connect(editor, &DoubleEditorRow::valueEdited, this, [this, index](double value) {
            if (!m_updating && m_currentObject && m_sceneController) {
                m_sceneController->setPrimitiveDimension(m_currentObject, index, float(value));
            }
        });
    }
}

void PrimitiveDimensionsSection::setObjects(const QList<SceneObject *> &objects)
{
    Q_UNUSED(objects)
}

void PrimitiveDimensionsSection::setCurrentObject(SceneObject *object)
{
    m_currentObject = qobject_cast<PrimitiveObject *>(object);
    refreshEditors();
}

void PrimitiveDimensionsSection::refreshObject(SceneObject *object)
{
    if (object == m_currentObject) {
        refreshEditors();
    }
}

void PrimitiveDimensionsSection::refreshEditors()
{
    m_updating = true;
    const int visibleCount = m_currentObject ? m_currentObject->editableDimensionCount() : 0;
    setVisible(visibleCount > 0);

    for (int index = 0; index < MaxDimensionEditors; ++index) {
        const bool visible = m_currentObject && index < visibleCount;
        auto *editor = m_dimensionEditors[index];
        editor->setVisible(visible);
        if (!visible) {
            editor->setValue(PrimitiveObject::minimumDimensionValue());
            continue;
        }

        editor->setLabelText(m_currentObject->dimensionLabel(index));
        editor->setRange(m_currentObject->dimensionMinimum(index), m_currentObject->dimensionMaximum(index));
        editor->setValue(m_currentObject->dimensionValue(index));
    }
    m_updating = false;
}
