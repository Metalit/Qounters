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
#include "copies.hpp"
#include "customtypes/editing.hpp"
#include "customtypes/settings.hpp"
#include "main.hpp"
#include "playtest.hpp"
#include "qounters.hpp"
#include "sources.hpp"
#include "utils.hpp"

using namespace Qounters;

static constexpr int AnchorCount = (int) Options::Group::Anchors::AnchorMax + 1;

static UnityEngine::Color const NormalColor = {1, 1, 1, 0.2};
static UnityEngine::Color const HighlightColor = {0, 0.7, 0.8, 0.2};

static std::array<CanvasHighlight*, AnchorCount> anchors;
static std::array<UnityEngine::GameObject*, AnchorCount> dragCanvases;
static UnityEngine::GameObject* detachedDragCanvas;

static int currentAnchor;

static std::map<std::pair<int, int>, EditingBase*> editing;
static EditingBase* selected;
static int selectedGroupIdx, selectedComponentIdx;

static std::vector<std::tuple<int, int, std::function<void()>>> undos;

static bool addDirectToPreset = false;

static Options::Preset preset;

static bool hasCopy;
static Options::Component copied;

static std::set<int> removedGroupIdxs;
static std::map<int, std::set<int>> removedComponentIdxs;

static int highestActionId = Copies::CopyMax;

static VRUIControls::VRInputModule* vrInput;

static Options::Group nextUndoGroup;
static Options::Component nextUndoComponent;
static int currentActionId;
static std::set<void (*)()> nextUndoUpdates;

static int lastActionId;
static float lastActionTime;
static std::set<void (*)()> lastUndoUpdates;

static bool runningUndo;

static bool disableActions;

static bool previewMode;

static int prevSelectedGroupIdx, prevSelectedComponentIdx;
static EditingBase* prevSelected;
static bool tempSelectIsReal;

static void TempSelect(int groupIdx, int componentIdx) {
    prevSelectedGroupIdx = selectedGroupIdx;
    prevSelectedComponentIdx = selectedComponentIdx;
    prevSelected = selected;
    selectedGroupIdx = groupIdx;
    selectedComponentIdx = componentIdx;
    selected = editing[{groupIdx, componentIdx}];
    tempSelectIsReal = prevSelected == selected;
}
static void EndTempSelect() {
    selectedGroupIdx = prevSelectedGroupIdx;
    selectedComponentIdx = prevSelectedComponentIdx;
    if (!tempSelectIsReal)
        selected = prevSelected;
    prevSelected = nullptr;
}

static inline void AddUpdate(void (*update)()) {
    if (runningUndo || disableActions)
        return;
    nextUndoUpdates.emplace(update);
}
static inline bool AddUndo(auto&& fn) {
    if (runningUndo || disableActions)
        return false;
    bool wasEmpty = undos.empty();
    undos.emplace_back(selectedGroupIdx, selectedComponentIdx, fn);
    if (wasEmpty)
        SettingsViewController::GetInstance()->UpdateUI();
    return true;
}

static void SetupAnchors() {
    for (int i = 0; i < AnchorCount; i++) {
        auto anchor = HUD::GetAnchor(i);
        if (!anchor) {
            anchors[i] = nullptr;
            continue;
        }
        anchors[i] = Utils::GetOrAddComponent<CanvasHighlight*>(anchor);
        anchors[i]->color = {1, 1, 1, 0.2};
        anchors[i]->raycastTarget = false;
    }
}

