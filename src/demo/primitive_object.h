#pragma once

#include <QColor>
#include <QVector3D>

#include "observable_object.h"

class PrimitiveObject : public QObservableObject {
    Q_OBJECT

public:
    enum class PrimitiveType {
        Box,
        Sphere,
        Cylinder,
        Cone,
    };
    Q_ENUM(PrimitiveType)

    explicit PrimitiveObject(QObject *parent = nullptr);

    X_PROPERTY_4(QString, name, name, setName)
    X_PROPERTY_4(QVector3D, position, position, setPosition)
    X_PROPERTY_4(QVector3D, rotation, rotation, setRotation)
    X_PROPERTY_4(QColor, color, color, setColor)

public:
    ~PrimitiveObject() override = default;

    [[nodiscard]] virtual PrimitiveType primitiveType() const = 0;
    [[nodiscard]] virtual QColor baseColor() const = 0;
    [[nodiscard]] virtual int editableDimensionCount() const = 0;
    [[nodiscard]] virtual QString dimensionLabel(int index) const = 0;
    [[nodiscard]] virtual float dimensionValue(int index) const = 0;
    virtual void setDimensionValue(int index, float value) = 0;

    [[nodiscard]] QString typeName() const;
    [[nodiscard]] PrimitiveObject *parentPrimitive() const;
    [[nodiscard]] QList<PrimitiveObject *> childPrimitives() const;
    [[nodiscard]] bool isAncestorOf(const PrimitiveObject *object) const;

    static constexpr float minimumDimensionValue() noexcept { return 0.05f; }
    static QString displayName(PrimitiveType type);
};

class BoxObject final : public PrimitiveObject {
    Q_OBJECT

public:
    explicit BoxObject(QObject *parent = nullptr);

    X_PROPERTY_4(float, width, width, setWidth)
    X_PROPERTY_4(float, height, height, setHeight)
    X_PROPERTY_4(float, depth, depth, setDepth)

public:
    QVector3D getExtents() const;
    [[nodiscard]] PrimitiveType primitiveType() const override;
    [[nodiscard]] QColor baseColor() const override;
    [[nodiscard]] int editableDimensionCount() const override;
    [[nodiscard]] QString dimensionLabel(int index) const override;
    [[nodiscard]] float dimensionValue(int index) const override;
    void setDimensionValue(int index, float value) override;
};

class SphereObject final : public PrimitiveObject {
    Q_OBJECT

public:
    explicit SphereObject(QObject *parent = nullptr);

    X_PROPERTY_4(float, radius, radius, setRadius)

public:
    [[nodiscard]] PrimitiveType primitiveType() const override;
    [[nodiscard]] QColor baseColor() const override;
    [[nodiscard]] int editableDimensionCount() const override;
    [[nodiscard]] QString dimensionLabel(int index) const override;
    [[nodiscard]] float dimensionValue(int index) const override;
    void setDimensionValue(int index, float value) override;
};

class CylinderObject final : public PrimitiveObject {
    Q_OBJECT

public:
    explicit CylinderObject(QObject *parent = nullptr);

    X_PROPERTY_4(float, radius, radius, setRadius)
    X_PROPERTY_4(float, height, height, setHeight)

public:
    [[nodiscard]] PrimitiveType primitiveType() const override;
    [[nodiscard]] QColor baseColor() const override;
    [[nodiscard]] int editableDimensionCount() const override;
    [[nodiscard]] QString dimensionLabel(int index) const override;
    [[nodiscard]] float dimensionValue(int index) const override;
    void setDimensionValue(int index, float value) override;
};

class ConeObject final : public PrimitiveObject {
    Q_OBJECT

public:
    explicit ConeObject(QObject *parent = nullptr);

    X_PROPERTY_4(float, radius, radius, setRadius)
    X_PROPERTY_4(float, height, height, setHeight)

public:
    [[nodiscard]] PrimitiveType primitiveType() const override;
    [[nodiscard]] QColor baseColor() const override;
    [[nodiscard]] int editableDimensionCount() const override;
    [[nodiscard]] QString dimensionLabel(int index) const override;
    [[nodiscard]] float dimensionValue(int index) const override;
    void setDimensionValue(int index, float value) override;
};
