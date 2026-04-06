//
// Created by merlin
//

#ifndef PMAXWELL_QT_EDGE_RENDER_NODE_H
#define PMAXWELL_QT_EDGE_RENDER_NODE_H

#include <QCuboidGeometry>

#include "geometry_render_node.h"

class EdgeRenderNode : public EdgeGeometryRenderNode {
    Q_OBJECT

public:
    explicit EdgeRenderNode(QColor color = QColorConstants::Magenta, Qt3DCore::QGeometry *geometry = nullptr);

    void onCreate() override;

    void onUpdate() override;

    void onRemove() override;

    void setRenderGeometry(Qt3DCore::QGeometry *data);

    [[nodiscard]] Qt3DCore::QGeometry *sourceGeometry() const;
protected:
    Qt3DCore::QGeometry *m_sourceGeometry{};
};

class BoxEdgeRenderNode : public EdgeRenderNode {
    Q_OBJECT

public:
    explicit BoxEdgeRenderNode(QColor color = QColorConstants::Magenta);

    void onUpdate() override;

    void onRemove() override;

private:
    Qt3DExtras::QCuboidGeometry *m_boxGeometry{};
};


#endif //PMAXWELL_QT_EDGE_RENDER_NODE_H
