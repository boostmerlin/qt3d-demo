#include "primitive_object.h"

#include <QtGlobal>

namespace {
float clampDimension(float value)
{
    return qMax(value, PrimitiveObject::minimumDimensionValue());
}
}

PrimitiveObject::PrimitiveObject(QObject *parent)
    : SceneObject(parent)
{
}

QString PrimitiveObject::typeName() const
{
    return displayName(primitiveType());
}

PrimitiveObject *PrimitiveObject::parentPrimitive() const
{
    return qobject_cast<PrimitiveObject *>(parentSceneObject());
}

float PrimitiveObject::dimensionMinimum(int) const
{
    return minimumDimensionValue();
}

float PrimitiveObject::dimensionMaximum(int) const
{
    return 9999.0f;
}

QList<PrimitiveObject *> PrimitiveObject::childPrimitives() const
{
    QList<PrimitiveObject *> result;
    for (auto *child : childSceneObjects()) {
        if (auto *primitiveChild = qobject_cast<PrimitiveObject *>(child)) {
            result.append(primitiveChild);
        }
    }
    return result;
}

bool PrimitiveObject::isAncestorOf(const PrimitiveObject *object) const
{
    return SceneObject::isAncestorOf(object);
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
    case PrimitiveType::Line:
        return QStringLiteral("Line");
    case PrimitiveType::Ring:
        return QStringLiteral("Ring");
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

LineObject::LineObject(QObject *parent)
    : PrimitiveObject(parent)
{
    setColor(baseColor());
    setStartPoint(QVector3D(-0.8f, 0.0f, 0.0f));
    setEndPoint(QVector3D(0.8f, 0.0f, 0.0f));
    setLineType(LineGeometry::Line);
    observableChain(false);
}

PrimitiveObject::PrimitiveType LineObject::primitiveType() const
{
    return PrimitiveType::Line;
}

QColor LineObject::baseColor() const
{
    return {0x0f, 0x76, 0x6e};
}

int LineObject::editableDimensionCount() const
{
    return 6;
}

QString LineObject::dimensionLabel(int index) const
{
    switch (index) {
    case 0:
        return QStringLiteral("Start X");
    case 1:
        return QStringLiteral("Start Y");
    case 2:
        return QStringLiteral("Start Z");
    case 3:
        return QStringLiteral("End X");
    case 4:
        return QStringLiteral("End Y");
    case 5:
        return QStringLiteral("End Z");
    default:
        return {};
    }
}

float LineObject::dimensionValue(int index) const
{
    const QVector3D start = startPoint();
    const QVector3D end = endPoint();
    switch (index) {
    case 0:
        return start.x();
    case 1:
        return start.y();
    case 2:
        return start.z();
    case 3:
        return end.x();
    case 4:
        return end.y();
    case 5:
        return end.z();
    default:
        return 0.0f;
    }
}

float LineObject::dimensionMinimum(int) const
{
    return -9999.0f;
}

float LineObject::dimensionMaximum(int) const
{
    return 9999.0f;
}

void LineObject::setDimensionValue(int index, float value)
{
    QVector3D start = startPoint();
    QVector3D end = endPoint();
    switch (index) {
    case 0:
        start.setX(value);
        setStartPoint(start);
        break;
    case 1:
        start.setY(value);
        setStartPoint(start);
        break;
    case 2:
        start.setZ(value);
        setStartPoint(start);
        break;
    case 3:
        end.setX(value);
        setEndPoint(end);
        break;
    case 4:
        end.setY(value);
        setEndPoint(end);
        break;
    case 5:
        end.setZ(value);
        setEndPoint(end);
        break;
    default:
        break;
    }
}

RingObject::RingObject(QObject *parent)
    : PrimitiveObject(parent)
{
    setInnerRadiusValidator([](const float &value, const QObservableObject *owner) {
        const auto *ring = qobject_cast<const RingObject *>(owner);
        return ring && value >= PrimitiveObject::minimumDimensionValue() && value <= ring->outerRadius();
    });
    setOuterRadiusValidator([](const float &value, const QObservableObject *owner) {
        const auto *ring = qobject_cast<const RingObject *>(owner);
        return ring && value >= ring->innerRadius();
    });
    setLengthValidator([](const float &value, const QObservableObject *) {
        return value >= PrimitiveObject::minimumDimensionValue();
    });
    setStartAngleValidator([](const float &value, const QObservableObject *) {
        return value >= 0.0f && value <= 360.0f;
    });
    setEndAngleValidator([](const float &value, const QObservableObject *) {
        return value >= 0.0f && value <= 360.0f;
    });
    setColor(baseColor());
    setInnerRadius(0.3f);
    setOuterRadius(0.7f);
    setLength(0.35f);
    setStartAngle(0.0f);
    setEndAngle(360.0f);
    observableChain(false);
}

PrimitiveObject::PrimitiveType RingObject::primitiveType() const
{
    return PrimitiveType::Ring;
}

QColor RingObject::baseColor() const
{
    return {0x9b, 0x51, 0xe0};
}

int RingObject::editableDimensionCount() const
{
    return 5;
}

QString RingObject::dimensionLabel(int index) const
{
    switch (index) {
    case 0:
        return QStringLiteral("Inner Radius");
    case 1:
        return QStringLiteral("Outer Radius");
    case 2:
        return QStringLiteral("Length");
    case 3:
        return QStringLiteral("Start Angle");
    case 4:
        return QStringLiteral("End Angle");
    default:
        return {};
    }
}

float RingObject::dimensionValue(int index) const
{
    switch (index) {
    case 0:
        return innerRadius();
    case 1:
        return outerRadius();
    case 2:
        return length();
    case 3:
        return startAngle();
    case 4:
        return endAngle();
    default:
        return 0.0f;
    }
}

float RingObject::dimensionMinimum(int index) const
{
    return index >= 3 ? 0.0f : minimumDimensionValue();
}

float RingObject::dimensionMaximum(int index) const
{
    return index >= 3 ? 360.0f : 9999.0f;
}

void RingObject::setDimensionValue(int index, float value)
{
    switch (index) {
    case 0:
        setInnerRadius(qMin(clampDimension(value), outerRadius()));
        break;
    case 1:
        setOuterRadius(qMax(clampDimension(value), innerRadius()));
        break;
    case 2:
        setLength(clampDimension(value));
        break;
    case 3:
        setStartAngle(qBound(0.0f, value, 360.0f));
        break;
    case 4:
        setEndAngle(qBound(0.0f, value, 360.0f));
        break;
    default:
        break;
    }
}
