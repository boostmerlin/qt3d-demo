#include <QDebug>
#include "compare_util.h"
#include "trapezoid_shape.h"

TrapezoidShape::TrapezoidShape(QObject *parent) : Shape(parent) {

}
/*
 * B----A
 * /    \
 * C----D
 */
Polygon2D TrapezoidShape::toPolygon() const {
    Polygon2D polygon;
    double halfUpside = m_upsideLength / 2;
    double halfDownside = m_downsideLength / 2;
    QPointF anchor = m_upsideAnchor;
    polygon.append(QPointF(anchor.x() + halfUpside, anchor.y()));
    polygon.append(QPointF(anchor.x() - halfUpside, anchor.y()));

    anchor = m_downsideAnchor;
    polygon.append(QPointF(anchor.x() - halfDownside, anchor.y()));
    polygon.append(QPointF(anchor.x() + halfDownside, anchor.y()));

    return polygon;
}

void TrapezoidShape::fromPolygon(const QPolygonF &polygon) {
    if (polygon.size() != 4) {
        qWarning() << "The polygon must have 4 vertices.";
        return;
    }
    QPointF a = polygon.at(0);
    QPointF b = polygon.at(1);
    QPointF c = polygon.at(2);
    QPointF d = polygon.at(3);
    m_upsideAnchor = (a + b) / 2.0;
    m_downsideAnchor = (c + d) / 2.0;
    m_upsideLength = qAbs(a.x() - b.x());
    m_downsideLength = qAbs(c.x() - d.x());
    emit shapeChanged();
}

bool TrapezoidShape::isTrapezoid() const {
    return m_downsideLength != m_upsideLength && m_downsideLength > 0 && m_upsideLength > 0;
}

static float length(const QPointF &a) {
    return float(qSqrt(a.x() * a.x() + a.y() * a.y()));
}

static float angleOf(const QPointF &a, const QPointF &b) {
    return qAcos(float(QPointF::dotProduct(a, b) / (length(a) * length(b))));
}

float TrapezoidShape::getDownsideLeftAngle() const {
    const Polygon2D &vertices = toPolygon();
    QPointF b = vertices.at(1);
    QPointF c = vertices.at(2);
    QPointF d = vertices.at(3);
    return angleOf(b - c, d - c);
}

float TrapezoidShape::getDownsideRightAngle() const {
    const Polygon2D &vertices = toPolygon();
    QPointF a = vertices.at(0);
    QPointF c = vertices.at(2);
    QPointF d = vertices.at(3);
    return angleOf(a - d, c - d);
}

float TrapezoidShape::getDownsideRightAngleInDegrees() const {
    return qRadiansToDegrees(getDownsideRightAngle());
}

float TrapezoidShape::getDownsideLeftAngleInDegrees() const {
    return qRadiansToDegrees(getDownsideLeftAngle());
}

bool TrapezoidShape::setDownsideLeftAngleInDegrees(float angle) {
    return setDownsideLeftAngle(qDegreesToRadians(angle));
}

bool TrapezoidShape::setDownsideRightAngleInDegrees(float angle) {
    return setDownsideRightAngle(qDegreesToRadians(angle));
}

bool TrapezoidShape::setDownsideLeftAngle(float angle) {
    if (angle <= 0 || angle >= M_PI) {
        return false;
    }

    if (approxEqual(angle, getDownsideLeftAngle())) {
        return false;
    }

    const Polygon2D &vertices = toPolygon();
    QPointF a = vertices.at(0);
    QPointF c = vertices.at(2);
    QPointF d = vertices.at(3);
    float angleThreshHold = angleOf(a - c, d - c);
    if (angle > angleThreshHold) {
        //keep height unchanged
        double height = getHeight();
        double lx = height / qSin(angle) * qCos(angle);
        QPointF newA = QPointF(c.x() + lx, a.y());
        m_upsideAnchor = (newA + a) / 2.0;
        m_upsideLength = qAbs(a.x() - newA.x());
    } else {
        double l = m_downsideLength;
        double tanA = qTan(angle);
        double tanB = qTan(getDownsideRightAngle());
        double rx = l * tanA / (tanA + tanB);
        QPointF newA = QPointF(d.x() - rx, d.y() + tanB * rx);
        m_upsideAnchor = newA;
        m_upsideLength = 0;
    }
    emit shapeChanged();
    return true;
}

bool TrapezoidShape::setDownsideRightAngle(float angle) {
    if (angle <= 0 || angle >= M_PI) {
        return false;
    }

    if (approxEqual(angle, getDownsideRightAngle())) {
        return false;
    }

    const Polygon2D &vertices = toPolygon();
    QPointF b = vertices.at(1);
    QPointF c = vertices.at(2);
    QPointF d = vertices.at(3);
    float angleThreshHold = angleOf(b - d, c - d);
    if (angle > angleThreshHold) {
        //keep height unchanged
        double height = getHeight();
        double rx = height / qSin(angle) * qCos(angle);
        QPointF newB = QPointF(d.x() - rx, b.y());
        m_upsideAnchor = (newB + b) / 2.0;
        m_upsideLength = qAbs(newB.x() - b.x());
    } else {
        double l = m_downsideLength;
        double tanA = qTan(getDownsideLeftAngle());
        double tanB = qTan(angle);
        double lx = l * tanB / (tanA + tanB);
        QPointF newB = QPointF(c.x() + lx, c.y() + tanA * lx);
        m_upsideAnchor = newB;
        m_upsideLength = 0;
    }
    emit shapeChanged();
    return true;
}

double TrapezoidShape::getHeight() const {
    return m_upsideAnchor.y() - m_downsideAnchor.y();
}

bool TrapezoidShape::setDownsideLength(double length) {
    if (length < 0 || m_downsideLength == length) {
        return false;
    }

    if (qIsNull(m_upsideLength) && qIsNull(length)) {
        qWarning() << "Both sides of the trapezoid cannot be zero at the same time.";
        return false;
    }
    m_downsideLength = length;
    emit shapeChanged();
    return true;
}

bool TrapezoidShape::setDownsideAnchor(const QPointF &anchor) {
    if (anchor.y() == m_upsideAnchor.y()) {
        qWarning() << "The two sides of the trapezoid cannot be in same line.";
        return false;
    }
    if (m_downsideAnchor == anchor) {
        return false;
    }
    m_downsideAnchor = anchor;
    emit shapeChanged();
    return true;
}

bool TrapezoidShape::setUpsideLength(double length) {
    if (length < 0 || m_upsideLength == length) {
        return false;
    }
    if (qIsNull(m_downsideLength) && qIsNull(length)) {
        qWarning() << "Both sides of the trapezoid cannot be zero at the same time.";
        return false;
    }
    m_upsideLength = length;
    emit shapeChanged();
    return true;
}

bool TrapezoidShape::setUpsideAnchor(const QPointF &anchor) {
    if (anchor.y() == m_downsideAnchor.y()) {
        qWarning() << "The two sides of the trapezoid cannot be in same line.";
        return false;
    }
    if (m_upsideAnchor == anchor) {
        return false;
    }
    m_upsideAnchor = anchor;
    emit shapeChanged();
    return true;
}

double TrapezoidShape::getDownsideLength() const {
    return m_downsideLength;
}

QPointF TrapezoidShape::getDownsideAnchor() const {
    return m_downsideAnchor;
}

QPointF TrapezoidShape::getUpsideAnchor() const {
    return m_upsideAnchor;
}

double TrapezoidShape::getUpsideLength() const {
    return m_upsideLength;
}


