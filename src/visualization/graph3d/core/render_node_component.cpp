//
// Created by merlin
//

#include "render_node_component.h"

RenderNodeComponent::RenderNodeComponent(RenderNode *renderNode) : m_renderNode(renderNode) {
    Q_ASSERT(renderNode);
    renderNode->addComponent(this);
}

