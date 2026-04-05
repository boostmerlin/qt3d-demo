//
// Created by merlin
//

#ifndef PMAXWELL_QT_COMPARE_UTIL_H
#define PMAXWELL_QT_COMPARE_UTIL_H

#include <QVector2D>
#include <QPointF>

constexpr float FLOAT_EPSILON = 0.00001f;
constexpr double DOUBLE_EPSILON = 0.000000000001;

template<typename T>
using ENABLE_IF_NUMBER = std::enable_if_t<std::is_arithmetic_v<T>>;

template<typename T, typename=ENABLE_IF_NUMBER<T> >
struct Epsilon {
    static constexpr T value = 0;
};

template<>
struct Epsilon<float> {
    static constexpr float value = FLOAT_EPSILON;
};

template<>
struct Epsilon<double> {
    static constexpr double value = DOUBLE_EPSILON;
};

template<typename T, typename=ENABLE_IF_NUMBER<T> >
constexpr inline bool approxEqual(T a, T b, T epsilon = Epsilon<T>::value) {
    return qAbs(a - b) <= epsilon;
}

template<typename T, typename=ENABLE_IF_NUMBER<T> >
constexpr inline bool approxZero(T a, T epsilon = Epsilon<T>::value) {
    return approxEqual(a, T(0), epsilon);
}

template<typename T>
using ENABLE_IF_VECTOR = std::enable_if_t<std::is_same_v<QVector3D, T> || std::is_same_v<QVector2D, T> ||
                                          std::is_same_v<QPointF, T>>;

template<typename T, typename=ENABLE_IF_VECTOR<T> >
constexpr inline bool approxEqual(const T &a, const T &b,
                                  decltype(std::declval<T>().x()) epsilon = Epsilon<decltype(std::declval<T>().x()
                                  )>::value) {
    auto eq = approxEqual(a.x(), b.x(), epsilon) && // x
              approxEqual(a.y(), b.y(), epsilon); // y
    if constexpr (std::is_same_v<QVector3D, T>) {
        return eq && approxEqual(a.z(), b.z(), epsilon); // z
    } else {
        return eq;
    }
}


template<typename T, typename=ENABLE_IF_VECTOR<T> >
constexpr inline bool approxZero(const T &a,
                                 decltype(std::declval<T>().x()) epsilon = Epsilon<decltype(std::declval<T>().x()
                                 )>::value) {
    auto eq = approxZero(a.x(), epsilon) && // x
              approxZero(a.y(), epsilon); // y
    if constexpr (std::is_same_v<QVector3D, T>) {
        return eq && approxZero(a.z(), epsilon); // z
    } else {
        return eq;
    }
}

#endif //PMAXWELL_QT_COMPARE_UTIL_H
