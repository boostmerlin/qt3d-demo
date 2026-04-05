#ifndef PMAXWELL_QT_POLYGONGEOMETRY_H
#define PMAXWELL_QT_POLYGONGEOMETRY_H

#include <Qt3DCore/QGeometry>
#include <QVector3D>

class PolygonGeometry : public Qt3DCore::QGeometry {
Q_OBJECT

public:
    explicit PolygonGeometry(Qt3DCore::QNode *parent = nullptr);

public slots:

    void setPolygon(const QList<QVector3D> &vertices);

    void setPolygon(const QList<QPointF> &vertices);

    void setHeight(float height);

    void setRings(uint rings);

    void setResolution(float resolution);

public:
    float height() const {
        return m_height;
    }

    const QList<QVector3D> &vertices() const {
        return m_vertices;
    }

    uint rings() const {
        return m_rings;
    }

    bool validateVertices() const;

private:
    void init();

    QByteArray generateVertices(uint count) const;

    QByteArray generateIndices(uint count, const QList<int> &triangles);

    void updateVertices();

    void updateIndices(const QList<int> &triangles);

    void update();

    void siftVertices(const QList<QVector3D> &vertices);
private:
    QList<QVector3D> m_vertices;
    QVector3D m_normal;
    float m_height;
    uint m_rings;
    Qt3DCore::QAttribute *m_positionAttribute{};
    Qt3DCore::QAttribute *m_normalAttribute{};
    Qt3DCore::QAttribute *m_indexAttribute{};
    Qt3DCore::QBuffer *m_vertexBuffer{};
    Qt3DCore::QBuffer *m_indexBuffer{};
    float m_resolution = 0.05f;
    size_t m_hash = 0;
};


#endif //PMAXWELL_QT_POLYGONGEOMETRY_H
