//
// Created by merlin
//

#include <Qt3DRender/QDirectionalLight>
#include <QScreen>
#include <QWidget>
#include "graph3d_scene.h"
#include "gizmo_axis.h"


#if TEST_SCENE
#include <QtCore>
#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QText2DEntity>
#include <Qt3dRender/QFilterKey>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QCuboidMesh>
#include <QSpotLight>
#include <QConeMesh>
#include <QPointLight>
#include <QPlaneMesh>
#include <QShaderProgram>
#include <Qt3dRender/QTechnique>
#include <Qt3dRender/QParameter>
#include <Qt3dRender/QGraphicsApiFilter>
#endif

static constexpr QVector3D gizmoPosition(0.0f, 0.0f, 0.0f);

void Graph3dScene::setupWorld() {
    m_view = new Qt3DExtras::Qt3DWindow(nullptr, Qt3DRender::API::RHI);

    m_sceneRoot = new Qt3DCore::QEntity();
    m_view->setRootEntity(m_sceneRoot);
    m_customRenderer = new CustomRenderer(m_sceneRoot);
    m_customRenderer->setGizmoAnchor(GizmoAnchor::BottomRight);
    m_cameraController = new CameraController(m_customRenderer->mainCamera());
    m_cameraController->setInvertPan(true);
    m_cameraController->setInvertTilt(true);
    m_cameraController->setInvertTransX(true);
    m_cameraController->setInvertTransY(true);
    m_axisGizmo = m_customRenderer->gizmoRoot();
    m_view->setActiveFrameGraph(m_customRenderer);
}

Graph3dScene::Graph3dScene(QObject *parent) : QObject(parent) {
    setupWorld();

#if TEST_SCENE
    testScene();
#else
    createAxisGizmo();
    setupLight(m_sceneRoot);
    m_cameraController->resetCamera();
#endif
}

