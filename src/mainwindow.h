#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <functional>

#include <QMainWindow>
#include <QPoint>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Graph3dScene;
class QEvent;
class Hierarchy;
class PropertyPanel;
class QSplitter;
class SceneController;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    SceneController *m_sceneController{};
    Graph3dScene *m_graphScene{};
    QWidget *m_graphContainer{};
    QSplitter *m_mainSplitter{};
    Hierarchy *m_hierarchy{};
    PropertyPanel *m_propertyPanel{};
    QPoint m_sceneClickPressPos;
    bool m_sceneClickPending = false;
    bool m_sceneClickMoved = false;
    bool m_scenePickHandled = false;

    void createActions();
    void addPrimitiveButton(const QString &label, int type, const QString &iconName = QString());
    void addToolbarButton(const QString &label,
                          const QString &iconName,
                          const std::function<void()> &handler);
    void setupSceneView();
    void setupHierarchy();
    void setupPropertyPanel();
    void setupEditorLayout();
    void setupWindowStyle();
    void connectController();

    void onExit();
};
#endif // MAINWINDOW_H
