//
// Created by merlin
//

#ifndef PMAXWELL_QT_SELECT_EFFECT_COMPONENT_H
#define PMAXWELL_QT_SELECT_EFFECT_COMPONENT_H

#include "render_node_component.h"

class ColourNodeComponent final : public RenderNodeComponent {
    Q_OBJECT

public:
    enum EffectType {
        Tint,
        TintColor,
        Additive,
        FixedColor,
        InvertColor,
    };

    explicit ColourNodeComponent(RenderNode *renderNode);

    void setEffectType(EffectType effectType);

    void setColor(QColor color);

protected:
    void onEnable() override;

    void onDisable() override;
private:
    void updateMatColor(QColor color) const;

private:
    EffectType m_effectType = FixedColor;
    QColor m_color = QColor(222, 12, 22);
};


#endif //PMAXWELL_QT_SELECT_EFFECT_COMPONENT_H

