#include <QtTest/QtTest>

#include "visualization/graph3d/nodes/render_node.h"

class DummyRenderNode final : public RenderNodeEntity
{
    Q_OBJECT

public:
    explicit DummyRenderNode(QObject *parent = nullptr)
        : m_removeCalls(0)
    {
        setParent(parent);
    }

    [[nodiscard]] int removeCalls() const
    {
        return m_removeCalls;
    }

protected:
    void onCreate() override
    {
    }

    void onUpdate() override
    {
    }

    void onRemove() override
    {
        ++m_removeCalls;
    }

private:
    int m_removeCalls;
};

class RenderNodeTest final : public QObject
{
    Q_OBJECT

private slots:
    void removeEntitySkipsNonRenderNodeChildren();
};

void RenderNodeTest::removeEntitySkipsNonRenderNodeChildren()
{
    DummyRenderNode parentNode;
    parentNode.createEntity(nullptr);

    DummyRenderNode childNode(&parentNode);
    childNode.createEntity(nullptr);

    QObject foreignChild(&parentNode);

    parentNode.removeEntity(false);

    QCOMPARE(parentNode.removeCalls(), 1);
    QCOMPARE(childNode.removeCalls(), 1);
}

QTEST_APPLESS_MAIN(RenderNodeTest)

#include "render_node_test.moc"