#if TEST_SCENE
void GraphicScene::testScene() {
    const auto scale = 20.f;

    // Torus shape data
    auto *torus = new Qt3DExtras::QTorusMesh();
    torus->setRadius(4.0f);
    torus->setMinorRadius(1.f);
    torus->setRings(100);
    torus->setSlices(20);

    // TorusMesh Transform
    auto *torusTransform = new Qt3DCore::QTransform();
    torusTransform->setScale(scale);

    //custom material
    auto *shader = new Qt3DRender::QShaderProgram;
    shader-> setVertexShaderCode(R"(
#version 450

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

layout(location = 0) out vec3 worldPosition;
layout(location = 1) out vec3 worldNormal;

layout(std140, binding = 0) uniform qt3d_render_view_uniforms {
   mat4 viewMatrix;
   mat4 projectionMatrix;
   mat4 uncorrectedProjectionMatrix;
   mat4 clipCorrectionMatrix;
   mat4 viewProjectionMatrix;
   mat4 inverseViewMatrix;
   mat4 inverseProjectionMatrix;
   mat4 inverseViewProjectionMatrix;
   mat4 viewportMatrix;
   mat4 inverseViewportMatrix;
   vec4 textureTransformMatrix;
   vec3 eyePosition;
   float aspectRatio;
   float gamma;
   float exposure;
   float time;
};

layout(std140, binding = 1) uniform qt3d_command_uniforms {
  mat4 modelMatrix;
  mat4 inverseModelMatrix;
  mat4 modelViewMatrix;
  mat3 modelNormalMatrix;
  mat4 inverseModelViewMatrix;
  mat4 modelViewProjection;
  mat4 inverseModelViewProjectionMatrix;
};

void main()
{
    worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    worldNormal = normalize(modelNormalMatrix * vertexNormal);
    vec3 modifiedPosition = vertexPosition;
    modifiedPosition.x += sin(time * 3.0 + vertexPosition.y) * 0.5;
    gl_Position = modelViewProjection * vec4(modifiedPosition, 1.0);
}
    )");

    shader->setFragmentShaderCode(
            R"(
#version 450
layout(location = 0) out vec4 fragColor;
void main()
{
    fragColor = vec4(0,1,1,1);
}
)"
            );
    auto *torusMaterial = new Qt3DRender::QMaterial;
    auto *effect = new MyEffect;
    auto *technique = new Qt3DRender::QTechnique;

    technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::RHI);
    technique->graphicsApiFilter()->setMajorVersion(1);
    technique->graphicsApiFilter()->setMinorVersion(0);

    auto *renderPass = new Qt3DRender::QRenderPass;
    renderPass->setShaderProgram(shader);
    auto* filterKey = new Qt3DRender::QFilterKey;
    filterKey->setName(QStringLiteral("renderingStyle"));
    filterKey->setValue(QStringLiteral("forward"));
    technique->addFilterKey(filterKey);

    technique->addRenderPass(renderPass);
    effect->addTechnique(technique);
    torusMaterial->setEffect(effect);

    auto *torusMaterial2 = new Qt3DExtras::QPhongMaterial();
    torusMaterial2->setDiffuse(QColor(QRgb(0xbeb32b)));
    torusMaterial2->setAmbient(QColor::fromRgbF(.1f,.1f,.1f));
    torusMaterial2->setShininess(0);

    auto *entity = new Qt3DCore::QEntity(m_sceneRoot);
    auto *torusEntity = new Qt3DCore::QEntity(entity);
    torusEntity->addComponent(torus);
    torusEntity->addComponent(torusMaterial2);

    entity->addComponent(torusTransform);

    auto *sphereMesh = new Qt3DExtras::QSphereMesh();
    sphereMesh->setRings(20);
    sphereMesh->setSlices(20);
    sphereMesh->setRadius(1);
    auto *sphereEntity = new Qt3DCore::QEntity(entity);
    sphereEntity->addComponent(sphereMesh);
    sphereEntity->addComponent(torusMaterial);

    m_cameraController->camera()->setPosition(QVector3D(0, 400, 600));
    m_cameraController->camera()->setFarPlane(2000);
    m_cameraController->camera()->rotation(-30, QVector3D(1, 0, 0));
    //rotate torus
    auto *torusRotateTransformAnimation = new QPropertyAnimation(torusTransform, "rotationY", this);
    torusRotateTransformAnimation->setStartValue(0);
    torusRotateTransformAnimation->setEndValue(360);
    torusRotateTransformAnimation->setDuration(10000);
    torusRotateTransformAnimation->setLoopCount(-1);
    torusRotateTransformAnimation->start();

    //! dir dirLight
    auto *lightEntity = new Qt3DCore::QEntity(m_sceneRoot);
    auto *dirLight = new Qt3DRender::QDirectionalLight();
    dirLight->setColor(QColor::fromRgbF(1, 0.1f, 0.1f));
    auto *lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(200, 200, 0));
    lightTransform->setScale(scale);
    dirLight->setIntensity(1);
    lightEntity->addComponent(dirLight);
    lightEntity->addComponent(lightTransform);
    //box
    auto *boxMesh = new Qt3DExtras::QCuboidMesh();
    const float size = 0.6f;
    boxMesh->setXExtent(size);
    boxMesh->setYExtent(size);
    boxMesh->setZExtent(size*2);
    auto *lightMat = new Qt3DExtras::QPhongMaterial();
    lightMat->setAmbient(dirLight->color());
    lightEntity->addComponent(boxMesh);
    lightEntity->addComponent(lightMat);

    auto *sequentialAnimationGroup = new QSequentialAnimationGroup(this);
    auto *lightRotateTransformAnimation = new QPropertyAnimation(lightTransform, "rotation", this);
    lightRotateTransformAnimation->setStartValue(QQuaternion::fromEulerAngles(QVector3D(-45, -90, 0)));
    lightRotateTransformAnimation->setEndValue(QQuaternion::fromEulerAngles(QVector3D(-135, -90, 0)));
    lightRotateTransformAnimation->setDuration(2000);
    sequentialAnimationGroup->addAnimation(lightRotateTransformAnimation);
    auto *lightRotateTransformAnimation2 = new QPropertyAnimation(lightTransform, "rotation", this);
    lightRotateTransformAnimation2->setStartValue(QQuaternion::fromEulerAngles(QVector3D(-135, -90, 0)));
    lightRotateTransformAnimation2->setEndValue(QQuaternion::fromEulerAngles(QVector3D(-45, -90, 0)));
    lightRotateTransformAnimation2->setDuration(2000);
    sequentialAnimationGroup->addAnimation(lightRotateTransformAnimation2);
    sequentialAnimationGroup->setLoopCount(-1);
    sequentialAnimationGroup->start();

    connect(lightTransform, &Qt3DCore::QTransform::rotationChanged, this, [dirLight, lightTransform](const QQuaternion &rotation) {
        auto v = rotation.rotatedVector( QVector3D(0, 0, -1));
//        qDebug () << "rotationChanged: " << rotation.toEulerAngles() << "  " << v;
        dirLight->setWorldDirection(v);
    });

    lightEntity->setEnabled(true);

    //! spot dirLight
    auto *spotLightEntity = new Qt3DCore::QEntity(m_sceneRoot);
    auto *spotLight = new Qt3DRender::QSpotLight();
    spotLight->setColor(QColor::fromRgbF(1.f, 0.9f, 0.7f));
    spotLight->setIntensity(1);
    spotLight->setCutOffAngle(50);
    spotLight->setLinearAttenuation(0);
    auto *spotTransform = new Qt3DCore::QTransform();
    spotTransform->setTranslation(QVector3D(0, 180, 0));
    spotTransform->setScale(scale);
    spotLightEntity->addComponent(spotLight);
    spotLightEntity->addComponent(spotTransform);
    //cone
    auto *coneMesh = new Qt3DExtras::QConeMesh();
    coneMesh->setBottomRadius(size);
    coneMesh->setTopRadius(size/5.0f);
    coneMesh->setLength(2*size);
    auto *spotMat = new Qt3DExtras::QPhongMaterial();
    spotMat->setAmbient(spotLight->color());
    spotLightEntity->addComponent(coneMesh);
    spotLightEntity->addComponent(spotMat);

    spotLightEntity->setEnabled(false);

    //! point dirLight
    auto *pointLightEntity = new Qt3DCore::QEntity(m_sceneRoot);
    auto *pointLight = new Qt3DRender::QPointLight();
    pointLight->setColor(QColor::fromRgbF(0.1f, 1.0f, 0.1f));
    pointLight->setIntensity(1);
    auto *pointTransform = new Qt3DCore::QTransform();
    pointTransform->setTranslation(QVector3D(0, 250, 0));
    pointTransform->setScale(scale);
    pointLightEntity->addComponent(pointLight);
    pointLightEntity->addComponent(pointTransform);
    //sphere
    auto *pointMat = new Qt3DExtras::QPhongMaterial();
    pointMat->setAmbient(pointLight->color());
    pointMat->setShininess(0);
    auto *pointMesh = new Qt3DExtras::QSphereMesh();
    pointMesh->setRadius(size);
    pointLightEntity->addComponent(pointMesh);
    pointLightEntity->addComponent(pointMat);

    auto *sequentialAnimationGroup2 = new QSequentialAnimationGroup(this);
    auto *pointLightAnim1 = new QPropertyAnimation(pointTransform, "translation", this);
    pointLightAnim1->setStartValue(QVector3D(0, 250, 0));
    pointLightAnim1->setEndValue(QVector3D(400, 250, 0));
    pointLightAnim1->setDuration(2000);
    sequentialAnimationGroup2->addAnimation(pointLightAnim1);
    auto *pointLightAnim2 = new QPropertyAnimation(pointTransform, "translation", this);
    pointLightAnim2->setStartValue(QVector3D(400, 250, 0));
    pointLightAnim2->setEndValue(QVector3D(0, 250, 0));
    pointLightAnim2->setDuration(2000);
    sequentialAnimationGroup2->addAnimation(pointLightAnim2);
    sequentialAnimationGroup2->setLoopCount(-1);
    sequentialAnimationGroup2->start();

    pointLightEntity->setEnabled(true);

    //plane1
    auto *planeMesh = new Qt3DExtras::QPlaneMesh();
    planeMesh->setWidth(30);
    planeMesh->setHeight(30);

    // Plane mesh transform
    auto *planeTransform = new Qt3DCore::QTransform();
    planeTransform->setScale(scale);
    planeTransform->setTranslation(QVector3D(0.0f, -150.0f, 0.0f));

    auto *planeMaterial = new Qt3DExtras::QPhongMaterial();
    planeMaterial->setDiffuse(QColor::fromRgbF(0.8f, 0.6f, 0.4f));
    // Plane
    {
        auto planeEntity = new Qt3DCore::QEntity(m_sceneRoot);
        planeEntity->addComponent(planeMesh);
        planeEntity->addComponent(planeMaterial);
        planeEntity->addComponent(planeTransform);
    }

    //plane2
    auto *planeMesh2 = new Qt3DExtras::QPlaneMesh();
    planeMesh2->setWidth(30);
    planeMesh2->setHeight(30);

    // Plane mesh transform
    auto *planeTransform2 = new Qt3DCore::QTransform();
    planeTransform2->setScale(scale);
    planeTransform2->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 90.0f));
    planeTransform2->setTranslation(QVector3D(0.0f, 0.0f, -400.0f));

    auto *planeMaterial2 = new Qt3DExtras::QPhongMaterial();
    planeMaterial2->setDiffuse(QColor::fromRgbF(0.8f, 0.8f, 0.9f));
    // Plane
    {
        auto planeEntity2 = new Qt3DCore::QEntity(m_sceneRoot);
        planeEntity2->addComponent(planeMesh2);
        planeEntity2->addComponent(planeMaterial2);
        planeEntity2->addComponent(planeTransform2);
    }

    //text
    auto *textMesh = new Qt3DExtras::QExtrudedTextMesh();
    textMesh->setText("Qt 3D!");
    textMesh->setDepth(0.1f);
