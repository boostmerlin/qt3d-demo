//
// Created by merlin
//

#ifndef PMAXWELL_QT_EXTRUDE_SHAPE_MESH_H
#define PMAXWELL_QT_EXTRUDE_SHAPE_MESH_H

#include <Qt3DRender/QGeometryRenderer>
#include "extrude_shape_geometry.h"

class ExtrudeShapeMesh : public Qt3DRender::QGeometryRenderer {
Q_OBJECT

public:
    explicit ExtrudeShapeMesh(Qt3DCore::QNode *parent = nullptr);

    void setCurve(const Curve *curve);

    void setShape(const Shape *shape);

signals:

    void meshChanged(const ExtrudeShapeMesh *mesh) const;

private:
    [[nodiscard]] ExtrudeShapeGeometry *extrudeShapeGeometry() const;

    void setInstanceCount(int instanceCount);

    void setVertexCount(int vertexCount);

    void setIndexOffset(int indexOffset);

    void setFirstInstance(int firstInstance);

    void setRestartIndexValue(int index);

    void setPrimitiveRestartEnabled(bool enabled);

    void setGeometry(Qt3DCore::QGeometry *geometry);

    void setPrimitiveType(PrimitiveType primitiveType);
};

#endif //PMAXWELL_QT_EXTRUDE_SHAPE_MESH_H
