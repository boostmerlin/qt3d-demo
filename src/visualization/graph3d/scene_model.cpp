//
// Created by merlin
//

#include <QLoggingCategory>

#include "scene_model.h"

Q_LOGGING_CATEGORY(qlcGraph3d, "visualization.graph3d");

SceneModel::SceneModel(QObject *parent) : QObject(parent) {
}

SceneModel::~SceneModel() {
    cleanup();
}

bool SceneModel::remove(const QObject *nodeData) {
    if (const auto v = m_renderNodeMap.take(nodeData)) {
        emit dataChanged(v, Remove);
        return true;
    }
    return false;
}

QList<RenderNode *> SceneModel::renderNodeList() const {
    return m_renderNodeMap.values();
}

bool SceneModel::update(const QObject *nodeData) {
    auto v = m_renderNodeMap.value(nodeData);
    if (v && v->updateRenderData(nodeData)) {
        syncRenderNodeParent(nodeData);
        emit dataChanged(v, Update);
        return true;
    }
    return false;
}

bool SceneModel::createOrUpdate(const QObservableObject *nodeData) {
    if (!update(nodeData)) {
        RenderNode *v = m_renderNodeFactory.create(nodeData);
        if (!v) {
            qCInfo(qlcGraph3d) << "no node creator for: " << nodeData->metaObject()->className();
            return false;
        }
        if (!v->updateRenderData(nodeData)) {
            remove(nodeData);
            return false;
        }
        connect(nodeData, &QObservableObject::destroyed, this, &SceneModel::remove);
        connect(nodeData, &QObservableObject::propertyChanged, this, [this, nodeData] {
            update(nodeData);
        });
        m_renderNodeMap.insert(nodeData, v);
        syncRenderNodeParent(nodeData);
        emit dataChanged(v, Create);
    }
    return true;
}

RenderNodeFactory &SceneModel::renderNodeFactory() {
    return m_renderNodeFactory;
}

void SceneModel::cleanup() {
    for (const auto obj: m_renderNodeMap.keys()) {
        remove(obj);
    }
}

RenderNode *SceneModel::getRenderNode(const QObject *nodeData) const {
    return m_renderNodeMap.value(nodeData, nullptr);
}

void SceneModel::selectNodes(const QList<QObject *> &nodeData) const {
    for (const auto node: nodeData) {
        const auto renderNode = getRenderNode(node);
        if (!renderNode) continue;
        if (const auto selectable = renderNode->asType<Selectable *>()) {
            selectable->select();
        }
    }
}

void SceneModel::deselectNodes(const QList<QObject *> &nodeData) const {
    for (const auto node: nodeData) {
        const auto renderNode = getRenderNode(node);
        if (!renderNode) continue;
        if (const auto selectable = renderNode->asType<Selectable *>()) {
            selectable->deselect();
        }
    }
}

void SceneModel::syncRenderNodeParent(const QObject *nodeData) const
{
    auto *renderNode = getRenderNode(nodeData);
    if (!renderNode) {
        return;
    }

    auto *parentRenderNode = getRenderNode(nodeData ? nodeData->parent() : nullptr);
    renderNode->QObject::setParent(parentRenderNode);
}