//    textMesh->setFont(QFont("Courier", 100, QFont::Bold));
    auto *textMat = new Qt3DExtras::QPhongMaterial();
    textMat->setDiffuse(QColor::fromRgbF(0.8f, 0.8f, 0.9f));
    auto *textTransform = new Qt3DCore::QTransform();
    textTransform->setTranslation(QVector3D(-180.0f, 180.0f, 0.0f));
    textTransform->setScale(scale);

    auto *textEntity = new Qt3DCore::QEntity(m_sceneRoot);
    textEntity->addComponent(textMesh);
    textEntity->addComponent(textMat);
    textEntity->addComponent(textTransform);

    //2d text, how to custom material???
    auto *text2dEntity = new Qt3DExtras::QText2DEntity(m_sceneRoot);
    text2dEntity->setText("Text 2D");
    text2dEntity->setColor(QColor::fromRgbF(0.f, 0.8f, 0.9f));
    text2dEntity->setFont(QFont("Courier", 20, QFont::Bold));
    auto *transform = new Qt3DCore::QTransform();
    transform->setTranslation(QVector3D(200.0f, 155.0f, 100.0f));
    text2dEntity->addComponent(transform);
    text2dEntity->setWidth(200);
    text2dEntity->setHeight(100);
}

#endif

QWidget *Graph3dScene::getWidgetContainer(QWidget *parent) const {
    Q_ASSERT(m_view);
    QWidget *container = QWidget::createWindowContainer(m_view.data(), parent);

    container->setMinimumSize(100, 100);
    container->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    return container;
}

