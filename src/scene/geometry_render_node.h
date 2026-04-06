#pragma once

#include <QPhongMaterial>

#include "primitive_object.h"
#include "nodes/render_node.h"

class EdgeGeometryRenderNode : public RenderNodeEntityMix<WithMaterial, WithColor> {
public:
    EdgeGeometryRenderNode()
    {
        auto *material = new Qt3DExtras::QPhongMaterial();
        material->setShininess(0.0f);
        addMaterial(material);
    }

    bool updateRenderData(const QObject *object) override
    {
        const auto *primitive = qobject_cast<const PrimitiveObject *>(object);
        if (!primitive) {
            return false;
        }
        RenderNode::updateRenderData(object);
        m_geometry = primitive;
        return true;
    }

protected:
    template<typename T = PrimitiveObject>
    [[nodiscard]] const T *geometry() const
    {
        return qobject_cast<const T *>(m_geometry);
    }

    void updateTransform() const
    {
        Q_ASSERT(m_geometry);
        auto *nodeTransform = transform();
        Q_ASSERT(nodeTransform);
        nodeTransform->setTranslation(m_geometry->position());
        nodeTransform->setRotation(QQuaternion::fromEulerAngles(m_geometry->rotation()));
    }

    void updateMaterial()
    {
        auto *mat = material<Qt3DExtras::QPhongMaterial>();
        Q_ASSERT(mat);
        const QColor c = color();
        mat->setDiffuse(c);
        mat->setAmbient(c);
        mat->setSpecular(QColorConstants::Black);
    }

    const PrimitiveObject *m_geometry{};
};
