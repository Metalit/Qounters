#include "editor.hpp"
#include "config.hpp"
#include "customtypes/editing.hpp"
#include "customtypes/settings.hpp"
#include "main.hpp"
#include "qounters.hpp"
#include "sources.hpp"
#include "utils.hpp"

#include "bsml/shared/BSML-Lite.hpp"

#include "VRUIControls/VRInputModule.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Ray.hpp"
#include "UnityEngine/Resources.hpp"

using namespace BSML;

namespace Qounters::Editor {
    constexpr int anchorCount = (int) Group::Anchors::AnchorMax + 1;

    std::array<CanvasHighlight*, anchorCount> anchors;
    std::array<UnityEngine::GameObject*, anchorCount> dragCanvases;
    UnityEngine::GameObject* detachedDragCanvas;

    int currentAnchor;

    std::map<std::pair<int, int>, EditingBase*> editing;
    EditingBase* selected;
    int selectedGroupIdx, selectedComponentIdx;

    std::vector<std::tuple<int, int, std::function<void ()>>> undos;

    bool addDirectToPreset = false;

    Preset preset;

    std::set<int> removedGroupIdxs;
    std::map<int, std::set<int>> removedComponentIdxs;

    int highestActionId = 0;

    VRUIControls::VRInputModule* vrInput;

    const UnityEngine::Color normalColor = {1, 1, 1, 0.2};
    const UnityEngine::Color highlightColor = {0, 0.7, 0.8, 0.2};

    Group nextUndoGroup;
    Component nextUndoComponent;
    int lastActionId;
    bool newAction;

    bool runningUndo;

    bool disableActions;

    bool previewMode;

    int prevSelectedGroupIdx, prevSelectedComponentIdx;
    EditingBase* prevSelected;

    void TempSelect(int groupIdx, int componentIdx) {
        prevSelectedGroupIdx = selectedGroupIdx;
        prevSelectedComponentIdx = selectedComponentIdx;
        prevSelected = selected;
        selectedGroupIdx = groupIdx;
        selectedComponentIdx = componentIdx;
        selected = editing[{groupIdx, componentIdx}];
    }
    void EndTempSelect() {
        selectedGroupIdx = prevSelectedGroupIdx;
        selectedComponentIdx = prevSelectedComponentIdx;
        selected = prevSelected;
        prevSelected = nullptr;
    }