void Graph3dScene::createAxisGizmo() {
    auto *axis = new GizmoAxis(m_axisGizmo);
    AxisOptions xOptions = GizmoAxis::defaultOption
            .setAxisColor(QColorConstants::Red)
            .setShowAxisName(true);

    axis->setAxis(GizmoAxis::x, xOptions);
    auto yOptions = xOptions.setAxisColor(QColorConstants::Green);
    axis->setAxis(GizmoAxis::y, yOptions);
    auto zOptions = xOptions.setAxisColor(QColorConstants::Blue);
    axis->setAxis(GizmoAxis::z, zOptions);

    m_axisGizmoTransform = axis->transform();
    m_axisGizmoTransform->setTranslation(gizmoPosition);
    auto *camera = m_customRenderer->gizmoCamera();
    camera->setPosition(m_axisGizmoTransform->translation() + QVector3D(0, 0, 20));
    camera->setViewCenter(m_axisGizmoTransform->translation());

    Q_ASSERT(m_cameraController);
    connect(m_cameraController, &CameraController::viewChanged, this, [this]() {
        m_axisGizmoTransform->setRotation(m_cameraController->cameraRotation());
    });
}

void Graph3dScene::setupLight(Qt3DCore::QEntity *parent) {
    auto *lightEntity = new Qt3DCore::QEntity(parent);
    LayerManager::attach(lightEntity, {LayerManager::opaque, LayerManager::transparent, LayerManager::_default});
    auto *light = new Qt3DRender::QDirectionalLight();
    light->setColor("white");
    light->setIntensity(0.4f);
    light->setWorldDirection(QVector3D(2.0f, -1.0f, -1.0f));
    lightEntity->addComponent(light);

    auto *light2 = new Qt3DRender::QDirectionalLight();
    light2->setColor("white");
    light2->setIntensity(0.20f);
    light2->setWorldDirection(QVector3D(-2.0f, 1.0f, -2.0f));
    lightEntity->addComponent(light2);
}

