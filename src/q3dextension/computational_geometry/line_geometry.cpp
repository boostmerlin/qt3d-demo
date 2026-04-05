
#include "line_geometry.h"

constexpr uint vertexSize = 3 * sizeof(float);
constexpr int minLoopVertices = 3;

LineGeometry::LineGeometry(Qt3DCore::QNode *parent) : Qt3DCore::QGeometry(parent) {
    m_vertexBuffer = new Qt3DCore::QBuffer(this);
    m_positionAttribute = new Qt3DCore::QAttribute(this);
    m_positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    m_positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    m_positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    m_positionAttribute->setVertexSize(3);
    m_positionAttribute->setBuffer(m_vertexBuffer);
    m_positionAttribute->setByteStride(vertexSize);
    addAttribute(m_positionAttribute);
    setNormalAttributeEnable(true);

    setLinePoints({QVector3D(0, 0, 0), QVector3D(1, 1, 1)});
}

void LineGeometry::setNormalAttributeEnable(bool enable) {
    if (enable) {
        m_normalAttribute = new Qt3DCore::QAttribute(this);
        m_normalAttribute->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
        m_normalAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        m_normalAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
        m_normalAttribute->setVertexSize(3);
        m_normalAttribute->setBuffer(m_vertexBuffer);
        m_normalAttribute->setByteStride(vertexSize);
        addAttribute(m_normalAttribute);
    } else {
        removeAttribute(m_normalAttribute);
        m_normalAttribute->deleteLater();
        m_normalAttribute = nullptr;
    }
}

void LineGeometry::update() {
    m_vertexBuffer->setData(generateVertices(m_points.size()));
    m_positionAttribute->setCount(m_points.size());
    m_normalAttribute->setCount(m_points.size());
}

LineGeometry::LineType LineGeometry::lineType() const {
    return m_lineType;
}

const QList<QVector3D> &LineGeometry::points() const {
    return m_points;
}

void LineGeometry::setLineType(LineGeometry::LineType lineType) {
    if (lineType != m_lineType) {
        auto processLoop = [this](bool loop) {
            if (m_lineType == LineLoop && loopPoints(loop)) {
                update();
            }
        };
        processLoop(false);
        m_lineType = lineType;
        processLoop(true);
        emit lineTypeChanged(m_lineType);
    }
}

QByteArray LineGeometry::generateVertices(uint count) {
    QByteArray data;
    const uint size = count * vertexSize;
    data.resize(size);
    auto *vptr = reinterpret_cast<float *>(data.data());
    for (uint i = 0; i < count; ++i) {
        const QVector3D &point = m_points.at(i);
        *vptr++ = point.x();
        *vptr++ = point.y();
        *vptr++ = point.z();
    }
    Q_ASSERT(reinterpret_cast<char *>(vptr) == data.data() + size);
    return data;
}

void LineGeometry::setLinePoints(const QList<QVector3D> &points) {
    m_points = points;
    if (m_lineType == LineLoop) {
        loopPoints(true);
    }
    update();
}

void LineGeometry::addPoint(const QVector3D &point, int at) {
    bool last = at < 0 || at == m_points.size();
    bool loopAndLast = m_lineType == LineLoop && last;
    if (loopAndLast) {
        loopPoints(false);
    }
    if (last) {
        m_points.append(point);
    } else {
        m_points.insert(at, point);
    }
    if (loopAndLast) {
        loopPoints(true);
    }
    update();
}

void LineGeometry::removePointAt(int at) {
    if (at >= m_points.size()) {
        return;
    }
    bool last = at < 0 || at == m_points.size() - 1;
    bool loopAndLast = m_lineType == LineLoop && last;
    if (loopAndLast) {
        loopPoints(false);
    }
    if (last) {
        m_points.removeLast();
    } else {
        m_points.removeAt(at);
    }
    if (loopAndLast) {
        loopPoints(true);
    }
    update();
}

void LineGeometry::setLinePoints(QList<QVector3D> &&points) {
    m_points = std::move(points);
    if (m_lineType == LineLoop) {
        loopPoints(true);
    }
    update();
}

bool LineGeometry::loopPoints(bool loop) {
    if (loop) {
        if (m_points.size() >= minLoopVertices) {
            m_points.append(m_points.at(0));
            return true;
        }
    } else {
        if (m_points.size() >= minLoopVertices + 1) {
            m_points.removeLast();
            return true;
        }
    }
    return false;
}