static UnityEngine::GameObject* CreateDragCanvas(std::string name, UnityEngine::Transform* parent) {
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

static void CreateDragCanvases() {
    for (int i = 0; i < AnchorCount; i++) {
        if (!anchors[i]) {
            dragCanvases[i] = nullptr;
            continue;
        }
        auto parent = anchors[i]->transform;
        dragCanvases[i] = CreateDragCanvas("QountersDragCanvas" + std::to_string(i), parent);
    }
    detachedDragCanvas = CreateDragCanvas("QountersDetachedDragCanvas", nullptr);
}

static void InitializeInternal(Options::Preset const& inPreset, bool newEnvironment) {
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
        HUD::CreateQounterGroup(preset.Qounters[i], i, true);
}

void Editor::Initialize(Options::Preset const& inPreset) {
    InitializeInternal(inPreset, true);
}

void Editor::LoadPreset(Options::Preset const& preset) {
    for (auto& [idxs, obj] : editing) {
        auto& [groupIdx, componentIdx] = idxs;
        if (componentIdx == -1)
            UnityEngine::Object::Destroy(obj->gameObject);
    }
    HUD::Reset(false);
    InitializeInternal(preset, false);
    SettingsViewController::GetInstance()->UpdateUI();
    Playtest::SetEnabled(false);
    SettingsFlowCoordinator::PresentTemplates();
}

void Editor::SetPreviewMode(bool preview) {
    previewMode = preview;
    for (auto& [_, obj] : editing)
        obj->outline->enabled = !preview;
    Deselect();
    HUD::UpdateAllEnables();
    if (preview)
        SettingsFlowCoordinator::PresentPlaytest();
    else
        SettingsFlowCoordinator::PresentTemplates();
    Playtest::SetEnabled(preview);
}

bool Editor::GetPreviewMode() {
    return previewMode;
}

void Editor::SetPresetForMigrating(Options::Preset migratingPreset) {
    addDirectToPreset = true;
    preset = migratingPreset;
}

Options::Preset Editor::GetPreset() {
    if (addDirectToPreset) {
        addDirectToPreset = false;
        return preset;
    }
    Options::Preset ret = preset;
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

Options::Group& Editor::GetGroup(int idx) {
    return preset.Qounters[idx];
}

void Editor::AddGroup(Options::Group group) {
    if (addDirectToPreset) {
        preset.Qounters.emplace_back(std::move(group));
        return;
    }

    int newIdx = preset.Qounters.size();
    auto& moved = preset.Qounters.emplace_back(std::move(group));
    HUD::CreateQounterGroup(moved, newIdx, true);

    auto created = editing[{newIdx, -1}];
    if (!runningUndo)
        created->Select();
    SelectEditing(created);

    AddUndo(Remove);
}

static void AddComponent(Options::Component component) {
    auto& vec = Editor::GetSelectedGroup(-1).Components;
    auto newIdx = vec.size();
    auto& newComponent = vec.emplace_back(std::move(component));

    HUD::CreateQounterComponent(newComponent, newIdx, editing[{selectedGroupIdx, -1}]->transform, true);
    auto created = editing[{selectedGroupIdx, newIdx}];
    created->Select();
    Editor::SelectEditing(created);

    AddUndo(Editor::Remove);
}

void Editor::Duplicate() {
    if (selectedComponentIdx == -1) {
        auto group = GetSelectedGroup(-1);
        if (group.Detached)
            group.DetachedPosition.z -= 0.25;
        else
            group.Position.y -= 5;
        AddGroup(std::move(group));
    } else {
        auto component = GetSelectedComponent(-1);
        component.Position.y -= 5;
        AddComponent(std::move(component));
    }
}

void Editor::CopyComponent() {
    hasCopy = true;
    copied = GetSelectedComponent(-1);
}

bool Editor::CanPasteComponent() {
    return hasCopy;
}

void Editor::PasteComponent() {
    if (hasCopy)
        AddComponent(copied);
}

void Editor::RegisterEditingGroup(EditingGroup* object, int groupIdx) {
    editing[{groupIdx, -1}] = object;
    removedGroupIdxs.erase(groupIdx);
    if (selectedGroupIdx == groupIdx && selectedComponentIdx == -1)
        SelectEditing(object);
}
void Editor::RegisterEditingComponent(EditingComponent* object, int groupIdx, int componentIdx) {
    editing[{groupIdx, componentIdx}] = object;
    if (removedComponentIdxs.contains(groupIdx))
        removedComponentIdxs[groupIdx].erase(componentIdx);
    if (selectedGroupIdx == groupIdx && selectedComponentIdx == componentIdx)
        SelectEditing(object);
}

void Editor::UnregisterEditing(EditingBase* object) {
    if (object == selected)
        selected = nullptr;
    for (auto it = editing.begin(); it != editing.end(); it++) {
        if (it->second == object) {
            editing.erase(it);
            return;
        }
    }
}

static void MarkAsRemoved(int groupIdx, int componentIdx) {
    if (componentIdx != -1) {
        if (!removedComponentIdxs.contains(groupIdx))
            removedComponentIdxs[groupIdx] = {};
        removedComponentIdxs[groupIdx].emplace(componentIdx);
    } else
        removedGroupIdxs.emplace(groupIdx);
}

void Editor::SelectEditing(EditingBase* object) {
    logger.debug("selected {} -> {}", fmt::ptr(selected), fmt::ptr(object));
    if (runningUndo || object == selected)
        return;
    if (selected)
        selected->Deselect();
    selected = object;

    auto options = OptionsViewController::GetInstance();

    if (auto group = Utils::ptr_cast<EditingGroup>(object)) {
        selectedGroupIdx = group->GetGroupIdx();
        selectedComponentIdx = -1;
        options->GroupSelected();
    } else if (auto component = Utils::ptr_cast<EditingComponent>(object)) {
        selectedGroupIdx = component->GetEditingGroup()->GetGroupIdx();
        selectedComponentIdx = component->GetComponentIdx();
        options->ComponentSelected();
    }

    SettingsFlowCoordinator::PresentOptions();
}

void Editor::Deselect() {
    if (runningUndo && !tempSelectIsReal)
        return;
    if (selected)
        selected->Deselect();
    selected = nullptr;

    SettingsFlowCoordinator::PresentTemplates();
}

void Editor::BeginDrag(int anchor, bool group) {
    raycastCanvases.clear();
    blockOtherRaycasts = true;
    for (int i = 0; i < AnchorCount; i++) {
        anchors[i]->raycastTarget = group;
        anchors[i]->SetHighlighted(i != anchor && group);
        anchors[i]->color = NormalColor;
        dragCanvases[i]->active = i == anchor;
        raycastCanvases.emplace(anchors[i]->GetComponent<UnityEngine::Canvas*>());
        raycastCanvases.emplace(dragCanvases[i]->GetComponent<UnityEngine::Canvas*>());
    }
    currentAnchor = anchor;
}
bool Editor::UpdateDrag(EditingGroup* dragged) {
    using RaycastResult = VRUIControls::VRGraphicRaycaster::VRGraphicRaycastResult;

    auto controller = vrInput->_vrPointer->_lastSelectedVrController->_viewAnchorTransform;
    auto ray = UnityEngine::Ray(controller->position, controller->forward);

    auto results = ListW<RaycastResult>::New();
    for (int i = 0; i < AnchorCount; i++) {
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
            anchors[i]->color = HighlightColor;
            auto hitPoint = anchors[i]->rectTransform->InverseTransformPoint(hit->position);
            GetSelectedGroup(-1).Position = UnityEngine::Vector2(hitPoint.x, hitPoint.y);
            dragged->UpdateDragAnchor(i);
            return true;
        }
    }
    return false;
}
void Editor::EndDrag() {
    raycastCanvases.clear();
    blockOtherRaycasts = false;
    for (int i = 0; i < AnchorCount; i++) {
        anchors[i]->raycastTarget = false;
        anchors[i]->SetHighlighted(false);
        dragCanvases[i]->active = false;
    }
}
void Editor::EnableDetachedCanvas(bool enabled) {
    detachedDragCanvas->active = enabled;
    raycastCanvases.clear();
    blockOtherRaycasts = enabled;
    if (enabled) {
        auto group = editing[{selectedGroupIdx, -1}];
        detachedDragCanvas->transform->SetParent(group->rectTransform);
        detachedDragCanvas->transform->SetLocalPositionAndRotation({0, 0, 0}, UnityEngine::Quaternion::get_identity());
        raycastCanvases.emplace(detachedDragCanvas->GetComponent<UnityEngine::Canvas*>());
    } else
        detachedDragCanvas->transform->SetParent(nullptr);
}
void Editor::UpdateDetachedDrag(UnityEngine::Vector3 const& pos, UnityEngine::Vector3 const& controller) {
    auto transform = detachedDragCanvas->transform;
    transform->position = pos;
    transform->LookAt(controller);
}

void Editor::NewComponent() {
    Options::Component newComponent;
    newComponent.Type = (int) Options::Component::Types::Text;
    Options::Text opts;
    opts.TextSource = Sources::Text::StaticName;
    Sources::Text::Static text;
    text.Input = "New Counter";
    opts.SourceOptions = text;
    newComponent.Options = opts;

    AddComponent(std::move(newComponent));
}

void Editor::ToggleAttachment() {
    auto& group = GetSelectedGroup(-1);

    AddUndo([state = group]() {
        GetSelectedGroup(-1) = state;
        HUD::UpdateGroupPosition(selected->rectTransform, state);
    });

    group.Detached = !group.Detached;
    group.DetachedPosition = selected->rectTransform->position;
    group.DetachedRotation = Utils::GetFixedEuler(selected->rectTransform->rotation);
    HUD::UpdateGroupPosition(selected->rectTransform, group);

    if (!runningUndo)
        OptionsViewController::GetInstance()->UpdateSimpleUI();
}

void Editor::SwapGradientColors() {
    AddUndo(SwapGradientColors);

    auto& comp = GetSelectedComponent(-1);
    std::swap(comp.GradientOptions.StartModifierHSV, comp.GradientOptions.EndModifierHSV);

    auto editingComponent = (EditingComponent*) selected;
    HUD::UpdateComponentColor(editingComponent->typeComponent, comp.ColorSource, comp.ColorOptions, comp.GradientOptions);

    if (!runningUndo)
        OptionsViewController::GetInstance()->UpdateUI();
}

static void RemoveWithoutDeselect(int groupIdx, int componentIdx) {
    auto remove = editing[{groupIdx, componentIdx}];
    if (componentIdx != -1) {
        auto comp = (EditingComponent*) remove;
        HUD::RemoveComponent(Editor::GetGroup(groupIdx).Components[componentIdx].Type, comp->typeComponent);
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
            Editor::UnregisterEditing(component);
            HUD::RemoveComponent(Editor::GetGroup(groupIdx).Components[otherComponentIdx].Type, component->typeComponent);
        }
    }
    Editor::UnregisterEditing(remove);
    UnityEngine::Object::Destroy(remove->gameObject);
}