void Graph3dScene::setColor(const QColor &color) const {
    m_customRenderer->setClearColor(color);
}

void Graph3dScene::setModel(SceneModel *model) {
    if (model != m_model) {
        if (!m_model.isNull()) {
            destroyWorld();
            disconnect(m_model, &SceneModel::dataChanged, this, &Graph3dScene::onDataChanged);
        }
        m_model = model;
        if (!model) {
            return;
        }
        connect(m_model, &SceneModel::dataChanged, this, &Graph3dScene::onDataChanged);
        initWorld();
    }
}

void Graph3dScene::onDataChanged(RenderNode *renderNode, SceneModel::ActionType actionType) const {
    auto attachToResolvedParent = [this, renderNode] {
        auto *parentEntity = m_sceneRoot;
        if (auto *parentNode = renderNode->parentNode()) {
            if (parentNode->entity()) {
                parentEntity = parentNode->entity();
            }
        }
        renderNode->entity()->setParent(parentEntity);
    };

    switch (actionType) {
        case SceneModel::Create:
            renderNode->createEntity(m_sceneRoot)->setObjectName(renderNode->objectName());
            attachToResolvedParent();
            break;
        case SceneModel::Update:
            renderNode->updateEntity();
            attachToResolvedParent();
            renderNode->entity()->setObjectName(renderNode->objectName());
            break;
        case SceneModel::Remove:
            renderNode->removeEntity(true);
            break;
    }
}

void Graph3dScene::initWorld() const {
    if (m_model.isNull()) {
        return;
    }
    for (auto node: m_model->renderNodeList()) {
        onDataChanged(node, SceneModel::Create);
    }
}

void Graph3dScene::destroyWorld() const {
    m_model->cleanup();
}

void Graph3dScene::resize(QSize size) const {
    m_customRenderer->resize(size);
}
