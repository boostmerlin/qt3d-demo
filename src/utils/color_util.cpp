//
// Created by merlin
//

#include "color_util.h"

QColor operator*(const QColor &color, float factor) {
    return QColor::fromRgbF(color.redF() * factor, color.greenF() * factor, color.blueF() * factor, color.alphaF());
}

QColor operator~(const QColor &color) {
    return QColor::fromRgbF(1.0f - color.redF(), 1.0f - color.greenF(), 1.0f - color.blueF(), color.alphaF());
}

QColor operator*(const QColor &color, const QColor &factor) {
    return QColor::fromRgbF(color.redF() * factor.redF(), color.greenF() * factor.greenF(),
                            color.blueF() * factor.blueF(), color.alphaF());
}

QColor operator+(const QColor &color, const QColor &factor) {
    return QColor::fromRgbF(color.redF() + factor.redF(), color.greenF() + factor.greenF(),
                            color.blueF() + factor.blueF(), color.alphaF());
}
