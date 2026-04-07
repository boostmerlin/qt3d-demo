#include "scene_controller.h"

#include <QQuaternion>

#include "extrude_render_node.h"
#include "polygon_render_node.h"
#include "primitive_render_node.h"
#include "scene_commands.h"

namespace {
QQuaternion identityRotation()
{
    return QQuaternion::fromEulerAngles(0.0f, 0.0f, 0.0f);
}

QQuaternion localRotation(const SceneObject *object)
{
    return object ? QQuaternion::fromEulerAngles(object->rotation()) : identityRotation();
}

QQuaternion worldRotation(const SceneObject *object)
{
    if (!object) {
        return identityRotation();
    }

    if (const auto *parent = object->parentSceneObject()) {
        return worldRotation(parent) * localRotation(object);
    }
    return localRotation(object);
}

QVector3D worldPosition(const SceneObject *object)
{
    if (!object) {
        return {};
    }

    if (const auto *parent = object->parentSceneObject()) {
        return worldPosition(parent) + worldRotation(parent).rotatedVector(object->position());
    }
    return object->position();
}
}

SceneController::SceneController(QObject *parent)
    : QObject(parent)
{
    auto &factory = m_sceneModel.renderNodeFactory();
    factory.registerCreator(BoxObject::staticMetaObject.className(), &PrimitiveRenderNode::create);
    factory.registerCreator(SphereObject::staticMetaObject.className(), &PrimitiveRenderNode::create);
    factory.registerCreator(CylinderObject::staticMetaObject.className(), &PrimitiveRenderNode::create);
    factory.registerCreator(ConeObject::staticMetaObject.className(), &PrimitiveRenderNode::create);
    factory.registerCreator(LineObject::staticMetaObject.className(), &PrimitiveRenderNode::create);
    factory.registerCreator(RingObject::staticMetaObject.className(), &PrimitiveRenderNode::create);
    factory.registerCreator(PolygonObject::staticMetaObject.className(), &PolygonRenderNode::create);
    factory.registerCreator(ExtrudeObject::staticMetaObject.className(), &ExtrudeRenderNode::create);
}

PrimitiveObject *SceneController::addPrimitive(PrimitiveObject::PrimitiveType type,
                                               SceneObject *parentObject)
{
    const SceneObjectSnapshot snapshot = makeSnapshotForNewPrimitive(type);
    m_undoStack.push(new AddObjectCommand(this,
                                          snapshot,
                                          parentObject ? parentObject->id() : QUuid(),
                                          tr("Add %1").arg(PrimitiveObject::displayName(type))));
    return qobject_cast<PrimitiveObject *>(findObject(snapshot.id));
}

PolygonObject *SceneController::addPolygon(SceneObject *parentObject)
{
    const SceneObjectSnapshot snapshot = makeSnapshotForNewPolygon();
    m_undoStack.push(new AddObjectCommand(this,
                                          snapshot,
                                          parentObject ? parentObject->id() : QUuid(),
                                          tr("Add Polygon")));
    return qobject_cast<PolygonObject *>(findObject(snapshot.id));
}

ExtrudeObject *SceneController::addExtrude(SceneObject *parentObject)
{
    const SceneObjectSnapshot snapshot = makeSnapshotForNewExtrude();
    m_undoStack.push(new AddObjectCommand(this,
                                          snapshot,
                                          parentObject ? parentObject->id() : QUuid(),
                                          tr("Add Extrude")));
    return qobject_cast<ExtrudeObject *>(findObject(snapshot.id));
}

bool SceneController::removeObject(SceneObject *object)
{
    if (!object || !m_objects.contains(object)) {
        return false;
    }

    m_undoStack.push(new RemoveObjectCommand(this,
                                             captureSceneObjectSnapshot(object, true),
                                             m_currentObject == object,
                                             tr("Delete Object")));
    return true;
}

bool SceneController::removeCurrentObject()
{
    return removeObject(m_currentObject);
}

