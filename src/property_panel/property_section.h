#pragma once

#include <QWidget>

#include "scene/scene_object.h"

class SceneController;

class PropertySection : public QWidget
{
public:
    explicit PropertySection(SceneController *sceneController = nullptr, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_sceneController(sceneController)
    {
    }

    ~PropertySection() override = default;

    virtual void setObjects(const QList<SceneObject *> &objects) = 0;
    virtual void setCurrentObject(SceneObject *object) = 0;
    virtual void refreshObject(SceneObject *object) = 0;

protected:
    SceneController *m_sceneController{};
};
