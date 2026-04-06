//
// Created by merlin
//

#include "draw_outline_component.h"
#include "edge_render_node.h"

DrawOutlineComponent::DrawOutlineComponent(RenderNode *renderNode) : RenderNodeComponent(renderNode) {
    onEnable();
}

void DrawOutlineComponent::setColor(QColor color) {
    m_color = color;
}

void DrawOutlineComponent::onEnable() {
    const auto node = nodeView();
    if (auto edgeNode = node->findChild<EdgeRenderNode *>(); !edgeNode) {
        if (const auto mesh = node->getComponentInChildren<Qt3DRender::QGeometryRenderer>(false);
            mesh->primitiveType() == Qt3DRender::QGeometryRenderer::Triangles) {
            const auto geometry = mesh->geometry() == nullptr ? mesh->view()->geometry() : mesh->geometry();
            edgeNode = new EdgeRenderNode(m_color, geometry);
            edgeNode->setParentNode(node);

            Q_ASSERT(mesh->entities().size() == 1);
            const auto e = mesh->entities().first();
            if (auto t = e->componentsOfType<Qt3DCore::QTransform>(); !t.isEmpty() && e != node->entity()) {
                const auto t2 = edgeNode->transform();
                t2->setMatrix(t.first()->matrix());
            }
        }
    }
}

void DrawOutlineComponent::onDisable() {
    if (const auto child = nodeView()->findChild<EdgeRenderNode *>()) {
        child->removeEntity(true);
    }
}

