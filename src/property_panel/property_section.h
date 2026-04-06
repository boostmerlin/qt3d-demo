#pragma once

#include <QWidget>

#include "scene/scene_object.h"

class PropertySection : public QWidget
{
    Q_OBJECT

public:
    explicit PropertySection(QWidget *parent = nullptr)
        : QWidget(parent)
    {
    }

    ~PropertySection() override = default;

    virtual void setObjects(const QList<SceneObject *> &objects) = 0;
    virtual void setCurrentObject(SceneObject *object) = 0;
    virtual void refreshObject(SceneObject *object) = 0;
};
