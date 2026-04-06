#include "property_panel/property_editor_factory.h"

#include "property_panel/editors/color_editor.h"
#include "property_panel/editors/double_editor_row.h"
#include "property_panel/editors/int_editor_row.h"
#include "property_panel/editors/text_editor_row.h"
#include "property_panel/editors/vector3_editor.h"

TextEditorRow *PropertyEditorFactory::createTextEditor(const QString &label, QWidget *parent)
{
    return new TextEditorRow(label, parent);
}

DoubleEditorRow *PropertyEditorFactory::createDoubleEditor(const QString &label, QWidget *parent)
{
    return new DoubleEditorRow(label, parent);
}

IntEditorRow *PropertyEditorFactory::createIntEditor(const QString &label, QWidget *parent)
{
    return new IntEditorRow(label, parent);
}

EnumEditor *PropertyEditorFactory::createEnumEditor(const QString &label, QWidget *parent)
{
    return new EnumEditor(label, parent);
}

Vector3Editor *PropertyEditorFactory::createVector3Editor(const QString &label, QWidget *parent)
{
    return new Vector3Editor(label, parent);
}

ColorEditor *PropertyEditorFactory::createColorEditor(const QString &label, QWidget *parent)
{
    return new ColorEditor(label, parent);
}

ReorderablePointListEditor *PropertyEditorFactory::createPointListEditor(const PointListEditorConfig &config,
                                                                         QWidget *parent)
{
    return new ReorderablePointListEditor(config, parent);
}
