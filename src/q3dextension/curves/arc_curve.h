//
// Created by merlin
//

#ifndef PMAXWELL_QT_ARC_CURVE_H
#define PMAXWELL_QT_ARC_CURVE_H

#include "ellipse_curve.h"

class ArcCurve : public EllipseCurve {
Q_OBJECT

public:
    explicit ArcCurve(const QVector2D &center = QVector2D(0, 0), float radius = 1, float startAngle = 0,
                      float sweepAngle = M_PI);

    //getter and setters
    float getRadius() const;

    void setRadius(float radius);

    float getCurveLength() const final;

private:

    float getMajorRadius() const;

    bool setMajorRadius(float majorRadius);

    float getMinorRadius() const;

    bool setMinorRadius(float minorRadius);
};

#endif //PMAXWELL_QT_ARC_CURVE_H
