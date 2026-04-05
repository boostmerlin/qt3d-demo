//
// Created by merlin
//

#pragma once

#include <QVector3D>
#include <QList>

bool isVerticesCounterClockWise(const QList<QVector3D> &vertices);

bool coplanar(const QVector3D &v, const QVector3D &plane, const QVector3D &normal);

double normaliseAngleInDegrees(double angle);

float normaliseAngleInDegrees(float angle);

bool triangulateV2(const QList<QVector3D> &vertices, QList<int> &triangles);

template<typename Out, typename In>
QList<Out> removeRedundantElements(const QList<In> &input, std::function<Out(const In &)> transformer = [](
        const In &in) { return static_cast<Out>(in); }) {
    QList<Out> output;
    auto size = input.size();
    for (int i = 0; i < size; ++i) {
        if (approxEqual(input.at(i), input.at((i + 1) % size))) {
            continue;
        }
        output.append(transformer(input.at(i)));
    }
    return output;
}

//fix 2pi problem
template<typename T>
constexpr inline T normaliseAngle(T angle) {
    const auto twoPI = T(2 * M_PI);
    while (angle < 0) {
        angle += twoPI;
    }
    while (angle > twoPI) {
        angle -= twoPI;
    }
    return angle;
}

double length(const QVector3D &v);
QVector3D normalize(const QVector3D &v);
