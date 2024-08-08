#include "editor.hpp"

#include "GlobalNamespace/VRController.hpp"
#include "System/Single.hpp"
#include "UnityEngine/Ray.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Time.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "config.hpp"
#include "customtypes/editing.hpp"
#include "customtypes/settings.hpp"
#include "main.hpp"
#include "playtest.hpp"
#include "qounters.hpp"
#include "sources.hpp"
#include "utils.hpp"

namespace Qounters::Editor {
    constexpr int anchorCount = (int) Group::Anchors::AnchorMax + 1;

    std::array<CanvasHighlight*, anchorCount> anchors;
    std::array<UnityEngine::GameObject*, anchorCount> dragCanvases;
    UnityEngine::GameObject* detachedDragCanvas;

    int currentAnchor;

    std::map<std::pair<int, int>, EditingBase*> editing;
    EditingBase* selected;
    int selectedGroupIdx, selectedComponentIdx;

    std::vector<std::tuple<int, int, std::function<void()>>> undos;

    bool addDirectToPreset = false;

    Preset preset;

    std::set<int> removedGroupIdxs;
    std::map<int, std::set<int>> removedComponentIdxs;

    int highestActionId = 0;

    VRUIControls::VRInputModule* vrInput;

    UnityEngine::Color const normalColor = {1, 1, 1, 0.2};
    UnityEngine::Color const highlightColor = {0, 0.7, 0.8, 0.2};

    Group nextUndoGroup;
    Component nextUndoComponent;
    int currentActionId;
    std::set<void (*)()> nextUndoUpdates;

    int lastActionId;
    float lastActionTime;
    std::set<void (*)()> lastUndoUpdates;

    bool runningUndo;

    bool disableActions;

    bool previewMode;

    int prevSelectedGroupIdx, prevSelectedComponentIdx;
    EditingBase* prevSelected;
    bool tempSelectIsReal;

    void TempSelect(int groupIdx, int componentIdx) {
        prevSelectedGroupIdx = selectedGroupIdx;
        prevSelectedComponentIdx = selectedComponentIdx;
        prevSelected = selected;
        selectedGroupIdx = groupIdx;
        selectedComponentIdx = componentIdx;
        selected = editing[{groupIdx, componentIdx}];
        tempSelectIsReal = prevSelected == selected;
    }
    void EndTempSelect() {
        selectedGroupIdx = prevSelectedGroupIdx;
        selectedComponentIdx = prevSelectedComponentIdx;
        if (!tempSelectIsReal)
            selected = prevSelected;
        prevSelected = nullptr;
    }

    inline void AddUpdate(void (*update)()) {
        if (runningUndo || disableActions)
            return;
        nextUndoUpdates.emplace(update);
    }
    inline void AddUndo(auto&& fn) {
        if (runningUndo || disableActions)
            return;
        bool wasEmpty = undos.empty();
        undos.emplace_back(selectedGroupIdx, selectedComponentIdx, fn);
        if (wasEmpty)
            SettingsViewController::GetInstance()->UpdateUI();
    }

    void SetupAnchors() {
        for (int i = 0; i < anchorCount; i++) {
            auto anchor = GetAnchor(i);
            if (!anchor) {
                anchors[i] = nullptr;
                continue;
            }
            anchors[i] = Utils::GetOrAddComponent<CanvasHighlight*>(anchor);
            anchors[i]->color = {1, 1, 1, 0.2};
            anchors[i]->raycastTarget = false;
        }
    }

    UnityEngine::GameObject* CreateDragCanvas(std::string name, UnityEngine::Transform* parent) {
        auto canvas = BSML::Lite::CreateCanvas();
        canvas->name = name;

        if (parent)
            canvas->transform->SetParent(parent, false);
        canvas->transform->localScale = {1, 1, 1};
        // might be nice to have a constrained area later
        canvas->GetComponent<UnityEngine::RectTransform*>()->sizeDelta = {1000, 1000};
        canvas->AddComponent<CanvasHighlight*>()->raycastTarget = true;
        canvas->active = false;
        return canvas;
    }

