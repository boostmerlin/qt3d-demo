#include "extrude_render_node.h"

#include <QPhongMaterial>
#include <QPolygonF>
#include <QQuaternion>

#include "q3dextension/computational_geometry/extrude_shape_mesh.h"
#include "q3dextension/curves/bezier_curve.h"
#include "q3dextension/curves/line_curve.h"
#include "q3dextension/shapes/polygon_profile_shape.h"

namespace {
QPolygonF toPolygon(const ExtrudeProfileVertices &vertices)
{
    QPolygonF polygon;
    polygon.reserve(vertices.size());
    for (const auto &vertex : vertices) {
        polygon.append(QPointF(vertex.x(), vertex.y()));
    }
    return polygon;
}
}

ExtrudeRenderNode::ExtrudeRenderNode()
{
    auto *material = new Qt3DExtras::QPhongMaterial();
    material->setShininess(40.0f);
    addMaterial(material);
}

RenderNode *ExtrudeRenderNode::create(const QObject *object)
{
    return qobject_cast<const ExtrudeObject *>(object) ? new ExtrudeRenderNode() : nullptr;
}

bool ExtrudeRenderNode::updateRenderData(const QObject *object)
{
    const auto *extrudeObject = qobject_cast<const ExtrudeObject *>(object);
    if (!extrudeObject) {
        return false;
    }
    RenderNode::updateRenderData(object);
    m_extrude = extrudeObject;
    setObjectName(extrudeObject->name());
    return true;
}

void ExtrudeRenderNode::onCreate()
{
    auto *mesh = new ExtrudeShapeMesh();
    entity()->addComponent(mesh);

    m_lineCurve = new LineCurve();
    m_lineCurve->setParent(this);
    m_bezierCurve = new CubicBezierCurve();
    m_bezierCurve->setParent(this);
    m_profileShape = new PolygonProfileShape(this);

    attachMaterial();
    onUpdate();
}

void ExtrudeRenderNode::onUpdate()
{
    Q_ASSERT(m_extrude);
    auto *mesh = getComponent<ExtrudeShapeMesh>();
    Q_ASSERT(mesh);

    applyTransform();
    updateMaterial();

    if (!m_extrude->hasValidPath() || !m_extrude->hasValidProfile()) {
        clearMesh();
        return;
    }

    const auto pathPoints = m_extrude->pathControlPoints();
    Curve *currentCurve = nullptr;
    switch (m_extrude->pathType()) {
    case ExtrudeObject::PathType::Line:
        m_lineCurve->setP1(pathPoints[0]);
        m_lineCurve->setP2(pathPoints[1]);
        m_lineCurve->setDivisions(m_extrude->pathDivisions());
        currentCurve = m_lineCurve;
        break;
    case ExtrudeObject::PathType::CubicBezier:
        m_bezierCurve->setControlPoints(pathPoints);
        m_bezierCurve->setDivisions(m_extrude->pathDivisions());
        currentCurve = m_bezierCurve;
        break;
    }

    m_profileShape->fromPolygon(toPolygon(m_extrude->profileVertices()));
    mesh->setShape(m_profileShape);
    mesh->setCurve(currentCurve);
}

void ExtrudeRenderNode::onSelect() noexcept
{
    m_selected = true;
    updateMaterial();
}

void ExtrudeRenderNode::onDeselect() noexcept
{
    m_selected = false;
    updateMaterial();
}

void ExtrudeRenderNode::applyTransform() const
{
    Q_ASSERT(m_extrude);
    auto *nodeTransform = transform();
    Q_ASSERT(nodeTransform);
    nodeTransform->setTranslation(m_extrude->position());
    nodeTransform->setRotation(QQuaternion::fromEulerAngles(m_extrude->rotation()));
}

void ExtrudeRenderNode::updateMaterial()
{
    auto *material = this->material<Qt3DExtras::QPhongMaterial>();
    Q_ASSERT(material);
    const QColor color = displayColor();
    material->setDiffuse(color);
    material->setAmbient(color.darker(180));
    material->setSpecular(QColorConstants::White);
}

void ExtrudeRenderNode::clearMesh() const
{
    auto *mesh = getComponent<ExtrudeShapeMesh>();
    Q_ASSERT(mesh);
    mesh->clearExtrude();
}

QColor ExtrudeRenderNode::displayColor() const
{
    Q_ASSERT(m_extrude);
    const QColor baseColor = m_extrude->color();
    return m_selected ? baseColor.lighter(145) : baseColor;
}
