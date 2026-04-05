#include <QAbstractItemView>
#include <QEvent>
#include <QList>
#include <QSizePolicy>
#include <QSplitter>
#include <QStatusBar>
#include <QToolButton>
#include <QToolBar>
#include <QVBoxLayout>

#include "demo/primitive_object.h"
#include "demo/scene_controller.h"
#include "hierarchy.h"
#include "mainwindow.h"
#include "property_panel.h"
#include "visualization/graph3d/graph3d_scene.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_sceneController = new SceneController(this);

    setupEditorLayout();
    setupWindowStyle();
    connect(ui->actionExit_e, &QAction::triggered, this, &MainWindow::onExit);
    setupSceneView();
    setupHierarchy();
    setupPropertyPanel();
    connectController();
    createActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onExit() {
    close();
}

void MainWindow::setupEditorLayout()
{
    auto *layout = ui->horizontalLayout;
    m_mainSplitter = new QSplitter(Qt::Horizontal, ui->frame);
    m_mainSplitter->setObjectName(QStringLiteral("mainSplitter"));
    m_mainSplitter->setChildrenCollapsible(false);
    m_mainSplitter->setHandleWidth(10);

    QList<QWidget *> panes;
    while (layout->count() > 0) {
        auto *item = layout->takeAt(0);
        if (auto *widget = item->widget()) {
            widget->setParent(nullptr);
            panes.append(widget);
        }
        delete item;
    }

    for (auto *pane : panes) {
        m_mainSplitter->addWidget(pane);
    }

    layout->addWidget(m_mainSplitter);
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->setStretchFactor(2, 0);
    m_mainSplitter->setSizes({260, 780, 420});
}

void MainWindow::setupWindowStyle()
{
    setWindowTitle(tr("Qt 3D Primitive Studio"));
    resize(1360, 860);
    setMinimumSize(1120, 720);

    ui->horizontalLayout_2->setSpacing(10);

    ui->tree_frame->setMinimumWidth(240);
    ui->property_frame->setMinimumWidth(360);
    ui->visualize_frame->setMinimumWidth(420);

    statusBar()->showMessage(tr("Ready"));

    setStyleSheet(R"(
QMainWindow {
    background: #eef3f8;
}
QMenuBar {
    background: #f7fafc;
    border-bottom: 1px solid #d7dee7;
}
QMenuBar::item {
    padding: 6px 10px;
}
QFrame#geometry_frame {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #f7fafc, stop:1 #eef4fb);
    border: 1px solid #d7dee7;
    border-radius: 10px;
}
QFrame#tree_frame, QFrame#visualize_frame, QFrame#property_frame {
    background: #ffffff;
    border: 1px solid #cfd8e3;
    border-radius: 10px;
}
QSplitter#mainSplitter::handle {
    background: transparent;
}
QSplitter#mainSplitter::handle:hover {
    background: #d7e7fb;
}
QToolButton {
    background: transparent;
    border: 1px solid transparent;
    border-radius: 8px;
    color: #2d3748;
    font-size: 12px;
    padding: 6px 4px;
}
QToolButton:hover {
    background: #edf4ff;
    border-color: #bfd2ea;
}
QToolButton:pressed {
    background: #dcecff;
}
QToolBar {
    background: #f8fbff;
    border: none;
    border-bottom: 1px solid #e2e8f0;
    spacing: 4px;
    padding: 4px 6px;
}
QToolBar QToolButton {
    padding: 4px 8px;
}
QTreeView {
    background: #ffffff;
    border: none;
    outline: 0;
    show-decoration-selected: 1;
}
QTreeView::item {
    padding: 6px 8px;
}
QTreeView::item:selected {
    background: #dcecff;
    color: #1a365d;
    border-radius: 4px;
}
QHeaderView::section {
    background: #f8fbff;
    border: none;
    border-bottom: 1px solid #e2e8f0;
    color: #516070;
    font-weight: 600;
    padding: 6px 8px;
}
QLabel {
    color: #2d3748;
}
QLineEdit, QDoubleSpinBox {
    background: #f8fbff;
    border: 1px solid #d6dfeb;
    border-radius: 6px;
    padding: 6px 8px;
}
QLineEdit:focus, QDoubleSpinBox:focus {
    border-color: #63a4ff;
    background: #ffffff;
}
QLineEdit:read-only {
    color: #5f6f82;
    background: #f1f5f9;
}
QStatusBar {
    background: #f7fafc;
    border-top: 1px solid #d7dee7;
}
)");
}

