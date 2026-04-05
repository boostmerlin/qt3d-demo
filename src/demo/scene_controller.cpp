#include "scene_controller.h"

#include <QQuaternion>

#include "primitive_render_node.h"

namespace {
QQuaternion identityRotation()
{
    return QQuaternion::fromEulerAngles(0.0f, 0.0f, 0.0f);
}

QQuaternion localRotation(const PrimitiveObject *object)
{
    return object ? QQuaternion::fromEulerAngles(object->rotation()) : identityRotation();
}

QQuaternion worldRotation(const PrimitiveObject *object)
{
    if (!object) {
        return identityRotation();
    }

    if (const auto *parent = object->parentPrimitive()) {
        return worldRotation(parent) * localRotation(object);
    }
    return localRotation(object);
}

QVector3D worldPosition(const PrimitiveObject *object)
{
    if (!object) {
        return {};
    }

    if (const auto *parent = object->parentPrimitive()) {
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
}

PrimitiveObject *SceneController::addPrimitive(PrimitiveObject::PrimitiveType type,
                                               PrimitiveObject *parentObject)
{
    PrimitiveObject *object = nullptr;
    QObject *owner = parentObject ? static_cast<QObject *>(parentObject)
                                  : static_cast<QObject *>(this);
    switch (type) {
    case PrimitiveObject::PrimitiveType::Box:
        object = new BoxObject(owner);
        break;
    case PrimitiveObject::PrimitiveType::Sphere:
        object = new SphereObject(owner);
        break;
    case PrimitiveObject::PrimitiveType::Cylinder:
        object = new CylinderObject(owner);
        break;
    case PrimitiveObject::PrimitiveType::Cone:
        object = new ConeObject(owner);
        break;
    }
    Q_ASSERT(object);
    object->setName(nextName(type));
    object->setPosition(nextSpawnPosition());

    registerObject(object);
    m_objects.append(object);
    m_sceneModel.createOrUpdate(object);
    emit objectAdded(object);
    setCurrentObject(object);
    return object;
}

bool SceneController::removeObject(PrimitiveObject *object)
{
    if (!object || !m_objects.contains(object)) {
        return false;
    }

    if (m_currentObject == object) {
        setCurrentObject(nullptr);
    }

    m_sceneModel.remove(object);
    object->deleteLater();
    return true;
}

bool SceneController::removeCurrentObject()
{
    return removeObject(m_currentObject);
}

bool SceneController::setObjectParent(PrimitiveObject *object, PrimitiveObject *parentObject)
{
    if (!object || !m_objects.contains(object) || object == parentObject) {
        return false;
    }
    if (parentObject && (!m_objects.contains(parentObject) || object->isAncestorOf(parentObject))) {
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

QList<PrimitiveObject *> SceneController::objects() const
{
    return m_objects;
}

PrimitiveObject *SceneController::currentObject() const
{
    return m_currentObject;
}

SceneModel *SceneController::sceneModel()
{
    return &m_sceneModel;
}

void SceneController::setCurrentObject(PrimitiveObject *object)
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

void SceneController::registerObject(PrimitiveObject *object)
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
    }

    Q_ASSERT(counter);
    ++(*counter);
    return QStringLiteral("%1 %2").arg(PrimitiveObject::displayName(type)).arg(*counter);
}

QVector3D SceneController::nextSpawnPosition() const
{
    const int index = m_objects.size();
    constexpr float spacing = 2.5f;
    const float x = static_cast<float>(index % 4) * spacing;
    const float y = static_cast<float>(index / 4) * spacing;
    return {x, y, 0.0f};
}