    void CreateDragCanvases() {
        for (int i = 0; i < anchorCount; i++) {
            if (!anchors[i]) {
                dragCanvases[i] = nullptr;
                continue;
            }
            auto parent = anchors[i]->transform;
            dragCanvases[i] = CreateDragCanvas("QountersDragCanvas" + std::to_string(i), parent);
        }
        detachedDragCanvas = CreateDragCanvas("QountersDetachedDragCanvas", nullptr);
    }

    void InitializeInternal(Preset const& inPreset, bool newEnvironment) {
        preset = inPreset;
        editing.clear();
        undos.clear();
        removedGroupIdxs.clear();
        removedComponentIdxs.clear();
        selected = nullptr;
        selectedGroupIdx = -1;
        selectedComponentIdx = -1;
        currentActionId = -1;
        nextUndoUpdates.clear();
        lastActionId = -1;
        lastActionTime = 0;
        lastUndoUpdates.clear();
        runningUndo = false;
        disableActions = false;
        previewMode = false;
        vrInput = Utils::GetCurrentInputModule();
        if (newEnvironment) {
            SetupAnchors();
            CreateDragCanvases();
        }

        for (int i = 0; i < preset.Qounters.size(); i++)
            CreateQounterGroup(preset.Qounters[i], i, true);
    }

    void Initialize(Preset const& inPreset) {
        InitializeInternal(inPreset, true);
    }

    void LoadPreset(Preset const& preset) {
        for (auto& [idxs, obj] : editing) {
            auto& [groupIdx, componentIdx] = idxs;
            if (componentIdx == -1)
                UnityEngine::Object::Destroy(obj->gameObject);
        }
        Reset(false);
        InitializeInternal(preset, false);
        SettingsViewController::GetInstance()->UpdateUI();
    }

    void SetPreviewMode(bool preview) {
        previewMode = preview;
        for (auto& [_, obj] : editing)
            obj->outline->enabled = !preview;
        Deselect();
        UpdateAllEnables();
        if (preview)
            SettingsFlowCoordinator::PresentPlaytest();
        else
            SettingsFlowCoordinator::PresentTemplates();
        Playtest::SetEnabled(preview);
    }

    bool GetPreviewMode() {
        return previewMode;
    }

    void SetPresetForMigrating(Preset migratingPreset) {
        addDirectToPreset = true;
        preset = migratingPreset;
    }

    Preset GetPreset() {
        if (addDirectToPreset) {
            addDirectToPreset = false;
            return preset;
        }
        Preset ret = preset;
        for (auto& [idx, set] : removedComponentIdxs) {
            if (removedGroupIdxs.contains(idx))
                continue;
            for (auto it = set.rbegin(); it != set.rend(); it++) {
                auto& components = ret.Qounters[idx].Components;
                components.erase(components.begin() + *it);
            }
        }
        for (auto it = removedGroupIdxs.rbegin(); it != removedGroupIdxs.rend(); it++)
            ret.Qounters.erase(ret.Qounters.begin() + *it);
        return ret;
    }

    Group& GetGroup(int idx) {
        return preset.Qounters[idx];
    }

    void AddGroup(Group group) {
        if (addDirectToPreset) {
            preset.Qounters.emplace_back(group);
            return;
        }

        int newIdx = preset.Qounters.size();
        preset.Qounters.emplace_back(group);
        CreateQounterGroup(group, newIdx, true);

        auto created = editing[{newIdx, -1}];
        if (!runningUndo)
            created->Select();
        SelectEditing(created);

        AddUndo(Remove);
    }

