//
// Created by merlin
//

#ifndef PMAXWELL_QT_RENDER_NODE_FACTORY_H
#define PMAXWELL_QT_RENDER_NODE_FACTORY_H

#include <Qt3DCore/QNodeId>
#include <QList>

#include "render_node.h"

class RenderNodeFactory {
    using RenderNodeCreateFunction = std::function<RenderNode *(const QObject *object)>;

public:
    [[nodiscard("MALLOC")]] RenderNode *create(const QObject *object) const;

    RenderNodeFactory() = default;

    RenderNodeFactory(std::initializer_list<std::pair<QString, RenderNodeCreateFunction>> list);

    void registerCreator(const QString &className, const RenderNodeCreateFunction &createFunction);
    void registerCreator(const std::pair<QString, RenderNodeCreateFunction>& pair);
    void registerCreator(std::initializer_list<std::pair<QString, RenderNodeCreateFunction>> list);

    void unregister(const QString &className);

    void clear();
private:
    QMap<QString, RenderNodeCreateFunction> m_renderNodeCreatorMap;
};

#define RENDER_NODE_CREATOR_1(Type) \
    std::make_pair(NODE_OBJECT_ID(Type), Type##RenderNode::create)

#define RENDER_NODE_CREATOR_2(Type, NodeType) \
    std::make_pair(NODE_OBJECT_ID(Type), NodeType::create)

#define RENDER_NODE_CREATOR(...) QT_OVERLOADED_MACRO(RENDER_NODE_CREATOR, __VA_ARGS__)

#endif //PMAXWELL_QT_RENDER_NODE_FACTORY_H
