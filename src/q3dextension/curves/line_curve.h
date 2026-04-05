//
// Created by merlin
//

#ifndef PMAXWELL_QT_LINE_CURVE_H
#define PMAXWELL_QT_LINE_CURVE_H

#include "curve.h"

class LineCurve : public Curve {
    Q_OBJECT
public:
    explicit LineCurve(const QVector2D &p1 = QVector2D(0, 0), const QVector2D &p2 = QVector2D(0.5f, 0.5f));

    QVector3D getPoint(float t) const final;

    QVector3D getTangent(float t) const final;

    void setP1(const QVector2D &p1);
    void setP2(const QVector2D &p2);

    QVector2D getP1() const;
    QVector2D getP2() const;
private:
    QVector2D m_p1;
    QVector2D m_p2;
};


#endif //PMAXWELL_QT_LINE_CURVE_H
