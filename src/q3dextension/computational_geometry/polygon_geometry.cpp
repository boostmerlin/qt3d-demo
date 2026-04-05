#include "polygon_geometry.h"
#include "q3dextension/utils/geometry_util.h"

constexpr uint vertexSize = (3 + 3) * sizeof(float);

uint vertexCount(qsizetype n, uint rings) {
    return 2 * ((rings + 1) * n);
}

uint indicesCount(qsizetype n, uint rings) {
    rings = qMax(rings, 1u);
    return 3 * (2 * (n - 2) + 2 * n * (rings - 1));
}

PolygonGeometry::PolygonGeometry(Qt3DCore::QNode *parent)
        : Qt3DCore::QGeometry(parent), m_height(1), m_rings(2) {
    init();
}

void PolygonGeometry::init() {
    m_vertexBuffer = new Qt3DCore::QBuffer(this);
    m_indexBuffer = new Qt3DCore::QBuffer(this);

    m_positionAttribute = new Qt3DCore::QAttribute(this);
    m_positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    m_positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    m_positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    m_positionAttribute->setVertexSize(3);
    m_positionAttribute->setBuffer(m_vertexBuffer);
    m_positionAttribute->setByteStride(vertexSize);
    addAttribute(m_positionAttribute);

    m_normalAttribute = new Qt3DCore::QAttribute(this);
    m_normalAttribute->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
    m_normalAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    m_normalAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    m_normalAttribute->setVertexSize(3);
    m_normalAttribute->setByteOffset(3 * sizeof(float));
    m_normalAttribute->setByteStride(vertexSize);
    m_normalAttribute->setBuffer(m_vertexBuffer);
    addAttribute(m_normalAttribute);

    m_indexAttribute = new Qt3DCore::QAttribute(this);
    m_indexAttribute->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    m_indexAttribute->setVertexBaseType(Qt3DCore::QAttribute::UnsignedShort);
    m_indexAttribute->setBuffer(m_indexBuffer);
    addAttribute(m_indexAttribute);
    m_normal = QVector3D(0, 0, 1);
}

void PolygonGeometry::setHeight(float height) {
    Q_ASSERT(height >= 0);
    if (m_height != height) {
        m_height = height;
        updateVertices();
    }
}

void PolygonGeometry::setRings(uint rings) {
    Q_ASSERT(rings != 1);
    if (m_rings != rings) {
        m_rings = rings;
        update();
    }
}

size_t qHash(const QList<QVector3D> &vertices) {
    size_t hash = 0;
    for (const QVector3D &v: vertices) {
        hash ^= qHashMulti(hash, v.x(), v.y(), v.z());
    }
    return hash;
}

void PolygonGeometry::setPolygon(const QList<QVector3D> &vertices) {
    if (vertices.size() < 3) {
        clearPolygon();
        return;
    }
    siftVertices(vertices);
    if (m_vertices.size() < 3) {
        clearPolygon();
        return;
    }
    auto hash = qHash(m_vertices);
    if (hash != m_hash) {
        update();
        m_hash = hash;
    }
}

void PolygonGeometry::clearPolygon()
{
    m_vertices.clear();
    m_hash = 0;
    clearGeometry();
}

void PolygonGeometry::siftVertices(const QList<QVector3D> &vertices) {
    if (!validateVertices()) {
        return;
    }
    m_vertices.clear();
    m_vertices.reserve(vertices.count());
    auto &v0 = vertices[0];
    QVector3D n = QVector3D::crossProduct(vertices[1] - v0, vertices[2] - v0);
    float area = 0.0f;
    float reso = m_resolution * m_resolution;
    m_vertices.append(v0);
    auto last = v0;
    for (int i = 1; i < vertices.size(); ++i) {
        auto &vi = vertices[i];
        if (!coplanar(vi, v0, n)) {
            qWarning() << "[PolygonGeometry::siftVertices] invalid vertices, not coplanar: " << i;
            continue;
        }
        if ((vi-last).lengthSquared() < reso) {
            continue;
        }
        area += (last.x() * vi.y()) - (vi.x() * last.y());
        m_vertices.append(vi);
        last = vi;
    }
    area += (last.x() * v0.y()) - (v0.x() * last.y());
    if (area < 0) {
        std::reverse(m_vertices.begin(), m_vertices.end());
    }
}

void PolygonGeometry::setPolygon(const QList<QPointF> &vertices) {
    QList<QVector3D> vertices3D(vertices.size());
    std::transform(vertices.begin(), vertices.end(), vertices3D.begin(), [](const QPointF &v) {
        return QVector3D(v);
    });

    setPolygon(vertices3D);
}

static void createPolygonVertices(float *&ptr, const QList<QVector3D> &vertices, QVector3D normal, float height) {
    if (height == 0) {
        normal = -normal;
    }
    QVector3D d = normal * height;
    for (const QVector3D &v: vertices) {
        *ptr++ = v.x() + d.x();
        *ptr++ = v.y() + d.y();
        *ptr++ = v.z() + d.z();
        *ptr++ = normal.x();
        *ptr++ = normal.y();
        *ptr++ = normal.z();
    }
}

