//
// Created by merlin
//

#include "extrude_shape_mesh.h"

ExtrudeShapeMesh::ExtrudeShapeMesh(Qt3DCore::QNode *parent) : Qt3DRender::QGeometryRenderer(parent) {
    auto geometry = new ExtrudeShapeGeometry(this);
    Qt3DRender::QGeometryRenderer::setGeometry(geometry);
}

ExtrudeShapeGeometry *ExtrudeShapeMesh::extrudeShapeGeometry() const {
    return dynamic_cast<ExtrudeShapeGeometry *>(geometry());
}

void ExtrudeShapeMesh::setCurve(const Curve *curve) {
    extrudeShapeGeometry()->setCurve(curve);
}

void ExtrudeShapeMesh::setShape(const Shape *shape) {
    extrudeShapeGeometry()->setShape(shape);
}
