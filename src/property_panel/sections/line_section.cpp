#include "property_panel/sections/line_section.h"

#include <QLabel>
#include <QVBoxLayout>

#include "scene/primitive_object.h"
#include "property_panel/editors/enum_editor.h"
#include "property_panel/property_editor_factory.h"

LineSection::LineSection(QWidget *parent)
    : PropertySection(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto *title = new QLabel(tr("Line"), this);
    title->setStyleSheet("font-weight: 600; color: #334155; padding-top: 4px;");
    layout->addWidget(title);

    m_lineTypeEditor = PropertyEditorFactory::createEnumEditor(tr("Line Type"), this);
    m_lineTypeEditor->setItems({
        {tr("Line"), int(LineGeometry::Line)},
        {tr("Line Segments"), int(LineGeometry::LineSegments)},
        {tr("Line Loop"), int(LineGeometry::LineLoop)},
    });
    layout->addWidget(m_lineTypeEditor);
    connect(m_lineTypeEditor, &EnumEditor::valueEdited, this, [this](const QVariant &value) {
        if (!m_updating && m_currentObject) {
            m_currentObject->setLineType(LineGeometry::LineType(value.toInt()));
        }
    });
}

void LineSection::setObjects(const QList<SceneObject *> &objects)
{
    Q_UNUSED(objects)
}

void LineSection::setCurrentObject(SceneObject *object)
{
    m_currentObject = qobject_cast<LineObject *>(object);
    refreshEditors();
}

void LineSection::refreshObject(SceneObject *object)
{
    if (object == m_currentObject) {
        refreshEditors();
    }
}

void LineSection::refreshEditors()
{
    m_updating = true;
    setVisible(m_currentObject != nullptr);
    if (m_currentObject) {
        m_lineTypeEditor->setCurrentValue(int(m_currentObject->lineType()));
    }
    m_updating = false;
}
