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
    void UnregisterEditing(EditingBase* object);

    void SelectEditing(EditingBase* object);
    void Deselect();

    void BeginDrag(int anchor, bool group);
    bool UpdateDrag(EditingGroup* dragged);
    void EndDrag();
    void EnableDetachedCanvas(bool enabled);
    void UpdateDetachedDrag(UnityEngine::Vector3 const& pos, UnityEngine::Vector3 const& controller);

    // adds an undo every time, no action ID or FinalizeAction
    void AddComponent();
    void ToggleAttachment();
    void SwapGradientColors();
    void Remove();
    // use GetSelectedX, modify the reference, call these, then call FinalizeAction
    void UpdatePosition(bool neverSnap = false);
    void UpdateType();
    void UpdateColorSource();
    void UpdateEnableSource();
    void UpdateOptions();
    void UpdateSourceOptions();
    void UpdateColorOptions();
    void UpdateGradientOptions();
    void UpdateEnableOptions();
    // GetSelectedX optional, but still needs FinalizeAction
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

    Group& GetSelectedGroup(int actionId);
    Component& GetSelectedComponent(int actionId);
    template <class T>
    T GetOptions(int actionId) {
        return GetSelectedComponent(actionId).Options.GetValue<T>().value_or(T());
    }
}
