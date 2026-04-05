//
// Created by merlin
//

#include <Qt3DExtras/QCuboidMesh>
#include <QVector3D>
#include <QtMath>
#include <QMetaEnum>
#include "gizmo_axis.h"

const AxisOptions GizmoAxis::defaultOption = AxisOptions();

GizmoAxis::GizmoAxis(Qt3DCore::QNode *parent, bool shareGeometry) : Qt3DCore::QEntity(parent) {
    m_transform = new Qt3DCore::QTransform();
    addComponent(m_transform);
    auto arrow = new QConeGeometry();
    auto line = new QCylinderGeometry();
    constexpr float arrowAngle = qRadiansToDegrees(M_PI_2);
    m_axisConfiguration[x][0] = {arrow, QVector3D(1.0f, 0, 0), QVector3D(0, 0, -arrowAngle)};
    m_axisConfiguration[x][1] = {line, QVector3D(0.5f, 0.0f, 0), QVector3D(0, 0, -arrowAngle)};
    if (!shareGeometry) {
        arrow = new QConeGeometry();
        line = new QCylinderGeometry();
    }
    m_axisConfiguration[y][0] = {arrow, QVector3D(0, 1.0f, 0), QVector3D(0, 0, 0)};
    m_axisConfiguration[y][1] = {line, QVector3D(0, 0.5f, 0), QVector3D(0, 0, 0)};

    if (!shareGeometry) {
        arrow = new QConeGeometry();
        line = new QCylinderGeometry();
    }
    m_axisConfiguration[z][0] = {arrow, QVector3D(0, 0, 1.0f), QVector3D(arrowAngle, 0, 0)};
    m_axisConfiguration[z][1] = {line, QVector3D(0, 0, 0.5f), QVector3D(arrowAngle, 0, 0)};
}

void GizmoAxis::createAxisElements(GizmoAxis::AxisType axis, const AxisOptions &options) {
    auto config = m_axisConfiguration[axis];
    AxisElements &a = m_axisElements[axis];

    if (!a.handle) {
        a.handle = new Qt3DCore::QEntity(this);
        a.handleTransform = new Qt3DCore::QTransform();
        a.handle->addComponent(a.handleTransform);
    }

    auto createHandle = [&](bool forward) {
        auto handle = new Qt3DCore::QEntity(a.handle);
        auto transform = new Qt3DCore::QTransform(handle);
        if (!forward) {
            QVector3D v = getArrowPosition(axis);
            transform->setRotation(QQuaternion::rotationTo(v, -v));
        }
        handle->addComponent(transform);
        for (int i = 0; i < 2; i++) {
            auto &[geometry, _, rotation] = config[i];
            auto entity = new Qt3DCore::QEntity(handle);
            auto renderer = new Qt3DRender::QGeometryRenderer(entity);
            renderer->setView(geometry);
            entity->addComponent(renderer);
            auto t = new Qt3DCore::QTransform(entity);
            t->setRotationX(rotation.x());
            t->setRotationY(rotation.y());
            t->setRotationZ(rotation.z());
            entity->addComponent(t);
            entity->addComponent(forward ? a.handleMaterial : a.mirrorHandleMaterial);

            if (i == 0) {
                entity->setObjectName("arrow");
                if (!a.handleText && options.showAxisName) {
                    a.handleText = new Text3dEntity(entity);
                    a.handleText->setText(QMetaEnum::fromType<GizmoAxis::AxisType>().valueToKey(axis));
                    a.handleText->setColor(QColorConstants::Black);
                    a.handleText->setScale(0.10f);
                }
            } else {
                entity->setObjectName("line");
            }
        }
        return handle;
    };

    if (!a.forwardHandle && options.showAxis) {
        a.handleMaterial = new AxisElements::HandleMaterial();
        a.forwardHandle = createHandle(true);
    }
    if (!a.backwardHandle && options.mirrorAxis) {
        a.mirrorHandleMaterial = new AxisElements::HandleMaterial();
        a.backwardHandle = createHandle(false);
    }
}

GizmoAxis::QGeometry *GizmoAxis::peekGeometry(GizmoAxis::AxisType axis) {
    auto &config = m_axisConfiguration[axis];
    return std::get<0>(config[0]);
}

GizmoAxis::QConeGeometry *GizmoAxis::getArrowGeometry(GizmoAxis::AxisType axis) {
    auto &config = m_axisConfiguration[axis];
    return qobject_cast<QConeGeometry *>(std::get<0>(config[0]));
}

