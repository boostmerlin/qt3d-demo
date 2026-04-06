#pragma once

#include <QObject>
#include <QPointer>

#include "extrude_object.h"
#include "polygon_object.h"
#include "primitive_object.h"
#include "scene_model.h"

class SceneController final : public QObject {
    Q_OBJECT

public:
    explicit SceneController(QObject *parent = nullptr);

    PrimitiveObject *addPrimitive(PrimitiveObject::PrimitiveType type,
                                  SceneObject *parentObject = nullptr);
    PolygonObject *addPolygon(SceneObject *parentObject = nullptr);
    ExtrudeObject *addExtrude(SceneObject *parentObject = nullptr);
    bool removeObject(SceneObject *object);
    bool removeCurrentObject();
    bool setObjectParent(SceneObject *object, SceneObject *parentObject);

    [[nodiscard]] QList<SceneObject *> objects() const;
    [[nodiscard]] SceneObject *currentObject() const;
    [[nodiscard]] SceneModel *sceneModel();

public slots:
    void setCurrentObject(SceneObject *object);

signals:
    void objectAdded(SceneObject *object);
    void objectChanged(SceneObject *object);
    void objectRemoved(SceneObject *object);
    void objectHierarchyChanged(SceneObject *object);
    void currentObjectChanged(SceneObject *object);

private:
    void registerObject(SceneObject *object);
    [[nodiscard]] QString nextName(PrimitiveObject::PrimitiveType type);
    [[nodiscard]] QString nextPolygonName();
    [[nodiscard]] QString nextExtrudeName();
    [[nodiscard]] QVector3D nextSpawnPosition() const;

    QList<SceneObject *> m_objects;
    QPointer<SceneObject> m_currentObject;
    SceneModel m_sceneModel;
    int m_boxCount = 0;
    int m_sphereCount = 0;
    int m_cylinderCount = 0;
    int m_coneCount = 0;
    int m_lineCount = 0;
    int m_ringCount = 0;
    int m_polygonCount = 0;
    int m_extrudeCount = 0;
};
