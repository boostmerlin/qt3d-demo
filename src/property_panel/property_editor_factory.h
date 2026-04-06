#pragma once

#include "property_panel/editors/enum_editor.h"
#include "property_panel/editors/reorderable_point_list_editor.h"

class QWidget;
class ColorEditor;
class DoubleEditorRow;
class IntEditorRow;
class TextEditorRow;
class Vector3Editor;

class PropertyEditorFactory
{
public:
    static TextEditorRow *createTextEditor(const QString &label, QWidget *parent = nullptr);
    static DoubleEditorRow *createDoubleEditor(const QString &label, QWidget *parent = nullptr);
    static IntEditorRow *createIntEditor(const QString &label, QWidget *parent = nullptr);
    static EnumEditor *createEnumEditor(const QString &label, QWidget *parent = nullptr);
    static Vector3Editor *createVector3Editor(const QString &label, QWidget *parent = nullptr);
    static ColorEditor *createColorEditor(const QString &label, QWidget *parent = nullptr);
    static ReorderablePointListEditor *createPointListEditor(const PointListEditorConfig &config,
                                                             QWidget *parent = nullptr);
};
