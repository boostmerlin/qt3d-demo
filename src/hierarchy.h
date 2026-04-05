#pragma once

#include <QList>
#include <QObject>
#include <QPointer>

#include "demo/primitive_object.h"

class QModelIndex;
class QStandardItem;
class QStandardItemModel;
class QTreeView;

class Hierarchy final : public QObject
{
    Q_OBJECT

public:
    explicit Hierarchy(QTreeView *view, QObject *parent = nullptr);

    void setObjects(const QList<PrimitiveObject *> &objects);
    void setCurrentObject(PrimitiveObject *object);
    void refreshObject(PrimitiveObject *object);

signals:
    void currentObjectRequested(PrimitiveObject *object);

private:
    static constexpr int ObjectRole = Qt::UserRole + 1;

    void rebuild();
    [[nodiscard]] QStandardItem *findObjectItem(PrimitiveObject *object,
                                                QStandardItem *parentItem = nullptr) const;

    QPointer<QTreeView> m_view;
    QStandardItemModel *m_model{};
    QList<PrimitiveObject *> m_objects;
    QPointer<PrimitiveObject> m_currentObject;
    bool m_syncingSelection = false;
    bool m_rebuildingHierarchy = false;
};
