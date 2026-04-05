//
// Created by merlin
//

#ifndef PMAXWELL_QT_WORLD_H
#define PMAXWELL_QT_WORLD_H

#include <Qt3DExtras/Qt3DWindow>
#include <QPointer>
#include "core/render_node.h"
#include "camera_controller.h"
#include "scene_model.h"
#include "custom_renderer.h"

#define TEST_SCENE 0

class Graph3dScene final : public QObject {
Q_OBJECT
#if TEST_SCENE
    void testScene();
#endif
public:
    explicit Graph3dScene(QObject* parent);

    QWidget *getWidgetContainer(QWidget *parent = nullptr) const;

    void setColor(const QColor &color) const;

    void setModel(SceneModel *model);

    void resize(QSize size) const;

    [[nodiscard]] inline CameraController* cameraController() const {
        return m_cameraController;
    }
private slots:
    void onDataChanged(RenderNode *renderNode, SceneModel::ActionType actionType) const;
private:
    QPointer<SceneModel> m_model;

    Qt3DCore::QEntity *m_sceneRoot{};

    Qt3DCore::QEntity *m_axisGizmo{};

    Qt3DCore::QTransform *m_axisGizmoTransform{};

    CameraController *m_cameraController{};

    CustomRenderer *m_customRenderer{};

    // The QWidget container created by createWindowContainer() owns the QWindow.
    QPointer<Qt3DExtras::Qt3DWindow> m_view;

    static void setupLight(Qt3DCore::QEntity *);

    void createAxisGizmo();

    void initWorld() const;

    void destroyWorld() const;

    void setupWorld();
};

#endif //PMAXWELL_QT_WORLD_H
