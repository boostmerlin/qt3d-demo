#include "property_panel/sections/object_section.h"

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "property_panel/editors/color_editor.h"
#include "property_panel/editors/enum_editor.h"
#include "property_panel/editors/text_editor_row.h"
#include "property_panel/property_editor_factory.h"
#include "scene/scene_controller.h"

ObjectSection::ObjectSection(SceneController *sceneController, QWidget *parent)
    : PropertySection(sceneController, parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto *title = new QLabel(tr("Object"), this);
    title->setStyleSheet("font-weight: 600; color: #334155; padding-top: 4px;");
    layout->addWidget(title);

    m_nameEditor = PropertyEditorFactory::createTextEditor(tr("Name"), this);
    layout->addWidget(m_nameEditor);
    connect(m_nameEditor, &TextEditorRow::editingFinished, this, [this](const QString &text) {
        if (!m_updating && m_currentObject && m_sceneController) {
            m_sceneController->setObjectName(m_currentObject, text);
        }
    });

    m_typeEditor = PropertyEditorFactory::createTextEditor(tr("Type"), this);
    m_typeEditor->setReadOnly(true);
    layout->addWidget(m_typeEditor);

    m_parentEditor = PropertyEditorFactory::createEnumEditor(tr("Parent"), this);
    layout->addWidget(m_parentEditor);
    connect(m_parentEditor, &EnumEditor::valueEdited, this, [this](const QVariant &value) {
        if (m_updating || !m_currentObject) {
            return;
        }
        auto *parentObject = qobject_cast<SceneObject *>(value.value<QObject *>());
        emit parentChangeRequested(m_currentObject, parentObject);
    });

    m_colorEditor = PropertyEditorFactory::createColorEditor(tr("Color"), this);
    m_colorEditor->setDialogTitle(tr("Select Object Color"));
    layout->addWidget(m_colorEditor);
    connect(m_colorEditor, &ColorEditor::valueEdited, this, [this](const QColor &color) {
        if (!m_updating && m_currentObject && m_sceneController) {
            m_sceneController->setObjectColor(m_currentObject, color);
        }
    });

    m_deleteButton = new QPushButton(tr("Delete Object"), this);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setStyleSheet(
        "QPushButton {"
        " background: #fff1f2;"
        " color: #be123c;"
        " border: 1px solid #fecdd3;"
        " border-radius: 8px;"
        " padding: 8px 12px;"
        " font-weight: 600;"
        "}"
        "QPushButton:hover {"
        " background: #ffe4e6;"
        "}"
        "QPushButton:pressed {"
        " background: #fecdd3;"
        "}"
        "QPushButton:disabled {"
        " color: #b8c0cc;"
        " background: #f8fafc;"
        " border-color: #e2e8f0;"
        "}");
    layout->addWidget(m_deleteButton);
    connect(m_deleteButton, &QPushButton::clicked, this, [this] {
        if (!m_currentObject) {
            return;
        }

        const auto answer = QMessageBox::question(
            this,
            tr("Delete Object"),
            tr("Delete \"%1\"?").arg(m_currentObject->name()),
            QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (answer == QMessageBox::Yes) {
            emit removeRequested(m_currentObject);
        }
    });
}

void ObjectSection::setObjects(const QList<SceneObject *> &objects)
{
    m_objects = objects;
    refreshParentOptions();
}

void ObjectSection::setCurrentObject(SceneObject *object)
{
    m_currentObject = object;
    refreshEditors();
}

void ObjectSection::refreshObject(SceneObject *object)
{
    if (!object) {
        return;
    }
    if (object == m_currentObject) {
        refreshEditors();
    } else {
        refreshParentOptions();
    }
}

void ObjectSection::refreshEditors()
{
    m_updating = true;
    const bool hasObject = m_currentObject != nullptr;
    setVisible(hasObject);
    m_nameEditor->setEnabled(hasObject);
    m_parentEditor->setEnabled(hasObject);
    m_colorEditor->setEnabled(hasObject);
    m_deleteButton->setEnabled(hasObject);

    if (!hasObject) {
        m_nameEditor->setText(QString());
        m_typeEditor->setText(QString());
        m_colorEditor->setColor(QColor(QStringLiteral("#e2e8f0")));
        refreshParentOptions();
        m_updating = false;
        return;
    }

    m_nameEditor->setText(m_currentObject->name());
    m_typeEditor->setText(m_currentObject->typeName());
    m_colorEditor->setColor(m_currentObject->color());
    refreshParentOptions();
    m_updating = false;
}

void ObjectSection::refreshParentOptions()
{
    QList<EnumEditorItem> items;
    items.append({tr("<Scene Root>"), QVariant::fromValue(static_cast<QObject *>(nullptr))});

    for (auto *candidate : m_objects) {
        if (!m_currentObject || candidate == m_currentObject || m_currentObject->isAncestorOf(candidate)) {
            continue;
        }
        items.append({candidate->name(), QVariant::fromValue(static_cast<QObject *>(candidate))});
    }

    m_parentEditor->setItems(items);
    if (!m_currentObject) {
        m_parentEditor->setCurrentValue(QVariant::fromValue(static_cast<QObject *>(nullptr)));
        return;
    }

    m_parentEditor->setCurrentValue(QVariant::fromValue(static_cast<QObject *>(m_currentObject->parentSceneObject())));
}