bool SceneController::setObjectParent(SceneObject *object, SceneObject *parentObject)
{
    if (!object || !m_objects.contains(object) || object == parentObject) {
        return false;
    }
    if (parentObject && (!m_objects.contains(parentObject) || object->isAncestorOf(parentObject))) {
        return false;
    }

    const QUuid oldParentId = object->parentSceneObject() ? object->parentSceneObject()->id() : QUuid();
    const QUuid newParentId = parentObject ? parentObject->id() : QUuid();
    if (oldParentId == newParentId) {
        return true;
    }

    m_undoStack.push(new SetParentCommand(this, object->id(), oldParentId, newParentId, tr("Reparent Object")));
    return true;
}

bool SceneController::setObjectName(SceneObject *object, const QString &name)
{
    return pushObjectStateCommand(object, tr("Rename Object"), [name](SceneObject *target) {
        target->setName(name);
    });
}

bool SceneController::setObjectColor(SceneObject *object, const QColor &color)
{
    return pushObjectStateCommand(object, tr("Change Color"), [color](SceneObject *target) {
        target->setColor(color);
    });
}

bool SceneController::setObjectPosition(SceneObject *object, const QVector3D &position)
{
    return pushObjectStateCommand(object, tr("Move Object"), [position](SceneObject *target) {
        target->setPosition(position);
    });
}

bool SceneController::setObjectRotation(SceneObject *object, const QVector3D &rotation)
{
    return pushObjectStateCommand(object, tr("Rotate Object"), [rotation](SceneObject *target) {
        target->setRotation(rotation);
    });
}

bool SceneController::setPrimitiveDimension(PrimitiveObject *object, int index, float value)
{
    return pushObjectStateCommand(object, tr("Change Dimensions"), [index, value](SceneObject *target) {
        auto *primitive = qobject_cast<PrimitiveObject *>(target);
        Q_ASSERT(primitive);
        primitive->setDimensionValue(index, value);
    });
}

bool SceneController::setLineType(LineObject *object, LineGeometry::LineType lineType)
{
    return pushObjectStateCommand(object, tr("Change Line Type"), [lineType](SceneObject *target) {
        auto *line = qobject_cast<LineObject *>(target);
        Q_ASSERT(line);
        line->setLineType(lineType);
    });
}

bool SceneController::setPolygonVertices(PolygonObject *object, const PolygonVertices &vertices)
{
    return pushObjectStateCommand(object, tr("Edit Polygon Vertices"), [vertices](SceneObject *target) {
        auto *polygon = qobject_cast<PolygonObject *>(target);
        Q_ASSERT(polygon);
        polygon->setVertices(vertices);
    });
}

bool SceneController::setPolygonHeight(PolygonObject *object, float height)
{
    return pushObjectStateCommand(object, tr("Change Polygon Height"), [height](SceneObject *target) {
        auto *polygon = qobject_cast<PolygonObject *>(target);
        Q_ASSERT(polygon);
        polygon->setHeight(height);
    });
}

bool SceneController::setPolygonRings(PolygonObject *object, uint rings)
{
    return pushObjectStateCommand(object, tr("Change Polygon Rings"), [rings](SceneObject *target) {
        auto *polygon = qobject_cast<PolygonObject *>(target);
        Q_ASSERT(polygon);
        polygon->setRings(rings);
    });
}

bool SceneController::addPolygonRegularVertex(PolygonObject *object)
{
    return pushObjectStateCommand(object, tr("Add Polygon Vertex"), [](SceneObject *target) {
        auto *polygon = qobject_cast<PolygonObject *>(target);
        Q_ASSERT(polygon);
        polygon->addRegularVertex();
    });
}

bool SceneController::movePolygonVertex(PolygonObject *object, int from, int to)
{
    return pushObjectStateCommand(object, tr("Move Polygon Vertex"), [from, to](SceneObject *target) {
        auto *polygon = qobject_cast<PolygonObject *>(target);
        Q_ASSERT(polygon);
        polygon->moveVertex(from, to);
    });
}

