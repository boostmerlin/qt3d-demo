#pragma once

#include <QPointer>

#include "property_panel/property_section.h"

class EnumEditor;
class LineObject;

class LineSection final : public PropertySection
{
    Q_OBJECT

public:
    explicit LineSection(QWidget *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects) override;
    void setCurrentObject(SceneObject *object) override;
    void refreshObject(SceneObject *object) override;

private:
    void refreshEditors();

    QPointer<LineObject> m_currentObject;
    EnumEditor *m_lineTypeEditor{};
    bool m_updating = false;
};
