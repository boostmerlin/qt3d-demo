#include "hierarchy.h"

#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QSignalBlocker>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QVariant>

namespace {
class HierarchyRenameDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override
    {
        Q_UNUSED(index);
        constexpr int verticalPadding = 3;
        constexpr int horizontalPadding = 1;
        editor->setGeometry(option.rect.adjusted(-horizontalPadding,
                                                 -verticalPadding,
                                                 horizontalPadding,
                                                 verticalPadding));
    }
};
}

Hierarchy::Hierarchy(QTreeView *view, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_model(new QStandardItemModel(this))
{
    Q_ASSERT(m_view);
    m_model->setHorizontalHeaderLabels({tr("Objects")});

    m_view->setModel(m_model);
    m_view->setItemDelegate(new HierarchyRenameDelegate(m_view));
    m_view->setHeaderHidden(false);
    m_view->setEditTriggers(QAbstractItemView::EditKeyPressed
                            | QAbstractItemView::SelectedClicked
                            | QAbstractItemView::DoubleClicked);

    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex &current) {
                if (m_syncingSelection || m_rebuildingHierarchy) {
                    return;
                }

                auto *item = m_model->itemFromIndex(current);
                auto *object = item
                                   ? qobject_cast<PrimitiveObject *>(item->data(ObjectRole).value<QObject *>())
                                   : nullptr;
                m_currentObject = object;
                emit currentObjectRequested(object);
            });

    connect(m_model, &QStandardItemModel::itemChanged, this, [](QStandardItem *item) {
        if (!item) {
            return;
        }

        auto *object = qobject_cast<PrimitiveObject *>(item->data(ObjectRole).value<QObject *>());
        if (!object) {
            return;
        }

        const QString text = item->text().trimmed();
        const QString nextName = text.isEmpty() ? object->name() : text;
        if (nextName != object->name()) {
            object->setName(nextName);
        }
        if (item->text() != nextName) {
            item->setText(nextName);
        }
    });
}

void Hierarchy::setObjects(const QList<PrimitiveObject *> &objects)
{
    m_objects = objects;
    rebuild();
}

void Hierarchy::setCurrentObject(PrimitiveObject *object)
{
    m_currentObject = object;

    if (!m_view || !m_view->selectionModel()) {
        return;
    }

    m_syncingSelection = true;
    if (!object) {
        m_view->selectionModel()->clearCurrentIndex();
    } else if (auto *item = findObjectItem(object)) {
        m_view->setCurrentIndex(item->index());
    }
    m_syncingSelection = false;
}

void Hierarchy::refreshObject(PrimitiveObject *object)
{
    if (!object) {
        return;
    }

    if (auto *item = findObjectItem(object)) {
        item->setText(object->name());
    }
}

void Hierarchy::rebuild()
{
    if (!m_view || !m_view->selectionModel()) {
        return;
    }

    m_rebuildingHierarchy = true;
    const QSignalBlocker blocker(m_model);

    auto buildItem = [this](auto &&self, PrimitiveObject *object) -> QStandardItem * {
        auto *item = new QStandardItem(object->name());
        item->setEditable(true);
        item->setData(QVariant::fromValue(static_cast<QObject *>(object)), ObjectRole);
        for (auto *child : object->childPrimitives()) {
            item->appendRow(self(self, child));
        }
        return item;
    };

    m_model->removeRows(0, m_model->rowCount());
    for (auto *object : m_objects) {
        if (!object->parentPrimitive()) {
            m_model->appendRow(buildItem(buildItem, object));
        }
    }

    m_view->expandAll();
    if (m_currentObject) {
        if (auto *item = findObjectItem(m_currentObject)) {
            m_view->selectionModel()->setCurrentIndex(item->index(),
                                                      QItemSelectionModel::ClearAndSelect
                                                          | QItemSelectionModel::Rows);
        }
    } else {
        m_view->selectionModel()->clearCurrentIndex();
    }

    m_rebuildingHierarchy = false;
}

QStandardItem *Hierarchy::findObjectItem(PrimitiveObject *object, QStandardItem *parentItem) const
{
    if (!object) {
        return nullptr;
    }

    const int rowCount = parentItem ? parentItem->rowCount() : m_model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        auto *item = parentItem ? parentItem->child(row) : m_model->item(row);
        if (!item) {
            continue;
        }

        auto *candidate = qobject_cast<PrimitiveObject *>(item->data(ObjectRole).value<QObject *>());
        if (candidate == object) {
            return item;
        }

        if (auto *nested = findObjectItem(object, item)) {
            return nested;
        }
    }
    return nullptr;
}
