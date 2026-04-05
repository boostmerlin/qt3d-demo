#include <QBuffer>
#include "q3dextension/utils/geometry_util.h"
#include "edge_mesh.h"

using Qt3DCore::QAttribute;

static int getAttributeByteSize(const QAttribute::VertexBaseType type) {
    switch (type) {
        case QAttribute::Byte:
        case QAttribute::UnsignedByte:
            return 1;
        case QAttribute::Short:
        case QAttribute::UnsignedShort:
        case QAttribute::HalfFloat:
            return 2;
        case QAttribute::Int:
        case QAttribute::UnsignedInt:
        case QAttribute::Float:
            return 4;
        case QAttribute::Double:
            return 8;
    }
    return 0;
};

static uint getAttributeByteStride(QAttribute *attrib) {
    uint byteStride = attrib->byteStride();
    const uint vertexTypeByteSize = getAttributeByteSize(attrib->vertexBaseType());
    //tightly packed
    if (byteStride == 0)
        byteStride = attrib->vertexSize() * vertexTypeByteSize;
    return byteStride;
}

static void getAttribBytesAt(QAttribute *attrib, qsizetype index, QByteArray &out) {
    Q_ASSERT(index < attrib->count());

    const uint byteSize = getAttributeByteSize(attrib->vertexBaseType()) * attrib->vertexSize();
    const uint byteStride = getAttributeByteStride(attrib);
    const uint byteOffset = attrib->byteOffset() + index * byteStride;
    const auto data = attrib->buffer()->data().constData();
    out.resize(byteSize);
    out.replace(qsizetype(0), byteSize, data + byteOffset, byteSize);
}

static ushort getUShortValue(QAttribute *attrib, qsizetype index) {
    Q_ASSERT(attrib->vertexBaseType() == QAttribute::UnsignedShort && attrib->vertexSize() == 1);
    QByteArray data;
    getAttribBytesAt(attrib, index, data);
    const auto *ptr = reinterpret_cast<const ushort *>(data.data());
    return *ptr;
}

static QVector3D getQVector3D(QAttribute *attribute, qsizetype index) {
    Q_ASSERT(attribute->vertexBaseType() == QAttribute::Float && attribute->vertexSize() == 3);
    QByteArray data;
    getAttribBytesAt(attribute, index, data);
    const auto *ptr = reinterpret_cast<const float *>(data.data());
    return {ptr[0], ptr[1], ptr[2]};
}

static QVector3D getQVector3DCached(QAttribute *attribute, qsizetype index, QList<QVector3D> &cache) {
    Q_ASSERT(index < cache.size());
    QVector3D v = cache[index];
    if (!v.isNull()) {
        return v;
    }
    else {
        v = getQVector3D(attribute, index);
        cache[index] = v;
    }
    return v;
}

struct Vert {
    long long int x;
    long long int y;
    long long int z;

