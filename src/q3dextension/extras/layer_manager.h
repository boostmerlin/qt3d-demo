//
// Created by merlin
//

#ifndef PMAXWELL_LAYER_MANAGER_H
#define PMAXWELL_LAYER_MANAGER_H

#include <QString>
#include <Qt3DRender/QLayer>
#include <Qt3DCore/QEntity>

class LayerManager {
public:
    static LayerManager instance;

    static inline const QString _default = QStringLiteral("default");
    static inline const QString opaque = QStringLiteral("opaque");
    static inline const QString transparent = QStringLiteral("transparent");

    static Qt3DRender::QLayer* layer(const QString& name, const QString& groupTag = QString());
    static void attach(Qt3DCore::QEntity* entity, const QString& layerName, bool groupExclusive = true);
    static void attach(Qt3DCore::QEntity* entity, const QList<QString>& list);

    static void detach(Qt3DCore::QEntity* entity, const QString& layerName);
    static void detach(Qt3DCore::QEntity* entity, const QList<QString>& list);

    static QList<QString> layers();
private:
    LayerManager();
    static void attach(Qt3DCore::QEntity* entity, Qt3DRender::QLayer* layer, bool groupExclusive);
    static void detach(Qt3DCore::QEntity* entity, Qt3DRender::QLayer* layer);
    Qt3DRender::QLayer* getLayer(const QString& name);
    Qt3DRender::QLayer* addLayer(const QString& name, const QString& groupTag = QString(), Qt3DRender::QLayer* layer = nullptr);
    QHash<QString, Qt3DRender::QLayer*> m_layers;
};


#endif //PMAXWELL_LAYER_MANAGER_H
