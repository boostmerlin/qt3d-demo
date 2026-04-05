#ifndef PMAXWELL_QT_GRAPH3D_FRAME_H
#define PMAXWELL_QT_GRAPH3D_FRAME_H

#include "../graph_frame.h"
#include "graph3d_scene.h"

class Graph3dFrame final : public GraphFrame {
    Q_OBJECT

public:
    explicit Graph3dFrame(QWidget *parent = nullptr);

private:
    void initCameraControlToolBar();

protected:
    void onResized(QResizeEvent *) override;

private:
    Graph3dScene *m_graphicScene;
    QWidget* m_graphicContainer;
};

#endif
