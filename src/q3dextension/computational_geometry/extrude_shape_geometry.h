//
// Created by merlin
//

#ifndef PMAXWELL_QT_EXTRUDE_SHAPE_GEOMETRY_H
#define PMAXWELL_QT_EXTRUDE_SHAPE_GEOMETRY_H

#include <Qt3DCore/QGeometry>
#include "curve.h"
#include "shape.h"

class ExtrudeShapeGeometry : public Qt3DCore::QGeometry {
Q_OBJECT

public:
    explicit ExtrudeShapeGeometry(Qt3DCore::QNode *parent = nullptr);

public:
    Curve *curve() const;

    Shape *shape() const;

public slots:
    void clearExtrude();

    void setCurve(const Curve *curve);

    void setShape(const Shape *shape);

signals:

    void curveChanged();

    void shapeChanged();

private:
    void init();

    void resetAttribute();

private slots:

    void update();

private:
    Curve *m_curve{};
    Shape *m_shape{};

    Qt3DCore::QAttribute *m_positionAttribute{};
    Qt3DCore::QAttribute *m_normalAttribute{};
    Qt3DCore::QAttribute *m_indexAttribute{};
    Qt3DCore::QBuffer *m_vertexBuffer{};
    Qt3DCore::QBuffer *m_indexBuffer{};
    Qt3DCore::QBuffer *m_normalBuffer{};
};


#endif
