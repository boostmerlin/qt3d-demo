//
// Created by merlin
//

#include <QQueue>
#include "layer_manager.h"

LayerManager LayerManager::instance;

LayerManager::LayerManager() {
    addLayer(_default, "render");
    addLayer(opaque, "render")->setRecursive(true);
    addLayer(transparent, "render");
}

Qt3DRender::QLayer *LayerManager::getLayer(const QString &name) {
    return m_layers.value(name, nullptr);
}

Qt3DRender::QLayer *LayerManager::addLayer(const QString &name, const QString &groupTag, Qt3DRender::QLayer *layer) {
    auto l = getLayer(name);
    if (l) {
        return l;
    }
    if (!layer) {
        layer = new Qt3DRender::QLayer();
        QObject::connect(layer, &QObject::destroyed, [this, name] {
            m_layers.remove(name);
        });
    }
    if (!groupTag.isEmpty()) {
        layer->setObjectName(groupTag);
    }
    m_layers.insert(name, layer);
    return layer;
}

Qt3DRender::QLayer *LayerManager::layer(const QString &name, const QString &groupTag) {
    auto l = instance.getLayer(name);
    if (!l) {
        l = instance.addLayer(name, groupTag);
    }
    return l;
}


void LayerManager::attach(Qt3DCore::QEntity *entity, const QList<QString> &list) {
    for (const auto &name: list) {
        attach(entity, name, false);
    }
}

void LayerManager::attach(Qt3DCore::QEntity *entity, const QString &layerName, bool groupExclusive) {
    auto layer = instance.getLayer(layerName);
    if (!layer) {
        qWarning() << "LayerManager::attach: layer not found:" << layerName;
        return;
    }
    LayerManager::attach(entity, layer, groupExclusive);
}

static void traverseEntity(Qt3DCore::QEntity *root, const std::function<bool(Qt3DCore::QEntity *)> &func) {
    QQueue<Qt3DCore::QEntity *> q;
    q.enqueue(root);
    while (!q.isEmpty()) {
        auto *e = q.dequeue();
        if (func(e)) {
            break;
        }
        for (auto *child: e->childNodes()) {
            if (auto *n = qobject_cast<Qt3DCore::QEntity *>(child)) {
                q.enqueue(n);
            }
        }
    }
}

void LayerManager::attach(Qt3DCore::QEntity *entity, Qt3DRender::QLayer *layer, bool groupExclusive) {
    traverseEntity(entity, [layer, groupExclusive](Qt3DCore::QEntity *e) {
        auto ll = e->componentsOfType<Qt3DRender::QLayer>();
        for (auto l: ll) {
            if (l == layer || (groupExclusive && l->objectName() == layer->objectName())) {
                return false;
            }
        }
        e->addComponent(layer);
        return layer->recursive();
    });
}

void LayerManager::detach(Qt3DCore::QEntity *entity, Qt3DRender::QLayer *layer) {
    traverseEntity(entity, [layer](Qt3DCore::QEntity *e) {
        e->removeComponent(layer);
        return layer->recursive();
    });
}

void LayerManager::detach(Qt3DCore::QEntity *entity, const QList<QString> &list) {
    for (const auto &name: list) {
        detach(entity, name);
    }
}

void LayerManager::detach(Qt3DCore::QEntity *entity, const QString &layerName) {
    auto layer = instance.getLayer(layerName);
    if (!layer) {
        qWarning() << "LayerManager::detach: layer not found:" << layerName;
        return;
    }
    LayerManager::detach(entity, layer);
}

QList<QString> LayerManager::layers() {
    return instance.m_layers.keys();
}