    void RegisterEditingGroup(EditingGroup* object, int groupIdx) {
        editing[{groupIdx, -1}] = object;
        removedGroupIdxs.erase(groupIdx);
        if (selectedGroupIdx == groupIdx && selectedComponentIdx == -1)
            SelectEditing(object);
    }
    void RegisterEditingComponent(EditingComponent* object, int groupIdx, int componentIdx) {
        editing[{groupIdx, componentIdx}] = object;
        if (removedComponentIdxs.contains(groupIdx))
            removedComponentIdxs[groupIdx].erase(componentIdx);
        if (selectedGroupIdx == groupIdx && selectedComponentIdx == componentIdx)
            SelectEditing(object);
    }

    void UnregisterEditing(EditingBase* object) {
        if (object == selected)
            selected = nullptr;
        for (auto it = editing.begin(); it != editing.end(); it++) {
            if (it->second == object) {
                editing.erase(it);
                return;
            }
        }
    }
    void MarkAsRemoved(int groupIdx, int componentIdx) {
        if (componentIdx != -1) {
            if (!removedComponentIdxs.contains(groupIdx))
                removedComponentIdxs[groupIdx] = {};
            removedComponentIdxs[groupIdx].emplace(componentIdx);
        } else
            removedGroupIdxs.emplace(groupIdx);
    }

    void SelectEditing(EditingBase* object) {
        logger.debug("selected {} -> {}", fmt::ptr(selected), fmt::ptr(object));
        if (runningUndo || object == selected)
            return;
        if (selected)
            selected->Deselect();
        selected = object;

        auto options = OptionsViewController::GetInstance();

        if (auto group = il2cpp_utils::try_cast<EditingGroup>(object).value_or(nullptr)) {
            selectedGroupIdx = group->GetGroupIdx();
            selectedComponentIdx = -1;
            options->GroupSelected();
        } else if (auto component = il2cpp_utils::try_cast<EditingComponent>(object).value_or(nullptr)) {
            selectedGroupIdx = component->GetEditingGroup()->GetGroupIdx();
            selectedComponentIdx = component->GetComponentIdx();
            options->ComponentSelected();
        }

        SettingsFlowCoordinator::PresentOptions();
    }

    void Deselect() {
        if (runningUndo && !tempSelectIsReal)
            return;
        if (selected)
            selected->Deselect();
        selected = nullptr;

        SettingsFlowCoordinator::PresentTemplates();
    }

    void BeginDrag(int anchor, bool group) {
        raycastCanvases.clear();
        blockOtherRaycasts = true;
        for (int i = 0; i < anchorCount; i++) {
            anchors[i]->raycastTarget = group;
            anchors[i]->SetHighlighted(i != anchor && group);
            anchors[i]->color = normalColor;
            dragCanvases[i]->active = i == anchor;
            raycastCanvases.emplace(anchors[i]->GetComponent<UnityEngine::Canvas*>());
            raycastCanvases.emplace(dragCanvases[i]->GetComponent<UnityEngine::Canvas*>());
        }
        currentAnchor = anchor;
    }
    bool UpdateDrag(EditingGroup* dragged) {
        using RaycastResult = VRUIControls::VRGraphicRaycaster::VRGraphicRaycastResult;

        auto controller = vrInput->_vrPointer->_lastSelectedVrController;
        auto ray = UnityEngine::Ray(controller->position, controller->forward);

        auto results = ListW<RaycastResult>::New();
        for (int i = 0; i < anchorCount; i++) {
            if (i == currentAnchor)
                continue;

            auto raycaster = anchors[i]->GetComponent<VRUIControls::VRGraphicRaycaster*>();
            raycaster->RaycastCanvas(raycaster->_canvas, ray, System::Single::MaxValue, 0, results);

            RaycastResult* hit = nullptr;
            for (int j = 0; !hit && j < results.size(); j++) {
                if (results[j].graphic == anchors[i])
                    hit = &results[j];
            }
            if (hit) {
                BeginDrag(i, true);
                anchors[i]->SetHighlighted(true);
                anchors[i]->color = highlightColor;
                auto hitPoint = anchors[i]->rectTransform->InverseTransformPoint(hit->position);
                GetSelectedGroup(-1).Position = UnityEngine::Vector2(hitPoint.x, hitPoint.y);
                dragged->UpdateDragAnchor(i);
                return true;
            }
        }
        return false;
    }
    void EndDrag() {
        raycastCanvases.clear();
        blockOtherRaycasts = false;
        for (int i = 0; i < anchorCount; i++) {
            anchors[i]->raycastTarget = false;
            anchors[i]->SetHighlighted(false);
            dragCanvases[i]->active = false;
        }
    }
    void EnableDetachedCanvas(bool enabled) {
        detachedDragCanvas->active = enabled;
        raycastCanvases.clear();
        blockOtherRaycasts = enabled;
        if (enabled) {
            auto group = editing[{selectedGroupIdx, -1}];
            detachedDragCanvas->transform->SetParent(group->rectTransform, false);
            raycastCanvases.emplace(detachedDragCanvas->GetComponent<UnityEngine::Canvas*>());
        }
    }

