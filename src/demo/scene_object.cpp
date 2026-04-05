#include "scene_object.h"

SceneObject::SceneObject(QObject *parent)
    : QObservableObject(parent)
{
    setPosition(QVector3D(0.0f, 0.0f, 0.0f));
    setRotation(QVector3D(0.0f, 0.0f, 0.0f));
    setColor(QColorConstants::White);
}

SceneObject *SceneObject::parentSceneObject() const
{
    return qobject_cast<SceneObject *>(parent());
}

QList<SceneObject *> SceneObject::childSceneObjects() const
{
    QList<SceneObject *> result;
    const auto objectChildren = children();
    for (auto *child : objectChildren) {
        if (auto *sceneChild = qobject_cast<SceneObject *>(child)) {
            result.append(sceneChild);
        }
    }
    return result;
}

bool SceneObject::isAncestorOf(const SceneObject *object) const
{
    auto *cursor = object ? object->parentSceneObject() : nullptr;
    while (cursor) {
        if (cursor == this) {
            return true;
        }
        cursor = cursor->parentSceneObject();
    }
    return false;
}
