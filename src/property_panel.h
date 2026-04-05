#pragma once

#include <array>

#include <QPointer>
#include <QWidget>

#include "demo/extrude_object.h"
#include "demo/polygon_object.h"
#include "demo/primitive_object.h"

class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QVBoxLayout;

class PropertyPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects);
    void setCurrentObject(SceneObject *object);
    void refreshObject(SceneObject *object);

signals:
    void parentChangeRequested(SceneObject *object, SceneObject *parentObject);
    void removeRequested(SceneObject *object);

private:
    void setPropertyEditorsEnabled(bool enabled) const;
    void chooseColor();
    void refreshColorEditor();
    void refreshParentOptions();
    void refreshLineTypeEditor();
    void refreshDimensionEditors();
    void refreshPolygonEditor();
    void refreshExtrudeEditor();
    void updatePositionFromEditors() const;
    void updateRotationFromEditors() const;
    void updateDimensionFromEditors(int index) const;
    void rebuildPolygonVertexRows(PolygonObject *polygonObject);
    void rebuildExtrudePathRows(ExtrudeObject *extrudeObject);
    void rebuildExtrudeProfileRows(ExtrudeObject *extrudeObject);

    static constexpr int MaxDimensionEditors = 6;

    QList<SceneObject *> m_objects;
    QPointer<SceneObject> m_currentObject;
    QWidget *m_propertyContentWidget{};
    QLabel *m_emptyStateLabel{};
    QLabel *m_dimensionsSectionLabel{};
    QLabel *m_polygonSectionLabel{};
    QLabel *m_extrudeSectionLabel{};
    QLabel *m_lineTypeLabel{};
    QLabel *m_polygonValidationLabel{};
    QLabel *m_extrudePathValidationLabel{};
    QLabel *m_extrudeProfileValidationLabel{};
    QLineEdit *m_nameEdit{};
    QLineEdit *m_typeEdit{};
    QComboBox *m_parentCombo{};
    QComboBox *m_lineTypeCombo{};
    QComboBox *m_extrudePathTypeCombo{};
    QDoubleSpinBox *m_polygonHeightEdit{};
    QSpinBox *m_polygonRingsEdit{};
    QSpinBox *m_extrudePathDivisionsEdit{};
    QPushButton *m_addPolygonVertexButton{};
    QPushButton *m_addExtrudeProfileVertexButton{};
    QPushButton *m_colorButton{};
    QPushButton *m_deleteButton{};
    QWidget *m_polygonSectionWidget{};
    QWidget *m_polygonVerticesWidget{};
    QWidget *m_extrudeSectionWidget{};
    QWidget *m_extrudePathPointsWidget{};
    QWidget *m_extrudeProfileVerticesWidget{};
    QVBoxLayout *m_polygonVerticesLayout{};
    QVBoxLayout *m_extrudePathPointsLayout{};
    QVBoxLayout *m_extrudeProfileVerticesLayout{};
    std::array<QDoubleSpinBox *, 3> m_positionEdits{};
    std::array<QDoubleSpinBox *, 3> m_rotationEdits{};
    std::array<QLabel *, MaxDimensionEditors> m_dimensionLabels{};
    std::array<QDoubleSpinBox *, MaxDimensionEditors> m_dimensionEdits{};
    bool m_updatingEditors = false;
};
