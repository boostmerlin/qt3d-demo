#include "line_mesh.h"

using Qt3DRender::QGeometryRenderer;

//loop can be supported in geometry.
static auto primitiveTypeByLineType(LineGeometry::LineType type) {
    switch (type) {
        case LineGeometry::LineType::LineSegments:
            return QGeometryRenderer::Lines;
        case LineGeometry::LineType::Line:
            return QGeometryRenderer::LineStrip;
        default:
            break;
    }
    return QGeometryRenderer::LineStrip;
}

LineMesh::LineMesh(Qt3DCore::QNode *parent) : Qt3DRender::QGeometryRenderer(parent) {
    auto geometry = new LineGeometry(this);
    QGeometryRenderer::setGeometry(geometry);
    QGeometryRenderer::setPrimitiveType(primitiveTypeByLineType(geometry->lineType()));
    connect(geometry, &LineGeometry::lineTypeChanged, this, [this](LineGeometry::LineType type) {
        QGeometryRenderer::setPrimitiveType(primitiveTypeByLineType(type));
    });
}

LineGeometry *LineMesh::lineGeometry() const {
    return dynamic_cast<LineGeometry *>(geometry());
}

void LineMesh::setLinePoints(const QList<QVector3D> &points) {
    lineGeometry()->setLinePoints(points);
}

void LineMesh::addPoint(const QVector3D &point, int at) {
    lineGeometry()->addPoint(point, at);
}

void LineMesh::removePointAt(int at) {
    lineGeometry()->removePointAt(at);
}

void LineMesh::setLineType(LineGeometry::LineType lineType) {
    lineGeometry()->setLineType(lineType);
}

const QList<QVector3D> &LineMesh::points() const {
    return lineGeometry()->points();
}

LineGeometry::LineType LineMesh::lineType() const {
    return lineGeometry()->lineType();
}

void LineMesh::setNormalAttributeEnable(bool enable) {
    lineGeometry()->setNormalAttributeEnable(enable);
}

void LineMesh::setLinePoints(QList<QVector3D> &&points) {
    lineGeometry()->setLinePoints(std::move(points));
}

void LineMesh::setLinePoints(const QList<QPointF> &points, float z) {
    QList<QVector3D> list;
    for (const auto &v : points) {
        list.append(QVector3D(float(v.x()), float(v.y()), z));
    }
    setLinePoints(std::move(list));
}
