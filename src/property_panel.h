#pragma once

#include <array>

#include <QPointer>
#include <QWidget>

#include "demo/primitive_object.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;

class PropertyPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget *parent = nullptr);

    void setObjects(const QList<PrimitiveObject *> &objects);
    void setCurrentObject(PrimitiveObject *object);
    void refreshObject(PrimitiveObject *object);

signals:
    void parentChangeRequested(PrimitiveObject *object, PrimitiveObject *parentObject);
    void removeRequested(PrimitiveObject *object);

private:
    void setPropertyEditorsEnabled(bool enabled) const;
    void chooseColor();
    void refreshColorEditor();
    void refreshParentOptions();
    void refreshDimensionEditors();
    void updatePositionFromEditors() const;
    void updateRotationFromEditors() const;
    void updateDimensionFromEditors(int index) const;

    QList<PrimitiveObject *> m_objects;
    QPointer<PrimitiveObject> m_currentObject;
    QWidget *m_propertyContentWidget{};
    QLabel *m_emptyStateLabel{};
    QLabel *m_dimensionsSectionLabel{};
    QLineEdit *m_nameEdit{};
    QLineEdit *m_typeEdit{};
    QComboBox *m_parentCombo{};
    QPushButton *m_colorButton{};
    QPushButton *m_deleteButton{};
    std::array<QDoubleSpinBox *, 3> m_positionEdits{};
    std::array<QDoubleSpinBox *, 3> m_rotationEdits{};
    std::array<QLabel *, 3> m_dimensionLabels{};
    std::array<QDoubleSpinBox *, 3> m_dimensionEdits{};
    bool m_updatingEditors = false;
};
