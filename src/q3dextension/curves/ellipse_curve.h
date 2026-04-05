//
// Created by merlin
//

#ifndef PMAXWELL_QT_ELLIPSE_CURVE_H
#define PMAXWELL_QT_ELLIPSE_CURVE_H

#include "curve.h"

class EllipseCurve : public Curve {
Q_OBJECT

public:
    // angle 方向为逆时针
    // rotation 旋转方向为逆时针
    explicit EllipseCurve(const QVector2D &center = QVector2D(0, 0), float majorRadius = 1, float minorRadius = 0.5,
                          float startAngle = 0,
                          float sweepAngle = M_PI,
                          float rotation = 0);

    [[nodiscard]] QVector3D getPoint(float t) const final;

    //getter and setters
    QVector2D getCenter() const;

    void setCenter(const QVector2D &center);

    float getStartAngle() const;

    void setStartAngle(float startAngle);

    float getSweepAngle() const;

    void setSweepAngle(float sweepAngle);

    //set angle in degrees
    void setStartAngleInDegrees(float startAngle);

    void setSweepAngleInDegrees(float sweepAngle);

    float getStartAngleInDegrees() const;

    float getSweepAngleInDegrees() const;

    float getMajorRadius() const;

    void setMajorRadius(float majorRadius);

    float getMinorRadius() const;

    void setMinorRadius(float minorRadius);

    float getRotation() const;

    void setRotation(float rotation);

    float getRotationInDegrees() const;

    void setRotationInDegrees(float rotation);

protected:
    QVector2D m_center;
    float m_majorRadius;
    float m_minorRadius;
    float m_rotation;
    float m_startAngle;
    float m_sweepAngle;
};


#endif //PMAXWELL_QT_ELLIPSE_CURVE_H
