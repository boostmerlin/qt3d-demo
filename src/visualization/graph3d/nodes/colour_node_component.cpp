//
// Created by merlin
//

#include <QPhongMaterial>
#include <QPhongAlphaMaterial>
#include "colour_node_component.h"
#include "color_util.h"

ColourNodeComponent::ColourNodeComponent(RenderNode *renderNode) : RenderNodeComponent(renderNode) {
    onEnable();
}

void ColourNodeComponent::onEnable() {
    if (const auto color = nodeView<WithColor>()) {
        QColor c = color->color();
        switch (m_effectType) {
            case Tint:
                c = c.lighter(127);
                break;
            case TintColor:
                c = c * m_color;
                break;
            case Additive:
                c = c + m_color;
                break;
            case FixedColor:
                c = m_color;
                break;
            case InvertColor:
                c = ~c;
                break;
        }
        updateMatColor(c);
    }
}

void ColourNodeComponent::onDisable() {
    if (const auto color = nodeView<WithColor>()) {
        updateMatColor(color->color());
    }
}

template<class M>
static void setMaterialColor(M *mat, QColor color) {
    mat->setDiffuse(color);
    if (mat->objectName() == "HAS_AMBIENT") {
        mat->setAmbient(color);
    }
}

void ColourNodeComponent::updateMatColor(QColor color) const {
    auto withMaterial = nodeView<WithMaterial>();
    if (auto mat1 = withMaterial->material<Qt3DExtras::QPhongMaterial>()) {
        setMaterialColor(mat1, color);
    } else if (auto mat2 = withMaterial->material<Qt3DExtras::QPhongAlphaMaterial>()) {
        setMaterialColor(mat2, color);
    }
}

void ColourNodeComponent::setEffectType(EffectType effectType) {
    m_effectType = effectType;
}

void ColourNodeComponent::setColor(QColor color) {
    m_color = color;
}

