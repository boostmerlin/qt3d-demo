//
// Created by merlin
//

#ifndef DRAW_OUTLINE_COMPONENT_H
#define DRAW_OUTLINE_COMPONENT_H

#include "render_node_component.h"

class DrawOutlineComponent final : public RenderNodeComponent {
    Q_OBJECT

public:
    explicit DrawOutlineComponent(RenderNode *renderNode);

    void setColor(QColor color);

protected:
    void onEnable() override;

    void onDisable() override;

private:
    QColor m_color = QColorConstants::Magenta;
};


#endif //DRAW_OUTLINE_COMPONENT_H

