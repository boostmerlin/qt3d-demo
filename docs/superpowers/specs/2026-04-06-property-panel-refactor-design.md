# PropertyPanel Refactor Design

## Background

Current `PropertyPanel` has grown into a single widget that handles:

- overall panel layout and empty state
- object selection lifecycle
- generic property editing
- object-type-specific UI branching
- repeated low-level editor construction
- direct write-back into scene objects

This works for the current prototype, but the change surface is too large when adding a new object type or adding new editable properties. The panel currently centralizes too much knowledge about `PrimitiveObject`, `PolygonObject`, and `ExtrudeObject`.

## Goals

- Reduce the amount of code that must be modified when adding a new object type or property.
- Split the current panel into smaller units with clear responsibilities.
- Reuse editor widgets instead of rebuilding similar `QDoubleSpinBox`, `QComboBox`, and list-row UI by hand.
- Keep the existing external API of `PropertyPanel` stable so `MainWindow` integration can remain mostly unchanged.
- Support incremental migration from the current implementation without a full rewrite.

## Non-Goals

- Do not introduce a fully declarative Property Schema system in this refactor.
- Do not redesign the scene object model or replace existing `SceneObject` and `PrimitiveObject` APIs.
- Do not change `SceneController` ownership or move controller logic into property editors.
- Do not attempt to generalize every editor interaction into a framework.

## Recommended Approach

Use `Section Registry + Editor Factory`.

The refactor will split the existing `PropertyPanel` into three layers:

1. `PropertyPanel` as a thin container and coordinator.
2. A set of independent property sections, each responsible for one area of the panel.
3. A small editor factory plus reusable editor widgets for common editing patterns.

This balances extensibility and implementation cost:

- lower risk than a full Property Schema architecture
- much smaller main-panel change surface than the current monolith
- strong reuse for repeated UI patterns
- compatible with incremental migration

## Architecture

### 1. PropertyPanel

`PropertyPanel` remains the public entry point used by `MainWindow`.

Responsibilities:

- create the scroll area and empty state container
- register and own all sections
- store `m_objects` and `m_currentObject`
- dispatch `setObjects`, `setCurrentObject`, and `refreshObject` to sections
- forward section-level signals such as `parentChangeRequested` and `removeRequested`

Responsibilities that move out of `PropertyPanel`:

- object-specific UI branching
- polygon and extrude row rebuilding
- low-level editor construction
- individual property refresh logic

### 2. Property Sections

Each section owns one bounded area of behavior and UI. Sections receive object lifecycle events from `PropertyPanel`, decide whether they apply to the current object, and refresh their own editors.

Initial section set:

- `ObjectSection`
- `TransformSection`
- `PrimitiveDimensionsSection`
- `PolygonSection`
- `ExtrudeSection`

Each section should expose a small interface similar to:

```cpp
class IPropertySection
{
public:
    virtual ~IPropertySection() = default;

    virtual QWidget *widget() const = 0;
    virtual void setObjects(const QList<SceneObject *> &objects) = 0;
    virtual void setCurrentObject(SceneObject *object) = 0;
    virtual void refreshObject(SceneObject *object) = 0;
};
```

Expected section behavior:

- `setObjects(...)`: update any data sourced from the scene object list, such as parent candidates
- `setCurrentObject(...)`: store the active object, decide visibility, and perform a full refresh
- `refreshObject(...)`: refresh only when the provided object matches the section's current object

Sections should not call each other directly.

### 3. Editor Factory and Reusable Editors

Reusable editors will absorb repeated layout and signal boilerplate. The factory will create editors and apply shared styling conventions, but will not know about object types or business rules.

Initial reusable editors:

- `TextEditorRow`
- `DoubleEditorRow`
- `IntEditorRow`
- `EnumEditor`
- `Vector3Editor`
- `ColorEditor`
- `ReorderablePointListEditor`

The factory will provide narrow helpers such as:

```cpp
class PropertyEditorFactory
{
public:
    static TextEditorRow *createTextEditor(const QString &label, QWidget *parent = nullptr);
    static DoubleEditorRow *createDoubleEditor(const QString &label, QWidget *parent = nullptr);
    static IntEditorRow *createIntEditor(const QString &label, QWidget *parent = nullptr);
    static EnumEditor *createEnumEditor(const QString &label, QWidget *parent = nullptr);
    static Vector3Editor *createVector3Editor(const QString &label, QWidget *parent = nullptr);
    static ColorEditor *createColorEditor(const QString &label, QWidget *parent = nullptr);
    static ReorderablePointListEditor *createPointListEditor(
        const PointListEditorConfig &config, QWidget *parent = nullptr);
};
```

