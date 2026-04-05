//
// Created by merlin
//

#include "line_curve.h"
#include "compare_util.h"

LineCurve::LineCurve(const QVector2D &p1, const QVector2D &p2) : m_p1(p1), m_p2(p2) {
    setAutoDivisions(false);
}

QVector3D LineCurve::getPoint(float t) const {
    t = qBound(0.0f, t, 1.0f);
    if (t == 1) {
        return m_p2.toVector3D();
    }
    return QVector3D(m_p1 + t * (m_p2 - m_p1));
}

QVector3D LineCurve::getTangent(float t) const {
    Q_UNUSED(t)
    return (m_p2 - m_p1).normalized().toVector3D();
}

void LineCurve::setP1(const QVector2D &p1) {
    if (approxEqual(m_p1, p1) || approxEqual(m_p2, p1)) {
        return;
    }
    m_p1 = p1;
    emit curveChanged();
}

void LineCurve::setP2(const QVector2D &p2) {
    if (approxEqual(m_p1, p2) || approxEqual(m_p2, p2)) {
        return;
    }
    m_p2 = p2;
    emit curveChanged();
}

QVector2D LineCurve::getP1() const {
    return m_p1;
}

QVector2D LineCurve::getP2() const {
    return m_p2;
}