    inline void AddUndo(auto&& fn) {
        if (runningUndo || disableActions || !newAction)
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
            anchors[i]->set_color({1, 1, 1, 0.2});
            anchors[i]->set_raycastTarget(false);
        }
    }

    UnityEngine::GameObject* CreateDragCanvas(std::string name, UnityEngine::Transform* parent) {
        auto canvas = Lite::CreateCanvas();
        canvas->set_name(name);

        if (parent)
            canvas->get_transform()->SetParent(parent, false);
        canvas->get_transform()->set_localScale({1, 1, 1});
        // might be nice to have a constrained area later
        canvas->GetComponent<UnityEngine::RectTransform*>()->set_sizeDelta({1000, 1000});
        canvas->AddComponent<CanvasHighlight*>()->set_raycastTarget(true);
        canvas->SetActive(false);
        return canvas;
    }

    void CreateDragCanvases() {
        for (int i = 0; i < anchorCount; i++) {
            if (!anchors[i]) {
                dragCanvases[i] = nullptr;
                continue;
            }
            auto parent = anchors[i]->get_transform();
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
        lastActionId = -1;
        newAction = true;
        runningUndo = false;
        disableActions = false;
        previewMode = false;
        if (newEnvironment) {
            vrInput = UnityEngine::Resources::FindObjectsOfTypeAll<VRUIControls::VRInputModule*>()->First();
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
                UnityEngine::Object::Destroy(obj->get_gameObject());
        }
        Reset();
        InitializeInternal(preset, false);
        SettingsViewController::GetInstance()->UpdateUI();
    }

    void SetPreviewMode(bool preview) {
        previewMode = preview;
        for (auto& [_, obj] : editing)
            obj->outline->set_enabled(!preview);
        Deselect();
        UpdateAllEnables();
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
    }
    void RegisterEditingComponent(EditingComponent* object, int groupIdx, int componentIdx) {
        editing[{groupIdx, componentIdx}] = object;
        if (removedComponentIdxs.contains(groupIdx))
            removedComponentIdxs[groupIdx].erase(componentIdx);
    }

    void UnregisterEditing(EditingBase* object) {
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
        QountersLogger::Logger.debug("selected {} -> {}", fmt::ptr(selected), fmt::ptr(object));
        if (runningUndo || object == selected)
            return;
        if (selected)
            selected->Deselect();
        selected = object;

        auto options = OptionsViewController::GetInstance();

        if (auto opt = il2cpp_utils::try_cast<EditingGroup>(object)) {
            auto group = *opt;
            selectedGroupIdx = group->GetGroupIdx();
            selectedComponentIdx = -1;
            options->GroupSelected();
        } else if (auto opt = il2cpp_utils::try_cast<EditingComponent>(object)) {
            auto component = *opt;
            selectedGroupIdx = component->GetEditingGroup()->GetGroupIdx();
            selectedComponentIdx = component->GetComponentIdx();
            options->ComponentSelected();
        }

        SettingsFlowCoordinator::PresentOptions();
    }

    void Deselect() {
        if (runningUndo && selected != prevSelected)
            return;
        if (selected)
            selected->Deselect();
        selected = nullptr;
        prevSelected = nullptr;

        SettingsFlowCoordinator::PresentTemplates();
    }

    void BeginDrag(int anchor, bool group) {
        raycastCanvases.clear();
        blockOtherRaycasts = true;
        for (int i = 0; i < anchorCount; i++) {
            anchors[i]->set_raycastTarget(group);
            anchors[i]->SetHighlighted(i != anchor && group);
            anchors[i]->set_color(normalColor);
            dragCanvases[i]->SetActive(i == anchor);
            raycastCanvases.emplace(anchors[i]->GetComponent<UnityEngine::Canvas*>());
            raycastCanvases.emplace(dragCanvases[i]->GetComponent<UnityEngine::Canvas*>());
        }
        currentAnchor = anchor;
    }
    bool UpdateDrag(EditingGroup* dragged) {
        using RaycastResult = VRUIControls::VRGraphicRaycaster::VRGraphicRaycastResult;

        auto controller = vrInput->_vrPointer->_lastSelectedVrController;
        auto ray = UnityEngine::Ray(controller->get_position(), controller->get_forward());

        auto results = System::Collections::Generic::List_1<RaycastResult>::New_ctor();
        for (int i = 0; i < anchorCount; i++) {
            if (i == currentAnchor)
                continue;

            auto raycaster = anchors[i]->GetComponent<VRUIControls::VRGraphicRaycaster*>();
            raycaster->RaycastCanvas(raycaster->_canvas, ray, std::numeric_limits<float>::max(), 0, results);

            RaycastResult* hit = nullptr;
            for (int j = 0; !hit && j < results->get_Count(); j++) {
                if (results->_items->_values[j].graphic == anchors[i])
                    hit = &results->_items->_values[j];
            }
            if (hit) {
                BeginDrag(i, true);
                anchors[i]->SetHighlighted(true);
                anchors[i]->set_color(highlightColor);
                auto hitPoint = anchors[i]->get_rectTransform()->InverseTransformPoint(hit->position);
                GetSelectedGroup(lastActionId).Position = UnityEngine::Vector2(hitPoint.x, hitPoint.y);
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
            anchors[i]->set_raycastTarget(false);
            anchors[i]->SetHighlighted(false);
            dragCanvases[i]->SetActive(false);
        }
    }
    void EnableDetachedCanvas(bool enabled) {
        detachedDragCanvas->SetActive(enabled);
        raycastCanvases.clear();
        blockOtherRaycasts = enabled;
        if (enabled) {
            auto group = editing[{selectedGroupIdx, -1}];
            detachedDragCanvas->get_transform()->SetParent(group->rectTransform, false);
            raycastCanvases.emplace(detachedDragCanvas->GetComponent<UnityEngine::Canvas*>());
        }
    }

    void AddComponent() {
        auto& vec = GetSelectedGroup(lastActionId).Components;
        auto newIdx = vec.size();
        auto& newComponent = vec.emplace_back();
        newComponent.Type = (int) Component::Types::Text;
        TextOptions opts;
        opts.TextSource = TextSource::StaticName;
        TextSource::Static text;
        text.Input = "New Component";
        opts.SourceOptions = text;
        newComponent.Options = opts;

        CreateQounterComponent(newComponent, newIdx, selected->get_transform(), true);
        auto created = editing[{selectedGroupIdx, newIdx}];
        if (!runningUndo)
            created->Select();
        SelectEditing(created);

        newAction = true;
        lastActionId = -1;

        AddUndo(Remove);
    }

    void ToggleAttachment() {
        auto& group = GetSelectedGroup(-1, false);

        newAction = true;
        lastActionId = -1;

        AddUndo([state = group]() {
            GetSelectedGroup(-1) = state;
            UpdateGroupPosition(selected->rectTransform, state);
        });

        group.Detached = !group.Detached;
        group.DetachedPosition = selected->rectTransform->get_position();
        group.DetachedRotation = selected->rectTransform->get_rotation().get_eulerAngles();
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
        UnityEngine::Object::Destroy(remove->get_gameObject());
        if (remove == selected)
            selected = nullptr;
        if (remove == prevSelected)
            prevSelected = nullptr;
    }

    void Remove() {
        newAction = true;
        lastActionId = -1;

        if (selectedComponentIdx != -1) {
            AddUndo([state = GetSelectedComponent(-1, false)]() {
                CreateQounterComponent(state, selectedComponentIdx, editing[{selectedGroupIdx, -1}]->get_transform(), true);
            });
        } else {
            AddUndo([state = GetSelectedGroup(-1, false)]() {
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

    void UpdatePosition(bool neverSnap) {
        if (!selected)
            return;
        bool snap = !neverSnap && !runningUndo && getConfig().Snap.GetValue();
        auto rect = selected->rectTransform;
        if (selectedComponentIdx != -1) {
            AddUndo([state = nextUndoComponent]() {
                GetSelectedComponent(-1) = state;
                UpdatePosition();
            });
            if (snap)
                SnapPosition(GetSelectedComponent(lastActionId).Position);
            UpdateComponentPosition(rect, GetSelectedComponent(lastActionId));
        } else {
            AddUndo([state = nextUndoGroup]() {
                GetSelectedGroup(-1) = state;
                UpdatePosition();
            });
            if (snap)
                SnapPosition(GetSelectedGroup(lastActionId).Position);
            UpdateGroupPosition(rect, GetSelectedGroup(lastActionId));
        }
    }

    void UpdateType() {
        AddUndo([state = nextUndoComponent]() {
            GetSelectedComponent(-1) = state;
            UpdateType();
        });

        auto& component = GetSelectedComponent(lastActionId);
        RemoveWithoutDeselect(selectedGroupIdx, selectedComponentIdx);
        if (!runningUndo)
            SetDefaultOptions(component);
        CreateQounterComponent(component, selectedComponentIdx, editing[{selectedGroupIdx, -1}]->get_transform(), true);

        if (!runningUndo) {
            selected = editing[{selectedGroupIdx, selectedComponentIdx}];
            selected->Select();
            OptionsViewController::GetInstance()->UpdateUI();
        }
    }

    void UpdateColorSource() {
        AddUndo([state = nextUndoComponent]() {
            GetSelectedComponent(-1) = state;
            UpdateColorSource();
        });

        if (!runningUndo) {
            SetColorOptions(lastActionId, ColorSource::Static());
            OptionsViewController::GetInstance()->UpdateUI();
        } else {
            auto& component = GetSelectedComponent(-1);
            auto editingComponent = (EditingComponent*) selected;
            UpdateComponentColor(editingComponent->typeComponent, component.ColorSource, component.ColorOptions);
        }
    }

    void UpdateEnableSource() {
        AddUndo([state = nextUndoComponent]() {
            GetSelectedComponent(-1) = state;
            UpdateEnableSource();
        });

        if (!runningUndo) {
            SetEnableOptions(lastActionId, EnableSource::Static());
            OptionsViewController::GetInstance()->UpdateUI();
        } else {
            auto& component = GetSelectedComponent(-1);
            auto editingComponent = (EditingComponent*) selected;
            UpdateComponentEnabled(editingComponent->typeComponent->get_gameObject(), component.EnableSource, component.EnableOptions, component.InvertEnable);
        }
    }

    void UpdateInvertEnabled() {
        AddUndo([state = nextUndoComponent]() {
            GetSelectedComponent(-1) = state;
            UpdateInvertEnabled();
        });

        auto& component = GetSelectedComponent(lastActionId);
        auto editingComponent = (EditingComponent*) selected;
        UpdateComponentEnabled(editingComponent->typeComponent->get_gameObject(), component.EnableSource, component.EnableOptions, component.InvertEnable);
    }

    void SetOptions(int actionId, Component::OptionsTypes options) {
        auto& component = GetSelectedComponent(actionId);

        AddUndo([state = nextUndoComponent]() {
            SetOptions(-1, state.Options);
        });

        component.Options = options;
        auto editingComponent = (EditingComponent*) selected;
        UpdateComponentOptions(component.Type, editingComponent->typeComponent, options);
    }

    void SetSourceOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);

        AddUndo([state = nextUndoComponent]() {
            SetOptions(-1, state.Options);
        });

        SetSourceOptions(component, options);
        auto editingComponent = (EditingComponent*) selected;
        UpdateComponentOptions(component.Type, editingComponent->typeComponent, component.Options);
    }

    void SetColorOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);

        AddUndo([state = nextUndoComponent]() {
            SetColorOptions(-1, state.ColorOptions);
        });

        component.ColorOptions = options;
        auto editingComponent = (EditingComponent*) selected;
        UpdateComponentColor(editingComponent->typeComponent, component.ColorSource, component.ColorOptions);
    }

    void SetEnableOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);

        AddUndo([state = nextUndoComponent]() {
            SetEnableOptions(-1, state.EnableOptions);
        });

        component.EnableOptions = options;
        auto editingComponent = (EditingComponent*) selected;
        UpdateComponentEnabled(editingComponent->typeComponent->get_gameObject(), component.EnableSource, component.EnableOptions, component.InvertEnable);
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
        if (!disableActions)
            lastActionId = -1;
    }

    void DisableActions() {
        disableActions = true;
    }

    void EnableActions() {
        disableActions = false;
    }

    Group& GetSelectedGroup(int actionId, bool addUndo) {
        addUndo = addUndo && !runningUndo && !disableActions;
        if (addUndo) {
            newAction = actionId != lastActionId;
            lastActionId = actionId;
        }
        auto& ret = GetGroup(selectedGroupIdx);
        if (addUndo && newAction)
            nextUndoGroup = ret;
        return ret;
    }

    Component& GetSelectedComponent(int actionId, bool addUndo) {
        addUndo = addUndo && !runningUndo && !disableActions;
        if (addUndo) {
            newAction = actionId != lastActionId;
            lastActionId = actionId;
        }
        auto& ret = GetGroup(selectedGroupIdx).Components[selectedComponentIdx];
        if (addUndo && newAction)
            nextUndoComponent = ret;
        return ret;
    }
}
