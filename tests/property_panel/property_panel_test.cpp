#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalSpy>
#include <QTimer>
#include <QtTest/QtTest>

#include "property_panel/editors/enum_editor.h"
#include "property_panel/editors/reorderable_point_list_editor.h"
#include "property_panel/editors/vector3_editor.h"
#include "property_panel/sections/object_section.h"
#include "scene/primitive_object.h"
#include "scene/scene_controller.h"
#include "scene/scene_object.h"

class TestSceneObject final : public SceneObject
{
    Q_OBJECT

public:
    explicit TestSceneObject(QObject *parent = nullptr)
        : SceneObject(parent)
    {
    }

    [[nodiscard]] QString typeName() const override
    {
        return QStringLiteral("Test Object");
    }
};

class PropertyPanelEditorTest final : public QObject
{
    Q_OBJECT

private slots:
    void vector3EditorEmitsCombinedVector();
    void vector3EditorUpdatesSpinBoxes();
    void enumEditorEmitsSelectedItemData();
    void enumEditorSelectsItemByValue();
    void pointListEditorEmitsPointEdited();
    void pointListEditorEmitsAddRequested();
    void objectSectionOnlyEmitsRemoveAfterConfirmation();
    void objectSectionDoesNotEmitRemoveWhenDialogCancelled();
    void objectSectionNameEditCreatesUndoableCommand();
};

void PropertyPanelEditorTest::vector3EditorEmitsCombinedVector()
{
    Vector3Editor editor(QStringLiteral("Position"));
    editor.setRange(-100.0, 100.0);
    editor.setValue(QVector3D(1.0f, 2.0f, 3.0f));

    QSignalSpy spy(&editor, &Vector3Editor::valueEdited);
    QVERIFY(spy.isValid());

    const auto edits = editor.findChildren<QDoubleSpinBox *>();
    QCOMPARE(edits.size(), 3);

    edits[1]->setValue(7.5);

    QCOMPARE(spy.count(), 1);
    const QVector3D value = spy.at(0).at(0).value<QVector3D>();
    QCOMPARE(value.x(), 1.0f);
    QCOMPARE(value.y(), 7.5f);
    QCOMPARE(value.z(), 3.0f);
}

void PropertyPanelEditorTest::vector3EditorUpdatesSpinBoxes()
{
    Vector3Editor editor(QStringLiteral("Rotation"));
    editor.setValue(QVector3D(4.0f, 5.5f, -6.25f));

    const auto edits = editor.findChildren<QDoubleSpinBox *>();
    QCOMPARE(edits.size(), 3);

    QCOMPARE(edits[0]->value(), 4.0);
    QCOMPARE(edits[1]->value(), 5.5);
    QCOMPARE(edits[2]->value(), -6.25);
}

void PropertyPanelEditorTest::enumEditorEmitsSelectedItemData()
{
    EnumEditor editor(QStringLiteral("Mode"));
    editor.setItems({
        {QStringLiteral("First"), 10},
        {QStringLiteral("Second"), 20},
        {QStringLiteral("Third"), 30},
    });

    QSignalSpy spy(&editor, &EnumEditor::valueEdited);
    QVERIFY(spy.isValid());

    auto *combo = editor.findChild<QComboBox *>();
    QVERIFY(combo != nullptr);

    combo->setCurrentIndex(2);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 30);
}

void PropertyPanelEditorTest::enumEditorSelectsItemByValue()
{
    EnumEditor editor(QStringLiteral("Mode"));
    editor.setItems({
        {QStringLiteral("Line"), 0},
        {QStringLiteral("Loop"), 2},
    });

    editor.setCurrentValue(2);

    auto *combo = editor.findChild<QComboBox *>();
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->currentText(), QStringLiteral("Loop"));
}

void PropertyPanelEditorTest::pointListEditorEmitsPointEdited()
{
    PointListEditorConfig config;
    config.addButtonText = QStringLiteral("Add Point");
    config.dimension = 3;
    config.rowLabel = [](int index) { return QStringLiteral("P%1").arg(index + 1); };

    ReorderablePointListEditor editor(config);
    editor.setPoints3D({QVector3D(1.0f, 2.0f, 3.0f)});

    QSignalSpy spy(&editor, &ReorderablePointListEditor::pointEdited);
    QVERIFY(spy.isValid());

    const auto edits = editor.findChildren<QDoubleSpinBox *>();
    QCOMPARE(edits.size(), 3);

    edits[2]->setValue(9.0);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 0);
    QCOMPARE(spy.at(0).at(1).toInt(), 2);
    QCOMPARE(spy.at(0).at(2).toDouble(), 9.0);
}

