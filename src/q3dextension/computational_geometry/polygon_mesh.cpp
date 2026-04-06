#include "polygon_mesh.h"

PolygonMesh::PolygonMesh(Qt3DCore::QNode *parent) : Qt3DRender::QGeometryRenderer(parent) {
    auto geometry = new PolygonGeometry(this);
    Qt3DRender::QGeometryRenderer::setGeometry(geometry);
    Qt3DRender::QGeometryRenderer::setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
}

PolygonGeometry* PolygonMesh::polygonGeometry() const {
    return dynamic_cast<PolygonGeometry *>(geometry());
}

float PolygonMesh::height() const {
    return polygonGeometry()->height();
}

const QList<QVector3D> &PolygonMesh::vertices() const {
    return polygonGeometry()->vertices();
}

uint PolygonMesh::rings() const {
    return polygonGeometry()->rings();
}

bool PolygonMesh::validateVertices() const {
    return polygonGeometry()->validateVertices();
}

void PolygonMesh::clearPolygon() {
    polygonGeometry()->clearPolygon();
    emit meshChanged(this);
}

void PolygonMesh::setPolygon(const QList<QVector3D> &vertices) {
    polygonGeometry()->setPolygon(vertices);
    emit meshChanged(this);
}

void PolygonMesh::setPolygon(const QList<QPointF> &vertices) {
    polygonGeometry()->setPolygon(vertices);
    emit meshChanged(this);
}

void PolygonMesh::setHeight(float height) {
    polygonGeometry()->setHeight(height);
    emit meshChanged(this);
}

void PolygonMesh::setRings(uint rings) {
    polygonGeometry()->setRings(rings);
    emit meshChanged(this);
}

void PolygonMesh::setResolution(float resolution) const {
    polygonGeometry()->setResolution(resolution);
}

