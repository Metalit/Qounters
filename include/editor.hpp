#pragma once

#include "config.hpp"
#include "customtypes/editing.hpp"

namespace Qounters::Editor {
    void Initialize(Preset const& preset);
    void LoadPreset(Preset const& preset);

    void SetPreviewMode(bool preview);
    bool GetPreviewMode();

    void SetPresetForMigrating(Preset preset);
    Preset GetPreset();
    Group& GetGroup(int idx);
    void AddGroup(Group group);

    void RegisterEditingGroup(EditingGroup* object, int groupIdx);
    void RegisterEditingComponent(EditingComponent* object, int groupIdx, int componentIdx);

    void SelectEditing(EditingBase* object);
    void Deselect();

    void BeginDrag(int anchor, bool group);
    bool UpdateDrag(EditingGroup* dragged);
    void EndDrag();
    void EnableDetachedCanvas(bool enabled);

    void AddComponent();
    void ToggleAttachment();
    void Remove();
    void UpdatePosition(bool neverSnap = false);
    void UpdateType();
    void UpdateColorSource();
    void UpdateEnableSource();
    void UpdateInvertEnabled();
    void SetOptions(int actionId, Component::OptionsTypes options);
    void SetSourceOptions(int actionId, UnparsedJSON options);
    void SetColorOptions(int actionId, UnparsedJSON options);
    void SetEnableOptions(int actionId, UnparsedJSON options);

    void Undo();
    bool HasUndo();
    void ClearUndos();
    int GetActionId();
    void FinalizeAction();

    void DisableActions();
    void EnableActions();

    Group& GetSelectedGroup(int actionId, bool addUndo = true);
    Component& GetSelectedComponent(int actionId, bool addUndo = true);
}
