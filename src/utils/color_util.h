//
// Created by merlin
//

#ifndef PMAXWELL_QT_COLOR_UTIL_H
#define PMAXWELL_QT_COLOR_UTIL_H

#include <QColor>

QColor operator*(const QColor &color, float factor);

QColor operator*(const QColor &color, const QColor &factor);

QColor operator+(const QColor &color, const QColor &factor);

QColor operator~(const QColor &color);

#endif //PMAXWELL_QT_COLOR_UTIL_H
