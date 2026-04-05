//
// Created by merlin
//

#ifndef PMAXWELL_QT_TEXT3D_ENTITY_H
#define PMAXWELL_QT_TEXT3D_ENTITY_H

#include <QColor>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DRender/QMaterial>

struct TextPivot {
    float x;
    float y;
    float z;

    constexpr TextPivot(float x, float y, float z) noexcept: x(x), y(y), z(z) {}

    constexpr TextPivot() noexcept: x(0.5f), y(0.5f), z(0.5f) {}

    constexpr TextPivot(float x, float y) noexcept: x(x), y(y), z(0.5f) {}
};

namespace TextPivotConstants {
    const TextPivot centerBottom = TextPivot(0.5f, 0);
}

class Text3dEntity final : public Qt3DCore::QEntity {
Q_OBJECT

public:
    explicit Text3dEntity(Qt3DCore::QNode *parent = nullptr);

    void setText(const QString &text);

    void setColor(const QColor &color);

    void setFont(const QFont &font);

    void setScale(float scale);

    void setDepth(float depth);

    void setTranslation(const QVector3D &translation);

    void setPivot(const TextPivot &pivot);

private:
    void updatePivot();

private:
    Qt3DCore::QTransform *m_transform;
    Qt3DExtras::QExtrudedTextMesh *m_textMesh;
    Qt3DRender::QMaterial *m_textMaterial;

    Qt3DCore::QTransform *m_pivotTransform;
    TextPivot m_pivot{TextPivotConstants::centerBottom};
};


#endif //PMAXWELL_QT_TEXT3D_ENTITY_H
