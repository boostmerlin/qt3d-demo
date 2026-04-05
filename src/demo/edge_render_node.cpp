//
// Created by merlin
//

#include "edge_render_node.h"

#include "primitive_object.h"
#include "q3dextension/computational_geometry/edge_mesh.h"

EdgeRenderNode::EdgeRenderNode(QColor color, Qt3DCore::QGeometry *geometry) {
    m_sourceGeometry = geometry;
    setColor(color);
}

void EdgeRenderNode::onRemove() {
    m_sourceGeometry = nullptr;
}

void EdgeRenderNode::setRenderGeometry(Qt3DCore::QGeometry *data) {
    m_sourceGeometry = data;
}

void EdgeRenderNode::onCreate() {
    Q_ASSERT(m_sourceGeometry);
    auto *mesh = new EdgeMesh(*m_sourceGeometry, 3);
    m_entity->addComponent(mesh);
    attachMaterial(m_entity);
//    for (auto t : material()->effect()->techniques())
//        for (auto rp : t->renderPasses()) {
//            auto w = new Qt3DRender::QLineWidth(rp);
//            w->setEnabled(true);
//            w->setValue(4);
//            rp->addRenderState(w);
//        }
    onUpdate();
}

void EdgeRenderNode::onUpdate() {
    updateTransform();
    updateMaterial();
}

Qt3DCore::QGeometry *EdgeRenderNode::sourceGeometry() const {
    return m_sourceGeometry;
}

BoxEdgeRenderNode::BoxEdgeRenderNode(QColor color) : EdgeRenderNode(color) {
    m_boxGeometry = new Qt3DExtras::QCuboidGeometry;
    setRenderGeometry(m_boxGeometry);
}

void BoxEdgeRenderNode::onUpdate() {
    auto *box = geometry<BoxObject>();
    const QVector3D extents = approx(box->getExtents());
    m_boxGeometry->setXExtent(extents.x());
    m_boxGeometry->setYExtent(extents.y());
    m_boxGeometry->setZExtent(extents.z());

    EdgeRenderNode::onUpdate();
}

void BoxEdgeRenderNode::onRemove() {
    EdgeRenderNode::onRemove();

    delete m_boxGeometry;
}