void Editor::Remove() {
    if (selectedComponentIdx != -1) {
        AddUndo([state = GetSelectedComponent(-1)]() {
            HUD::CreateQounterComponent(state, selectedComponentIdx, editing[{selectedGroupIdx, -1}]->transform, true);
        });
    } else {
        AddUndo([state = GetSelectedGroup(-1)]() {
            std::set<int> toRemove;
            if (removedComponentIdxs.contains(selectedGroupIdx))
                toRemove = removedComponentIdxs[selectedGroupIdx];
            HUD::CreateQounterGroup(state, selectedGroupIdx, true);
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

static void SnapPosition(ConfigUtils::Vector2& position) {
    float step = getConfig().SnapStep.GetValue();
    position.x = floor((position.x / step) + 0.5) * step;
    position.y = floor((position.y / step) + 0.5) * step;
}

static void UpdatePositionUndo() {
    Editor::UpdatePosition(true);
}
void Editor::UpdatePosition(bool neverSnap) {
    if (!selected)
        return;
    AddUpdate(UpdatePositionUndo);
    bool snap = !neverSnap && getConfig().Snap.GetValue();
    auto rect = selected->rectTransform;
    if (selectedComponentIdx != -1) {
        if (snap)
            SnapPosition(GetSelectedComponent(-1).Position);
        HUD::UpdateComponentPosition(rect, GetSelectedComponent(-1));
    } else {
        if (snap)
            SnapPosition(GetSelectedGroup(-1).Position);
        HUD::UpdateGroupPosition(rect, GetSelectedGroup(-1));
    }
}

void Editor::UpdateType() {
    UpdateAll();
}
void Editor::SetType(int actionId, Options::Component::Types type) {
    auto& component = GetSelectedComponent(actionId);
    component.Type = (int) type;
    HUD::SetDefaultOptions(component);
    UpdateType();
}

void Editor::SetColorSource(int actionId, std::string source) {
    auto& component = GetSelectedComponent(actionId);
    component.ColorSource = source;
    SetColorOptions(actionId, Sources::Color::Static());
    OptionsViewController::GetInstance()->UpdateUI();
}

void Editor::SetEnableSource(int actionId, std::string source) {
    auto& component = GetSelectedComponent(actionId);
    component.EnableSource = source;
    SetEnableOptions(actionId, Sources::Enable::Static());
    OptionsViewController::GetInstance()->UpdateUI();
}

void Editor::UpdateOptions() {
    auto& component = GetSelectedComponent(-1);
    auto editingComponent = (EditingComponent*) selected;
    HUD::UpdateComponentOptions(component.Type, editingComponent->typeComponent, component.Options);
    AddUpdate(UpdateOptions);
}
void Editor::SetOptions(int actionId, Options::Component::OptionsTypes options) {
    auto& component = GetSelectedComponent(actionId);
    component.Options = options;
    UpdateOptions();
}

void Editor::UpdateSourceOptions() {
    // updates a few unnecessary things, but it's ok
    UpdateOptions();
}
void Editor::SetSourceOptions(int actionId, UnparsedJSON options) {
    auto& component = GetSelectedComponent(actionId);
    HUD::SetSourceOptions(component, options);
    UpdateSourceOptions();
}

void Editor::UpdateColor() {
    auto& component = GetSelectedComponent(-1);
    auto editingComponent = (EditingComponent*) selected;
    HUD::UpdateComponentColor(editingComponent->typeComponent, component.ColorSource, component.ColorOptions, component.GradientOptions);
    AddUpdate(UpdateColor);
}
void Editor::SetColorOptions(int actionId, UnparsedJSON options) {
    auto& component = GetSelectedComponent(actionId);
    component.ColorOptions = options;
    UpdateColor();
}

void Editor::UpdateEnable() {
    auto& component = GetSelectedComponent(-1);
    auto editingComponent = (EditingComponent*) selected;
    HUD::UpdateComponentEnabled(editingComponent->typeComponent->gameObject, component.EnableSource, component.EnableOptions, component.InvertEnable);
    AddUpdate(UpdateEnable);
}
void Editor::SetEnableOptions(int actionId, UnparsedJSON options) {
    auto& component = GetSelectedComponent(actionId);
    component.EnableOptions = options;
    UpdateEnable();
}

void Editor::UpdateAll() {
    if (selectedComponentIdx != -1) {
        auto& component = GetSelectedComponent(-1);
        RemoveWithoutDeselect(selectedGroupIdx, selectedComponentIdx);
        HUD::CreateQounterComponent(component, selectedComponentIdx, editing[{selectedGroupIdx, -1}]->transform, true);
    } else {
        auto& group = GetSelectedGroup(-1);
        RemoveWithoutDeselect(selectedGroupIdx, -1);
        HUD::CreateQounterGroup(group, selectedGroupIdx, true);
    }

    if (!runningUndo || tempSelectIsReal) {
        selected = editing[{selectedGroupIdx, selectedComponentIdx}];
        selected->Select();
        OptionsViewController::GetInstance()->UpdateUI();
    }
    AddUpdate(UpdateAll);
}

void Editor::Undo() {
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

bool Editor::HasUndo() {
    return !undos.empty();
}

void Editor::ClearUndos() {
    undos.clear();
    SettingsViewController::GetInstance()->UpdateUI();
}

int Editor::GetActionId() {
    return highestActionId++;
}

bool Editor::FinalizeAction() {
    if (disableActions)
        return false;
    float time = UnityEngine::Time::get_time();
    bool merge = currentActionId == lastActionId && time - lastActionTime < UNDO_MERGE_THRESHOLD;
    lastActionTime = time;
    lastActionId = currentActionId;
    currentActionId = -1;
    bool unchanged = selectedComponentIdx == -1 ? GetSelectedGroup(-1) == nextUndoGroup : GetSelectedComponent(-1) == nextUndoComponent;
    if (unchanged || nextUndoUpdates.empty()) {
        nextUndoUpdates.clear();
        return false;
    }
    bool added = false;
    if (merge && !runningUndo && !undos.empty()) {
        // most of this is just preventing redundant updates, so not strictly necessary
        if (!lastUndoUpdates.contains(UpdateAll)) {
            std::set<void (*)()> newUpdates = {};
            for (auto& update : nextUndoUpdates) {
                if (!lastUndoUpdates.contains(update))
                    newUpdates.emplace(update);
            }
            if (!newUpdates.empty()) {
                if (newUpdates.contains(UpdateAll))
                    newUpdates = {UpdateAll};
                auto& oldUndo = std::get<2>(undos.back());
                oldUndo = [oldUndo, updates = std::move(newUpdates)]() {
                    oldUndo();
                    for (auto& update : updates)
                        update();
                };
            }
            for (auto& update : lastUndoUpdates)
                nextUndoUpdates.emplace(update);
        } else
            nextUndoUpdates = {UpdateAll};
    } else if (selectedComponentIdx == -1) {
        added = AddUndo([state = nextUndoGroup, updates = nextUndoUpdates]() {
            GetSelectedGroup(-1) = state;
            for (auto& update : updates)
                update();
        });
    } else {
        added = AddUndo([state = nextUndoComponent, updates = nextUndoUpdates]() {
            GetSelectedComponent(-1) = state;
            for (auto& update : updates)
                update();
        });
    }
    lastUndoUpdates.swap(nextUndoUpdates);
    nextUndoUpdates.clear();
    return added;
}

void Editor::DisableActions() {
    disableActions = true;
}

void Editor::EnableActions() {
    disableActions = false;
}

Options::Group& Editor::GetSelectedGroup(int actionId) {
    auto& ret = GetGroup(selectedGroupIdx);
    if (!runningUndo && !disableActions && actionId >= 0 && actionId != currentActionId) {
        currentActionId = actionId;
        nextUndoGroup = ret;
    }
    return ret;
}

Options::Component& Editor::GetSelectedComponent(int actionId) {
    auto& ret = GetGroup(selectedGroupIdx).Components[selectedComponentIdx];
    if (!runningUndo && !disableActions && actionId >= 0 && actionId != currentActionId) {
        currentActionId = actionId;
        nextUndoComponent = ret;
    }
    return ret;
}
