#pragma once

#include <functional>

#include <QObject>
#include <QPointer>
#include <QUndoStack>

#include "extrude_object.h"
#include "polygon_object.h"
#include "primitive_object.h"
#include "scene_snapshot.h"
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
    bool setObjectName(SceneObject *object, const QString &name);
    bool setObjectColor(SceneObject *object, const QColor &color);
    bool setObjectPosition(SceneObject *object, const QVector3D &position);
    bool setObjectRotation(SceneObject *object, const QVector3D &rotation);
    bool setPrimitiveDimension(PrimitiveObject *object, int index, float value);
    bool setLineType(LineObject *object, LineGeometry::LineType lineType);
    bool setPolygonVertices(PolygonObject *object, const PolygonVertices &vertices);
    bool setPolygonHeight(PolygonObject *object, float height);
    bool setPolygonRings(PolygonObject *object, uint rings);
    bool addPolygonRegularVertex(PolygonObject *object);
    bool movePolygonVertex(PolygonObject *object, int from, int to);
    bool removePolygonVertex(PolygonObject *object, int index);
    bool setExtrudePathType(ExtrudeObject *object, ExtrudeObject::PathType pathType);
    bool setExtrudePathControlPoints(ExtrudeObject *object, const ExtrudePathControlPoints &points);
    bool setExtrudeProfileVertices(ExtrudeObject *object, const ExtrudeProfileVertices &vertices);
    bool setExtrudePathDivisions(ExtrudeObject *object, uint divisions);
    bool setExtrudeEllipseRotation(ExtrudeObject *object, float rotationDegrees);
    bool setExtrudePathStartAngle(ExtrudeObject *object, float degrees);
    bool setExtrudePathEndAngle(ExtrudeObject *object, float degrees);
    bool addExtrudeRegularProfileVertex(ExtrudeObject *object);
    bool moveExtrudeProfileVertex(ExtrudeObject *object, int from, int to);
    bool removeExtrudeProfileVertex(ExtrudeObject *object, int index);

    [[nodiscard]] QList<SceneObject *> objects() const;
    [[nodiscard]] SceneObject *currentObject() const;
    [[nodiscard]] SceneModel *sceneModel();
    [[nodiscard]] QUndoStack *undoStack();

    SceneObject *applyAddObject(const SceneObjectSnapshot &snapshot, const QUuid &parentId);
    bool applyRemoveObject(const QUuid &objectId);
    bool applyObjectState(const SceneObjectSnapshot &snapshot);
    bool applyObjectParent(const QUuid &objectId, const QUuid &parentId);
    [[nodiscard]] SceneObject *findObject(const QUuid &id) const;

public slots:
    void setCurrentObject(SceneObject *object);

signals:
    void objectAdded(SceneObject *object);
    void objectChanged(SceneObject *object);
    void objectRemoved(SceneObject *object);
    void objectHierarchyChanged(SceneObject *object);
    void currentObjectChanged(SceneObject *object);

private:
    bool pushObjectStateCommand(SceneObject *object,
                                const QString &commandText,
                                const std::function<void(SceneObject *)> &mutator);
    SceneObject *restoreObjectTree(const SceneObjectSnapshot &snapshot, SceneObject *parentObject = nullptr);
    [[nodiscard]] SceneObjectSnapshot makeSnapshotForNewPrimitive(PrimitiveObject::PrimitiveType type);
    [[nodiscard]] SceneObjectSnapshot makeSnapshotForNewPolygon();
    [[nodiscard]] SceneObjectSnapshot makeSnapshotForNewExtrude();
    void registerObject(SceneObject *object);
    [[nodiscard]] QString nextName(PrimitiveObject::PrimitiveType type);
    [[nodiscard]] QString nextPolygonName();
    [[nodiscard]] QString nextExtrudeName();
    [[nodiscard]] QVector3D nextSpawnPosition() const;

    QList<SceneObject *> m_objects;
    QPointer<SceneObject> m_currentObject;
    SceneModel m_sceneModel;
    QUndoStack m_undoStack;
    int m_boxCount = 0;
    int m_sphereCount = 0;
    int m_cylinderCount = 0;
    int m_coneCount = 0;
    int m_lineCount = 0;
    int m_ringCount = 0;
    int m_polygonCount = 0;
    int m_extrudeCount = 0;
};
