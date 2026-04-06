#pragma once

#include <QPointer>

#include "property_panel/property_section.h"

class DoubleEditorRow;
class IntEditorRow;
class QLabel;
class PolygonObject;
class ReorderablePointListEditor;

class PolygonSection final : public PropertySection
{
    Q_OBJECT

public:
    explicit PolygonSection(QWidget *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects) override;
    void setCurrentObject(SceneObject *object) override;
    void refreshObject(SceneObject *object) override;

private:
    void refreshEditors();

    QPointer<PolygonObject> m_currentObject;
    QLabel *m_validationLabel{};
    ReorderablePointListEditor *m_verticesEditor{};
    DoubleEditorRow *m_heightEditor{};
    IntEditorRow *m_ringsEditor{};
    bool m_updating = false;
};