bool SceneController::removePolygonVertex(PolygonObject *object, int index)
{
    return pushObjectStateCommand(object, tr("Remove Polygon Vertex"), [index](SceneObject *target) {
        auto *polygon = qobject_cast<PolygonObject *>(target);
        Q_ASSERT(polygon);
        polygon->removeVertexAt(index);
    });
}

bool SceneController::setExtrudePathType(ExtrudeObject *object, ExtrudeObject::PathType pathType)
{
    return pushObjectStateCommand(object, tr("Change Extrude Path Type"), [pathType](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->setPathType(pathType);
    });
}

bool SceneController::setExtrudePathControlPoints(ExtrudeObject *object, const ExtrudePathControlPoints &points)
{
    return pushObjectStateCommand(object, tr("Edit Extrude Path"), [points](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->setPathControlPoints(points);
    });
}

bool SceneController::setExtrudeProfileVertices(ExtrudeObject *object, const ExtrudeProfileVertices &vertices)
{
    return pushObjectStateCommand(object, tr("Edit Extrude Profile"), [vertices](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->setProfileVertices(vertices);
    });
}

bool SceneController::setExtrudePathDivisions(ExtrudeObject *object, uint divisions)
{
    return pushObjectStateCommand(object, tr("Change Extrude Divisions"), [divisions](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->setPathDivisions(divisions);
    });
}

bool SceneController::setExtrudeEllipseRotation(ExtrudeObject *object, float rotationDegrees)
{
    return pushObjectStateCommand(object, tr("Change Extrude Rotation"), [rotationDegrees](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->setEllipseRotationDegrees(rotationDegrees);
    });
}

bool SceneController::setExtrudePathStartAngle(ExtrudeObject *object, float degrees)
{
    return pushObjectStateCommand(object, tr("Change Extrude Start Angle"), [degrees](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->setPathStartAngleDegrees(degrees);
    });
}

bool SceneController::setExtrudePathEndAngle(ExtrudeObject *object, float degrees)
{
    return pushObjectStateCommand(object, tr("Change Extrude End Angle"), [degrees](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->setPathEndAngleDegrees(degrees);
    });
}

bool SceneController::addExtrudeRegularProfileVertex(ExtrudeObject *object)
{
    return pushObjectStateCommand(object, tr("Add Extrude Profile Vertex"), [](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->addRegularProfileVertex();
    });
}

bool SceneController::moveExtrudeProfileVertex(ExtrudeObject *object, int from, int to)
{
    return pushObjectStateCommand(object, tr("Move Extrude Profile Vertex"), [from, to](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->moveProfileVertex(from, to);
    });
}

bool SceneController::removeExtrudeProfileVertex(ExtrudeObject *object, int index)
{
    return pushObjectStateCommand(object, tr("Remove Extrude Profile Vertex"), [index](SceneObject *target) {
        auto *extrude = qobject_cast<ExtrudeObject *>(target);
        Q_ASSERT(extrude);
        extrude->removeProfileVertexAt(index);
    });
}

QList<SceneObject *> SceneController::objects() const
{
    return m_objects;
}

SceneObject *SceneController::currentObject() const
{
    return m_currentObject;
}

SceneModel *SceneController::sceneModel()
{
    return &m_sceneModel;
}

QUndoStack *SceneController::undoStack()
{
    return &m_undoStack;
}

SceneObject *SceneController::applyAddObject(const SceneObjectSnapshot &snapshot, const QUuid &parentId)
{
    return restoreObjectTree(snapshot, findObject(parentId));
}

bool SceneController::applyRemoveObject(const QUuid &objectId)
{
    auto *object = findObject(objectId);
    if (!object) {
        return false;
    }

    if (m_currentObject && (m_currentObject == object || object->isAncestorOf(m_currentObject))) {
        setCurrentObject(nullptr);
    }

    m_sceneModel.remove(object);
    delete object;
    return true;
}

bool SceneController::applyObjectState(const SceneObjectSnapshot &snapshot)
{
    auto *object = findObject(snapshot.id);
    if (!applySceneObjectState(object, snapshot, true)) {
        return false;
    }

    m_sceneModel.createOrUpdate(object);
    emit objectChanged(object);
    return true;
}

