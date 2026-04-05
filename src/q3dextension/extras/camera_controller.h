//
// Created by merlin
//

#ifndef PMAXWELL_QT_CAMERA_CONTROLLER_H
#define PMAXWELL_QT_CAMERA_CONTROLLER_H

#include <Qt3DRender/QCamera>
#include <Qt3DExtras/QAbstractCameraController>
/*
 * 操作：
 * 1. 鼠标左键拖动：旋转
 * 2. 鼠标右键拖动：平移
 * 3. 鼠标滚轮或者左右键同时：缩放
 * 4. ESC 适合窗口大小
 */
class CameraController : public Qt3DExtras::QAbstractCameraController {
Q_OBJECT
public:
    explicit CameraController(Qt3DRender::QCamera *camera);

    void resetCamera();

    void xYView();
    void xZView();
    void yZView();
    void viewAll();

    void setInvertPan(bool inverse);
    void setInvertTilt(bool inverse);
    void setInvertTransX(bool inverse);
    void setInvertTransY(bool inverse);

    void zoomIn(float value = 1);
    void zoomOut(float value = -1);

    [[nodiscard]] QQuaternion cameraRotation() const;
signals:
    void viewChanged();
private:
    void init();
    void moveCamera(const Qt3DExtras::QAbstractCameraController::InputState &state, float dt) override;

    void zoom(float value);

    void updateCamera(const QVector3D &position);
private:
    const static int sign = 1;

    int m_invertPan = sign;
    int m_invertTilt = sign;
    int m_invertTransX = sign;
    int m_invertTransY = sign;

    int m_xyViewIndex = 0;
    int m_xzViewIndex = 0;
    int m_yzViewIndex = 0;

    QVector3D m_upVector;
    float m_zoomLimit = 2.f;
};

#endif //PMAXWELL_QT_CAMERA_CONTROLLER_H
