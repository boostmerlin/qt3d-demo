//
// Created by merlin
//

#ifndef PMAXWELL_QT_GIZMO_AXIS_H
#define PMAXWELL_QT_GIZMO_AXIS_H

#include <QColor>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QGeometryView>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QCylinderGeometryView>
#include <Qt3DExtras/QConeGeometryView>
#include "text3d_entity.h"

// immutable
struct AxisOptions {
    const bool showAxisName = false;
    const bool showAxis = true;
    const QColor colorOfAxis = QColorConstants::Black;
    const QColor mirrorColor;
    const bool mirrorAxis = false;
    const float scale = 1.0f;

    const float lineLength = 0.55f;
    const float arrowLength = 0;
    const float arrowRadius = 0;
    const float lineRadius = 0;
    const QVector3D offset = QVector3D(0, 0, 0);
    const float anchor = 0.0f; //1 means arrow top
    const QVector3D rotation = QVector3D(0, 0, 0);

    [[nodiscard]] float calcArrowLength() const {
        return arrowLength == 0 ? 0.18f * lineLength : arrowLength;
    }

    [[nodiscard]] float calcArrowRadius() const {
        return arrowRadius == 0 ? 0.4f * calcArrowLength() : arrowRadius;
    }

    [[nodiscard]] float calcLineRadius() const {
        return lineRadius == 0 ? 0.5f * calcArrowRadius() : lineRadius;
    }

    AxisOptions(bool showAxisName,
                bool showAxis,
                const QColor &color,
                const QColor &backwardColor,
                bool mirrorAxis,
                float scale,
                float lineLength,
                float arrowLength,
                float arrowRadius,
                float lineRadius,
                const QVector3D &offset,
                float anchor,
                const QVector3D &rotation) noexcept
            : showAxisName(showAxisName), showAxis(showAxis), colorOfAxis(color), mirrorAxis(mirrorAxis), scale(scale),
              lineLength(lineLength), arrowLength(arrowLength), arrowRadius(arrowRadius), lineRadius(lineRadius),
              mirrorColor(backwardColor), offset(offset), anchor(anchor), rotation(rotation) {
    }

    AxisOptions() = default;

    [[nodiscard]] AxisOptions setMirrorAxis(bool flag) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, flag, scale, lineLength, arrowLength, arrowRadius,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setShowAxisName(bool flag) const {
        return {flag, showAxis, colorOfAxis, mirrorColor, mirrorAxis, scale, lineLength, arrowLength, arrowRadius,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setShowAxis(bool flag) const {
        return {showAxisName, flag, colorOfAxis, mirrorColor, mirrorAxis, scale, lineLength, arrowLength, arrowRadius,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setAxisColor(const QColor &color) const {
        return {showAxisName, showAxis, color, mirrorColor, mirrorAxis, scale, lineLength, arrowLength, arrowRadius,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setMirrorAxisColor(const QColor &color) const {
        return {showAxisName, showAxis, colorOfAxis, color, mirrorAxis, scale, lineLength, arrowLength, arrowRadius,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setScale(float value) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, mirrorAxis, value, lineLength, arrowLength,
                arrowRadius,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setLineLength(float value) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, mirrorAxis, scale, value, arrowLength, arrowRadius,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setArrowLength(float value) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, mirrorAxis, scale, lineLength, value, arrowRadius,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setArrowRadius(float value) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, mirrorAxis, scale, lineLength, arrowLength, value,
                lineRadius, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setLineRadius(float value) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, mirrorAxis, scale, lineLength, arrowLength,
                arrowRadius, value, offset, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setOffset(const QVector3D &value) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, mirrorAxis, scale, lineLength, arrowLength,
                arrowRadius, lineRadius, value, anchor, rotation};
    }

    [[nodiscard]] AxisOptions setAnchor(float value) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, mirrorAxis, scale, lineLength, arrowLength,
                arrowRadius, lineRadius, offset, value, rotation};
    }

    [[nodiscard]] AxisOptions setRotation(const QVector3D &value) const {
        return {showAxisName, showAxis, colorOfAxis, mirrorColor, mirrorAxis, scale, lineLength, arrowLength,
                arrowRadius, lineRadius, offset, anchor, value};
    }

};

class GizmoAxis final : public Qt3DCore::QEntity {
Q_OBJECT

public:
    enum AxisType {
        x = 0, y, z, N
    };

    Q_ENUM(AxisType)

    static const AxisOptions defaultOption;

    using QGeometry = Qt3DCore::QGeometryView;
    using QCylinderGeometry = Qt3DExtras::QCylinderGeometryView;
    using QConeGeometry = Qt3DExtras::QConeGeometryView;

    explicit GizmoAxis(Qt3DCore::QNode *parent = nullptr, bool shareGeometry = true);

    inline void setScale(float scale) {
        m_transform->setScale3D(QVector3D(scale, scale, scale));
    }

    [[nodiscard]] inline Qt3DCore::QTransform *transform() const {
        return m_transform;
    }

    void setAxis(AxisType axis, const AxisOptions &options = defaultOption);

    [[nodiscard]] QVector3D getArrowPosition(GizmoAxis::AxisType axis) const;
private:
    void createAxisElements(GizmoAxis::AxisType axis, const AxisOptions &options);

    struct AxisElements {
        using HandleMaterial = Qt3DExtras::QPhongMaterial;
        Qt3DCore::QEntity *handle;
        Qt3DCore::QTransform *handleTransform;
        HandleMaterial *handleMaterial;
        HandleMaterial *mirrorHandleMaterial;
        Text3dEntity *handleText;

        Qt3DCore::QEntity *forwardHandle;
        Qt3DCore::QEntity *backwardHandle;
    };

    QVector3D getAnchoredPosition(GizmoAxis::AxisType axis, const AxisOptions &options) const;

    QVector3D getLinePosition(GizmoAxis::AxisType axis);

    QGeometry *peekGeometry(GizmoAxis::AxisType axis);

    QCylinderGeometry *getLineGeometry(GizmoAxis::AxisType axis);

    QConeGeometry *getArrowGeometry(GizmoAxis::AxisType axis);

    void updateHandleGeometry(Qt3DCore::QEntity *handle, GizmoAxis::AxisType axis, const AxisOptions &options);

private:
    Qt3DCore::QTransform *m_transform;

    AxisElements m_axisElements[N]{nullptr};

    std::tuple<QGeometry *, QVector3D, QVector3D> m_axisConfiguration[N][2];
};


#endif //PMAXWELL_QT_GIZMO_AXIS_H
