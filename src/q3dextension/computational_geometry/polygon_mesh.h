#ifndef PMAXWELL_QT_POLYGONMESH_H
#define PMAXWELL_QT_POLYGONMESH_H

#include <Qt3DRender/QGeometryRenderer>
#include "polygon_geometry.h"

class PolygonMesh : public Qt3DRender::QGeometryRenderer {
Q_OBJECT

public:
    explicit PolygonMesh(Qt3DCore::QNode *parent = nullptr);

    [[nodiscard]] float height() const;

    [[nodiscard]] const QList<QVector3D> &vertices() const;

    [[nodiscard]] uint rings() const;

    [[nodiscard]] bool validateVertices() const;

    void clearPolygon();
    void setPolygon(const QList<QVector3D> &vertices);

    void setPolygon(const QList<QPointF> &vertices);

    void setResolution(float resolution);

    void setHeight(float height);

    void setRings(uint rings);

    void setValidateVertices(bool validateVertices) const;
signals:
    void meshChanged(const PolygonMesh* mesh) const;
private:
    [[nodiscard]] PolygonGeometry* polygonGeometry() const;

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


#endif //PMAXWELL_QT_POLYGONMESH_H
