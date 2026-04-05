#include "arc_curve.h"
#include "compare_util.h"

ArcCurve::ArcCurve(const QVector2D &center, float radius, float startAngle, float sweepAngle) : EllipseCurve(center,
                                                                                                             radius,
                                                                                                             radius,
                                                                                                             startAngle,
                                                                                                             sweepAngle) {
}

float ArcCurve::getRadius() const {
    Q_ASSERT_X(EllipseCurve::getMajorRadius() == EllipseCurve::getMinorRadius(), "ArcCurve::getRadius",
               "major radius and minor radius should be equal");
    return EllipseCurve::getMajorRadius();
}

void ArcCurve::setRadius(float radius) {
    Q_ASSERT_X(radius > 0, "ArcCurve::setRadius", "radius must be greater than 0");
    if (approxEqual(m_majorRadius, radius)) {
        return;
    }
    m_majorRadius = m_minorRadius = radius;
    emit curveChanged();
}

float ArcCurve::getCurveLength() const {
    return getSweepAngle() * getRadius();
}

