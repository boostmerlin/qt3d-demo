//
// Created by merlin
//

#ifndef PMAXWELL_QT_TRAPEZOID_SHAPE_H
#define PMAXWELL_QT_TRAPEZOID_SHAPE_H

#include "shape.h"

class TrapezoidShape : public Shape {
Q_OBJECT

public:
    explicit TrapezoidShape(QObject* parent = nullptr);

    bool setDownsideLength(double length);

    double getDownsideLength() const;

    QPointF getDownsideAnchor() const;

    QPointF getUpsideAnchor() const;

    double getUpsideLength() const;

    bool setDownsideAnchor(const QPointF &anchor);

    bool setUpsideLength(double length);

    bool setUpsideAnchor(const QPointF &anchor);

    [[nodiscard]] Polygon2D toPolygon() const override;
    void fromPolygon(const QPolygonF &polygon) override;

    //计算属性
    bool isTrapezoid() const;

    double getHeight() const;

    float getDownsideLeftAngleInDegrees() const;

    float getDownsideRightAngleInDegrees() const;

    float getDownsideLeftAngle() const;

    float getDownsideRightAngle() const;

    bool setDownsideLeftAngleInDegrees(float angle);

    bool setDownsideRightAngleInDegrees(float angle);

    bool setDownsideLeftAngle(float angle);

    bool setDownsideRightAngle(float angle);

private:
    double m_upsideLength = 0.2;
    QPointF m_upsideAnchor = QPointF(0, 0.3);

    double m_downsideLength = 0.3;
    QPointF m_downsideAnchor = QPointF(0, 0);
};

#endif //PMAXWELL_QT_TRAPEZOID_SHAPE_H
