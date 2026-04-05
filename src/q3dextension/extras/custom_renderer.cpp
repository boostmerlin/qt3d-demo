#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QFrustumCulling>
#include <Qt3DRender/QSortPolicy>
#include "custom_renderer.h"
#include "layer_manager.h"

static const float gizmoViewSize = 140.0f;

CustomRenderer::CustomRenderer(Qt3DCore::QNode *parent)
        : QTechniqueFilter(parent), m_surfaceSelector(new Qt3DRender::QRenderSurfaceSelector),
          m_viewport(new Qt3DRender::QViewport()), m_cameraSelector(new Qt3DRender::QCameraSelector()),
          m_clearBuffer(new Qt3DRender::QClearBuffers()), m_debugOverlay(new Qt3DRender::QDebugOverlay()),
          m_gizmoCameraSelector(new Qt3DRender::QCameraSelector()),
          m_gizmoViewport(new Qt3DRender::QViewport()) {
    init();
}

void CustomRenderer::init() {
    auto culling = new Qt3DRender::QFrustumCulling();
    m_debugOverlay->setParent(culling);
    m_cameraSelector->setParent(m_debugOverlay);
    m_debugOverlay->setEnabled(false);
    culling->setParent(m_clearBuffer);
    m_clearBuffer->setParent(m_viewport);

    auto *sortPolicy = new Qt3DRender::QSortPolicy();
    sortPolicy->setSortTypes(QList<int>{Qt3DRender::QSortPolicy::BackToFront});
    m_viewport->setParent(sortPolicy);
    sortPolicy->setParent(m_surfaceSelector);
    m_surfaceSelector->setParent(this);

    m_viewport->setNormalizedRect(QRectF(0.0, 0.0, 1.0, 1.0));
    m_clearBuffer->setClearColor(Qt::white);
    m_clearBuffer->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);

    auto *forwardRenderingStyle = new Qt3DRender::QFilterKey(this);
    forwardRenderingStyle->setName(QStringLiteral("renderingStyle"));
    forwardRenderingStyle->setValue(QStringLiteral("forward"));
    this->addMatch(forwardRenderingStyle);

    auto *cameraMain = new Qt3DRender::QCamera();
    m_cameraSelector->setCamera(cameraMain);

    m_gizmoViewport->setParent(m_viewport);
    m_gizmoCameraSelector->setParent(m_gizmoViewport);
    m_gizmoCameraSelector->setCamera(new Qt3DRender::QCamera);
    const float halfFrustumSize = gizmoViewSize / REFERENCE_DPI * 0.5f + 0.1f;
    gizmoCamera()->lens()->setOrthographicProjection(-halfFrustumSize, halfFrustumSize, -halfFrustumSize,
                                                     halfFrustumSize,
                                                     0.0f, 100.0f);
    initLayer();
}

void CustomRenderer::setClearColor(const QColor &clearColor) {
    m_clearBuffer->setClearColor(clearColor);
}

void CustomRenderer::setMainCamera(Qt3DRender::QCamera *camera) {
    m_cameraSelector->setCamera(camera);
}

Qt3DCore::QEntity *CustomRenderer::gizmoRoot() const {
    return m_gizmoRoot;
}

void CustomRenderer::setShowDebugOverlay(bool showDebugOverlay) {
    m_debugOverlay->setEnabled(showDebugOverlay);
}

void CustomRenderer::setGizmoAnchor(GizmoAnchor anchor) {
    m_gizmoAnchor = anchor;
}

Qt3DRender::QCamera *CustomRenderer::mainCamera() const {
    return qobject_cast<Qt3DRender::QCamera *>(m_cameraSelector->camera());
}

Qt3DRender::QCamera *CustomRenderer::gizmoCamera() const {
    return qobject_cast<Qt3DRender::QCamera *>(m_gizmoCameraSelector->camera());
}

void CustomRenderer::resize(QSize size) const {
    float ratio = float(size.width()) / qMax(1.0f, float(size.height()));
    mainCamera()->setAspectRatio(ratio);
    float w = gizmoViewSize / float(size.width());
    float h = gizmoViewSize / float(size.height());
    auto r = QRectF(0, 0, w, h);
    switch (m_gizmoAnchor) {
        case GizmoAnchor::TopLeft:
            r.moveTopLeft({0, 0});
            break;
        case GizmoAnchor::TopRight:
            r.moveTopRight({1, 0});
            break;
        case GizmoAnchor::BottomLeft:
            r.moveBottomLeft({0, 1});
            break;
        case GizmoAnchor::BottomRight:
            r.moveBottomRight({1, 1});
            break;
    }
    m_gizmoViewport->setNormalizedRect(r);
}

Qt3DRender::QLayerFilter *
CustomRenderer::createLayerFilter(Qt3DCore::QNode *parent, Qt3DRender::QLayerFilter::FilterMode mode) {
    auto *layerFilter = new Qt3DRender::QLayerFilter(parent);
    layerFilter->setFilterMode(mode);
    return layerFilter;
}

void CustomRenderer::initLayer() {
    //for gizmo
    auto axisGizmoLayer = LayerManager::layer("gizmo");
    axisGizmoLayer->setRecursive(true);
    auto *layerExcept = createLayerFilter(m_cameraSelector, Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
    layerExcept->addLayer(axisGizmoLayer);

    auto opaqueLayer = createLayerFilter(layerExcept, Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
    opaqueLayer->addLayer(LayerManager::layer(LayerManager::_default));
    opaqueLayer->addLayer(LayerManager::layer(LayerManager::opaque));
    createLayerFilter(layerExcept, Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers)->addLayer(
            LayerManager::layer(LayerManager::transparent));

    auto *layerAccept = createLayerFilter(m_gizmoCameraSelector, Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
    layerAccept->addLayer(axisGizmoLayer);
    m_gizmoRoot = new Qt3DCore::QEntity(layerAccept);
    LayerManager::attach(m_gizmoRoot, "gizmo");
}
