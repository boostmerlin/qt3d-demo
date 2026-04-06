//
// Created by merlin
//

#ifndef PMAXWELL_QT_RENDER_NODE_H
#define PMAXWELL_QT_RENDER_NODE_H

#include <QPointer>
#include <QList>
#include <QQueue>
#include <Qt3DCore/QTransform>
#include <QColor>
#include "layer_manager.h"

#define NODE_OBJECT_ID(Type) QString(Type::staticMetaObject.className())
#define DECL_NODE_CREATOR(param) static RenderNode *create(const QObject *(param))

class RenderNode : public QObject {
Q_OBJECT

    template<class C>
    QList<C *> findComponentsHelper(const Qt3DCore::QEntity *root, bool includeDisable, bool onlyFirst) const {
        QList<C *> matchComponents;
        if (!root) {
            return matchComponents;
        }

        QQueue<const Qt3DCore::QEntity *> q;
        q.enqueue(root);
        while (!q.isEmpty()) {
            const auto *node = q.dequeue();
            for (auto *child: node->childNodes()) {
                if (auto *renderNode = qobject_cast<const Qt3DCore::QEntity *>(child)) {
                    q.enqueue(renderNode);
                }
            }
            const auto &comps = node->components();
            for (Qt3DCore::QComponent *component: comps) {
                if (!includeDisable && !component->isEnabled())
                    continue;
                C *typedComponent = qobject_cast<C *>(component);
                if (typedComponent != nullptr) {
                    matchComponents.append(typedComponent);
                    if (onlyFirst) {
                        return matchComponents;
                    }
                }
            }
        }
        return matchComponents;
    }

public:
    static constexpr float NEAREST_ZERO = 0.00001f;

    //update render property here
    virtual bool updateRenderData(const QObject *);

    [[nodiscard]] const QObject *objectData() const;

    virtual Qt3DCore::QEntity *createEntity(Qt3DCore::QNode *parent) noexcept = 0;

    virtual void updateEntity() = 0;

    virtual void removeEntity(bool disposeSelf) = 0;

    [[nodiscard]] virtual QString layer() const = 0;

    [[nodiscard]] virtual Qt3DCore::QEntity *entity() const = 0;

    [[nodiscard]] Qt3DCore::QTransform *transform() const;

    void addComponent(Qt3DCore::QComponent *component) const;

    void removeComponent(Qt3DCore::QComponent *component) const;

    [[nodiscard]] virtual bool isProxyNode() const;

    void setParentNode(RenderNode *parent);

    [[nodiscard]] RenderNode *parentNode() const;

    template<class C>
    void removeComponents() const {
        auto comps = getComponents<C>();
        for (auto *component: comps) {
            entity()->removeComponent(component);
        }
    }

    template<class C>
    C *getComponent(bool includeDisable = true) const {
        static_assert(std::is_base_of_v<Qt3DCore::QComponent, C>, "C must be a subclass of Qt3DCore::QComponent");
        const auto &comps = entity()->components();
        for (auto *component: comps) {
            if (!includeDisable && !component->isEnabled())
                continue;
            C *typedComponent = qobject_cast<C *>(component);
            if (typedComponent != nullptr) {
                return typedComponent;
            }
        }
        return nullptr;
    }

    template<class C>
    QList<C *> getComponents(bool includeDisable = true) const {
        static_assert(std::is_base_of_v<Qt3DCore::QComponent, C>, "C must be a subclass of Qt3DCore::QComponent");
        QList<C *> matchComponents;
        const auto &comps = entity()->components();
        for (Qt3DCore::QComponent *component: comps) {
            if (!includeDisable && !component->isEnabled())
                continue;
            C *typedComponent = qobject_cast<C *>(component);
            if (typedComponent != nullptr)
                matchComponents.append(typedComponent);
        }
        return matchComponents;
    }

    template<class C>
    C *getComponentInChildren(bool includeDisable = true) const {
        static_assert(std::is_base_of_v<Qt3DCore::QComponent, C>, "C must be a subclass of Qt3DCore::QComponent");
        auto comps = findComponentsHelper<C>(entity(), includeDisable, true);
        return comps.isEmpty() ? nullptr : comps.first();
    }

    template<class C>
    QList<C *> getComponentsInChildren(bool includeDisable = true) const {
        static_assert(std::is_base_of_v<Qt3DCore::QComponent, C>, "C must be a subclass of Qt3DCore::QComponent");
        return findComponentsHelper<C>(entity(), includeDisable, false);
    }

    template<class T>
    T asType() const {
        return dynamic_cast<T>(const_cast<RenderNode *>(this));
    }

    static QVector3D approx(const QVector3D &v) {
        return {qFuzzyIsNull(v.x()) ? NEAREST_ZERO : v.x(),
                qFuzzyIsNull(v.y()) ? NEAREST_ZERO : v.y(),
                qFuzzyIsNull(v.z()) ? NEAREST_ZERO : v.z()};
    }

