//
// Created by merlin
//

#include <Qt3DExtras/QPhongMaterial>
#include "text3d_entity.h"

Text3dEntity::Text3dEntity(Qt3DCore::QNode *parent) : Qt3DCore::QEntity(parent) {
    m_transform = new Qt3DCore::QTransform();
    addComponent(m_transform);

    auto entity = new Qt3DCore::QEntity(this);
    m_textMesh = new Qt3DExtras::QExtrudedTextMesh;
    m_textMesh->setDepth(0.0f);
    m_textMaterial = new Qt3DExtras::QPhongMaterial;
    entity->addComponent(m_textMesh);
    entity->addComponent(m_textMaterial);
    m_pivotTransform = new Qt3DCore::QTransform;
    entity->addComponent(m_pivotTransform);

    connect(m_textMesh->geometry(), &Qt3DCore::QGeometry::minExtentChanged, this, &Text3dEntity::updatePivot);
    connect(m_textMesh->geometry(), &Qt3DCore::QGeometry::maxExtentChanged, this, &Text3dEntity::updatePivot);
}

void Text3dEntity::setColor(const QColor &color) {
    //todo
}

void Text3dEntity::setFont(const QFont &font) {
    m_textMesh->setFont(font);
}

void Text3dEntity::setScale(float scale) {
    m_transform->setScale(scale);
}

void Text3dEntity::setDepth(float depth) {
    m_textMesh->setDepth(depth);
}

 void Text3dEntity::setTranslation(const QVector3D &translation) {
    m_transform->setTranslation(translation);
}

void Text3dEntity::setText(const QString &text) {
    m_textMesh->setText(text);
}

void Text3dEntity::setPivot(const TextPivot &pivot) {
    m_pivot = pivot;
    updatePivot();
}

void Text3dEntity::updatePivot() {
    auto geometry = m_textMesh->geometry();
    auto min = geometry->minExtent();
    auto max = geometry->maxExtent();
    auto size = max - min;
    auto offset = QVector3D(size.x() * -m_pivot.x, size.y() * -m_pivot.y, size.z() * -m_pivot.z);
    m_pivotTransform->setTranslation(offset);
}
