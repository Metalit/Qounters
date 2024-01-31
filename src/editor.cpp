#include "editor.hpp"
#include "config.hpp"
#include "customtypes/editing.hpp"
#include "customtypes/settings.hpp"
#include "main.hpp"
#include "qounters.hpp"
#include "sources.hpp"
#include "utils.hpp"

#include "questui/shared/BeatSaberUI.hpp"

#include "VRUIControls/VRInputModule.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"
#include "UnityEngine/Rect.hpp"

using namespace QuestUI;

namespace Qounters::Editor {
    constexpr int anchorCount = (int) Group::Anchors::AnchorMax + 1;

    std::array<CanvasHighlight*, anchorCount> anchors;
    std::array<UnityEngine::GameObject*, anchorCount> dragCanvases;

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
        undos.emplace_back(selectedGroupIdx, selectedComponentIdx, fn);
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

    void CreateDragCanvases() {
        for (int i = 0; i < anchorCount; i++) {
            if (!anchors[i]) {
                dragCanvases[i] = nullptr;
                continue;
            }
            auto canvas = BeatSaberUI::CreateCanvas();
            canvas->set_name("QountersDragCanvas");
            canvas->get_transform()->SetParent(anchors[i]->get_transform(), false);
            canvas->get_transform()->set_localScale({1, 1, 1});
            // might be nice to have a constrained area later
            canvas->GetComponent<UnityEngine::RectTransform*>()->set_sizeDelta({1000, 1000});
            canvas->AddComponent<CanvasHighlight*>()->set_raycastTarget(true);
            canvas->SetActive(false);
            dragCanvases[i] = canvas;
        }
    }

    void Initialize(Preset const& inPreset) {
        preset = inPreset;
        vrInput = UnityEngine::Resources::FindObjectsOfTypeAll<VRUIControls::VRInputModule*>().First();
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
        SetupAnchors();
        CreateDragCanvases();

        for (int i = 0; i < preset.Qounters.size(); i++)
            CreateQounterGroup(preset.Qounters[i], i, true);
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
                break;
            }
        }
        if (auto opt = il2cpp_utils::try_cast<EditingGroup>(object)) {
            auto group = *opt;
            int groupIdx = group->GetGroupIdx();
            removedGroupIdxs.emplace(groupIdx);
        } else if (auto opt = il2cpp_utils::try_cast<EditingComponent>(object)) {
            auto component = *opt;
            int groupIdx = component->GetEditingGroup()->GetGroupIdx();
            int componentIdx = component->GetComponentIdx();
            if (!removedComponentIdxs.contains(groupIdx))
                removedComponentIdxs[groupIdx] = {};
            removedComponentIdxs[groupIdx].emplace(componentIdx);
        }
    }

    void SelectEditing(EditingBase* object) {
        getLogger().debug("selected %p -> %p", selected, object);
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
        for (int i = 0; i < anchorCount; i++) {
            anchors[i]->set_raycastTarget(group);
            anchors[i]->SetHighlighted(i != anchor && group);
            anchors[i]->set_color(normalColor);
            dragCanvases[i]->SetActive(i == anchor);
        }
        currentAnchor = anchor;
    }
    bool UpdateDrag(EditingGroup* dragged) {
        using RaycastResult = VRUIControls::VRGraphicRaycaster::VRGraphicRaycastResult;

        auto controller = vrInput->vrPointer->vrController;
        auto ray = UnityEngine::Ray(controller->get_position(), controller->get_forward());

        auto results = List<RaycastResult>::New_ctor();
        for (int i = 0; i < anchorCount; i++) {
            if (i == currentAnchor)
                continue;

            auto raycaster = anchors[i]->GetComponent<VRUIControls::VRGraphicRaycaster*>();
            raycaster->RaycastCanvas(raycaster->canvas, ray, System::Single::MaxValue, 0, results);

            RaycastResult* hit = nullptr;
            for (int j = 0; !hit && j < results->get_Count(); j++) {
                if (results->items->values[j].graphic == anchors[i])
                    hit = &results->items->values[j];
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
        FinalizeAction();

        for (int i = 0; i < anchorCount; i++) {
            anchors[i]->set_raycastTarget(false);
            anchors[i]->SetHighlighted(false);
            dragCanvases[i]->SetActive(false);
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

    void RemoveWithoutDeselect(int groupIdx, int componentIdx) {
        auto remove = editing[{groupIdx, componentIdx}];
        if (componentIdx != -1) {
            auto comp = (EditingComponent*) remove;
            RemoveComponent(GetGroup(groupIdx).Components[componentIdx].Type, comp->typeComponent);
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
                for (auto componentIdx : toRemove)
                    RemoveWithoutDeselect(selectedGroupIdx, componentIdx);
            });
        }

        RemoveWithoutDeselect(selectedGroupIdx, selectedComponentIdx);
        Deselect();
    }

    void UpdatePosition() {
        if (!selected)
            return;
        auto rect = selected->rectTransform;
        if (selectedComponentIdx != -1) {
            AddUndo([state = nextUndoComponent]() {
                GetSelectedComponent(-1) = state;
                UpdatePosition();
            });
            UpdateComponentPosition(rect, GetSelectedComponent(lastActionId));
        } else {
            AddUndo([state = nextUndoGroup]() {
                GetSelectedGroup(-1) = state;
                UpdatePosition();
            });
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
        }
    }

    void UpdateInvertEnabled() {
        AddUndo([state = nextUndoComponent]() {
            GetSelectedComponent(-1) = state;
            UpdateInvertEnabled();
        });

        auto& component = GetSelectedComponent(lastActionId);
        auto editingComponent = (EditingComponent*) editing[{selectedGroupIdx, selectedComponentIdx}];
        UpdateComponentEnabled(editingComponent->typeComponent->get_gameObject(), component.EnableSource, component.EnableOptions, component.InvertEnable);
    }

    void SetOptions(int actionId, Component::OptionsTypes options) {
        auto& component = GetSelectedComponent(actionId);

        AddUndo([state = nextUndoComponent]() {
            SetOptions(-1, state.Options);
        });

        component.Options = options;
        auto editingComponent = (EditingComponent*) editing[{selectedGroupIdx, selectedComponentIdx}];
        UpdateComponentOptions(component.Type, editingComponent->typeComponent, options);
    }

    void SetSourceOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);

        AddUndo([state = nextUndoComponent]() {
            SetOptions(-1, state.Options);
        });

        SetSourceOptions(component, options);
        auto editingComponent = (EditingComponent*) editing[{selectedGroupIdx, selectedComponentIdx}];
        UpdateComponentOptions(component.Type, editingComponent->typeComponent, component.Options);
    }

    void SetColorOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);

        AddUndo([state = nextUndoComponent]() {
            SetColorOptions(-1, state.ColorOptions);
        });

        component.ColorOptions = options;
        auto editingComponent = (EditingComponent*) editing[{selectedGroupIdx, selectedComponentIdx}];
        UpdateComponentColor(editingComponent->typeComponent, component.ColorSource, component.ColorOptions);
    }

    void SetEnableOptions(int actionId, UnparsedJSON options) {
        auto& component = GetSelectedComponent(actionId);

        AddUndo([state = nextUndoComponent]() {
            SetEnableOptions(-1, state.EnableOptions);
        });

        component.EnableOptions = options;
        auto editingComponent = (EditingComponent*) editing[{selectedGroupIdx, selectedComponentIdx}];
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
