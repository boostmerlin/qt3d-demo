//
// Created by merlin
//

#include <QVector3D>
#include "extrude_shape_geometry.h"
#include "geometry_util.h"

constexpr uint vertexSize = 3 * sizeof(float);

ExtrudeShapeGeometry::ExtrudeShapeGeometry(Qt3DCore::QNode *parent)
        : Qt3DCore::QGeometry(parent), m_shape(nullptr), m_curve(nullptr) {
    init();
    resetAttribute();
}

void ExtrudeShapeGeometry::setCurve(const Curve *curve) {
    Q_ASSERT(curve);
    if (m_curve == curve) {
        return;
    }
    if (m_curve) {
        disconnect(m_curve, &Curve::curveChanged, this, nullptr);
    }
    m_curve = const_cast<Curve *>(curve);
    if (!m_curve->parent()) {
        m_curve->setParent(this);
    }

    connect(m_curve, &Curve::curveChanged, this, &ExtrudeShapeGeometry::update);

    emit curveChanged();
    update();
}

void ExtrudeShapeGeometry::setShape(const Shape *shape) {
    Q_ASSERT(shape);
    if (m_shape == shape) {
        return;
    }
    if (m_shape) {
        disconnect(m_shape, &Shape::shapeChanged, this, nullptr);
    }
    m_shape = const_cast<Shape *>(shape);
    if (!m_shape->parent()) {
        m_shape->setParent(this);
    }
    if (m_shape) {
        connect(m_shape, &Shape::shapeChanged, this, &ExtrudeShapeGeometry::update);
    }
    emit shapeChanged();
    update();
}

struct ExtrudeResult {
    QByteArray vertexData;
    QByteArray indexData;
    QByteArray normalData;

    uint vertexCount() const {
        return uint(vertexData.size() / vertexSize);
    }

    uint indexCount() const {
        return uint(indexData.size() / sizeof(ushort));
    }

    uint normalCount() const {
        return uint(normalData.size() / vertexSize);
    }
};

static QVector3D addVector(QByteArray &vertexData, const QVector3D &v) {
    const auto pv = reinterpret_cast<const char *>(&v);
    vertexData.append(pv, sizeof(v));
    return v;
}

static void addShort(QByteArray &data, ushort i) {
    const auto pi = reinterpret_cast<const char *>(&i);
    data.append(pi, sizeof(i));
}

