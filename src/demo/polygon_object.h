#pragma once

#include <QVector3D>

#include "scene_object.h"

using PolygonVertices = QList<QVector3D>;
Q_DECLARE_METATYPE(PolygonVertices)

class PolygonObject final : public SceneObject {
    Q_OBJECT

public:
    explicit PolygonObject(QObject *parent = nullptr);

    X_PROPERTY_4(PolygonVertices, vertices, vertices, setVertices)
    X_PROPERTY_4(float, height, height, setHeight)
    X_PROPERTY_4(uint, rings, rings, setRings)

public:
    [[nodiscard]] QString typeName() const override;
    [[nodiscard]] bool hasValidPolygon() const;

    void addRegularVertex();
    void addVertex(const QVector3D &point, int at = -1);
    void removeVertexAt(int at);
    void moveVertex(int from, int to);
};
