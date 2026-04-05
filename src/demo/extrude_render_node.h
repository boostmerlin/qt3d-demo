#pragma once

#include "extrude_object.h"
#include "core/render_node.h"
#include "q3dextension/curves/bezier_curve.h"
#include "q3dextension/curves/line_curve.h"
#include "q3dextension/shapes/polygon_profile_shape.h"

class ExtrudeRenderNode final : public RenderNodeEntityMix<WithMaterial, Selectable> {
    Q_OBJECT

public:
    ExtrudeRenderNode();

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
    void clearMesh() const;
    [[nodiscard]] QColor displayColor() const;

    const ExtrudeObject *m_extrude{};
    LineCurve *m_lineCurve{};
    CubicBezierCurve *m_bezierCurve{};
    PolygonProfileShape *m_profileShape{};
    bool m_selected = false;
};
