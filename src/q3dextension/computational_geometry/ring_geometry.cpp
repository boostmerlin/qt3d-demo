#include "ring_geometry.h"
#include <QtMath>

#include "q3dextension/utils/geometry_util.h"

constexpr uint vertexSize = (3 + 3) * sizeof(float);

int faceCount(int slices, int rings) {
    return 2 * ((slices * 2) * (rings - 1) + 2 * (rings - 1) //side
                + slices * 2); //cap
}

int vertexCount(int slices, int rings) {
    int n = (slices + 1) * (rings + 2);
    return 2 * (n + rings * 2);
}

RingGeometry::RingGeometry(Qt3DCore::QNode *parent) : Qt3DCore::QGeometry(parent), m_rings(2), m_slices(32),
                                                      m_outerRadius(0.5f), m_innerRadius(0.25f), m_length(0.5f),
                                                      m_startAngle(0.0f), m_endAngle(0.0f) {
    m_positionAttribute = new Qt3DCore::QAttribute(this);
    m_normalAttribute = new Qt3DCore::QAttribute(this);
    m_indexAttribute = new Qt3DCore::QAttribute(this);
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

    updateVertices();
    updateIndices();
}

void setVertex(float *&verticesPtr, float x, float y, float z, QVector3D n) {
    *verticesPtr++ = x;
    *verticesPtr++ = y;
    *verticesPtr++ = z;
    *verticesPtr++ = n.x();
    *verticesPtr++ = n.y();
    *verticesPtr++ = n.z();
}

void RingGeometry::createSidesVertices(float *&ptr) {
    int slices = getSlices();
    const float dd = m_length / float(m_rings - 1);
    const double dTheta = getArc() / double(slices);
    QList<float> notch;
    // Create the side vertices: m_rings * ((slices + 1)*2 + 2 + 2))
    for (int ring = 0; ring < m_rings; ++ring) {
        const float z = -m_length / 2.0f + float(ring) * dd;
        for (int slice = 0; slice <= slices; ++slice) {
            const float theta = float(slice * dTheta) + m_startAngle;
            const float ct = qCos(theta);
            const float st = qSin(theta);

            QVector3D n(ct, st, 0);
            n.normalize();

            const float ox = m_outerRadius * ct;
            const float oy = m_outerRadius * st;
            setVertex(ptr, ox, oy, z, n);
            const float ix = m_innerRadius * ct;
            const float iy = m_innerRadius * st;
            setVertex(ptr, ix, iy, z, -n);

            if (slice == 0 || slice == slices) {
                auto v = QVector3D(ox, oy, 0);
                QVector3D n2;
                if (slice == 0) {
                    n2 = QVector3D::normal(QVector3D(0, 0, 1), v);
                } else {
                    n2 = QVector3D::normal(v, QVector3D(0, 0, 1));
                }
                notch.append({ox, oy, z, n2.x(), n2.y(), n2.z()});
                notch.append({ix, iy, z, n2.x(), n2.y(), n2.z()});
            }
        }
    }
    for (float v: notch) {
        *ptr++ = v;
    }
}

void RingGeometry::createSidesIndices(ushort *&ptr) const {
    int slices = getSlices();
    for (int ring = 0; ring < m_rings - 1; ++ring) {
        const int ringStart = ring * (slices + 1) * 2;
        const int nextRingStart = (ring + 1) * (slices + 1) * 2;

        for (int slice = 0; slice < slices; slice++) {
            int currentSlice = slice * 2;
            int nextSlice = currentSlice + 2;

            *ptr++ = ringStart + currentSlice;
            *ptr++ = ringStart + nextSlice;
            *ptr++ = nextRingStart + currentSlice;
            *ptr++ = nextRingStart + currentSlice;
            *ptr++ = ringStart + nextSlice;
            *ptr++ = nextRingStart + nextSlice;

            currentSlice += 1, nextSlice += 1;
            *ptr++ = ringStart + currentSlice;
            *ptr++ = nextRingStart + currentSlice;
            *ptr++ = ringStart + nextSlice;
            *ptr++ = ringStart + nextSlice;
            *ptr++ = nextRingStart + currentSlice;
            *ptr++ = nextRingStart + nextSlice;
        }
    }
    int start = m_rings * (slices + 1) * 2;
    for (int ring = 0; ring < m_rings - 1; ++ring) {
        const int ringStart = ring * 4 + start;
        const int nextRingStart = (ring + 1) * 4 + start;
        //startAngle
        *ptr++ = ringStart;
        *ptr++ = nextRingStart;
        *ptr++ = ringStart + 1;
        *ptr++ = ringStart + 1;
        *ptr++ = nextRingStart;
        *ptr++ = nextRingStart + 1;

        //endAngle
        *ptr++ = ringStart + 2;
        *ptr++ = ringStart + 2 + 1;
        *ptr++ = nextRingStart + 2;
        *ptr++ = nextRingStart + 2;
        *ptr++ = ringStart + 2 + 1;
        *ptr++ = nextRingStart + 2 + 1;
    }
}