void MainWindow::createActions() {
    ui->horizontalLayout_2->setAlignment(Qt::AlignLeft);
    addPrimitiveButton(tr("Box"), int(PrimitiveObject::PrimitiveType::Box), QStringLiteral("box"));
    addPrimitiveButton(tr("Sphere"), int(PrimitiveObject::PrimitiveType::Sphere), QStringLiteral("sphere"));
    addPrimitiveButton(tr("Cylinder"), int(PrimitiveObject::PrimitiveType::Cylinder), QStringLiteral("cylinder"));
    addPrimitiveButton(tr("Cone"), int(PrimitiveObject::PrimitiveType::Cone), QStringLiteral("cone"));
}

void MainWindow::addPrimitiveButton(const QString &label, int type, const QString &iconName)
{
    auto *button = new QToolButton(this);
    button->setText(label);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIconSize(QSize(28, 28));
    button->setAutoRaise(true);
    button->setFixedSize(78, 64);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (!iconName.isEmpty()) {
        button->setIcon(QIcon(QStringLiteral(":/icons/%1.svg").arg(iconName)));
    }
    ui->horizontalLayout_2->addWidget(button);
    connect(button, &QToolButton::clicked, this, [this, type] {
        m_sceneController->addPrimitive(static_cast<PrimitiveObject::PrimitiveType>(type));
    });
}

void MainWindow::setupSceneView()
{
    auto *layout = new QVBoxLayout(ui->visualize_frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *toolBar = new QToolBar(ui->visualize_frame);
    layout->addWidget(toolBar);

    m_graphScene = new Graph3dScene(this);
    m_graphScene->setColor(QColor(0xea, 0xea, 0xea));
    m_graphScene->setModel(m_sceneController->sceneModel());
    m_graphContainer = m_graphScene->getWidgetContainer(ui->visualize_frame);
    m_graphContainer->installEventFilter(this);
    layout->addWidget(m_graphContainer, 1);

    auto *cameraController = m_graphScene->cameraController();
    toolBar->addAction(tr("Reset"), cameraController, &CameraController::resetCamera);
    toolBar->addAction(tr("XY"), cameraController, &CameraController::xYView);
    toolBar->addAction(tr("XZ"), cameraController, &CameraController::xZView);
    toolBar->addAction(tr("YZ"), cameraController, &CameraController::yZView);
    toolBar->addAction(tr("Fit"), cameraController, &CameraController::viewAll);

    m_graphScene->resize(m_graphContainer->size());
}

void MainWindow::setupHierarchy()
{
    m_hierarchy = new Hierarchy(ui->treeView, this);
}

void MainWindow::setupPropertyPanel()
{
    auto *layout = new QVBoxLayout(ui->property_frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_propertyPanel = new PropertyPanel(ui->property_frame);
    layout->addWidget(m_propertyPanel);
}

void MainWindow::connectController()
{
    auto refreshObjectViews = [this] {
        const auto objects = m_sceneController->objects();
        m_hierarchy->setObjects(objects);
        m_propertyPanel->setObjects(objects);
    };

    connect(m_hierarchy, &Hierarchy::currentObjectRequested, m_sceneController, &SceneController::setCurrentObject);
    connect(m_propertyPanel, &PropertyPanel::parentChangeRequested, this,
            [this](PrimitiveObject *object, PrimitiveObject *parentObject) {
                m_sceneController->setObjectParent(object, parentObject);
            });
    connect(m_propertyPanel, &PropertyPanel::removeRequested, this, [this](PrimitiveObject *object) {
        m_sceneController->removeObject(object);
    });

    connect(m_sceneController, &SceneController::objectAdded, this, [refreshObjectViews](PrimitiveObject *) {
        refreshObjectViews();
    });
    connect(m_sceneController, &SceneController::objectRemoved, this, [refreshObjectViews](PrimitiveObject *) {
        refreshObjectViews();
    });
    connect(m_sceneController, &SceneController::objectHierarchyChanged, this, [refreshObjectViews](PrimitiveObject *) {
        refreshObjectViews();
    });
    connect(m_sceneController, &SceneController::objectChanged, m_hierarchy, &Hierarchy::refreshObject);
    connect(m_sceneController, &SceneController::objectChanged, m_propertyPanel, &PropertyPanel::refreshObject);
    connect(m_sceneController, &SceneController::currentObjectChanged, m_hierarchy, &Hierarchy::setCurrentObject);
    connect(m_sceneController, &SceneController::currentObjectChanged, m_propertyPanel, &PropertyPanel::setCurrentObject);

    refreshObjectViews();
    m_hierarchy->setCurrentObject(m_sceneController->currentObject());
    m_propertyPanel->setCurrentObject(m_sceneController->currentObject());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_graphContainer && event->type() == QEvent::Resize && m_graphScene) {
        m_graphScene->resize(m_graphContainer->size());
    }
    return QMainWindow::eventFilter(watched, event);
}
