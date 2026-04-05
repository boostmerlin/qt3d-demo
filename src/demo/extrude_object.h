#pragma once

#include <QVector2D>

#include "scene_object.h"

using ExtrudePathControlPoints = QList<QVector2D>;
Q_DECLARE_METATYPE(ExtrudePathControlPoints)

using ExtrudeProfileVertices = QList<QVector2D>;

class ExtrudeObject final : public SceneObject {
    Q_OBJECT

public:
    enum class PathType {
        Line,
        CubicBezier,
    };
    Q_ENUM(PathType)

    explicit ExtrudeObject(QObject *parent = nullptr);

    X_PROPERTY_4(PathType, pathType, pathType, setPathType)
    X_PROPERTY_4(ExtrudePathControlPoints, pathControlPoints, pathControlPoints, setPathControlPoints)
    X_PROPERTY_4(ExtrudeProfileVertices, profileVertices, profileVertices, setProfileVertices)
    X_PROPERTY_4(uint, pathDivisions, pathDivisions, setPathDivisions)

public:
    [[nodiscard]] QString typeName() const override;
    [[nodiscard]] bool hasValidPath() const;
    [[nodiscard]] bool hasValidProfile() const;

    void addRegularProfileVertex();
    void removeProfileVertexAt(int at);
    void moveProfileVertex(int from, int to);

protected:
    void afterPropertySet(const char *name) override;
};