Factory constraints:

- no `SceneObject *` parameters
- no object-specific branching such as `qobject_cast<PolygonObject *>`
- no section orchestration
- no attempt to generate entire sections

## Section Responsibilities

### ObjectSection

Owns:

- `name`
- `type` read-only display
- `parent` selection
- `color`
- `delete`

Signals emitted upward:

- `parentChangeRequested(SceneObject *object, SceneObject *parentObject)`
- `removeRequested(SceneObject *object)`

This keeps controller-facing actions outside the section and preserves the current `MainWindow` integration model.

### TransformSection

Owns:

- `position`
- `rotation`

Uses:

- two `Vector3Editor` instances

Writes back directly to `SceneObject`.

### PrimitiveDimensionsSection

Owns:

- all generic primitive dimension editors

Uses existing `PrimitiveObject` API:

- `editableDimensionCount()`
- `dimensionLabel(int index)`
- `dimensionValue(int index)`
- `dimensionMinimum(int index)`
- `dimensionMaximum(int index)`
- `setDimensionValue(int index, float value)`

This section is the key extensibility point for normal primitive types. A new primitive that fits the generic dimension model and implements the existing dimension interface requires no `PropertyPanel` changes.

### PolygonSection

Owns:

- `height`
- `rings`
- polygon validation state
- polygon vertices editing

Uses:

- `DoubleEditorRow`
- `IntEditorRow`
- `ReorderablePointListEditor`

Business logic remains in the section:

- add regular vertex
- move vertex
- remove vertex
- validation label visibility

### ExtrudeSection

Owns:

- `pathType`
- `pathDivisions`
- `ellipseRotationDegrees`
- `pathStartAngleDegrees`
- `pathEndAngleDegrees`
- path validation state
- profile validation state
- path control point editing
- profile vertex editing

Uses:

- `EnumEditor`
- scalar row editors
- `ReorderablePointListEditor`

Business logic remains in the section:

- path-type-specific label rules
- path-type-specific field visibility
- path control point semantics for line, bezier, ellipse, and arc

## ReorderablePointListEditor Scope

`ReorderablePointListEditor` is the most valuable shared component in this refactor, but its scope must remain narrow.

It is responsible for:

- rendering a list of point rows
- editing coordinates
- add, move, and remove controls
- row title customization
- support for 2D and 3D point lists
- basic per-axis visibility and labeling

It is not responsible for:

- polygon validation rules
- extrude path semantics
- deciding what a point means for ellipse or arc editing
- writing values back into domain objects

That logic stays in `PolygonSection` and `ExtrudeSection`.

## Data Flow

### Current Object Selection

1. `MainWindow` calls `PropertyPanel::setCurrentObject(object)`.
2. `PropertyPanel` stores the current object and updates empty-state visibility.
3. `PropertyPanel` forwards the new object to all sections.
4. Each section stores its typed current object, updates its own visibility, and refreshes its editors.

### External Object Refresh

1. `SceneController` emits an object-changed signal.
2. `MainWindow` forwards it to `PropertyPanel::refreshObject(object)`.
3. `PropertyPanel` forwards the refresh to all sections.
4. Each section refreshes only if the changed object matches its active object.

### User Editing

1. An editor emits a typed `valueEdited(...)` signal.
2. The owning section handles the signal.
3. The section checks its local `m_updating` guard.
4. The section writes back into the current object or emits an upward request signal.
5. Existing scene refresh and hierarchy refresh paths continue to work.

## Error Handling and State Safety

### Section-local update guard

Each section will own its own `m_updating` flag. This replaces the current single `PropertyPanel::m_updatingEditors` guard.

Rationale:

- avoids cross-section interference
- keeps update loops local
- makes section behavior easier to reason about

### Visibility ownership

Each section is responsible for showing or hiding its own root widget based on the current object type.

Rationale:

- keeps object-type branching local
- avoids a new central visibility switch in `PropertyPanel`

### Controller boundary

Sections do not call `SceneController` directly.

Rationale:

- preserves current architecture boundaries
- avoids UI widgets acquiring controller ownership
- keeps `MainWindow` integration stable

## File and Directory Layout

Recommended structure:

