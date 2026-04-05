
#include "camera_controller.h"

constexpr static QVector3D zUp(0, 0, 1);
constexpr static QVector3D yUp(0, 1, 0);

constexpr static float initX = 5.0f;
constexpr static float initY = 5.0f;
constexpr static float initZ = 5.0f;
const static float initLength = QVector3D(initX, initY, initZ).length();

constexpr static std::pair<float, float> signs[4] = {{1,  1},
                                                     {-1, 1},
                                                     {-1, -1},
                                                     {1,  -1}};

CameraController::CameraController(Qt3DRender::QCamera *camera)
        : Qt3DExtras::QAbstractCameraController(camera) {
    setCamera(camera);

    connect(camera, &Qt3DRender::QCamera::viewMatrixChanged, this, &CameraController::viewChanged);
    setAcceleration(1.f);
    setLinearSpeed(1.0f);
    setLookSpeed(180.0f);
}

void CameraController::resetCamera() {
    m_upVector = zUp;
    m_xyViewIndex = 0;
    m_xzViewIndex = 0;
    m_yzViewIndex = 0;
    init();
}

void CameraController::init() {
    auto *camera = this->camera();
    camera->lens()->setFieldOfView(45);
    camera->lens()->setNearPlane(0.1f);
    camera->lens()->setFarPlane(1000.0f);
    updateCamera(QVector3D(initX, initY, initZ));
}

void CameraController::moveCamera(const Qt3DExtras::QAbstractCameraController::InputState &state, float dt) {
    auto *camera = this->camera();
    if (!camera) {
        return;
    }
    if (state.leftMouseButtonActive && state.rightMouseButtonActive || state.tzAxisValue != 0) {
        float v = state.rxAxisValue + state.tzAxisValue;
        if (v != 0) {
            zoom(v);
        }
    } else if (state.leftMouseButtonActive && (state.rxAxisValue != 0 || state.ryAxisValue != 0)) {
        camera->panAboutViewCenter((state.rxAxisValue * lookSpeed() * float(m_invertPan)) * dt, m_upVector);
        camera->tiltAboutViewCenter((state.ryAxisValue * lookSpeed() * float(m_invertTilt)) * dt);
    } else if (state.rightMouseButtonActive && (state.rxAxisValue != 0 || state.ryAxisValue != 0)) {
        float multiplier = linearSpeed() * (camera->position() - camera->viewCenter()).length();
        camera->translate(QVector3D(state.rxAxisValue * multiplier * float(m_invertTransX),
                                    state.ryAxisValue * multiplier * float(m_invertTransY),
                                    0.0f) * dt);
    }
}

void CameraController::zoom(float value) {
    auto *camera = this->camera();
    Q_ASSERT(camera);
    float d = (camera->position() - camera->viewCenter()).length() - m_zoomLimit;
    if (d > 0) {
        float z = linearSpeed() * value;
        if (z > 0) { //dolly when z > 0
            z = qMin(z, d);
        }
        camera->translate(QVector3D(0, 0, z),
                          Qt3DRender::QCamera::DontTranslateViewCenter);
    } else {
        camera->translate(QVector3D(0, 0, -0.1f), Qt3DRender::QCamera::DontTranslateViewCenter);
    }
}

void CameraController::zoomIn(float value) {
    zoom(qAbs(value));
}

void CameraController::zoomOut(float value) {
    zoom(-qAbs(value));
}

void CameraController::setInvertPan(bool inverse) {
    m_invertPan = inverse ? -sign : sign;
}


void CameraController::setInvertTilt(bool inverse) {
    m_invertTilt = inverse ? -sign : sign;
}

void CameraController::setInvertTransX(bool inverse) {
    m_invertTransX = inverse ? -sign : sign;
}

void CameraController::setInvertTransY(bool inverse) {
    m_invertTransY = inverse ? -sign : sign;
}

QQuaternion CameraController::cameraRotation() const {
    auto *camera = this->camera();
    Q_ASSERT(camera);
    return camera->transform()->rotation().inverted();
}

void CameraController::xYView() {
    int index = (m_xyViewIndex++) % 4;
    m_upVector = yUp * signs[index].first;
    updateCamera(QVector3D(0, 0, initLength * signs[index].second));
}

void CameraController::xZView() {
    int index = (m_xzViewIndex++) % 4;
    m_upVector = zUp * signs[index].first;
    updateCamera(QVector3D(0, initLength * signs[index].second, 0));
}

void CameraController::yZView() {
    int index = (m_yzViewIndex++) % 4;
    m_upVector = zUp * signs[index].first;
    updateCamera(QVector3D(initLength * signs[index].second, 0, 0));
}

void CameraController::updateCamera(const QVector3D &position) {
    auto *camera = this->camera();
    Q_ASSERT(camera);
    camera->setPosition(position);
    camera->setUpVector(m_upVector);
    camera->setViewCenter(QVector3D(0, 0, 0));
}

void CameraController::viewAll() {
    auto *camera = this->camera();
    Q_ASSERT(camera);
    //maybe wait for new version in the future to solve this problem
    camera->viewAll();
}
