//
// Created by merlin
//
#ifndef BEZIER_CURVE_TPP
#define BEZIER_CURVE_TPP

#include "bezier_curve.h"
#include "compare_util.h"

template<typename Ty, uint N>
int BezierCurve<Ty, N>::controlPointCount() {
    return getDegrees() + 1;
}

template<typename Ty, uint N>
uint BezierCurve<Ty, N>::getDegrees() const {
    return N;
}


template<typename Ty, uint N>
template<typename It>
bool BezierCurve<Ty, N>::anyPointsChanged(It begin, It end) {
    Q_ASSERT(std::distance(begin, end) <= N + 1);
    auto old = std::cbegin(m_controlPoints);
    for (auto it = begin; it != end; ++it) {
        if (!approxEqual(*it, *old++)) {
            return false;
        }
    }
    return true;
}

template<typename Ty, uint N>
std::array<Ty, N + 1> BezierCurve<Ty, N>::deCasteljau(float t, uint iterations) const {
    Q_ASSERT(iterations <= N);
    std::array<Ty, N + 1> points;
    std::copy(std::cbegin(m_controlPoints), std::cend(m_controlPoints), points.begin());
    for (uint r = 1; r <= iterations; ++r) {
        for (uint i = 0; i <= N - r; ++i) {
            points[i] = (1 - t) * points[i] + t * points[i + 1];
        }
    }
    return points;
}

template<typename Ty, uint N>
constexpr uint BezierCurve<Ty, N>::calcArrayLength(uint iterations) {
    return N - iterations + 1;
}

template<typename Ty, uint N>
Ty BezierCurve<Ty, N>::getControlPoint(uint index) const {
    Q_ASSERT(index <= N);
    return m_controlPoints[index];
}

template<typename Ty, uint N>
void BezierCurve<Ty, N>::setControlPoints(const QList<Ty> &points) {
    int offset = 0;
    if (points.size() > N + 1) {
        offset = points.size() - N - 1;
    }
    if (anyPointsChanged(points.begin(), points.end() - offset)) {
        return;
    }
    //0-1-2...N-N+1-N+2-end
    std::copy(points.begin(), points.end() - offset, std::begin(m_controlPoints));
    emit Curve::curveChanged();
}

template<typename Ty, uint N>
bool BezierCurve<Ty, N>::setControlPoint(uint index, const Ty &point) {
    if (index > N) {
        return false;
    }
    if (approxEqual(m_controlPoints[index], point)) {
        return true;
    }
    m_controlPoints[index] = point;
    emit Curve::curveChanged();
    return true;
}

template<typename Ty, uint N>
QVector3D BezierCurve<Ty, N>::getTangent(float t) const {
    const auto points = deCasteljau(t, N - 1);
    constexpr uint len = calcArrayLength(N - 1);
    Q_ASSERT(len == 2);
    return QVector3D(points[len - 1] - points[len - 2]).normalized();
}

template<typename Ty, uint N>
QVector3D BezierCurve<Ty, N>::getPoint(float t) const {
    const auto points = deCasteljau(t, N);
    return QVector3D(points[calcArrayLength(N) - 1]);
}

template<typename Ty, uint N>
BezierCurve<Ty, N>::BezierCurve(const std::array<Ty, N + 1> &points) {
    std::copy(points.begin(), points.end(), std::begin(m_controlPoints));
}
#endif // BEZIER_CURVE_TPP
