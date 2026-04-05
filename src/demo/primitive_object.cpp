#include "primitive_object.h"

#include <QtGlobal>

namespace {
float clampDimension(float value)
{
    return qMax(value, PrimitiveObject::minimumDimensionValue());
}
}

PrimitiveObject::PrimitiveObject(QObject *parent)
    : QObservableObject(parent)
{
    setPosition(QVector3D(0.0f, 0.0f, 0.0f));
    setRotation(QVector3D(0.0f, 0.0f, 0.0f));
    setColor(QColorConstants::White);
}

QString PrimitiveObject::typeName() const
{
    return displayName(primitiveType());
}

PrimitiveObject *PrimitiveObject::parentPrimitive() const
{
    return qobject_cast<PrimitiveObject *>(parent());
}

QList<PrimitiveObject *> PrimitiveObject::childPrimitives() const
{
    QList<PrimitiveObject *> result;
    const auto objectChildren = children();
    for (auto *child : objectChildren) {
        if (auto *primitiveChild = qobject_cast<PrimitiveObject *>(child)) {
            result.append(primitiveChild);
        }
    }
    return result;
}

bool PrimitiveObject::isAncestorOf(const PrimitiveObject *object) const
{
    auto *cursor = object ? object->parentPrimitive() : nullptr;
    while (cursor) {
        if (cursor == this) {
            return true;
        }
        cursor = cursor->parentPrimitive();
    }
    return false;
}

QString PrimitiveObject::displayName(PrimitiveType type)
{
    switch (type) {
    case PrimitiveType::Box:
        return QStringLiteral("Box");
    case PrimitiveType::Sphere:
        return QStringLiteral("Sphere");
    case PrimitiveType::Cylinder:
        return QStringLiteral("Cylinder");
    case PrimitiveType::Cone:
        return QStringLiteral("Cone");
    }
    return QStringLiteral("Primitive");
}

BoxObject::BoxObject(QObject *parent)
    : PrimitiveObject(parent)
{
    setColor(baseColor());
    setWidth(1.4f);
    setHeight(1.0f);
    setDepth(0.8f);
    observableChain(false);
}

QVector3D BoxObject::getExtents() const {
    return {width(), height(), depth()};
}

PrimitiveObject::PrimitiveType BoxObject::primitiveType() const
{
    return PrimitiveType::Box;
}

QColor BoxObject::baseColor() const
{
    return {0x2f, 0x80, 0xed};
}

int BoxObject::editableDimensionCount() const
{
    return 3;
}

QString BoxObject::dimensionLabel(int index) const
{
    switch (index) {
    case 0:
        return QStringLiteral("Width");
    case 1:
        return QStringLiteral("Height");
    case 2:
        return QStringLiteral("Depth");
    default:
        return {};
    }
}

float BoxObject::dimensionValue(int index) const
{
    switch (index) {
    case 0:
        return width();
    case 1:
        return height();
    case 2:
        return depth();
    default:
        return 0.0f;
    }
}

void BoxObject::setDimensionValue(int index, float value)
{
    value = clampDimension(value);
    switch (index) {
    case 0:
        setWidth(value);
        break;
    case 1:
        setHeight(value);
        break;
    case 2:
        setDepth(value);
        break;
    default:
        break;
    }
}

SphereObject::SphereObject(QObject *parent)
    : PrimitiveObject(parent)
{
    setColor(baseColor());
    setRadius(0.7f);
    observableChain(false);
}

PrimitiveObject::PrimitiveType SphereObject::primitiveType() const
{
    return PrimitiveType::Sphere;
}

QColor SphereObject::baseColor() const
{
    return {0x27, 0xae, 0x60};
}

int SphereObject::editableDimensionCount() const
{
    return 1;
}

QString SphereObject::dimensionLabel(int index) const
{
    return index == 0 ? QStringLiteral("Radius") : QString();
}

float SphereObject::dimensionValue(int index) const
{
    return index == 0 ? radius() : 0.0f;
}

void SphereObject::setDimensionValue(int index, float value)
{
    if (index == 0) {
        setRadius(clampDimension(value));
    }
}

CylinderObject::CylinderObject(QObject *parent)
    : PrimitiveObject(parent)
{
    setColor(baseColor());
    setRadius(0.5f);
    setHeight(1.6f);
    observableChain(false);
}

PrimitiveObject::PrimitiveType CylinderObject::primitiveType() const
{
    return PrimitiveType::Cylinder;
}

QColor CylinderObject::baseColor() const
{
    return {0xf2, 0x99, 0x4a};
}

int CylinderObject::editableDimensionCount() const
{
    return 2;
}

QString CylinderObject::dimensionLabel(int index) const
{
    switch (index) {
    case 0:
        return QStringLiteral("Radius");
    case 1:
        return QStringLiteral("Height");
    default:
        return {};
    }
}

float CylinderObject::dimensionValue(int index) const
{
    switch (index) {
    case 0:
        return radius();
    case 1:
        return height();
    default:
        return 0.0f;
    }
}

void CylinderObject::setDimensionValue(int index, float value)
{
    value = clampDimension(value);
    switch (index) {
    case 0:
        setRadius(value);
        break;
    case 1:
        setHeight(value);
        break;
    default:
        break;
    }
}

ConeObject::ConeObject(QObject *parent)
    : PrimitiveObject(parent)
{
    setColor(baseColor());
    setRadius(0.6f);
    setHeight(1.6f);
    observableChain(false);
}

PrimitiveObject::PrimitiveType ConeObject::primitiveType() const
{
    return PrimitiveType::Cone;
}

QColor ConeObject::baseColor() const
{
    return {0xeb, 0x57, 0x57};
}

int ConeObject::editableDimensionCount() const
{
    return 2;
}

QString ConeObject::dimensionLabel(int index) const
{
    switch (index) {
    case 0:
        return QStringLiteral("Radius");
    case 1:
        return QStringLiteral("Height");
    default:
        return {};
    }
}

float ConeObject::dimensionValue(int index) const
{
    switch (index) {
    case 0:
        return radius();
    case 1:
        return height();
    default:
        return 0.0f;
    }
}

void ConeObject::setDimensionValue(int index, float value)
{
    value = clampDimension(value);
    switch (index) {
    case 0:
        setRadius(value);
        break;
    case 1:
        setHeight(value);
        break;
    default:
        break;
    }
}