GizmoAxis::QCylinderGeometry *GizmoAxis::getLineGeometry(GizmoAxis::AxisType axis) {
    auto &config = m_axisConfiguration[axis];
    return qobject_cast<QCylinderGeometry *>(std::get<0>(config[1]));
}

QVector3D GizmoAxis::getArrowPosition(GizmoAxis::AxisType axis) const {
    auto &config = m_axisConfiguration[axis];
    return std::get<1>(config[0]);
}

QVector3D GizmoAxis::getLinePosition(GizmoAxis::AxisType axis) {
    auto &config = m_axisConfiguration[axis];
    return std::get<1>(config[1]);
}

void GizmoAxis::updateHandleGeometry(Qt3DCore::QEntity *handle, GizmoAxis::AxisType axis, const AxisOptions &options) {
    if (!handle) {
        return;
    }
    auto arrowGeometry = getArrowGeometry(axis);
    arrowGeometry->setLength(options.calcArrowLength());
    arrowGeometry->setBottomRadius(options.calcArrowRadius());
    arrowGeometry->setSlices(qMax(30, int(options.calcArrowRadius() * 30)));
    auto arrow = handle->findChild<Qt3DCore::QEntity *>("arrow");
    auto transform = arrow->componentsOfType<Qt3DCore::QTransform>().first();
    float totalLength = options.lineLength + options.calcArrowLength() * 0.5f;
    auto arrowPosition = getArrowPosition(axis);
    auto anchorPosition = getAnchoredPosition(axis, options);
    transform->setTranslation(arrowPosition * totalLength - anchorPosition);

    auto lineGeometry = getLineGeometry(axis);
    lineGeometry->setRadius(options.calcLineRadius());
    lineGeometry->setLength(options.lineLength);
    lineGeometry->setSlices(qMax(20, int(options.calcLineRadius() * 30)));
    auto line = handle->findChild<Qt3DCore::QEntity *>("line");
    transform = line->componentsOfType<Qt3DCore::QTransform>().first();
    auto linePosition = getLinePosition(axis);
    transform->setTranslation(linePosition * options.lineLength - anchorPosition);
}

void GizmoAxis::setAxis(GizmoAxis::AxisType axis, const AxisOptions &options) {
    createAxisElements(axis, options);

    AxisElements &a = m_axisElements[axis];
    if (a.handle) {
        a.handle->setEnabled(options.showAxis || options.mirrorAxis);
        a.handleTransform->setScale(options.scale);
        a.handleTransform->setTranslation(options.offset);
    }

    auto current = peekGeometry(axis);
    auto next = peekGeometry(static_cast<GizmoAxis::AxisType>((axis + 1) % GizmoAxis::N));
    bool sharedGeometry = current == next;
    Q_UNUSED(sharedGeometry)
    if (a.forwardHandle) {
        a.forwardHandle->setEnabled(options.showAxis);
        if (sharedGeometry) { //reset all offset
            updateHandleGeometry(m_axisElements[GizmoAxis::x].forwardHandle, GizmoAxis::x, options);
            updateHandleGeometry(m_axisElements[GizmoAxis::y].forwardHandle, GizmoAxis::y, options);
            updateHandleGeometry(m_axisElements[GizmoAxis::z].forwardHandle, GizmoAxis::z, options);
        } else {
            updateHandleGeometry(a.forwardHandle, axis, options);
        }
        a.handleMaterial->setAmbient(options.colorOfAxis);
    }
    if (a.backwardHandle) {
        a.backwardHandle->setEnabled(options.mirrorAxis);
        if (sharedGeometry) { //reset all offset
            updateHandleGeometry(m_axisElements[GizmoAxis::x].backwardHandle, GizmoAxis::x, options);
            updateHandleGeometry(m_axisElements[GizmoAxis::y].backwardHandle, GizmoAxis::y, options);
            updateHandleGeometry(m_axisElements[GizmoAxis::z].backwardHandle, GizmoAxis::z, options);
        } else {
            updateHandleGeometry(a.backwardHandle, axis, options);
        }
        a.mirrorHandleMaterial->setAmbient(
                options.mirrorColor.isValid() ? options.mirrorColor : options.colorOfAxis);
    }
    if (a.handleText) {
        a.handleText->setEnabled(options.showAxisName);
        a.handleText->setTranslation(QVector3D(0, 0.8f * options.calcArrowLength(), 0));
    }
}

QVector3D GizmoAxis::getAnchoredPosition(GizmoAxis::AxisType axis, const AxisOptions &options) const {
    return getArrowPosition(axis) * (options.lineLength + options.calcArrowLength()) * options.anchor;
}