//todo: holes and dup vert in the future
static ExtrudeResult extrudeAlongCurve(const Shape *shape, const Curve *curve) {
    Q_ASSERT(shape);
    Q_ASSERT(curve);

    ExtrudeResult result;
    const Curve::CurveFrames curveFrames = curve->getFrames();
    auto shapeVertices = shape->toPolygon().toVector3DList(true);
    if (!isVerticesCounterClockWise(shapeVertices)) {
        std::reverse(shapeVertices.begin(), shapeVertices.end());
    }
    if (shapeVertices.size() < 3) {
        qWarning() << "[ExtrudeShapeGeometry] valid vert size < 3";
        return result;
    }
    QList<int> faceIndices;
    if (!triangulateV2(shapeVertices, faceIndices)) {
        return result;
    }
    QList<QVector3D> vertices;
    for (const auto &vert: shapeVertices) {
        auto n = curveFrames.normals[0];
        auto b = curveFrames.binormals[0];
        vertices.append(curveFrames.points[0] + b * vert.x() + n * vert.y());
    }

    //steps vert
    const auto divisions = curve->getDivisions();
    for (unsigned s = 1; s <= divisions; ++s) {
        for (auto vert: shapeVertices) {
            auto n = curveFrames.normals[s];
            auto b = curveFrames.binormals[s];
            vertices.append(curveFrames.points[s] + b * vert.x() + n * vert.y());
        }
    }
    const auto vertSize = qsizetype(vertices.size() * 3 * sizeof(float));
    result.vertexData.reserve((2 + divisions) * vertSize);

    const auto addVertexAt = [&result, &vertices](qsizetype index) {
        addShort(result.indexData, ushort(result.vertexData.size() / vertexSize));
        return addVector(result.vertexData, vertices[index]);
    };

    const auto addFace = [&result, &addVertexAt](qsizetype a, qsizetype b, qsizetype c) {
        auto va = addVertexAt(a);
        auto vb = addVertexAt(b);
        auto vc = addVertexAt(c);
        auto normal = QVector3D::normal(va, vb, vc).normalized();
        addVector(result.normalData, normal);
        addVector(result.normalData, normal);
        addVector(result.normalData, normal);
    };

    const auto addQuad = [&result, &addVertexAt](qsizetype a, qsizetype b, qsizetype c, qsizetype d) {
        auto va = addVertexAt(a);
        auto vb = addVertexAt(b);
        auto vd = addVertexAt(d);
        auto normal = QVector3D::normal(va, vb, vd).normalized();
        addVector(result.normalData, normal);
        addVector(result.normalData, normal);
        addVector(result.normalData, normal);

        vb = addVertexAt(b);
        auto vc = addVertexAt(c);
        vd = addVertexAt(d);
        normal = QVector3D::normal(vb, vc, vd).normalized();
        addVector(result.normalData, normal);
        addVector(result.normalData, normal);
        addVector(result.normalData, normal);
    };

    Q_ASSERT(faceIndices.size() % 3 == 0);
    for (int i = 0; i < faceIndices.size(); i += 3) {
        auto a = faceIndices[i + 0];
        auto b = faceIndices[i + 1];
        auto c = faceIndices[i + 2];
        addFace(a, b, c);
    }

    const auto stepsVertSize = shapeVertices.size() * divisions;
    for (int i = 0; i < faceIndices.size(); i += 3) {
        auto a = faceIndices[i + 2] + stepsVertSize;
        auto b = faceIndices[i + 1] + stepsVertSize;
        auto c = faceIndices[i + 0] + stepsVertSize;
        addFace(a, b, c);
    }
    qsizetype contourN = shapeVertices.length();
    //pre
    for (auto i = contourN - 1; i >= 0; i--) {
        auto j = i - 1;
        if (j < 0) {
            j = contourN - 1;
        }
        for (unsigned s = 0; s < divisions; ++s) {
            auto a = i + s * contourN;
            auto b = j + s * contourN;
            auto c = j + (s + 1) * contourN;
            auto d = i + (s + 1) * contourN;
            addQuad(a, b, c, d);
        }
    }
    return result;
}

void ExtrudeShapeGeometry::init() {
    m_vertexBuffer = new Qt3DCore::QBuffer(this);

    m_positionAttribute = new Qt3DCore::QAttribute(this);
    m_positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
    m_positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    m_positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    m_positionAttribute->setVertexSize(3);
    m_positionAttribute->setByteStride(vertexSize);
    m_positionAttribute->setBuffer(m_vertexBuffer);
    addAttribute(m_positionAttribute);

    m_normalBuffer = new Qt3DCore::QBuffer(this);
    m_normalAttribute = new Qt3DCore::QAttribute(this);
    m_normalAttribute->setName(Qt3DCore::QAttribute::defaultNormalAttributeName());
    m_normalAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
    m_normalAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
    m_normalAttribute->setVertexSize(3);
    m_normalAttribute->setByteStride(vertexSize);
    m_normalAttribute->setBuffer(m_normalBuffer);
    addAttribute(m_normalAttribute);

    m_indexBuffer = new Qt3DCore::QBuffer(this);
    m_indexAttribute = new Qt3DCore::QAttribute(this);
    m_indexAttribute->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
    m_indexAttribute->setVertexBaseType(Qt3DCore::QAttribute::UnsignedShort);
    m_indexAttribute->setBuffer(m_indexBuffer);
    addAttribute(m_indexAttribute);
}

void ExtrudeShapeGeometry::resetAttribute() {
    m_indexAttribute->setCount(0);
    m_normalAttribute->setCount(0);
    m_positionAttribute->setCount(0);
}

Curve *ExtrudeShapeGeometry::curve() const {
    return m_curve;
}

Shape *ExtrudeShapeGeometry::shape() const {
    return m_shape;
}

void ExtrudeShapeGeometry::update() {
    if (m_shape && m_curve) {
        const auto r = extrudeAlongCurve(m_shape, m_curve);
        m_vertexBuffer->setData(r.vertexData);
        m_indexBuffer->setData(r.indexData);
        m_normalBuffer->setData(r.normalData);

        m_positionAttribute->setCount(r.vertexCount());
        m_indexAttribute->setCount(r.indexCount());
        m_normalAttribute->setCount(r.normalCount());
    } else {
        resetAttribute();
    }
}