bool SceneController::applyObjectParent(const QUuid &objectId, const QUuid &parentId)
{
    auto *object = findObject(objectId);
    auto *parentObject = findObject(parentId);
    if (!object || object == parentObject) {
        return false;
    }
    if (parentObject && object->isAncestorOf(parentObject)) {
        return false;
    }

    QObject *nextParent = parentObject ? static_cast<QObject *>(parentObject)
                                       : static_cast<QObject *>(this);
    if (object->parent() == nextParent) {
        return true;
    }

    const QVector3D objectWorldPosition = worldPosition(object);
    const QQuaternion objectWorldRotation = worldRotation(object);
    const QVector3D parentWorldPosition = worldPosition(parentObject);
    const QQuaternion parentWorldRotation = worldRotation(parentObject);

    const QQuaternion localRotationAfterReparent = parentObject
                                                       ? parentWorldRotation.conjugated() * objectWorldRotation
                                                       : objectWorldRotation;
    const QVector3D localPositionAfterReparent = parentObject
                                                     ? parentWorldRotation.conjugated().rotatedVector(objectWorldPosition
                                                                                                      - parentWorldPosition)
                                                     : objectWorldPosition;

    auto shotGuard = object->disableShotGuarded();
    object->setParent(nextParent);
    object->setPosition(localPositionAfterReparent);
    object->setRotation(localRotationAfterReparent.toEulerAngles());

    m_sceneModel.createOrUpdate(object);
    emit objectHierarchyChanged(object);
    return true;
}

SceneObject *SceneController::findObject(const QUuid &id) const
{
    if (id.isNull()) {
        return nullptr;
    }

    for (auto *object : m_objects) {
        if (object && object->id() == id) {
            return object;
        }
    }
    return nullptr;
}

void SceneController::setCurrentObject(SceneObject *object)
{
    if (m_currentObject == object) {
        return;
    }

    if (m_currentObject) {
        m_sceneModel.deselectNodes({m_currentObject});
    }

    m_currentObject = object;

    if (m_currentObject) {
        m_sceneModel.selectNodes({m_currentObject});
    }

    emit currentObjectChanged(m_currentObject);
}

bool SceneController::pushObjectStateCommand(SceneObject *object,
                                             const QString &commandText,
                                             const std::function<void(SceneObject *)> &mutator)
{
    if (!object || !m_objects.contains(object)) {
        return false;
    }

    const SceneObjectSnapshot before = captureSceneObjectSnapshot(object, false);
    SceneObjectSnapshot after;
    {
        auto shotGuard = object->disableShotGuarded();
        mutator(object);
        after = captureSceneObjectSnapshot(object, false);
        applySceneObjectState(object, before, false);
    }

    if (after == before) {
        return false;
    }

    m_undoStack.push(new SceneObjectStateCommand(this, before, after, commandText));
    return true;
}

SceneObject *SceneController::restoreObjectTree(const SceneObjectSnapshot &snapshot, SceneObject *parentObject)
{
    QObject *owner = parentObject ? static_cast<QObject *>(parentObject)
                                  : static_cast<QObject *>(this);

    SceneObject *object = nullptr;
    switch (snapshot.kind) {
    case SceneObjectKind::Box:
        object = new BoxObject(owner);
        break;
    case SceneObjectKind::Sphere:
        object = new SphereObject(owner);
        break;
    case SceneObjectKind::Cylinder:
        object = new CylinderObject(owner);
        break;
    case SceneObjectKind::Cone:
        object = new ConeObject(owner);
        break;
    case SceneObjectKind::Line:
        object = new LineObject(owner);
        break;
    case SceneObjectKind::Ring:
        object = new RingObject(owner);
        break;
    case SceneObjectKind::Polygon:
        object = new PolygonObject(owner);
        break;
    case SceneObjectKind::Extrude:
        object = new ExtrudeObject(owner);
        break;
    }

    Q_ASSERT(object);
    object->restoreId(snapshot.id);
    applySceneObjectState(object, snapshot, false);

    registerObject(object);
    m_objects.append(object);
    m_sceneModel.createOrUpdate(object);
    emit objectAdded(object);

    for (const auto &childSnapshot : snapshot.children) {
        restoreObjectTree(childSnapshot, object);
    }

    return object;
}

