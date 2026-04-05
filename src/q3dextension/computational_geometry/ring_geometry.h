#ifndef PMAXWELL_QT_RING_GEOMETRY_H
#define PMAXWELL_QT_RING_GEOMETRY_H

#include <Qt3DCore/QGeometry>
#include <QVector3D>

class RingGeometry : public Qt3DCore::QGeometry {
Q_OBJECT

public:
    explicit RingGeometry(Qt3DCore::QNode *parent = nullptr);

    int rings() const {
        return m_rings;
    }

    int slices() const {
        return m_slices;
    }

    float innerRadius() const {
        return m_innerRadius;
    }

    float outerRadius() const {
        return m_outerRadius;
    }

    float length() const {
        return m_length;
    }

    float startAngle() const {
        return m_startAngle;
    }

    float endAngle() const {
        return m_endAngle;
    }

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

private:
    void updateVertices();

    void updateIndices();

    void createSidesVertices(float *&ptr);

    void createDiscVertices(float *&ptr, float z);

    void createDiscIndices(ushort *&ptr, int offset, bool swap) const;

    void createSidesIndices(ushort *&ptr) const;

    QByteArray generateVertexData();

    QByteArray generateIndexData();

    int getSlices() const;

    float getArc() const;

private:
    int m_rings;
    int m_slices;
    float m_innerRadius;
    float m_outerRadius;
    float m_length;
    float m_startAngle;
    float m_endAngle;
    Qt3DCore::QAttribute *m_positionAttribute{};
    Qt3DCore::QAttribute *m_normalAttribute{};
    Qt3DCore::QAttribute *m_indexAttribute{};
    Qt3DCore::QBuffer *m_vertexBuffer{};
    Qt3DCore::QBuffer *m_indexBuffer{};
};

#endif //PMAXWELL_QT_RING_GEOMETRY_H