    void AddComponent() {
        auto& vec = GetSelectedGroup(-1).Components;
        auto newIdx = vec.size();
        auto& newComponent = vec.emplace_back();
        newComponent.Type = (int) Component::Types::Text;
        TextOptions opts;
        opts.TextSource = TextSource::StaticName;
        TextSource::Static text;
        text.Input = "New Component";
        opts.SourceOptions = text;
        newComponent.Options = opts;

        CreateQounterComponent(newComponent, newIdx, selected->transform, true);
        auto created = editing[{selectedGroupIdx, newIdx}];
        if (!runningUndo) {
            created->Select();
            SelectEditing(created);
        }

        AddUndo(Remove);
    }

    void ToggleAttachment() {
        auto& group = GetSelectedGroup(-1);

        AddUndo([state = group]() {
            GetSelectedGroup(-1) = state;
            UpdateGroupPosition(selected->rectTransform, state);
        });

        group.Detached = !group.Detached;
        group.DetachedPosition = selected->rectTransform->position;
        group.DetachedRotation = selected->rectTransform->rotation.eulerAngles;
        UpdateGroupPosition(selected->rectTransform, group);

        if (!runningUndo)
            OptionsViewController::GetInstance()->UpdateSimpleUI();
    }

    void RemoveWithoutDeselect(int groupIdx, int componentIdx) {
        auto remove = editing[{groupIdx, componentIdx}];
        if (componentIdx != -1) {
            auto comp = (EditingComponent*) remove;
            RemoveComponent(GetGroup(groupIdx).Components[componentIdx].Type, comp->typeComponent);
        } else {
            // remove references to destroyed children
            std::vector<std::pair<int, EditingComponent*>> toRemove = {};
            for (auto& [idxs, obj] : editing) {
                auto& [otherGroupIdx, otherComponentIdx] = idxs;
                if (otherGroupIdx != groupIdx || otherComponentIdx == -1)
                    continue;
                toRemove.emplace_back(otherComponentIdx, (EditingComponent*) obj);
            }
            for (auto& [otherComponentIdx, component] : toRemove) {
                UnregisterEditing(component);
                RemoveComponent(GetGroup(groupIdx).Components[otherComponentIdx].Type, component->typeComponent);
            }
        }
        UnregisterEditing(remove);
        UnityEngine::Object::Destroy(remove->gameObject);
    }