void RingGeometry::createDiscVertices(float *&ptr, float z) {
    const int slices = getSlices();
    const double dTheta = getArc() / double(slices);
    const float zz = (z < 0.0f) ? -1.0f : 1.0f;

    for (int slice = 0; slice <= slices; ++slice) {
        const auto theta = float(slice * dTheta) + m_startAngle;
        const float ct = qCos(theta);
        const float st = qSin(theta);

        float ox = m_outerRadius * ct;
        float oy = m_outerRadius * st;
        const QVector3D n = QVector3D(0, 0, zz);
        setVertex(ptr, ox, oy, z, n);
        float ix = m_innerRadius * ct;
        float iy = m_innerRadius * st;
        setVertex(ptr, ix, iy, z, n);
    }
}

void RingGeometry::createDiscIndices(ushort *&ptr, int start, bool swap) const {
    for (int slice = 0; slice < getSlices(); ++slice) {
        const int current = slice * 2 + start;
        const int nextSlice = current + 2;
        *ptr++ = current;   //0
        *ptr++ = nextSlice; //2
        *ptr++ = current + 1;//1
        if (swap) {
            qSwap(*(ptr - 1), *(ptr - 2));
        }
        *ptr++ = current + 1;
        *ptr++ = nextSlice;
        *ptr++ = nextSlice + 1;
        if (swap) {
            qSwap(*(ptr - 1), *(ptr - 2));
        }
    }
}

QByteArray RingGeometry::generateVertexData() {
    const int verticesCount = vertexCount(getSlices(), m_rings);

    QByteArray data;
    data.resize(vertexSize * verticesCount);
    auto *ptr = reinterpret_cast<float *>(data.data());

    createSidesVertices(ptr);
    createDiscVertices(ptr, m_length * 0.5f);
    createDiscVertices(ptr, -m_length * 0.5f);

    Q_ASSERT(reinterpret_cast<char *>(ptr) == data.data() + verticesCount * vertexSize);

    return data;
}

QByteArray RingGeometry::generateIndexData() {
    const int facesCount = faceCount(getSlices(), m_rings);
    const int indicesCount = facesCount * 3;
    const qsizetype size = (uint) sizeof(ushort) * indicesCount;
    Q_ASSERT(indicesCount < 65536);

    QByteArray data;
    data.resize(size);
    auto *indicesPtr = reinterpret_cast<ushort *>(data.data());

    createSidesIndices(indicesPtr);
    int start = m_rings * ((getSlices() + 1) * 2 + 2 + 2);
    createDiscIndices(indicesPtr, start, m_length <= 0);
    start += (getSlices() + 1) * 2;
    createDiscIndices(indicesPtr, start, -m_length <= 0);
    Q_ASSERT(reinterpret_cast<char *>(indicesPtr) == data.data() + size);

    return data;
}

void RingGeometry::setRings(int rings) {
    if (rings != m_rings && rings > 0) {
        m_rings = rings;
        updateIndices();
        updateVertices();
    }
}

void RingGeometry::setSlices(int slices) {
    if (slices != m_slices && slices > 0) {
        m_slices = slices;
        updateIndices();
        updateVertices();
    }
}

void RingGeometry::setInnerRadius(float radius) {
    Q_ASSERT(radius >= 0);
    if (radius > m_outerRadius) {
        radius = m_outerRadius;
    }
    if (radius != m_innerRadius) {
        m_innerRadius = radius;
        updateVertices();
    }
}

void RingGeometry::setOuterRadius(float radius) {
    Q_ASSERT(radius >= 0);
    if (radius < m_innerRadius) {
        radius = m_innerRadius;
    }
    if (radius != m_outerRadius) {
        m_outerRadius = radius;
        updateVertices();
    }
}

void RingGeometry::setLength(float length) {
    if (length != m_length) {
        m_length = length;
        updateVertices();
    }
}

void RingGeometry::setStartAngle(float angle) {
    if (m_startAngle != angle) {
        m_startAngle = angle;
        updateVertices();
    }
}

void RingGeometry::setEndAngle(float angle) {
    if (m_endAngle != angle) {
        m_endAngle = angle;
        updateVertices();
    }
}

void RingGeometry::setStartAngleInDegrees(float angle) {
    setStartAngle(qDegreesToRadians(angle));
}

void RingGeometry::setEndAngleInDegrees(float angle) {
    setEndAngle(qDegreesToRadians(angle));
}

void RingGeometry::updateVertices() {
    const int count = vertexCount(m_slices, m_rings);
    m_positionAttribute->setCount(count);
    m_normalAttribute->setCount(count);
    m_vertexBuffer->setData(generateVertexData());
}

void RingGeometry::updateIndices() {
    const int faces = faceCount(m_slices, m_rings);
    m_indexAttribute->setCount(faces * 3);
    m_indexBuffer->setData(generateIndexData());
}

//dynamic slice opt maybe
int RingGeometry::getSlices() const {
    return m_slices;
}

float RingGeometry::getArc() const {
    float radians = m_endAngle - m_startAngle;
    radians = float(normaliseAngle(radians));
    if (qFuzzyIsNull(radians)) {
        radians = float(2 * M_PI);
    }
    return float(radians);
}