    bool operator==(const Vert &other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Edge {
    Edge(const Vert &v0, const Vert &v1) : v0(v0), v1(v1) {}

    Vert v0;
    Vert v1;

    bool operator==(const Edge &other) const {
        return v0 == other.v0 && v1 == other.v1;
    }
};

uint qHash(const Edge &key) {
    return qHash((key.v0.x << 8) + key.v1.x) ^ qHash((key.v0.y << 8) + key.v1.y) ^ qHash((key.v0.z << 8) + key.v1.z);
}

static QList<QVector3D>
generateEdgeVertices(QAttribute *positionAttrib, float threshold, int precision, QAttribute *indicesAttrib) {
    QList<QVector3D> vertices;
    qsizetype count = indicesAttrib ? indicesAttrib->count() : positionAttrib->count();
    QList<QVector3D> edgeVertices;
    edgeVertices.resize(count, QVector3D());

    const qsizetype indicesSize = 3;

    const auto precisionFactor = qPow(10, precision);
    const auto thresholdDot = qCos(qDegreesToRadians(threshold)) + std::numeric_limits<float>::epsilon();

    QHash<Edge, std::tuple<int, int, QVector3D> > edgeData;
    int indices[indicesSize];
    QVector3D vertexes[indicesSize];
    Vert vert[indicesSize];

    for (int i = 0; i < count; i += indicesSize) {
        if (indicesAttrib) {
            indices[0] = getUShortValue(indicesAttrib, i);
            indices[1] = getUShortValue(indicesAttrib, i + 1);
            indices[2] = getUShortValue(indicesAttrib, i + 2);
        } else {
            indices[0] = i;
            indices[1] = i + 1;
            indices[2] = i + 2;
        }
        auto &v0 = vertexes[0] = getQVector3DCached(positionAttrib, indices[0], edgeVertices);
        auto &v1 = vertexes[1] = getQVector3DCached(positionAttrib, indices[1], edgeVertices);
        auto &v2 = vertexes[2] = getQVector3DCached(positionAttrib, indices[2], edgeVertices);

        auto cross = QVector3D::crossProduct(v1 - v0, v2 - v0);
        auto normal = normalize(cross);
        if (normal.isNull()) {
            qWarning() << "Degenerate edge" << cross;
            continue;
        }
        vert[0] = Vert{qRound64(v0.x() * precisionFactor),
                       qRound64(v0.y() * precisionFactor),
                       qRound64(v0.z() * precisionFactor)};
        vert[1] = Vert{qRound64(v1.x() * precisionFactor),
                       qRound64(v1.y() * precisionFactor),
                       qRound64(v1.z() * precisionFactor)};
        vert[2] = Vert{qRound64(v2.x() * precisionFactor),
                       qRound64(v2.y() * precisionFactor),
                       qRound64(v2.z() * precisionFactor)};

        if (vert[0] == vert[1] || vert[1] == vert[2] || vert[0] == vert[2]) {
            continue;
        }

        for (int j = 0; j < 3; ++j) {
            int next = (j + 1) % 3;
            const auto &hash0 = vert[j];
            const auto &hash1 = vert[next];
            const auto &vj = vertexes[j];
            const auto &vn = vertexes[next];
            const auto hash = Edge(hash0, hash1);
            const auto reverseHash = Edge(hash1, hash0);
            auto iter = edgeData.constFind(reverseHash);
            if (iter != edgeData.constEnd()) {
                const auto &d = iter.value();
                auto dot = QVector3D::dotProduct(normal, std::get<2>(d));
                if (dot <= thresholdDot) {
                    vertices.push_back(vj);
                    vertices.push_back(vn);
                }
                edgeData.erase(iter);
            } else if (!edgeData.contains(hash)) {
                edgeData[hash] = std::make_tuple(indices[j], indices[next], normal);
            }
        }
    }
    for (const auto &d: edgeData) {
        auto v0 = getQVector3DCached(positionAttrib, std::get<0>(d), edgeVertices);
        auto v1 = getQVector3DCached(positionAttrib, std::get<1>(d), edgeVertices);
        vertices.push_back(v0);
        vertices.push_back(v1);
    }

    return vertices;
}

LineGeometry *EdgeMesh::lineGeometry() const {
    return dynamic_cast<LineGeometry *>(geometry());
}

const QList<QVector3D> &EdgeMesh::points() const {
    return lineGeometry()->points();
}

EdgeMesh::EdgeMesh(const Qt3DCore::QGeometry &geometry, float thresholdAngle, int precision, QNode *parent)
        : QGeometryRenderer(
        parent) {
    QAttribute *position = nullptr;
    QAttribute *indices = nullptr;
    for (auto attribute: geometry.attributes()) {
        if (attribute->attributeType() == QAttribute::VertexAttribute &&
            attribute->name() == QAttribute::defaultPositionAttributeName()) {
            position = attribute;
        } else if (attribute->attributeType() == QAttribute::IndexAttribute) {
            indices = attribute;
        }
    }
    if (position == nullptr) {
        return;
    }
    auto lineGeometry = new LineGeometry(this);
    QGeometryRenderer::setGeometry(lineGeometry);
    QGeometryRenderer::setPrimitiveType(Lines);
    auto updateLine = [lineGeometry, position, thresholdAngle, precision, indices]() {
        lineGeometry->setLinePoints(
                generateEdgeVertices(position, thresholdAngle, precision, indices)
        );
    };
    updateLine();
    //connect to buffer
    connect(position->buffer(), &Qt3DCore::QBuffer::dataChanged, this, updateLine,
            Qt::QueuedConnection);
}
