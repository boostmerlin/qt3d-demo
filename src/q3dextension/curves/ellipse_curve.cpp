//
// Created by merlin
//

#include "ellipse_curve.h"
#include "geometry_util.h"
#include "compare_util.h"


EllipseCurve::EllipseCurve(const QVector2D &center, float majorRadius, float minorRadius, float startAngle,
                           float sweepAngle, float rotation) : m_center(center), m_majorRadius(majorRadius),
                                                               m_minorRadius(minorRadius), m_startAngle(startAngle),
                                                               m_sweepAngle(sweepAngle), m_rotation(rotation) {
}

QVector3D EllipseCurve::getPoint(float t) const {
    const float sweepAngle = normaliseAngle(m_sweepAngle);
    const auto angle = normaliseAngle(m_startAngle) + sweepAngle * t;
    auto x = m_majorRadius * qCos(angle);
    auto y = m_minorRadius * qSin(angle);
    if (m_rotation != 0) {
        const auto cosR = qCos(m_rotation);
        const auto sinR = qSin(m_rotation);
        const auto x1 = x * cosR - y * sinR;
        y = x * sinR + y * cosR;
        x = x1;
    }
    return {x + m_center.x(), y + m_center.y(), 0};
}

QVector2D EllipseCurve::getCenter() const {
    return m_center;
}

void EllipseCurve::setCenter(const QVector2D &center) {
    if (m_center == center) {
        return;
    }
    m_center = center;
    emit curveChanged();
}

float EllipseCurve::getMajorRadius() const {
    return m_majorRadius;
}

void EllipseCurve::setMajorRadius(float majorRadius) {
    Q_ASSERT_X(majorRadius > 0, "EllipseCurve::setMajorRadius", "majorRadius must be greater than 0");
    if (approxEqual(m_majorRadius, majorRadius)) {
        return;
    }
    m_majorRadius = majorRadius;
    emit curveChanged();
}

float EllipseCurve::getMinorRadius() const {
    return m_minorRadius;
}

void EllipseCurve::setMinorRadius(float minorRadius) {
    Q_ASSERT_X(minorRadius > 0, "EllipseCurve::setMinorRadius", "minorRadius must be greater than 0");
    if (approxEqual(m_minorRadius, minorRadius)) {
        return;
    }
    m_minorRadius = minorRadius;
    emit curveChanged();
}

void EllipseCurve::setRotation(float rotation) {
    if (m_rotation == rotation) {
        return;
    }
    m_rotation = rotation;
    emit curveChanged();
}

float EllipseCurve::getRotation() const {
    return m_rotation;
}

float EllipseCurve::getSweepAngleInDegrees() const {
    return qRadiansToDegrees(m_sweepAngle);
}

float EllipseCurve::getStartAngleInDegrees() const {
    return qRadiansToDegrees(m_startAngle);
}

void EllipseCurve::setSweepAngleInDegrees(float sweepAngle) {
    setSweepAngle(qDegreesToRadians(sweepAngle));
}

void EllipseCurve::setStartAngleInDegrees(float startAngle) {
    setStartAngle(qDegreesToRadians(startAngle));
}

void EllipseCurve::setSweepAngle(float sweepAngle) {
    if (approxEqual(m_sweepAngle, sweepAngle)) {
        return;
    }
    m_sweepAngle = sweepAngle;
    emit curveChanged();
}

float EllipseCurve::getSweepAngle() const {
    return m_sweepAngle;
}

void EllipseCurve::setStartAngle(float startAngle) {
    if (approxEqual(m_startAngle, startAngle)) {
        return;
    }
    m_startAngle = startAngle;
    emit curveChanged();
}

float EllipseCurve::getStartAngle() const {
    return m_startAngle;
}

float EllipseCurve::getRotationInDegrees() const {
    return qRadiansToDegrees(m_rotation);
}

void EllipseCurve::setRotationInDegrees(float rotation) {
    setRotation(qDegreesToRadians(rotation));
}
