#pragma once

#include <QObject>
#include <QPointer>

#include "primitive_object.h"
#include "scene_model.h"

class SceneController final : public QObject {
    Q_OBJECT

public:
    explicit SceneController(QObject *parent = nullptr);

    PrimitiveObject *addPrimitive(PrimitiveObject::PrimitiveType type,
                                  PrimitiveObject *parentObject = nullptr);
    bool removeObject(PrimitiveObject *object);
    bool removeCurrentObject();
    bool setObjectParent(PrimitiveObject *object, PrimitiveObject *parentObject);

    [[nodiscard]] QList<PrimitiveObject *> objects() const;
    [[nodiscard]] PrimitiveObject *currentObject() const;
    [[nodiscard]] SceneModel *sceneModel();

public slots:
    void setCurrentObject(PrimitiveObject *object);

signals:
    void objectAdded(PrimitiveObject *object);
    void objectChanged(PrimitiveObject *object);
    void objectRemoved(PrimitiveObject *object);
    void objectHierarchyChanged(PrimitiveObject *object);
    void currentObjectChanged(PrimitiveObject *object);

private:
    void registerObject(PrimitiveObject *object);
    [[nodiscard]] QString nextName(PrimitiveObject::PrimitiveType type);
    [[nodiscard]] QVector3D nextSpawnPosition() const;

    QList<PrimitiveObject *> m_objects;
    QPointer<PrimitiveObject> m_currentObject;
    SceneModel m_sceneModel;
    int m_boxCount = 0;
    int m_sphereCount = 0;
    int m_cylinderCount = 0;
    int m_coneCount = 0;
};
