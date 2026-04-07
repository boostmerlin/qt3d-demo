#pragma once

#include <QWidget>

#include "scene/scene_object.h"

class QLabel;
class ObjectSection;
class PropertySection;
class SceneController;

class PropertyPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(SceneController *sceneController = nullptr, QWidget *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects);
    void setCurrentObject(SceneObject *object);
    void refreshObject(SceneObject *object);

signals:
    void parentChangeRequested(SceneObject *object, SceneObject *parentObject);
    void removeRequested(SceneObject *object);

private:
    QList<SceneObject *> m_objects;
    QPointer<SceneObject> m_currentObject;
    QWidget *m_propertyContentWidget{};
    QLabel *m_emptyStateLabel{};
    QList<PropertySection *> m_sections;
    ObjectSection *m_objectSection{};
};
