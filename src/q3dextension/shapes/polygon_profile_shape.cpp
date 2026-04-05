#include "polygon_profile_shape.h"

PolygonProfileShape::PolygonProfileShape(QObject *parent)
    : Shape(parent)
{
}

Polygon2D PolygonProfileShape::toPolygon() const
{
    Polygon2D polygon;
    for (const auto &point : m_polygon) {
        polygon.append(point);
    }
    return polygon;
}

void PolygonProfileShape::fromPolygon(const QPolygonF &polygon)
{
    if (m_polygon == polygon) {
        return;
    }
    m_polygon = polygon;
    emit shapeChanged();
}

QPolygonF PolygonProfileShape::polygon() const
{
    return m_polygon;
}
