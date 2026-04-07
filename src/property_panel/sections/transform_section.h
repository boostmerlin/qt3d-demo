#pragma once

#include <QPointer>

#include "property_panel/property_section.h"

class Vector3Editor;

class TransformSection final : public PropertySection
{
    Q_OBJECT

public:
    explicit TransformSection(SceneController *sceneController = nullptr, QWidget *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects) override;
    void setCurrentObject(SceneObject *object) override;
    void refreshObject(SceneObject *object) override;

private:
    void refreshEditors();

    QPointer<SceneObject> m_currentObject;
    Vector3Editor *m_positionEditor{};
    Vector3Editor *m_rotationEditor{};
    bool m_updating = false;
};
