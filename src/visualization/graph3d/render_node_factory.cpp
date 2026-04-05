
#include "render_node_factory.h"

RenderNode *RenderNodeFactory::create(const QObject *object) const {
    Q_ASSERT(object);
    auto className = object->metaObject()->className();
    auto it = m_renderNodeCreatorMap.constFind(className);
    if (it != m_renderNodeCreatorMap.constEnd()) {
        return it.value()(object);
    }
    return nullptr;
}

void RenderNodeFactory::registerCreator(const QString &className,
                                        const RenderNodeCreateFunction &createFunction) {
    if (m_renderNodeCreatorMap.contains(className)) {
        qWarning() << "RenderNodeFactory::registerRenderNode: " << className << " already registered.";
        return;
    }
    m_renderNodeCreatorMap.insert(className, createFunction);
}

void RenderNodeFactory::unregister(const QString &className) {
    m_renderNodeCreatorMap.remove(className);
}

void RenderNodeFactory::clear() {
    m_renderNodeCreatorMap.clear();
}

RenderNodeFactory::RenderNodeFactory(std::initializer_list<std::pair<QString, RenderNodeCreateFunction>> list)
        : m_renderNodeCreatorMap(list) {
}

void RenderNodeFactory::registerCreator(std::initializer_list<std::pair<QString, RenderNodeCreateFunction>> list) {
    for (const auto &[fst, snd]: list) {
        registerCreator(fst, snd);
    }
}

void RenderNodeFactory::registerCreator(const std::pair<QString, RenderNodeCreateFunction> &pair) {
    registerCreator(pair.first, pair.second);
}


