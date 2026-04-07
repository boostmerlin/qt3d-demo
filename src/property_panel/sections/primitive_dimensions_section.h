#pragma once

#include <array>

#include <QPointer>

#include "property_panel/property_section.h"

class DoubleEditorRow;
class PrimitiveObject;

class PrimitiveDimensionsSection final : public PropertySection
{
    Q_OBJECT

public:
    explicit PrimitiveDimensionsSection(SceneController *sceneController = nullptr, QWidget *parent = nullptr);

    void setObjects(const QList<SceneObject *> &objects) override;
    void setCurrentObject(SceneObject *object) override;
    void refreshObject(SceneObject *object) override;

private:
    void refreshEditors();

    static constexpr int MaxDimensionEditors = 6;

    QPointer<PrimitiveObject> m_currentObject;
    std::array<DoubleEditorRow *, MaxDimensionEditors> m_dimensionEditors{};
    bool m_updating = false;
};