void PropertyPanelEditorTest::pointListEditorEmitsAddRequested()
{
    PointListEditorConfig config;
    config.addButtonText = QStringLiteral("Add Vertex");
    config.dimension = 2;
    config.rowLabel = [](int index) { return QStringLiteral("V%1").arg(index + 1); };

    ReorderablePointListEditor editor(config);
    QSignalSpy spy(&editor, &ReorderablePointListEditor::addRequested);
    QVERIFY(spy.isValid());

    auto *button = editor.findChild<QPushButton *>();
    QVERIFY(button != nullptr);

    button->click();

    QCOMPARE(spy.count(), 1);
}

static QPushButton *findButtonByText(QWidget *root, const QString &text)
{
    for (auto *button : root->findChildren<QPushButton *>()) {
        if (button->text() == text) {
            return button;
        }
    }
    return nullptr;
}

void PropertyPanelEditorTest::objectSectionOnlyEmitsRemoveAfterConfirmation()
{
    ObjectSection section;
    TestSceneObject object;
    object.setName(QStringLiteral("Cube A"));
    section.setCurrentObject(&object);

    QSignalSpy spy(&section, &ObjectSection::removeRequested);
    QVERIFY(spy.isValid());

    auto *deleteButton = findButtonByText(&section, QStringLiteral("Delete Object"));
    QVERIFY(deleteButton != nullptr);

    QTimer::singleShot(0, [] {
        auto *dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        QVERIFY(dialog != nullptr);
        auto *button = dialog->button(QMessageBox::Yes);
        QVERIFY(button != nullptr);
        button->click();
    });

    deleteButton->click();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<SceneObject *>(), static_cast<SceneObject *>(&object));
}

void PropertyPanelEditorTest::objectSectionDoesNotEmitRemoveWhenDialogCancelled()
{
    ObjectSection section;
    TestSceneObject object;
    object.setName(QStringLiteral("Cube B"));
    section.setCurrentObject(&object);

    QSignalSpy spy(&section, &ObjectSection::removeRequested);
    QVERIFY(spy.isValid());

    auto *deleteButton = findButtonByText(&section, QStringLiteral("Delete Object"));
    QVERIFY(deleteButton != nullptr);

    QTimer::singleShot(0, [] {
        auto *dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        QVERIFY(dialog != nullptr);
        auto *button = dialog->button(QMessageBox::Cancel);
        QVERIFY(button != nullptr);
        button->click();
    });

    deleteButton->click();

    QCOMPARE(spy.count(), 0);
}

void PropertyPanelEditorTest::objectSectionNameEditCreatesUndoableCommand()
{
    SceneController controller;
    auto *object = controller.addPrimitive(PrimitiveObject::PrimitiveType::Box);
    QVERIFY(object != nullptr);
    const QString originalName = object->name();
    controller.undoStack()->clear();

    ObjectSection section(&controller);
    section.setObjects(controller.objects());
    section.setCurrentObject(object);

    QLineEdit *nameEdit = nullptr;
    for (auto *lineEdit : section.findChildren<QLineEdit *>()) {
        if (!lineEdit->isReadOnly()) {
            nameEdit = lineEdit;
            break;
        }
    }
    QVERIFY(nameEdit != nullptr);

    nameEdit->setText(QStringLiteral("Box Renamed"));
    const bool invoked = QMetaObject::invokeMethod(nameEdit, "editingFinished", Qt::DirectConnection);
    QVERIFY(invoked);

    QCOMPARE(object->name(), QStringLiteral("Box Renamed"));
    QVERIFY(controller.undoStack()->canUndo());

    controller.undoStack()->undo();
    QCOMPARE(object->name(), originalName);

    controller.undoStack()->redo();
    QCOMPARE(object->name(), QStringLiteral("Box Renamed"));
}

QTEST_MAIN(PropertyPanelEditorTest)

#include "property_panel_test.moc"