SceneObjectSnapshot SceneController::makeSnapshotForNewPrimitive(PrimitiveObject::PrimitiveType type)
{
    PrimitiveObject *object = nullptr;
    switch (type) {
    case PrimitiveObject::PrimitiveType::Box:
        object = new BoxObject();
        break;
    case PrimitiveObject::PrimitiveType::Sphere:
        object = new SphereObject();
        break;
    case PrimitiveObject::PrimitiveType::Cylinder:
        object = new CylinderObject();
        break;
    case PrimitiveObject::PrimitiveType::Cone:
        object = new ConeObject();
        break;
    case PrimitiveObject::PrimitiveType::Line:
        object = new LineObject();
        break;
    case PrimitiveObject::PrimitiveType::Ring:
        object = new RingObject();
        break;
    }

    Q_ASSERT(object);
    object->setName(nextName(type));
    object->setPosition(nextSpawnPosition());
    const SceneObjectSnapshot snapshot = captureSceneObjectSnapshot(object, false);
    delete object;
    return snapshot;
}

SceneObjectSnapshot SceneController::makeSnapshotForNewPolygon()
{
    auto *object = new PolygonObject();
    object->setName(nextPolygonName());
    object->setPosition(nextSpawnPosition());
    const SceneObjectSnapshot snapshot = captureSceneObjectSnapshot(object, false);
    delete object;
    return snapshot;
}

SceneObjectSnapshot SceneController::makeSnapshotForNewExtrude()
{
    auto *object = new ExtrudeObject();
    object->setName(nextExtrudeName());
    object->setPosition(nextSpawnPosition());
    const SceneObjectSnapshot snapshot = captureSceneObjectSnapshot(object, false);
    delete object;
    return snapshot;
}

void SceneController::registerObject(SceneObject *object)
{
    connect(object, &QObservableObject::propertyChanged, this, [this, object](const ObservableChangeNotify &) {
        emit objectChanged(object);
    });

    connect(object, &QObject::destroyed, this, [this, object] {
        m_objects.removeAll(object);
        if (m_currentObject == object) {
            setCurrentObject(nullptr);
        }
        emit objectRemoved(object);
    });
}

QString SceneController::nextName(PrimitiveObject::PrimitiveType type)
{
    int *counter = nullptr;
    switch (type) {
    case PrimitiveObject::PrimitiveType::Box:
        counter = &m_boxCount;
        break;
    case PrimitiveObject::PrimitiveType::Sphere:
        counter = &m_sphereCount;
        break;
    case PrimitiveObject::PrimitiveType::Cylinder:
        counter = &m_cylinderCount;
        break;
    case PrimitiveObject::PrimitiveType::Cone:
        counter = &m_coneCount;
        break;
    case PrimitiveObject::PrimitiveType::Line:
        counter = &m_lineCount;
        break;
    case PrimitiveObject::PrimitiveType::Ring:
        counter = &m_ringCount;
        break;
    }

    Q_ASSERT(counter);
    ++(*counter);
    return QStringLiteral("%1 %2").arg(PrimitiveObject::displayName(type)).arg(*counter);
}

QString SceneController::nextPolygonName()
{
    ++m_polygonCount;
    return QStringLiteral("Polygon %1").arg(m_polygonCount);
}

QString SceneController::nextExtrudeName()
{
    ++m_extrudeCount;
    return QStringLiteral("Extrude %1").arg(m_extrudeCount);
}

QVector3D SceneController::nextSpawnPosition() const
{
    const int index = m_objects.size();
    constexpr float spacing = 2.5f;
    const float x = static_cast<float>(index % 4) * spacing;
    const float y = static_cast<float>(index / 4) * spacing;
    return {x, y, 0.0f};
}