static void
createSideVertices(float *&ptr, const QList<QVector3D> &vertices, QVector3D normal, int rings, float height) {
    if (rings <= 1) {
        return;
    }
    //hard edge
    const QVector3D dd = normal * height / float(rings - 1);
    const qsizetype verticesCount = vertices.size();
    for (qsizetype i = 0; i < verticesCount; ++i) {
        const QVector3D &v1 = vertices[i];
        const QVector3D &v2 = vertices[(i + 1) % verticesCount];
        const QVector3D v3 = v2 + dd;
        //share this normal, compute once
        const QVector3D normal2 = QVector3D::normal(v1, v2, v3);
        for (uint j = 0; j < rings; ++j) {
            *ptr++ = v1.x() + dd.x() * j;
            *ptr++ = v1.y() + dd.y() * j;
            *ptr++ = v1.z() + dd.z() * j;
            *ptr++ = normal2.x();
            *ptr++ = normal2.y();
            *ptr++ = normal2.z();
            *ptr++ = v2.x() + dd.x() * j;
            *ptr++ = v2.y() + dd.y() * j;
            *ptr++ = v2.z() + dd.z() * j;
            *ptr++ = normal2.x();
            *ptr++ = normal2.y();
            *ptr++ = normal2.z();
        }
    }
}

QByteArray PolygonGeometry::generateVertices(uint count) const {
    QByteArray data;
    const uint size = count * vertexSize;
    data.resize(size);
    auto *vptr = reinterpret_cast<float *>(data.data());
    createSideVertices(vptr, m_vertices, m_normal, int(m_rings), m_height);
    createPolygonVertices(vptr, m_vertices, m_normal, 0);
    createPolygonVertices(vptr, m_vertices, m_normal, m_height);

    Q_ASSERT(reinterpret_cast<char *>(vptr) == data.data() + size);
    return data;
}

void createSideIndices(ushort *&ptr, uint n, uint rings) {
    if (rings <= 1) {
        return;
    }
    for (uint i = 0; i < n; ++i) {
        for (uint j = 0; j < rings - 1; ++j) {
            uint start = i * rings * 2 + j * 2;
            *ptr++ = start;
            *ptr++ = start + 1;
            *ptr++ = start + 2;
            *ptr++ = start + 1;
            *ptr++ = start + 3;
            *ptr++ = start + 2;
        }
    }
}

static void createPolygonIndices(ushort *&ptr, const QList<int> &triangles, int startIndex, bool swap) {
    for (int i = 0; i < triangles.size(); i += 3) {
        *ptr++ = triangles[i] + startIndex;
        *ptr++ = triangles[i + 1] + startIndex;
        *ptr++ = triangles[i + 2] + startIndex;
        if (swap) {
            qSwap(*(ptr - 1), *(ptr - 2));
        }
    }
}

QByteArray PolygonGeometry::generateIndices(uint count, const QList<int> &triangles) {
    QByteArray data;
    Q_ASSERT(count < std::numeric_limits<ushort>::max());

    data.resize(qsizetype(count * sizeof(ushort)));
    auto *vptr = reinterpret_cast<ushort *>(data.data());
    createSideIndices(vptr, m_vertices.size(), m_rings);
    int offset = int(m_vertices.size() * m_rings * 2);
    createPolygonIndices(vptr, triangles, offset, true);
    offset += int(m_vertices.size());
    createPolygonIndices(vptr, triangles, offset, false);
    Q_ASSERT(reinterpret_cast<char *>(vptr) == data.data() + count * sizeof(ushort));
    return data;
}

void PolygonGeometry::updateVertices() {
    uint count = vertexCount(m_vertices.size(), m_rings);
    m_positionAttribute->setCount(count);
    m_normalAttribute->setCount(count);
    m_vertexBuffer->setData(generateVertices(count));
}

void PolygonGeometry::updateIndices(const QList<int> &triangles) {
//    if (m_vertices.isEmpty()) {
//        return;
//    }
    uint count = indicesCount(m_vertices.size(), m_rings);
    m_indexAttribute->setCount(count);
    m_indexBuffer->setData(generateIndices(count, triangles));
}

void PolygonGeometry::update() {
    QList<int> triangles;
    if (!triangulateV2(m_vertices, triangles)) {
        qWarning() << "[PolygonGeometry::update]: invalid polygon, triangulate failed.";
        m_indexAttribute->setCount(0);
        m_normalAttribute->setCount(0);
        m_positionAttribute->setCount(0);
        return;
    }
    updateIndices(triangles);
    updateVertices();
}

void PolygonGeometry::clearGeometry()
{
    m_indexAttribute->setCount(0);
    m_normalAttribute->setCount(0);
    m_positionAttribute->setCount(0);
    m_vertexBuffer->setData({});
    m_indexBuffer->setData({});
}

bool PolygonGeometry::validateVertices() const {
    return m_resolution > 0;
}

void PolygonGeometry::setResolution(float resolution) {
    m_resolution = resolution;
}

