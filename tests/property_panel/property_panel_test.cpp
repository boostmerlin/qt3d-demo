#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "property_panel/editors/enum_editor.h"
#include "property_panel/editors/reorderable_point_list_editor.h"
#include "property_panel/editors/vector3_editor.h"

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

QTEST_MAIN(PropertyPanelEditorTest)

#include "property_panel_test.moc"
