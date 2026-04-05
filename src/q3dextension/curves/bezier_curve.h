//
// Created by merlin
//

#ifndef PMAXWELL_QT_BEZIER_CURVE_H
#define PMAXWELL_QT_BEZIER_CURVE_H

#include "curve.h"

template<class Ty>
class IBezierCurve : public Curve {
public:
    virtual bool setControlPoint(uint index, const Ty &point) = 0;

    virtual Ty getControlPoint(uint index) const = 0;

    virtual void setControlPoints(const QList<Ty> &points) = 0;

    //constexpr virtual is meaningless
    virtual uint getDegrees() const = 0;

    virtual int controlPointCount() = 0;
};

using BezierCurve2D = IBezierCurve<QVector2D>;
using BezierCurve3D = IBezierCurve<QVector3D>;

/*
 * 任意n阶空间贝塞尔曲线
 */
template<typename Ty, uint N>
class BezierCurve : public IBezierCurve<Ty> {
    static_assert(N > 0, "N must be greater than 0");
    static_assert(std::is_same_v<Ty, QVector2D> || std::is_same_v<Ty, QVector3D>, "Ty must be QVector2D or QVector3D");
public:
    BezierCurve() = default;

    explicit BezierCurve(const std::array<Ty, N + 1> &points);

    template<typename... Args>
    explicit BezierCurve(Args... args) : m_controlPoints{args...} {
        static_assert(sizeof...(Args) == N + 1u, "The number of control points must be N + 1");
    }

    [[nodiscard]] QVector3D getPoint(float t) const final;

    //减少一次额外的计算
    [[nodiscard]] QVector3D getTangent(float t) const final;

    bool setControlPoint(uint index, const Ty &point) final;

    Ty getControlPoint(uint index) const final;

    void setControlPoints(const QList<Ty> &points) final;

    uint getDegrees() const final;

    int controlPointCount() final;

private:
    constexpr static uint calcArrayLength(uint iterations);

    //德卡斯特里奥算法（De Casteljau）, 数值稳定性更好
    std::array<Ty, N + 1> deCasteljau(float t, uint iterations) const;

    template<typename InIt>
    bool anyPointsChanged(InIt begin, InIt end);

private:
    Ty m_controlPoints[N + 1];
};

//---definition here---

//二次贝塞尔曲线
using QuadraticBezierCurve = BezierCurve<QVector2D, 2>;
using QuadraticBezierCurve3D = BezierCurve<QVector3D, 2>;
//三次贝塞尔曲线
using CubicBezierCurve = BezierCurve<QVector2D, 3>;
using CubicBezierCurve3D = BezierCurve<QVector3D, 3>;
//四次贝塞尔曲线
using QuarticBezierCurve = BezierCurve<QVector2D, 4>;
using QuarticBezierCurve3D = BezierCurve<QVector3D, 4>;

#endif //PMAXWELL_QT_BEZIER_CURVE_H
