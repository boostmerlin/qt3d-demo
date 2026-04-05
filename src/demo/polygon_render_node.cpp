#include "polygon_render_node.h"

#include <QPhongMaterial>
#include <QQuaternion>

#include "q3dextension/computational_geometry/polygon_mesh.h"

PolygonRenderNode::PolygonRenderNode()
{
    auto *material = new Qt3DExtras::QPhongMaterial();
    material->setShininess(40.0f);
    addMaterial(material);
}

RenderNode *PolygonRenderNode::create(const QObject *object)
{
    return qobject_cast<const PolygonObject *>(object) ? new PolygonRenderNode() : nullptr;
}

bool PolygonRenderNode::updateRenderData(const QObject *object)
{
    const auto *polygonObject = qobject_cast<const PolygonObject *>(object);
    if (!polygonObject) {
        return false;
    }
    RenderNode::updateRenderData(object);
    m_polygon = polygonObject;
    setObjectName(polygonObject->name());
    return true;
}

void PolygonRenderNode::onCreate()
{
    auto *mesh = new PolygonMesh();
    entity()->addComponent(mesh);
    attachMaterial();
    onUpdate();
}

void PolygonRenderNode::onUpdate()
{
    Q_ASSERT(m_polygon);
    auto *mesh = getComponent<PolygonMesh>();
    Q_ASSERT(mesh);

    if (m_polygon->hasValidPolygon()) {
        mesh->setPolygon(m_polygon->vertices());
    } else {
        mesh->clearPolygon();
    }
    mesh->setHeight(m_polygon->height());
    mesh->setRings(m_polygon->rings());
    applyTransform();
    updateMaterial();
}

void PolygonRenderNode::onSelect() noexcept
{
    m_selected = true;
    updateMaterial();
}

void PolygonRenderNode::onDeselect() noexcept
{
    m_selected = false;
    updateMaterial();
}

void PolygonRenderNode::applyTransform() const
{
    Q_ASSERT(m_polygon);
    auto *nodeTransform = transform();
    Q_ASSERT(nodeTransform);
    nodeTransform->setTranslation(m_polygon->position());
    nodeTransform->setRotation(QQuaternion::fromEulerAngles(m_polygon->rotation()));
}

void PolygonRenderNode::updateMaterial()
{
    auto *material = this->material<Qt3DExtras::QPhongMaterial>();
    Q_ASSERT(material);
    const QColor color = displayColor();
    material->setDiffuse(color);
    material->setAmbient(color.darker(180));
    material->setSpecular(QColorConstants::White);
}

QColor PolygonRenderNode::displayColor() const
{
    Q_ASSERT(m_polygon);
    const QColor baseColor = m_polygon->color();
    return m_selected ? baseColor.lighter(145) : baseColor;
}
