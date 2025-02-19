#pragma once

#include "config.hpp"
#include "customtypes/editing.hpp"

namespace Qounters::Editor {
    void Initialize(Options::Preset const& preset);
    void LoadPreset(Options::Preset const& preset);

    void SetPreviewMode(bool preview);
    bool GetPreviewMode();

    void SetPresetForMigrating(Options::Preset preset);
    Options::Preset GetPreset();
    Options::Group& GetGroup(int idx);
    void AddGroup(Options::Group group);

    void Duplicate();
    void CopyComponent();
    bool CanPasteComponent();
    void PasteComponent();

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
    void NewComponent();
    void ToggleAttachment();
    void SwapGradientColors();
    void Remove();
    // use GetSelectedX, modify the reference, call these, then call FinalizeAction
    void UpdatePosition(bool neverSnap = false);
    void UpdateType();
    void UpdateColor();
    void UpdateEnable();
    void UpdateOptions();
    void UpdateSourceOptions();
    void UpdateAll();
    // GetSelectedX optional, but still needs FinalizeAction
    void SetType(int actionId, Options::Component::Types type);
    void SetColorSource(int actionId, std::string source);
    void SetEnableSource(int actionId, std::string source);
    void SetType(int actionId, Options::Component::Types type);
    void SetOptions(int actionId, Options::Component::OptionsTypes options);
    void SetSourceOptions(int actionId, UnparsedJSON options);
    void SetColorOptions(int actionId, UnparsedJSON options);
    void SetEnableOptions(int actionId, UnparsedJSON options);

    void Undo();
    bool HasUndo();
    void ClearUndos();
    int GetActionId();
    bool FinalizeAction();

    void DisableActions();
    void EnableActions();

    Options::Group& GetSelectedGroup(int actionId);
    Options::Component& GetSelectedComponent(int actionId);
    template <class T>
    T GetOptions(int actionId) {
        return GetSelectedComponent(actionId).Options.GetValue<T>().value_or(T());
    }
}
