//
// Created by merlin
//

#ifndef RENDER_NODE_COMPONENT_H
#define RENDER_NODE_COMPONENT_H

#include "behavior_component.h"
#include "render_node.h"

class RenderNodeComponent : public BehaviorComponent {
    Q_OBJECT

public:
    ~RenderNodeComponent() override = default;

    explicit RenderNodeComponent(RenderNode *renderNode);

    template<class T = RenderNode>
    T *nodeView() const {
        Q_ASSERT(m_renderNode);
        if (m_renderNode->isProxyNode()) {
            const auto proxy = qobject_cast<ProxyRenderNode *>(m_renderNode);
            return proxy->proxyRenderNode<RenderNode>()->asType<T *>();
        }
        return m_renderNode->asType<T *>();
    }

private:
    RenderNode *m_renderNode;
};


#endif //RENDER_NODE_COMPONENT_H

