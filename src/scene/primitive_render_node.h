#pragma once

#include <Qt3DExtras/QConeMesh>

#include "primitive_object.h"
#include "nodes/render_node.h"

class PrimitiveRenderNodeBase : public RenderNodeEntityMix<WithMaterial, Selectable> {
    Q_OBJECT

public:
    explicit PrimitiveRenderNodeBase();

    bool updateRenderData(const QObject *object) override;

protected:
    [[nodiscard]] const PrimitiveObject *primitive() const;
    template<typename T>
    [[nodiscard]] const T *primitiveAs() const
    {
        return qobject_cast<const T *>(m_primitive);
    }

    void applyTransform() const;
    void updateMaterial();

    void onSelect() noexcept override;
    void onDeselect() noexcept override;

private:
    [[nodiscard]] QColor displayColor() const;

    const PrimitiveObject *m_primitive{};
    bool m_selected = false;
};

class BoxPrimitiveRenderNode final : public PrimitiveRenderNodeBase {
    Q_OBJECT

public:
    BoxPrimitiveRenderNode();

    void onCreate() override;
    void onUpdate() override;
};

class SpherePrimitiveRenderNode final : public PrimitiveRenderNodeBase {
    Q_OBJECT

public:
    SpherePrimitiveRenderNode();

    void onCreate() override;
    void onUpdate() override;
};

class CylinderPrimitiveRenderNode final : public PrimitiveRenderNodeBase {
    Q_OBJECT

public:
    CylinderPrimitiveRenderNode();

    void onCreate() override;
    void onUpdate() override;
};

class ConePrimitiveRenderNode final : public PrimitiveRenderNodeBase {
    Q_OBJECT

public:
    ConePrimitiveRenderNode();

    void onCreate() override;
    void onUpdate() override;
};

class RingPrimitiveRenderNode final : public PrimitiveRenderNodeBase {
    Q_OBJECT

public:
    RingPrimitiveRenderNode();

    void onCreate() override;
    void onUpdate() override;
};

class LinePrimitiveRenderNode final : public PrimitiveRenderNodeBase {
    Q_OBJECT

public:
    LinePrimitiveRenderNode();

    void onCreate() override;
    void onUpdate() override;
};

class PrimitiveRenderNode {
public:
    DECL_NODE_CREATOR(object);
};
