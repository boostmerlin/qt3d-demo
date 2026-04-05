
#ifndef PMAXWELL_QT_LINE_MESH_H
#define PMAXWELL_QT_LINE_MESH_H

#include <Qt3DRender/QGeometryRenderer>
#include "line_geometry.h"

class LineMesh : public Qt3DRender::QGeometryRenderer {
    Q_OBJECT
public:
    explicit LineMesh(Qt3DCore::QNode *parent = nullptr);

    void setLinePoints(const QList<QVector3D> &points);

    void setLinePoints(QList<QVector3D> &&points);

    void setLinePoints(const QList<QPointF> &points, float z = 0);

    void addPoint(const QVector3D &point, int at = -1);

    void removePointAt(int at);

    void setLineType(LineGeometry::LineType lineType);

    const QList<QVector3D> &points() const;

    LineGeometry::LineType lineType() const;

    void setNormalAttributeEnable(bool enable);

private:
    [[nodiscard]] LineGeometry* lineGeometry() const;

    void setInstanceCount(int instanceCount);

    void setVertexCount(int vertexCount);

    void setIndexOffset(int indexOffset);

    void setFirstInstance(int firstInstance);

    void setRestartIndexValue(int index);

    void setPrimitiveRestartEnabled(bool enabled);

    void setGeometry(Qt3DCore::QGeometry *geometry);

    void setPrimitiveType(PrimitiveType primitiveType);
};


#endif //PMAXWELL_QT_LINE_MESH_H
