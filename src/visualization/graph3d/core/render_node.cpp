#include <Qt3DRender/QMaterial>
#include "render_node.h"
#include "behavior_component.h"

bool RenderNode::updateRenderData(const QObject *) {
    return true;
}

Qt3DCore::QTransform *RenderNode::transform() const {
    return getComponent<Qt3DCore::QTransform>();
}

void RenderNode::addComponent(Qt3DCore::QComponent *component) const {
    entity()->addComponent(component);
}

void RenderNode::removeComponent(Qt3DCore::QComponent *component) const {
    entity()->removeComponent(component);
}

bool RenderNode::isProxyNode() const {
    return false;
}

ProxyRenderNode::ProxyRenderNode(RenderNode *renderNode) : m_renderNode(renderNode) {
}

ProxyRenderNode::~ProxyRenderNode() {
    if (m_renderNode) {
        m_renderNode->deleteLater();
        m_renderNode = nullptr;
    }
}

bool ProxyRenderNode::isProxyNode() const {
    return true;
}

bool ProxyRenderNode::updateRenderData(const QObject *data) {
    Q_ASSERT(m_renderNode);
    return m_renderNode->updateRenderData(data);
}

Qt3DCore::QEntity *ProxyRenderNode::createEntity(Qt3DCore::QNode *parent) noexcept {
    Q_ASSERT(m_renderNode);
    return m_renderNode->createEntity(parent);
}

void ProxyRenderNode::updateEntity() {
    Q_ASSERT(m_renderNode);
    m_renderNode->updateEntity();
}

void ProxyRenderNode::removeEntity(bool disposeSelf) {
    Q_ASSERT(m_renderNode);
    m_renderNode->removeEntity(disposeSelf);
    m_renderNode = nullptr;
    if (disposeSelf) {
        deleteLater();
    }
}

Qt3DCore::QEntity *ProxyRenderNode::entity() const {
    Q_ASSERT(m_renderNode);
    return m_renderNode->entity();
}

QString ProxyRenderNode::layer() const {
    return m_renderNode->layer();
}

void RenderNode::setParentNode(RenderNode *parent) {
    setParent(parent);
    auto e = entity();
    if (!e) {
        e = createEntity(nullptr);
    }
    if (e) {
        if (parent) {
            e->setParent(parent->entity());
        } else {
            e->setParent(static_cast<Qt3DCore::QNode *>(nullptr));
        }
    }
}

RenderNode *RenderNode::parentNode() const {
    return qobject_cast<RenderNode *>(parent());
}

Qt3DCore::QEntity *RenderNodeEntity::entity() const {
    return m_entity;
}

void RenderNodeEntity::onRemove() {
}

//移除时生命周期都能调用，移除时entity能正确释放
void RenderNodeEntity::removeEntity(bool disposeSelf) {
    if (m_entity == nullptr) {
        return;
    }
    onRemove();
    m_entity->deleteLater();
    m_entity = nullptr;
    for (auto *child: children()) {
        auto *node = qobject_cast<RenderNode *>(child);
        node->removeEntity(false);
    }

    if (disposeSelf) {
        deleteLater();
    }
}

Qt3DCore::QEntity *RenderNodeEntity::createEntity(Qt3DCore::QNode *parent) noexcept {
    Q_ASSERT(m_entity == nullptr);
    m_entity = new Qt3DCore::QEntity(parent);
    m_transform = new Qt3DCore::QTransform;
    m_entity->addComponent(m_transform);
    connect(m_entity, &QObject::destroyed, this, [this] {
        m_entity = nullptr;
    });
    onCreate();
    LayerManager::attach(m_entity, m_layer = layer());
    return m_entity;
}

void RenderNodeEntity::updateEntity() {
    Q_ASSERT(m_entity != nullptr);
    onUpdate();
    //hack:
    auto comps = getComponents<BehaviorComponent>(false);
    for (auto *component: comps) {
        component->setValid();
    }
    if (m_layer != layer()) {
        LayerManager::detach(m_entity, m_layer);
        LayerManager::attach(m_entity, m_layer = layer());
    }
}

QString RenderNodeEntity::layer() const {
    return LayerManager::_default;
}

void WithColor::setColor(const QColor &color) {
    m_color = color;
}

QColor WithColor::color() const {
    if (m_color == std::nullopt) {
        return defaultColor;
    }
    auto &c = *m_color;
    if (!c.isValid()) {
        return defaultColor;
    }
    return c;
}

void Selectable::setSelectable(bool selectable) {
    m_selectable = selectable;
    if (!selectable && m_selected) {
        onDeselect();
        m_selected = false;
    }
}

bool Selectable::selectable() const {
    return m_selectable;
}

void Selectable::select() {
    if (!m_selectable) {
        return;
    }
    if (!m_selected) {
        onSelect();
        m_selected = true;
    }
}

void Selectable::deselect() {
    if (!m_selectable) {
        return;
    }
    if (m_selected) {
        onDeselect();
        m_selected = false;
    }
}

void WithAlpha::setAlpha(float alpha) {
    m_alpha = alpha < 0 ? 0 : alpha > 1 ? 1 : alpha;
}

float WithAlpha::alpha() const {
    return m_alpha;
}

Qt3DCore::QComponent* WithMaterial::attachMaterial(Qt3DCore::QEntity *entity) const {
    if (!entity) {
        auto node = dynamic_cast<const RenderNode *>(this);
        Q_ASSERT(node);
        entity = node->entity();
    }
    if (!entity) {
        return nullptr;
    }
    auto mats = entity->componentsOfType<Qt3DRender::QMaterial>();
    if (!mats.isEmpty()) {
        for (auto *mat: mats) {
            entity->removeComponent(mat);
        }
    }
    auto m = material();
    if (m) {
        entity->addComponent(m);
    }
    return m;
}

qsizetype WithMaterial::size() const {
    return m_materials.size();
}

qsizetype WithMaterial::materialIndex() const {
    return m_current;
}

void WithMaterial::addMaterial(Qt3DCore::QComponent *material) {
    Q_ASSERT(material);
    if (m_materials.contains(material)) {
        return;
    }
    material->setShareable(false);
    m_materials.append(material);
}

WithMaterial::~WithMaterial() {
    for (const auto& m: m_materials) {
        if (!m.isNull()) {
            m->deleteLater();
        }
    }
}

