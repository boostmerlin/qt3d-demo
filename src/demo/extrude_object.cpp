#include "extrude_object.h"

#include <QtMath>

#include "utils/compare_util.h"

namespace {
constexpr float DefaultProfileRadius = 0.45f;
constexpr uint DefaultLineDivisions = 1;
constexpr uint DefaultCubicBezierDivisions = 24;

int requiredPathControlPointCount(ExtrudeObject::PathType pathType)
{
    switch (pathType) {
    case ExtrudeObject::PathType::Line:
        return 2;
    case ExtrudeObject::PathType::CubicBezier:
        return 4;
    }
    return 0;
}

ExtrudePathControlPoints defaultPathControlPoints(ExtrudeObject::PathType pathType)
{
    switch (pathType) {
    case ExtrudeObject::PathType::Line:
        return {QVector2D(-1.0f, 0.0f), QVector2D(1.0f, 0.0f)};
    case ExtrudeObject::PathType::CubicBezier:
        return {
            QVector2D(-1.0f, 0.0f),
            QVector2D(-0.35f, 0.7f),
            QVector2D(0.35f, -0.7f),
            QVector2D(1.0f, 0.0f),
        };
    }
    return {};
}

uint defaultPathDivisions(ExtrudeObject::PathType pathType)
{
    switch (pathType) {
    case ExtrudeObject::PathType::Line:
        return DefaultLineDivisions;
    case ExtrudeObject::PathType::CubicBezier:
        return DefaultCubicBezierDivisions;
    }
    return DefaultLineDivisions;
}

ExtrudeProfileVertices regularProfileVertices(int sideCount,
                                              const QVector2D &center = QVector2D(0.0f, 0.0f),
                                              float radius = DefaultProfileRadius)
{
    ExtrudeProfileVertices vertices;
    if (sideCount < 3) {
        return vertices;
    }

    vertices.reserve(sideCount);
    const float startAngle = -float(M_PI_2);
    const float step = float(2.0 * M_PI) / float(sideCount);
    for (int index = 0; index < sideCount; ++index) {
        const float angle = startAngle + step * float(index);
        vertices.append(QVector2D(center.x() + radius * qCos(angle),
                                  center.y() + radius * qSin(angle)));
    }
    return vertices;
}

QVector2D profileCenter(const ExtrudeProfileVertices &vertices)
{
    if (vertices.isEmpty()) {
        return QVector2D(0.0f, 0.0f);
    }

    QVector2D center(0.0f, 0.0f);
    for (const auto &vertex : vertices) {
        center += vertex;
    }
    return center / float(vertices.size());
}

float profileRadius(const ExtrudeProfileVertices &vertices, const QVector2D &center)
{
    if (vertices.isEmpty()) {
        return DefaultProfileRadius;
    }

    float totalDistance = 0.0f;
    for (const auto &vertex : vertices) {
        totalDistance += (vertex - center).length();
    }

    const float radius = totalDistance / float(vertices.size());
    return radius > FLOAT_EPSILON ? radius : DefaultProfileRadius;
}
}

ExtrudeObject::ExtrudeObject(QObject *parent)
    : SceneObject(parent)
{
    setPathTypeValidator([](const PathType &value, const QObservableObject *) {
        return value == PathType::Line || value == PathType::CubicBezier;
    });
    setPathControlPointsValidator([](const ExtrudePathControlPoints &value, const QObservableObject *owner) {
        const auto *extrude = qobject_cast<const ExtrudeObject *>(owner);
        if (!extrude) {
            return false;
        }
        const int expectedCount = requiredPathControlPointCount(extrude->pathType());
        return value.size() == expectedCount;
    });
    setProfileVerticesValidator([](const ExtrudeProfileVertices &, const QObservableObject *) {
        return true;
    });
    setPathDivisionsValidator([](const uint &value, const QObservableObject *) {
        return value >= 1 && value <= 128;
    });

    setColor(QColor(0x0f, 0x76, 0xe4));
    setProfileVertices(regularProfileVertices(3));
    setPathType(PathType::Line);
    observableChain(false);
}

QString ExtrudeObject::typeName() const
{
    return QStringLiteral("Extrude");
}

bool ExtrudeObject::hasValidPath() const
{
    const auto points = pathControlPoints();
    if (points.size() != requiredPathControlPointCount(pathType())) {
        return false;
    }
    if (pathType() == PathType::Line) {
        return !approxEqual(points[0], points[1]);
    }
    return true;
}

bool ExtrudeObject::hasValidProfile() const
{
    return profileVertices().size() >= 3;
}

void ExtrudeObject::addRegularProfileVertex()
{
    const auto vertices = profileVertices();
    const int sideCount = qMax(3, vertices.size() + 1);
    const QVector2D center = profileCenter(vertices);
    const float radius = profileRadius(vertices, center);
    setProfileVertices(regularProfileVertices(sideCount, center, radius));
}

void ExtrudeObject::removeProfileVertexAt(int at)
{
    auto vertices = profileVertices();
    if (at < 0 || at >= vertices.size()) {
        return;
    }
    vertices.removeAt(at);
    setProfileVertices(vertices);
}

void ExtrudeObject::moveProfileVertex(int from, int to)
{
    auto vertices = profileVertices();
    if (from < 0 || from >= vertices.size() || to < 0 || to >= vertices.size() || from == to) {
        return;
    }
    vertices.move(from, to);
    setProfileVertices(vertices);
}

void ExtrudeObject::afterPropertySet(const char *name)
{
    if (IS_PROPERTY(name, pathType)) {
        beginGroupChange();
        setPathControlPoints(defaultPathControlPoints(pathType()));
        setPathDivisions(defaultPathDivisions(pathType()));
        endGroupChange();
    }
}
