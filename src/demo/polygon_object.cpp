#include "polygon_object.h"

#include <QtMath>
#include <QtGlobal>

namespace {
constexpr float defaultPolygonRadius = 0.9f;

QList<QVector3D> regularPolygonVertices(int sideCount,
                                        const QVector3D &center = QVector3D(0.0f, 0.0f, 0.0f),
                                        float radius = defaultPolygonRadius)
{
    QList<QVector3D> vertices;
    if (sideCount < 3) {
        return vertices;
    }

    vertices.reserve(sideCount);
    const float startAngle = -float(M_PI_2);
    const float step = float(2.0 * M_PI) / float(sideCount);
    for (int index = 0; index < sideCount; ++index) {
        const float angle = startAngle + step * float(index);
        vertices.append(QVector3D(center.x() + radius * std::cos(angle),
                                  center.y() + radius * std::sin(angle),
                                  center.z()));
    }
    return vertices;
}

QVector3D polygonCenter(const QList<QVector3D> &vertices)
{
    if (vertices.isEmpty()) {
        return QVector3D(0.0f, 0.0f, 0.0f);
    }

    QVector3D center(0.0f, 0.0f, 0.0f);
    for (const auto &vertex : vertices) {
        center += vertex;
    }
    return center / float(vertices.size());
}

float polygonRadius(const QList<QVector3D> &vertices, const QVector3D &center)
{
    if (vertices.isEmpty()) {
        return defaultPolygonRadius;
    }

    float totalDistance = 0.0f;
    for (const auto &vertex : vertices) {
        totalDistance += QVector2D(vertex.x() - center.x(), vertex.y() - center.y()).length();
    }

    const float radius = totalDistance / float(vertices.size());
    return radius > 0.001f ? radius : defaultPolygonRadius;
}
}

PolygonObject::PolygonObject(QObject *parent)
    : SceneObject(parent)
{
    setHeightValidator([](const float &value, const QObservableObject *) {
        return value >= 0.0f;
    });
    setRingsValidator([](const uint &value, const QObservableObject *) {
        return value != 1;
    });

    setColor(QColor(0xd9, 0x77, 0x06));
    setVertices(regularPolygonVertices(3));
    setHeight(0.5f);
    setRings(2);
    observableChain(false);
}

QString PolygonObject::typeName() const
{
    return QStringLiteral("Polygon");
}

bool PolygonObject::hasValidPolygon() const
{
    return vertices().size() >= 3;
}

void PolygonObject::addRegularVertex()
{
    const auto currentVertices = vertices();
    const int sideCount = qMax(3, currentVertices.size() + 1);
    const QVector3D center = polygonCenter(currentVertices);
    const float radius = polygonRadius(currentVertices, center);
    setVertices(regularPolygonVertices(sideCount, center, radius));
}

void PolygonObject::addVertex(const QVector3D &point, int at)
{
    auto currentVertices = vertices();
    const bool append = at < 0 || at >= currentVertices.size();
    if (append) {
        currentVertices.append(point);
    } else {
        currentVertices.insert(at, point);
    }
    setVertices(currentVertices);
}

void PolygonObject::removeVertexAt(int at)
{
    auto currentVertices = vertices();
    if (at < 0 || at >= currentVertices.size()) {
        return;
    }
    currentVertices.removeAt(at);
    setVertices(currentVertices);
}

void PolygonObject::moveVertex(int from, int to)
{
    auto currentVertices = vertices();
    if (from < 0 || from >= currentVertices.size() || to < 0 || to >= currentVertices.size() || from == to) {
        return;
    }
    currentVertices.move(from, to);
    setVertices(currentVertices);
}