    void Remove() {
        if (selectedComponentIdx != -1) {
            AddUndo([state = GetSelectedComponent(-1)]() {
                CreateQounterComponent(state, selectedComponentIdx, editing[{selectedGroupIdx, -1}]->transform, true);
            });
        } else {
            AddUndo([state = GetSelectedGroup(-1)]() {
                std::set<int> toRemove;
                if (removedComponentIdxs.contains(selectedGroupIdx))
                    toRemove = removedComponentIdxs[selectedGroupIdx];
                CreateQounterGroup(state, selectedGroupIdx, true);
                // removed components aren't actually erased, so we have to remove them again when recreating a group
                for (auto componentIdx : toRemove)
                    RemoveWithoutDeselect(selectedGroupIdx, componentIdx);
                removedComponentIdxs[selectedGroupIdx] = toRemove;
            });
        }

        RemoveWithoutDeselect(selectedGroupIdx, selectedComponentIdx);
        MarkAsRemoved(selectedGroupIdx, selectedComponentIdx);
        Deselect();
    }

    void SnapPosition(ConfigUtils::Vector2& position) {
        float step = getConfig().SnapStep.GetValue();
        position.x = floor((position.x / step) + 0.5) * step;
        position.y = floor((position.y / step) + 0.5) * step;
    }

    void UpdatePositionUndo() {
        UpdatePosition(true);
    }
    void UpdatePosition(bool neverSnap) {
        if (!selected)
            return;
        AddUpdate(UpdatePositionUndo);
        bool snap = !neverSnap && getConfig().Snap.GetValue();
        auto rect = selected->rectTransform;
        if (selectedComponentIdx != -1) {
            if (snap)
                SnapPosition(GetSelectedComponent(-1).Position);
            UpdateComponentPosition(rect, GetSelectedComponent(-1));
        } else {
            if (snap)
                SnapPosition(GetSelectedGroup(-1).Position);
            UpdateGroupPosition(rect, GetSelectedGroup(-1));
        }
    }

    void UpdateType() {
        AddUpdate(UpdateType);

        auto& component = GetSelectedComponent(-1);
        RemoveWithoutDeselect(selectedGroupIdx, selectedComponentIdx);
        if (!runningUndo)
            SetDefaultOptions(component);
        CreateQounterComponent(component, selectedComponentIdx, editing[{selectedGroupIdx, -1}]->transform, true);

        if (!runningUndo || tempSelectIsReal) {
            selected = editing[{selectedGroupIdx, selectedComponentIdx}];
            selected->Select();
            OptionsViewController::GetInstance()->UpdateUI();
        }
    }

    void UpdateColorSource() {
        AddUpdate(UpdateColorSource);

        if (!runningUndo) {
            SetColorOptions(-1, ColorSource::Static());
            OptionsViewController::GetInstance()->UpdateUI();
        } else {
            auto& component = GetSelectedComponent(-1);
            auto editingComponent = (EditingComponent*) selected;
            UpdateComponentColor(editingComponent->typeComponent, component.ColorSource, component.ColorOptions);
        }
    }

    void UpdateEnableSource() {
        AddUpdate(UpdateEnableSource);

        if (!runningUndo) {
            SetEnableOptions(-1, EnableSource::Static());
            OptionsViewController::GetInstance()->UpdateUI();
        } else {
            auto& component = GetSelectedComponent(-1);
            auto editingComponent = (EditingComponent*) selected;
            UpdateComponentEnabled(
                editingComponent->typeComponent->gameObject, component.EnableSource, component.EnableOptions, component.InvertEnable
            );
        }
    }

    void UpdateOptions() {
        auto& component = GetSelectedComponent(-1);
        auto editingComponent = (EditingComponent*) selected;
        UpdateComponentOptions(component.Type, editingComponent->typeComponent, component.Options);
        AddUpdate(UpdateOptions);
    }
    void SetOptions(int actionId, Component::OptionsTypes options) {
        auto& component = GetSelectedComponent(actionId);
        component.Options = options;
        UpdateOptions();
    }

