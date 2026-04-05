#ifndef PMAXWELL_QT_CUSTOM_RENDERER_H
#define PMAXWELL_QT_CUSTOM_RENDERER_H

#include <Qt3DRender/QTechniqueFilter>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QDebugOverlay>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QCamera>

constexpr float REFERENCE_DPI = 100.0f;

enum class GizmoAnchor {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

class CustomRenderer : public Qt3DRender::QTechniqueFilter {
    Q_OBJECT
public:
    explicit CustomRenderer(Qt3DCore::QNode *parent = nullptr);
    void setClearColor(const QColor &clearColor);
    void setMainCamera(Qt3DRender::QCamera *camera);
    void setShowDebugOverlay(bool showDebugOverlay);
    void setGizmoAnchor(GizmoAnchor anchor);

    [[nodiscard]] Qt3DCore::QEntity *gizmoRoot() const;
    [[nodiscard]] Qt3DRender::QCamera *mainCamera() const;
    [[nodiscard]] Qt3DRender::QCamera *gizmoCamera() const;
public slots:
    void resize(QSize size) const;

private:
    void init();
    void initLayer();
    static Qt3DRender::QLayerFilter* createLayerFilter(Qt3DCore::QNode *parent, Qt3DRender::QLayerFilter::FilterMode mode);
private:
    Qt3DRender::QRenderSurfaceSelector *m_surfaceSelector;
    Qt3DRender::QViewport *m_viewport;
    Qt3DRender::QCameraSelector *m_cameraSelector;
    Qt3DRender::QClearBuffers *m_clearBuffer;
    Qt3DRender::QDebugOverlay *m_debugOverlay;

    Qt3DRender::QCameraSelector *m_gizmoCameraSelector;
    Qt3DRender::QViewport *m_gizmoViewport;

    Qt3DCore::QEntity *m_gizmoRoot{};
    GizmoAnchor m_gizmoAnchor;
};

#endif //PMAXWELL_QT_CUSTOM_RENDERER_H
