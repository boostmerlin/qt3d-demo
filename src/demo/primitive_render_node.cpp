#include "primitive_render_node.h"

#include <QQuaternion>
#include <QPhongMaterial>
#include <QCuboidMesh>
#include <QSphereMesh>
#include <QCylinderMesh>

PrimitiveRenderNodeBase::PrimitiveRenderNodeBase()
{
    auto *material = new Qt3DExtras::QPhongMaterial();
    material->setShininess(40.0f);
    addMaterial(material);
}

bool PrimitiveRenderNodeBase::updateRenderData(const QObject *object)
{
    const auto *primitiveObject = qobject_cast<const PrimitiveObject *>(object);
    if (!primitiveObject) {
        return false;
    }
    m_primitive = primitiveObject;
    setObjectName(primitiveObject->name());
    return true;
}

const PrimitiveObject *PrimitiveRenderNodeBase::primitive() const
{
    return m_primitive;
}

void PrimitiveRenderNodeBase::applyTransform() const
{
    Q_ASSERT(m_primitive);
    auto *nodeTransform = transform();
    Q_ASSERT(nodeTransform);
    nodeTransform->setTranslation(m_primitive->position());
    nodeTransform->setRotation(QQuaternion::fromEulerAngles(m_primitive->rotation()));
}

void PrimitiveRenderNodeBase::updateMaterial()
{
    auto *material = this->material<Qt3DExtras::QPhongMaterial>();
    Q_ASSERT(material);
    const QColor color = displayColor();
    material->setDiffuse(color);
    material->setAmbient(color.darker(180));
    material->setSpecular(QColorConstants::White);
}

void PrimitiveRenderNodeBase::onSelect() noexcept
{
    m_selected = true;
    updateMaterial();
}

void PrimitiveRenderNodeBase::onDeselect() noexcept
{
    m_selected = false;
    updateMaterial();
}

QColor PrimitiveRenderNodeBase::displayColor() const
{
    Q_ASSERT(m_primitive);
    const QColor baseColor = m_primitive->color();
    return m_selected ? baseColor.lighter(145) : baseColor;
}

BoxPrimitiveRenderNode::BoxPrimitiveRenderNode()
    : PrimitiveRenderNodeBase()
{
}

void BoxPrimitiveRenderNode::onCreate()
{
    entity()->addComponent(new Qt3DExtras::QCuboidMesh());
    attachMaterial();
    onUpdate();
}

void BoxPrimitiveRenderNode::onUpdate()
{
    const auto *object = primitiveAs<BoxObject>();
    Q_ASSERT(object);
    auto *mesh = getComponent<Qt3DExtras::QCuboidMesh>();
    Q_ASSERT(mesh);
    mesh->setXExtent(object->width());
    mesh->setYExtent(object->height());
    mesh->setZExtent(object->depth());
    applyTransform();
    updateMaterial();
}

SpherePrimitiveRenderNode::SpherePrimitiveRenderNode()
    : PrimitiveRenderNodeBase()
{
}

void SpherePrimitiveRenderNode::onCreate()
{
    auto *mesh = new Qt3DExtras::QSphereMesh();
    mesh->setRings(32);
    mesh->setSlices(32);
    entity()->addComponent(mesh);
    attachMaterial();
    onUpdate();
}

void SpherePrimitiveRenderNode::onUpdate()
{
    const auto *object = primitiveAs<SphereObject>();
    Q_ASSERT(object);
    auto *mesh = getComponent<Qt3DExtras::QSphereMesh>();
    Q_ASSERT(mesh);
    mesh->setRadius(object->radius());
    applyTransform();
    updateMaterial();
}

CylinderPrimitiveRenderNode::CylinderPrimitiveRenderNode()
    : PrimitiveRenderNodeBase()
{
}

void CylinderPrimitiveRenderNode::onCreate()
{
    auto *mesh = new Qt3DExtras::QCylinderMesh();
    mesh->setRings(8);
    entity()->addComponent(mesh);
    attachMaterial();
    onUpdate();
}

void CylinderPrimitiveRenderNode::onUpdate()
{
    const auto *object = primitiveAs<CylinderObject>();
    Q_ASSERT(object);
    auto *mesh = getComponent<Qt3DExtras::QCylinderMesh>();
    Q_ASSERT(mesh);
    mesh->setRadius(object->radius());
    mesh->setLength(object->height());
    mesh->setSlices(36);
    applyTransform();
    updateMaterial();
}

ConePrimitiveRenderNode::ConePrimitiveRenderNode()
= default;

void ConePrimitiveRenderNode::onCreate()
{
    auto *mesh = new Qt3DExtras::QConeMesh();
    entity()->addComponent(mesh);
    attachMaterial();
    onUpdate();
}

void ConePrimitiveRenderNode::onUpdate()
{
    const auto *object = primitiveAs<ConeObject>();
    Q_ASSERT(object);
    auto *mesh = getComponent<Qt3DExtras::QConeMesh>();
    Q_ASSERT(mesh);
    mesh->setBottomRadius(object->radius());
    mesh->setTopRadius(0.0f);
    mesh->setLength(object->height());
    mesh->setSlices(36);
    applyTransform();
    updateMaterial();
}

RenderNode *PrimitiveRenderNode::create(const QObject *object)
{
    if (qobject_cast<const BoxObject *>(object)) {
        return new BoxPrimitiveRenderNode();
    }
    if (qobject_cast<const SphereObject *>(object)) {
        return new SpherePrimitiveRenderNode();
    }
    if (qobject_cast<const CylinderObject *>(object)) {
        return new CylinderPrimitiveRenderNode();
    }
    if (qobject_cast<const ConeObject *>(object)) {
        return new ConePrimitiveRenderNode();
    }
    return nullptr;
}
