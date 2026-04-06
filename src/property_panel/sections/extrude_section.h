#pragma once

#include <QPointer>

#include "property_panel/property_section.h"

class DoubleEditorRow;
class EnumEditor;
class ExtrudeObject;
class IntEditorRow;
class QLabel;
class ReorderablePointListEditor;

class ExtrudeSection final : public PropertySection
{
    Q_OBJECT

public:
    explicit ExtrudeSection(QWidget *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects) override;
    void setCurrentObject(SceneObject *object) override;
    void refreshObject(SceneObject *object) override;

private:
    void refreshEditors();

    QPointer<ExtrudeObject> m_currentObject;
    EnumEditor *m_pathTypeEditor{};
    IntEditorRow *m_divisionsEditor{};
    DoubleEditorRow *m_rotationEditor{};
    DoubleEditorRow *m_startAngleEditor{};
    DoubleEditorRow *m_endAngleEditor{};
    QLabel *m_pathValidationLabel{};
    QLabel *m_profileValidationLabel{};
    ReorderablePointListEditor *m_pathPointsEditor{};
    ReorderablePointListEditor *m_profileVerticesEditor{};
    bool m_updating = false;
};
