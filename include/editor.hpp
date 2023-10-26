#pragma once

#include "config.hpp"
#include "customtypes/editing.hpp"

namespace Qounters::Editor {
    void Initialize(Preset const& preset);

    void SetPreviewMode(bool preview);

    void SetPresetForMigrating(Preset preset);
    Preset GetPreset();
    Group& GetGroup(int idx);
    void AddGroup(Group group);

    void RegisterEditingGroup(EditingGroup* object, int groupIdx);
    void RegisterEditingComponent(EditingComponent* object, int groupIdx, int componentIdx);
    void UnregisterEditing(EditingBase* object);

    void SelectEditing(EditingBase* object);
    void Deselect();

    void BeginDrag(int anchor, bool group);
    bool UpdateDrag(EditingGroup* dragged);
    void EndDrag();

    void AddComponent();
    void Remove();
    void UpdatePosition();
    void UpdateType();
    void UpdateColorSource();
    void SetOptions(int actionId, Component::OptionsTypes options);
    void SetSourceOptions(int actionId, UnparsedJSON options);
    void SetColorOptions(int actionId, UnparsedJSON options);

    void Undo();
    int GetActionId();
    void FinalizeAction();

    void DisableActions();
    void EnableActions();

    Group& GetSelectedGroup(int actionId, bool addUndo = true);
    Component& GetSelectedComponent(int actionId, bool addUndo = true);
}
