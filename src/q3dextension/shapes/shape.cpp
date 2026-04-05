//
// Created by merlin
//

#include "shape.h"
#include "compare_util.h"
#include "geometry_util.h"

QList<QVector3D> Polygon2D::toVector3DList(bool optimizeVertices) const {
    auto toV3 = [](const QPointF &point) {
        return QVector3D(float(point.x()), float(point.y()), 0);
    };
    if (optimizeVertices) {
        return removeRedundantElements<QVector3D, QPointF>(this->toList(), toV3);
    } else {
        QList<QVector3D> vertices;
        std::transform(constBegin(), constEnd(), std::back_inserter(vertices), toV3);
        return vertices;
    }
}

Shape::Shape(QObject *parent) : QObject(parent) {
}

