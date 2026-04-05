#ifndef PMAXWELL_QT_RING_MESH_H
#define PMAXWELL_QT_RING_MESH_H

#include <Qt3DRender/QGeometryRenderer>
#include "ring_geometry.h"

class RingMesh : public Qt3DRender::QGeometryRenderer {
    Q_OBJECT
public:
    explicit RingMesh(Qt3DCore::QNode *parent = nullptr);

    int rings() const;

    int slices() const;

    float innerRadius();

    float outerRadius();

    float length();

    float startAngle();

    float endAngle();

    float startAngleInDegrees();

    float endAngleInDegrees();

public slots:

    void setRings(int rings);

    void setSlices(int slices);

    void setInnerRadius(float radius);

    void setOuterRadius(float radius);

    void setLength(float length);

    void setStartAngle(float angle);

    void setEndAngle(float angle);

    void setStartAngleInDegrees(float angle);

    void setEndAngleInDegrees(float angle);

signals:

    void meshChanged(const RingMesh* mesh) const;

private:
    [[nodiscard]] RingGeometry *geometry() const;

    // no one should be able to modify the QGeometryRenderer's properties
    void setInstanceCount(int instanceCount);

    void setVertexCount(int vertexCount);

    void setIndexOffset(int indexOffset);

    void setFirstInstance(int firstInstance);

    void setRestartIndexValue(int index);

    void setPrimitiveRestartEnabled(bool enabled);

    void setGeometry(Qt3DCore::QGeometry *geometry);

    void setPrimitiveType(PrimitiveType primitiveType);
};

#endif //PMAXWELL_QT_RING_MESH_H
