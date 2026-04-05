#include "ring_mesh.h"

#include <QtMath>

RingMesh::RingMesh(Qt3DCore::QNode *parent) : Qt3DRender::QGeometryRenderer(parent) {
    auto geometry = new RingGeometry(this);
    Qt3DRender::QGeometryRenderer::setGeometry(geometry);
    Qt3DRender::QGeometryRenderer::setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
}

RingGeometry *RingMesh::geometry() const {
    return dynamic_cast<RingGeometry *>(Qt3DRender::QGeometryRenderer::geometry());
}

int RingMesh::rings() const {
    return geometry()->rings();
}

int RingMesh::slices() const {
    return geometry()->slices();
}

float RingMesh::innerRadius() {
    return geometry()->innerRadius();
}

float RingMesh::outerRadius() {
    return geometry()->outerRadius();
}

float RingMesh::length() {
    return geometry()->length();
}

float RingMesh::startAngle() {
    return geometry()->startAngle();
}

float RingMesh::endAngle() {
    return geometry()->endAngle();
}

void RingMesh::setRings(int rings) {
    geometry()->setRings(rings);
    emit meshChanged(this);
}

void RingMesh::setSlices(int slices) {
    geometry()->setSlices(slices);
    emit meshChanged(this);
}

void RingMesh::setInnerRadius(float radius) const {
    geometry()->setInnerRadius(radius);
    emit meshChanged(this);
}

void RingMesh::setOuterRadius(float radius) const {
    geometry()->setOuterRadius(radius);
    emit meshChanged(this);
}

void RingMesh::setLength(float length) const {
    geometry()->setLength(length);
    emit meshChanged(this);
}

void RingMesh::setStartAngle(float angle) const {
    geometry()->setStartAngle(angle);
    emit meshChanged(this);
}

void RingMesh::setEndAngle(float angle) const {
    geometry()->setEndAngle(angle);
    emit meshChanged(this);
}

void RingMesh::setStartAngleInDegrees(float angle) {
    geometry()->setStartAngleInDegrees(angle);
    emit meshChanged(this);
}

void RingMesh::setEndAngleInDegrees(float angle) {
    geometry()->setEndAngleInDegrees(angle);
    emit meshChanged(this);
}


float RingMesh::startAngleInDegrees() {
    return qRadiansToDegrees(geometry()->startAngle());
}

float RingMesh::endAngleInDegrees() {
    return qRadiansToDegrees(geometry()->endAngle());
}
