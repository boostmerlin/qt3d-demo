//
// Created by merlin
//

#ifndef PMAXWELL_QT_WORLD_MODEL_H
#define PMAXWELL_QT_WORLD_MODEL_H

#include <QObject>
#include <QLoggingCategory>
#include "observable_object.h"
#include "render_node.h"
#include "render_node_factory.h"

Q_DECLARE_LOGGING_CATEGORY(qlcGraph3d)

class SceneModel final : public QObject {
Q_OBJECT

public:
    enum ActionType {
        Create,
        Update,
        Remove,
    };

    explicit SceneModel(QObject *parent = nullptr);

    ~SceneModel() override;

    [[nodiscard]] QList<RenderNode *> renderNodeList() const;

    RenderNodeFactory &renderNodeFactory();

    RenderNode *getRenderNode(const QObject *nodeData) const;

    void selectNodes(const QList<QObject *> &nodeData) const;

    void deselectNodes(const QList<QObject *> &nodeData) const;

    template<class LT, class PT>
    auto listOperation(bool (SceneModel::*op)(const PT *nodeData)) {
        return [=](const QList<LT> &list) {
            bool ret = true;
            for (auto node: list) {
                ret = ret && (this->*op)(node);
            }
            return ret;
        };
    }
public slots:
    bool createOrUpdate(const QObservableObject *nodeData);
    bool remove(const QObject *nodeData);
    bool update(const QObject *nodeData);
    void cleanup();
signals:
    void dataChanged(RenderNode *renderNode, ActionType actionType);
private:
    void syncRenderNodeParent(const QObject *nodeData) const;

    QMap<const QObject *, RenderNode *> m_renderNodeMap;
    RenderNodeFactory m_renderNodeFactory;
};

#endif //PMAXWELL_QT_WORLD_MODEL_H
