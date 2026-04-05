//
// Created by merlin
//

#include "behavior_component.h"

BehaviorComponent::BehaviorComponent(Qt3DCore::QNode *parent) : Qt3DCore::QComponent(parent), m_order(0) {
    connect(this, &BehaviorComponent::enabledChanged, [this](bool enabled) {
        if (enabled) {
            onEnable();
        } else {
            onDisable();
        }
    });
}

void BehaviorComponent::onEnable() {
}

void BehaviorComponent::onDisable() {
}

void BehaviorComponent::setOrder(int order) {
    m_order = order;
}

int BehaviorComponent::order() const {
    return m_order;
}

void BehaviorComponent::setValid() {
    if (isEnabled()) {
        onEnable();
    }
}

void BehaviorComponent::setInvalid() {
    if (isEnabled()) {
        onDisable();
    }
}
