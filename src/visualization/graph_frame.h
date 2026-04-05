#ifndef PMAXWELL_QT_GRAPH_SCENE_H
#define PMAXWELL_QT_GRAPH_SCENE_H

#include <QFrame>
#include <QResizeEvent>
#include <QToolBar>
#include <QBoxLayout>

class GraphFrame : public QFrame {
Q_OBJECT

public:
    explicit GraphFrame(QWidget *parent = nullptr);

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QToolBar* toolBar() const {
        return m_toolBar;
    }
    [[nodiscard]] QBoxLayout* boxLayout() const {
        return m_layout;
    }
protected:
    void resizeEvent(QResizeEvent *) override;
    virtual void onResized(QResizeEvent *) = 0;
private:
    QToolBar* m_toolBar{};
    QBoxLayout* m_layout{};
};

#endif
