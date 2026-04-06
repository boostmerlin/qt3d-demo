#pragma once

#include "polygon_object.h"
#include "nodes/render_node.h"

class PolygonRenderNode final : public RenderNodeEntityMix<WithMaterial, Selectable> {
    Q_OBJECT

public:
    PolygonRenderNode();

    static RenderNode *create(const QObject *object);
    bool updateRenderData(const QObject *object) override;

    void onCreate() override;
    void onUpdate() override;

protected:
    void onSelect() noexcept override;
    void onDeselect() noexcept override;

private:
    void applyTransform() const;
    void updateMaterial();
    [[nodiscard]] QColor displayColor() const;

    const PolygonObject *m_polygon{};
    bool m_selected = false;
};
