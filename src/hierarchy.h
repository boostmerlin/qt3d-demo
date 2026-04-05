#pragma once

#include <QList>
#include <QObject>
#include <QPointer>

#include "demo/scene_object.h"

class QModelIndex;
class QStandardItem;
class QStandardItemModel;
class QTreeView;

class Hierarchy final : public QObject
{
    Q_OBJECT

public:
    explicit Hierarchy(QTreeView *view, QObject *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects);
    void setCurrentObject(SceneObject *object);
    void refreshObject(SceneObject *object);

signals:
    void currentObjectRequested(SceneObject *object);

private:
    static constexpr int ObjectRole = Qt::UserRole + 1;

    void rebuild();
    [[nodiscard]] QStandardItem *findObjectItem(SceneObject *object,
                                                QStandardItem *parentItem = nullptr) const;

    QPointer<QTreeView> m_view;
    QStandardItemModel *m_model{};
    QList<SceneObject *> m_objects;
    QPointer<SceneObject> m_currentObject;
    bool m_syncingSelection = false;
    bool m_rebuildingHierarchy = false;
};