    static QVector3D neglect(const QVector3D &v, bool x = false, bool y = false, bool z = false) {
        return approx({x ? 0 : v.x(),
                       y ? 0 : v.y(),
                       z ? 0 : v.z()});
    }

private:
    QPointer<QObject> m_objectData;
};

class ProxyRenderNode : public RenderNode {
Q_OBJECT

public:
    explicit ProxyRenderNode(RenderNode *renderNode);

    ~ProxyRenderNode() override;

    bool updateRenderData(const QObject *data) override;

    Qt3DCore::QEntity *createEntity(Qt3DCore::QNode *parent) noexcept override;

    void updateEntity() override;

    void removeEntity(bool disposeSelf) override;

    [[nodiscard]] Qt3DCore::QEntity *entity() const override;

    template<typename T>
    T *proxyRenderNode() const {
        return qobject_cast<T *>(m_renderNode);
    }

    [[nodiscard]] bool isProxyNode() const override;

    [[nodiscard]] QString layer() const override;

private:
    RenderNode *m_renderNode;
};

class RenderNodeEntity : public RenderNode {
Q_OBJECT

public:
    Qt3DCore::QEntity *createEntity(Qt3DCore::QNode *parent) noexcept override;

    void updateEntity() override;

    void removeEntity(bool disposeSelf) override;

    [[nodiscard]] Qt3DCore::QEntity *entity() const override;

    [[nodiscard]] QString layer() const override;

protected:
    virtual void onCreate() = 0;

    virtual void onUpdate() = 0;

    virtual void onRemove();

protected:
    Qt3DCore::QEntity *m_entity{};
    Qt3DCore::QTransform *m_transform{};
    QString m_layer;
};

template<typename RenderNode, typename... With>
class RenderNodeMix : public RenderNode, public With ... {
public:
    template<typename... Args>
    explicit RenderNodeMix(Args... args) : RenderNode(args...) {
    }
};

template<typename... With>
using RenderNodeEntityMix = RenderNodeMix<RenderNodeEntity, With...>;

//include <QMaterial> cause compile error, why?
class WithMaterial {
public:
    virtual ~WithMaterial();

    virtual void updateMaterial() = 0;

    [[nodiscard]] qsizetype size() const;

    template<typename Material = Qt3DCore::QComponent>
    [[nodiscard]] Material *material(qsizetype at = -1) const {
        auto i = at == -1 ? materialIndex() : at;
        if (i >= size()) {
            return nullptr;
        }
        return qobject_cast<Material *>(m_materials[i]);
    }

    void addMaterial(Qt3DCore::QComponent *material);

    Qt3DCore::QComponent *attachMaterial(Qt3DCore::QEntity *entity = nullptr) const;

    template<typename Material>
    void useMaterial(std::function<void(Material *)> callback = nullptr) {
        int i = -1;
        for (int j = 0; j < m_materials.size(); ++j) {
            if (auto _ = qobject_cast<Material *>(m_materials[j].data())) {
                i = j;
                break;
            }
        }
        useMaterial(i, callback);
    }

    template<typename Material=Qt3DCore::QComponent>
    void useMaterial(qsizetype index, std::function<void(Material *)> callback = nullptr) {
        if (index >= size() || index < 0) {
            qWarning() << "Material index out of range";
            return;
        }
        auto m = material(index);
        if (!m) {
            return;
        }
        if (callback) {
            if (auto mat = qobject_cast<Material *>(m)) {
                callback(mat);
            }
        }
        if (m_current != index) {
            auto prev = material(m_current);
            m_current = index;
            attachMaterial(prev->entities().first());
        }
    }

    [[nodiscard]] qsizetype materialIndex() const;

private:
    qsizetype m_current = 0;
    QList<QPointer<Qt3DCore::QComponent>> m_materials;
};

class WithAlpha {
public:
    void setAlpha(float alpha);

    [[nodiscard]] float alpha() const;

private:
    float m_alpha = 1.f;
};

class WithColor {
public:
    constexpr static QColor defaultColor = QColorConstants::Cyan;
    constexpr static QColor defaultAmbientColor = QColor(160, 160, 160);

    void setColor(const QColor &color);

    [[nodiscard]] QColor color() const;

protected:
    std::optional<QColor> m_color;
};

class Selectable {
public:
    virtual ~Selectable() = default;

    void setSelectable(bool selectable);

    [[nodiscard]] bool selectable() const;

    void select();

    void deselect();

protected:
    virtual void onSelect() noexcept = 0;

    virtual void onDeselect() noexcept = 0;

private:
    bool m_selectable = true;
    bool m_selected = false;
};

#endif //PMAXWELL_QT_RENDER_NODE_H
