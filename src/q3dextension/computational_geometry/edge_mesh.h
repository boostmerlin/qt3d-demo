#ifndef PMAXWELL_QT_EDGE_MESH_H
#define PMAXWELL_QT_EDGE_MESH_H

#include <Qt3DRender/QGeometryRenderer>
#include "line_geometry.h"

class EdgeMesh final : public Qt3DRender::QGeometryRenderer {
    Q_OBJECT

public:
    //sharable geometry is better managed outside
    explicit EdgeMesh(const Qt3DCore::QGeometry &geometry, float thresholdAngle = 1, int precision = 5,
                      QNode *parent = nullptr);

    const QList<QVector3D> &points() const;

private:
    [[nodiscard]] LineGeometry *lineGeometry() const;

    void setInstanceCount(int instanceCount);

    void setVertexCount(int vertexCount);

    void setIndexOffset(int indexOffset);

    void setFirstInstance(int firstInstance);

    void setRestartIndexValue(int index);

    void setPrimitiveRestartEnabled(bool enabled);

    void setGeometry(Qt3DCore::QGeometry *geometry);

    void setPrimitiveType(PrimitiveType primitiveType);
};


#endif //PMAXWELL_QT_EDGE_MESH_H
