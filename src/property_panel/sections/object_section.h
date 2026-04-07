#pragma once

#include <QPointer>

#include "property_panel/property_section.h"

class ColorEditor;
class EnumEditor;
class QPushButton;
class TextEditorRow;

class ObjectSection final : public PropertySection
{
    Q_OBJECT

public:
    explicit ObjectSection(SceneController *sceneController = nullptr, QWidget *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects) override;
    void setCurrentObject(SceneObject *object) override;
    void refreshObject(SceneObject *object) override;

signals:
    void parentChangeRequested(SceneObject *object, SceneObject *parentObject);
    void removeRequested(SceneObject *object);

private:
    void refreshEditors();
    void refreshParentOptions();

    QList<SceneObject *> m_objects;
    QPointer<SceneObject> m_currentObject;
    TextEditorRow *m_nameEditor{};
    TextEditorRow *m_typeEditor{};
    EnumEditor *m_parentEditor{};
    ColorEditor *m_colorEditor{};
    QPushButton *m_deleteButton{};
    bool m_updating = false;
};
