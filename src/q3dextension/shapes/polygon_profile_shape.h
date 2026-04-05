#pragma once

#include "shape.h"

class PolygonProfileShape final : public Shape {
    Q_OBJECT

public:
    explicit PolygonProfileShape(QObject *parent = nullptr);

    [[nodiscard]] Polygon2D toPolygon() const override;
    void fromPolygon(const QPolygonF &polygon) override;
    [[nodiscard]] QPolygonF polygon() const;

private:
    QPolygonF m_polygon;
};
