//
// Created by merlin
//

#include <QVBoxLayout>
#include "graph3d_frame.h"
#include "icon_provider.h"
#include "app.h"

Graph3dFrame::Graph3dFrame(QWidget *parent) : GraphFrame(parent) {
    m_graphicScene = new Graph3dScene(this);
    m_graphicScene->setColor(QColor(0xeaeaea));
    QBoxLayout* layout = boxLayout();
    m_graphicContainer = m_graphicScene->getWidgetContainer();
    layout->addWidget(m_graphicContainer, 1);

    connect(&App::instance, &App::projectChanged, m_graphicScene, [this]() {
        m_graphicScene->setModel(App::instance.sceneModel());
    });

    initCameraControlToolBar();
}

void Graph3dFrame::initCameraControlToolBar() {
    auto cc = m_graphicScene->cameraController();
    toolBar()->addAction(IconProvider::provide("xyz"), "reset", cc, &CameraController::resetCamera);
    toolBar()->addAction(IconProvider::provide("xy"), "xy view", cc, &CameraController::xYView);
    toolBar()->addAction(IconProvider::provide("xz"), "xz view", cc, &CameraController::xZView);
    toolBar()->addAction(IconProvider::provide("yz"), "yz view", cc, &CameraController::yZView);
    toolBar()->addAction(IconProvider::provide("fit"), "fit to window", cc, &CameraController::viewAll);
    toolBar()->setVisible(true);
}

void Graph3dFrame::onResized(QResizeEvent *event) {
    m_graphicScene->resize(m_graphicContainer->size());
}