    void UpdateSourceOptions() {
        // updates a few unnecessary things, but it's ok
        UpdateOptions();
    }
    void SetSourceOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);
        SetSourceOptions(component, options);
        UpdateSourceOptions();
    }

    void UpdateColorOptions() {
        auto& component = GetSelectedComponent(-1);
        auto editingComponent = (EditingComponent*) selected;
        UpdateComponentColor(editingComponent->typeComponent, component.ColorSource, component.ColorOptions);
        AddUpdate(UpdateColorOptions);
    }
    void SetColorOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);
        component.ColorOptions = options;
        UpdateColorOptions();
    }

    void UpdateEnableOptions() {
        auto& component = GetSelectedComponent(-1);
        auto editingComponent = (EditingComponent*) selected;
        UpdateComponentEnabled(editingComponent->typeComponent->gameObject, component.EnableSource, component.EnableOptions, component.InvertEnable);
        AddUpdate(UpdateEnableOptions);
    }
    void SetEnableOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);
        component.EnableOptions = options;
        UpdateEnableOptions();
    }

    void Undo() {
        if (undos.empty())
            return;
        auto undo = undos.end() - 1;
        auto& [groupIdx, componentIdx, undoFn] = *undo;
        runningUndo = true;
        TempSelect(groupIdx, componentIdx);
        undoFn();
        EndTempSelect();
        runningUndo = false;
        undos.erase(undo);
        if (groupIdx == selectedGroupIdx && componentIdx == selectedComponentIdx)
            OptionsViewController::GetInstance()->UpdateUI();
        if (undos.empty())
            SettingsViewController::GetInstance()->UpdateUI();
    }

    bool HasUndo() {
        return !undos.empty();
    }

    void ClearUndos() {
        undos.clear();
        SettingsViewController::GetInstance()->UpdateUI();
    }

    int GetActionId() {
        return highestActionId++;
    }

    void FinalizeAction() {
        if (disableActions)
            return;
        float time = UnityEngine::Time::get_time();
        bool merge = currentActionId == lastActionId && time - lastActionTime < UNDO_MERGE_THRESHOLD;
        lastActionTime = time;
        lastActionId = currentActionId;
        currentActionId = -1;
        if (!nextUndoUpdates.empty()) {
            if (merge && !runningUndo && !disableActions && !undos.empty()) {
                std::set<void (*)()> newUpdates = {};
                for (auto& update : nextUndoUpdates) {
                    if (!lastUndoUpdates.contains(update))
                        newUpdates.emplace(update);
                }
                if (!newUpdates.empty()) {
                    auto& [a, b, oldUndo] = undos.back();
                    oldUndo = [oldUndo, updates = std::move(newUpdates)]() {
                        oldUndo();
                        for (auto& update : updates)
                            update();
                    };
                }
            } else if (selectedComponentIdx == -1) {
                AddUndo([state = nextUndoGroup, updates = nextUndoUpdates]() {
                    GetSelectedGroup(-1) = state;
                    for (auto& update : updates)
                        update();
                });
            } else {
                AddUndo([state = nextUndoComponent, updates = nextUndoUpdates]() {
                    GetSelectedComponent(-1) = state;
                    for (auto& update : updates)
                        update();
                });
            }
        }
        lastUndoUpdates.swap(nextUndoUpdates);
        nextUndoUpdates.clear();
    }

    void DisableActions() {
        disableActions = true;
    }

    void EnableActions() {
        disableActions = false;
    }

    Group& GetSelectedGroup(int actionId) {
        auto& ret = GetGroup(selectedGroupIdx);
        if (!runningUndo && !disableActions && actionId >= 0 && actionId != currentActionId) {
            currentActionId = actionId;
            nextUndoGroup = ret;
        }
        return ret;
    }

    Component& GetSelectedComponent(int actionId) {
        auto& ret = GetGroup(selectedGroupIdx).Components[selectedComponentIdx];
        if (!runningUndo && !disableActions && actionId >= 0 && actionId != currentActionId) {
            currentActionId = actionId;
            nextUndoComponent = ret;
        }
        return ret;
    }
}
