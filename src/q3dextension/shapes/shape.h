//
// Created by merlin
//

#ifndef PMAXWELL_QT_SHAPE_H
#define PMAXWELL_QT_SHAPE_H

#include <QObject>
#include <QList>

#include <QPolygonF>
#include <QVector3D>

class Polygon2D : public QPolygonF {
public:
    [[nodiscard]] QList<QVector3D> toVector3DList(bool optimizeVertices = false) const;
};

class Shape : public QObject {
    Q_OBJECT

public:
    explicit Shape(QObject *parent = nullptr);
    [[nodiscard]] virtual Polygon2D toPolygon() const = 0;
    virtual void fromPolygon(const QPolygonF &polygon){}
signals:
    void shapeChanged();
};

#endif //PMAXWELL_QT_SHAPE_H
