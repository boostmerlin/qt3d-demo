#pragma once

#include <QColor>
#include <QUuid>
#include <QVector3D>

#include "observable_object.h"

class SceneObject : public QObservableObject {
    Q_OBJECT

public:
    explicit SceneObject(QObject *parent = nullptr);
    ~SceneObject() override = default;

    X_PROPERTY_4(QString, name, name, setName)
    X_PROPERTY_4(QVector3D, position, position, setPosition)
    X_PROPERTY_4(QVector3D, rotation, rotation, setRotation)
    X_PROPERTY_4(QColor, color, color, setColor)

public:
    [[nodiscard]] const QUuid &id() const;
    [[nodiscard]] virtual QString typeName() const = 0;
    [[nodiscard]] SceneObject *parentSceneObject() const;
    [[nodiscard]] QList<SceneObject *> childSceneObjects() const;
    [[nodiscard]] bool isAncestorOf(const SceneObject *object) const;
    void restoreId(const QUuid &id);

private:
    QUuid m_id;
};