```text
src/
  property_panel.h
  property_panel.cpp
  property_panel/
    property_section.h
    property_editor_factory.h
    property_editor_factory.cpp
    sections/
      object_section.h
      object_section.cpp
      transform_section.h
      transform_section.cpp
      primitive_dimensions_section.h
      primitive_dimensions_section.cpp
      polygon_section.h
      polygon_section.cpp
      extrude_section.h
      extrude_section.cpp
    editors/
      text_editor_row.h
      text_editor_row.cpp
      double_editor_row.h
      double_editor_row.cpp
      int_editor_row.h
      int_editor_row.cpp
      enum_editor.h
      enum_editor.cpp
      vector3_editor.h
      vector3_editor.cpp
      color_editor.h
      color_editor.cpp
      reorderable_point_list_editor.h
      reorderable_point_list_editor.cpp
```

## Incremental Migration Plan

### Phase 1: Introduce section infrastructure

- add `IPropertySection`
- add section container support inside `PropertyPanel`
- keep the public `PropertyPanel` interface unchanged

Exit criteria:

- `PropertyPanel` can own and dispatch to sections
- no external integration changes required

### Phase 2: Migrate TransformSection

- create `Vector3Editor`
- migrate `position` and `rotation`
- verify direct write-back to `SceneObject`

Exit criteria:

- position and rotation editing work through the new section
- no duplicate refresh or write-back logic remains for those fields

### Phase 3: Migrate ObjectSection

- create text, color, and enum-style editors as needed
- migrate `name`, `type`, `parent`, `color`, and `delete`
- forward upward signals through `PropertyPanel`

Exit criteria:

- parent changes and delete actions still flow through `MainWindow`
- no direct controller calls from sections

### Phase 4: Migrate PrimitiveDimensionsSection

- create reusable scalar rows
- migrate dynamic primitive dimensions

Exit criteria:

- box, sphere, cylinder, cone, line, and ring dimensions all work
- adding a new primitive only requires implementing the `PrimitiveObject` dimension API

### Phase 5: Implement ReorderablePointListEditor

- support add, move, remove, and coordinate editing
- support both 2D and 3D lists

Exit criteria:

- editor is reusable by both polygon and extrude sections
- editor remains domain-agnostic

### Phase 6: Migrate PolygonSection

- migrate polygon-specific editors and validation
- replace manual vertex row rebuilding with `ReorderablePointListEditor`

Exit criteria:

- polygon editing behavior matches current functionality
- repeated vertex-row UI code is removed from `PropertyPanel`

### Phase 7: Migrate ExtrudeSection

- migrate extrude-specific editors and validation
- replace manual path/profile row rebuilding with `ReorderablePointListEditor`

Exit criteria:

- extrude editing behavior matches current functionality
- path-type-specific visibility still works correctly

### Phase 8: Remove legacy PropertyPanel logic

- delete old refresh helpers and row-rebuild functions
- keep `PropertyPanel` as a thin coordinator

Exit criteria:

- `PropertyPanel` no longer contains object-type-specific editing logic

## Testing Strategy

### Manual verification

After each migration phase, verify:

- switching selection refreshes the correct editors
- editing a value updates the scene object and render output
- external object changes refresh the currently visible panel state
- hierarchy, scene selection, and property panel remain in sync

Object-specific manual checks:

- primitive dimensions: box, sphere, cylinder, cone, line, ring
- polygon: vertex add, move, remove, validation visibility
- extrude: path type switching, path/profile editing, angle/rotation field visibility

### Automated tests

Add focused widget tests for reusable editors where practical:

- `Vector3Editor` emits the correct combined value
- `EnumEditor` preserves item value mapping
- `ColorEditor` updates display state correctly
- `ReorderablePointListEditor` emits add, move, remove, and point-edit signals correctly

When the current Qt test setup can host section widgets cleanly, add section tests for:

- visibility rules per object type
- write-back guards preventing refresh loops

## Key Risks and Mitigations

### Risk: editor factory becomes a new monolith

Mitigation:

- keep factory limited to widget construction and shared defaults
- keep object-specific logic in sections

### Risk: section refresh loops become hard to reason about

Mitigation:

- use one `m_updating` flag per section
- avoid a new global updating guard

### Risk: `ReorderablePointListEditor` becomes over-generalized

Mitigation:

- keep it focused on point-list UI only
- leave path semantics and validation in sections

### Risk: migration leaves duplicate logic in old and new code

Mitigation:

- remove or disable legacy logic as soon as a section is migrated
- do not keep long-term dual ownership of the same fields

## Expected Outcome

After this refactor:

- `PropertyPanel` is small and orchestration-focused
- object-specific complexity is isolated in sections
- repeated low-level editor code is reused instead of rewritten
- adding a new normal primitive becomes primarily a model-side change
- adding a new complex object type becomes primarily a new section plus editor reuse, not a rewrite of the main panel
