#include "customtypes/editing.hpp"

#include "GlobalNamespace/VRController.hpp"
#include "UnityEngine/Bounds.hpp"
#include "UnityEngine/DrivenTransformProperties.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "config.hpp"
#include "customtypes/settings.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "utils.hpp"

DEFINE_TYPE(Qounters, Outline);
DEFINE_TYPE(Qounters, CanvasHighlight);
DEFINE_TYPE(Qounters, TextOutlineSizer);
DEFINE_TYPE(Qounters, GroupOutlineSizer);
DEFINE_TYPE(Qounters, EditingBase);
DEFINE_TYPE(Qounters, EditingGroup);
DEFINE_TYPE(Qounters, EditingComponent);

using namespace Qounters;
using namespace UnityEngine;

Color const offColor = {0.7, 0.7, 0.7, 1};
Color const highlightColor = {0, 0.7, 0.8, 1};
Color const selectColor = {0.2, 0.5, 0.6, 1};

Material* Outline::material = nullptr;
int Outline::count = 0;

void Outline::Awake() {
    count++;
    if (!material)
        material = Resources::FindObjectsOfTypeAll<Material*>()->First([](auto x) { return x->name == StringW("UINoGlow"); });
    set_material(material);
}

void Outline::OnDestroy() {
    count--;
    if (count == 0)
        material = nullptr;
}

void Outline::OnEnable() {
    static auto base = il2cpp_utils::FindMethodUnsafe(classof(UI::Graphic*), "OnEnable", 0);
    il2cpp_utils::RunMethod(this, base);

    rectTransform->anchorMin = {0, 0};
    rectTransform->anchorMax = {1, 1};
    UpdateSize();
}

void Outline::SetBaseSize(Vector2 value) {
    baseSize = value;
    UpdateSize();
}

void Outline::SetBorderWidth(float value) {
    borderWidth = value;
    UpdateSize();
}

void Outline::SetBorderGap(float value) {
    borderGap = value;
    UpdateSize();
}

void Outline::UpdateSize() {
    auto scale = transform->lossyScale;
    auto pad = 2 * (borderGap + borderWidth);
    rectTransform->sizeDelta = {baseSize.x + (pad * 0.02f / scale.x), baseSize.y + (pad * 0.02f / scale.y)};
    SetVerticesDirty();
}

void Outline::OnPopulateMesh(UI::VertexHelper* vh) {
    if (borderWidth <= 0)
        return;

    auto scale = transform->lossyScale;

    float borderX = borderWidth * 0.02 / scale.x;
    float borderY = borderWidth * 0.02 / scale.y;

    auto rect = GetPixelAdjustedRect();
    float width = rect.width;
    float height = rect.height;
    float xMin = rect.xMin;
    float yMin = rect.yMin;
    float xMax = rect.xMax;
    float yMax = rect.yMax;

    float perimeter = width * 2 + height * 2;

    auto color32 = Color32::op_Implicit___UnityEngine__Color32(color);

    vh->Clear();

    float uv = height / perimeter;
    vh->AddVert({xMin, yMin, 0}, color32, {0, 0, 0, 0});
    vh->AddVert({xMin, yMax, 0}, color32, {uv, 0, 0, 0});
    vh->AddVert({xMin + borderX, yMax - borderY, 0}, color32, {uv, 1, 0, 0});
    vh->AddVert({xMin + borderX, yMin + borderY, 0}, color32, {0, 1, 0, 0});
    vh->AddTriangle(0, 1, 2);
    vh->AddTriangle(2, 3, 0);

    uv = (height + width) / perimeter;
    vh->AddVert({xMax, yMax, 0}, color32, {uv, 0, 0, 0});
    vh->AddVert({xMax - borderX, yMax - borderY, 0}, color32, {uv, 1, 0, 0});
    vh->AddTriangle(1, 4, 5);
    vh->AddTriangle(5, 2, 1);

    uv = (height + width + height) / perimeter;
    vh->AddVert({xMax, yMin, 0}, color32, {uv, 0, 0, 0});
    vh->AddVert({xMax - borderX, yMin + borderY, 0}, color32, {uv, 1, 0, 0});
    vh->AddTriangle(4, 6, 7);
    vh->AddTriangle(7, 5, 4);
    vh->AddTriangle(6, 0, 3);
    vh->AddTriangle(3, 7, 6);
}

Outline* Outline::Create(Component* obj) {
    auto child = GameObject::New_ctor("QountersOutline");
    child->transform->SetParent(obj->transform, false);
    return child->AddComponent<Outline*>();
}

void CanvasHighlight::OnEnable() {
    static auto base = il2cpp_utils::FindMethodUnsafe(classof(UI::Graphic*), "OnEnable", 0);
    il2cpp_utils::RunMethod(this, base);

    auto mat = Resources::FindObjectsOfTypeAll<Material*>()->First([](auto x) { return x->name == StringW("UINoGlow"); });  // TODO: cache
    material = mat;
}

void CanvasHighlight::SetHighlighted(bool value) {
    highlighted = value;
    SetVerticesDirty();
}

// rounded corners might look nice
void CanvasHighlight::OnPopulateMesh(UI::VertexHelper* vh) {
    vh->Clear();

    if (!highlighted)
        return;

    auto rect = GetPixelAdjustedRect();

    auto color32 = Color32::op_Implicit___UnityEngine__Color32(color);

    vh->AddVert({rect.m_XMin, rect.m_YMin, 0}, color32, {0, 0, 0, 0});
    vh->AddVert({rect.m_XMin, rect.m_YMin + rect.m_Height, 0}, color32, {0, 1, 0, 0});
    vh->AddVert({rect.m_XMin + rect.m_Width, rect.m_YMin + rect.m_Height, 0}, color32, {1, 1, 0, 0});
    vh->AddVert({rect.m_XMin + rect.m_Width, rect.m_YMin, 0}, color32, {1, 0, 0, 0});
    vh->AddTriangle(0, 1, 2);
    vh->AddTriangle(2, 3, 0);
}

void TextOutlineSizer::OnEnable() {
    static auto base = il2cpp_utils::FindMethodUnsafe(classof(EventSystems::UIBehaviour*), "OnEnable", 0);
    il2cpp_utils::RunMethod(this, base);

    if (!text)
        text = GetComponent<TMPro::TextMeshProUGUI*>();
    if (!rectTransform)
        rectTransform = GetComponent<RectTransform*>();

    rectTransform->sizeDelta = {0, 0};
    SetDirty();
}

void TextOutlineSizer::OnDisable() {
    static auto base = il2cpp_utils::FindMethodUnsafe(classof(EventSystems::UIBehaviour*), "OnDisable", 0);
    il2cpp_utils::RunMethod(this, base);

    UI::LayoutRebuilder::MarkLayoutForRebuild(rectTransform);
}

void TextOutlineSizer::SetLayoutHorizontal() {
    SetLayout();
}

void TextOutlineSizer::SetLayoutVertical() {
    SetLayout();
}

void TextOutlineSizer::OnRectTransformDimensionsChange() {
    SetDirty();
}

void TextOutlineSizer::SetDirty() {
    if (IsActive())
        UI::LayoutRebuilder::MarkLayoutForRebuild(rectTransform);
}

Outline* TextOutlineSizer::GetOutline() {
    if (!outline) {
        if (auto obj = rectTransform->Find("QountersOutline"))
            outline = obj->GetComponent<Outline*>();
    }
    return outline;
}

void TextOutlineSizer::SetLayout() {
    if (settingLayout)
        return;
    settingLayout = true;

    BSML::MainThreadScheduler::ScheduleNextFrame([this]() {
        settingLayout = false;

        auto outline = GetOutline();
        if (!outline)
            return;

        auto bounds = text->bounds;
        if (text->text == "")
            bounds = Bounds({0, 0, 0}, {0, 0, 0});

        outline->rectTransform->anchoredPosition = {bounds.m_Center.x, bounds.m_Center.y};
        outline->SetBaseSize({bounds.m_Extents.x * 2, bounds.m_Extents.y * 2});
    });
}

void GroupOutlineSizer::OnEnable() {
    if (!rectTransform)
        rectTransform = GetComponent<RectTransform*>();
    rectTransform->sizeDelta = {0, 0};
}

Outline* GroupOutlineSizer::GetOutline() {
    if (!outline) {
        if (auto obj = rectTransform->Find("QountersOutline"))
            outline = obj->GetComponent<Outline*>();
    }
    return outline;
}

Vector4 GroupOutlineSizer::CalculateBounds() {
    auto children = GetComponentsInChildren<RectTransform*>();

    auto corners = ArrayW<Vector3>(4);
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = minX;
    float maxY = maxX;
    bool hasChild = false;

    for (auto child : children) {
        auto childOutline = child->GetComponentInChildren<Outline*>();
        if (!childOutline || childOutline == GetOutline())
            continue;

        hasChild = true;
        childOutline->rectTransform->GetWorldCorners(corners);
        for (auto& corner : corners) {
            auto point = rectTransform->InverseTransformPoint(corner);
            if (point.x < minX)
                minX = point.x;
            else if (point.x > maxX)
                maxX = point.x;
            if (point.y < minY)
                minY = point.y;
            else if (point.y > maxY)
                maxY = point.y;
        }
    }

    if (!hasChild)
        return {0, 0, 0, 0};
    return {minX, minY, maxX, maxY};
}

void GroupOutlineSizer::Update() {
    if (!GetOutline())
        return;

    auto bounds = CalculateBounds();
    auto outline = GetOutline();

    outline->rectTransform->anchoredPosition = {(bounds.x + bounds.z) / 2, (bounds.y + bounds.w) / 2};
    outline->SetBaseSize({bounds.z - bounds.x + padding * 2, bounds.w - bounds.y + padding * 2});

    outline->transform->SetAsFirstSibling();

    return;
}

void EditingBase::Awake() {
    if (!rectTransform)
        rectTransform = Utils::GetOrAddComponent<RectTransform*>(this);
    if (!outline)
        outline = Outline::Create(this);
    dragActionId = Editor::GetActionId();
    UpdateColor();
}

void EditingBase::BasePointerEnter() {
    pointer = true;
    UpdateColor();
}

void EditingBase::BasePointerExit() {
    pointer = false;
    UpdateColor();
}

void EditingBase::OnInitializePotentialDrag(EventSystems::PointerEventData* eventData) {
    eventData->useDragThreshold = false;
    Select();
    Editor::SelectEditing(this);
}

UnityEngine::Vector2 EditingBase::GetPointerPos(EventSystems::PointerEventData* eventData) {
    auto parent = rectTransform->parent;
    // fix for mask image of shape components
    if (parent->name == "BSMLImage")
        parent = parent->parent;
    auto pos = parent->InverseTransformPoint(eventData->pointerCurrentRaycast.worldPosition);
    return {pos.x, pos.y};
}

void EditingBase::Select() {
    selected = true;
    UpdateColor();
}

void EditingBase::Deselect() {
    selected = false;
    UpdateColor();
}

void EditingBase::UpdateColor() {
    if (pointer || dragging)
        outline->color = highlightColor;
    else if (selected)
        outline->color = selectColor;
    else
        outline->color = offColor;
    if (auto group = il2cpp_utils::try_cast<EditingGroup>(this).value_or(nullptr))
        group->UpdateColorChildren();
}

void EditingGroup::OnEnable() {
    if (!outlineSizer)
        outlineSizer = Utils::GetOrAddComponent<GroupOutlineSizer*>(this);
}

void EditingGroup::OnPointerEnter(EventSystems::PointerEventData* eventData) {
    BasePointerEnter();
}

void EditingGroup::OnPointerExit(EventSystems::PointerEventData* eventData) {
    BasePointerExit();
}

void EditingGroup::OnDrag(EventSystems::PointerEventData* eventData) {
    auto& group = Editor::GetSelectedGroup(dragActionId);
    if (group.Detached)
        OnDragDetached(eventData);
    else
        OnDragNormal(eventData);

    if (!dragging) {
        dragStart = Time::get_time();
        outlineSizer->enabled = false;
    }
    dragging = true;

    OptionsViewController::GetInstance()->UpdateSimpleUI();
}

void EditingGroup::OnEndDrag(EventSystems::PointerEventData* eventData) {
    auto& group = Editor::GetSelectedGroup(dragActionId);
    if (group.Detached)
        OnEndDragDetached(eventData);
    else
        OnEndDragNormal(eventData);

    Editor::FinalizeAction();
    bool tooShort = Time::get_time() - dragStart < MAX_SECS_WITHOUT_DRAG;
    if (tooShort)
        Editor::Undo();

    outlineSizer->enabled = true;
    dragging = false;
    UpdateColor();
}

void EditingGroup::OnDragNormal(EventSystems::PointerEventData* eventData) {
    auto position = GetPointerPos(eventData);
    auto& group = GetGroup();

    if (!dragging) {
        grabOffset = Vector2::op_Subtraction(group.Position, position);
        Editor::BeginDrag(group.Anchor, true);
    }

    if (!Editor::UpdateDrag(this)) {
        group.Position = Vector2::op_Addition(position, grabOffset);
        Editor::UpdatePosition();
    }
}

void EditingGroup::OnEndDragNormal(EventSystems::PointerEventData* eventData) {
    Editor::EndDrag();
}

void EditingGroup::OnDragDetached(EventSystems::PointerEventData* eventData) {
    if (!cachedInputModule)
        cachedInputModule = eventData->currentInputModule.try_cast<VRUIControls::VRInputModule>().value_or(nullptr);
    if (!cachedInputModule)
        return;
    auto position = eventData->worldPosition;
    auto& group = GetGroup();

    auto pointer = cachedInputModule->_vrPointer;
    auto controller = pointer->_lastSelectedVrController;
    if (!dragging) {
        detachedGrabPos = controller->transform->InverseTransformPoint(transform->position);
        detachedGrabRot = Quaternion::op_Multiply(Quaternion::Inverse(controller->transform->rotation), transform->rotation);
        Editor::EnableDetachedCanvas(true);
    }

    float unscaledDeltaTime = Time::get_unscaledDeltaTime();
    // thumbstick movement
    if (pointer->_lastSelectedControllerWasRight) {
        float diff = controller->thumbstick.y * unscaledDeltaTime;
        // no movement if too close
        if (detachedGrabPos.magnitude < 0.5 && diff > 0)
            diff = 0;
        else
            detachedGrabPos = Vector3::op_Subtraction(detachedGrabPos, Vector3::op_Multiply(Vector3::get_forward(), diff));
        // rotate on the third axis horizontally
        float zRot = -30 * controller->thumbstick.x * unscaledDeltaTime;
        detachedGrabRot = Quaternion::op_Multiply(detachedGrabRot, Quaternion::Euler({0, 0, zRot}));
    } else {
        float xRot = -30 * controller->thumbstick.y * unscaledDeltaTime;
        float yRot = -30 * controller->thumbstick.x * unscaledDeltaTime;
        auto extraRot = Quaternion::op_Multiply(Quaternion::Euler({0, yRot, 0}), Quaternion::Euler({xRot, 0, 0}));
        detachedGrabRot = Quaternion::op_Multiply(detachedGrabRot, extraRot);
    }

    auto pos = controller->transform->TransformPoint(detachedGrabPos);
    auto rot = Quaternion::op_Multiply(controller->transform->rotation, detachedGrabRot);

    group.DetachedPosition = Vector3::Lerp(group.DetachedPosition, pos, 10 * unscaledDeltaTime);
    group.DetachedRotation = Quaternion::Slerp(Quaternion::Euler(group.DetachedRotation), rot, 5 * unscaledDeltaTime).eulerAngles;
    Editor::UpdatePosition();
}

void EditingGroup::OnEndDragDetached(EventSystems::PointerEventData* eventData) {
    Editor::EnableDetachedCanvas(false);
}

void EditingGroup::UpdateColorChildren() {
    if (!highlightedComponents.empty()) {
        if (selected)
            outline->color = selectColor;
        else
            outline->color = offColor;
    }
}

void EditingGroup::UpdateDragAnchor(int anchor) {
    auto& group = Editor::GetSelectedGroup(dragActionId);
    group.Anchor = anchor;
    group.Position = Vector2::op_Addition(group.Position, grabOffset);
    Editor::UpdatePosition();
}

int EditingGroup::GetGroupIdx() {
    return group;
}

Group& EditingGroup::GetGroup() {
    return Editor::GetGroup(group);
}

void EditingGroup::Init(int groupIdx) {
    group = groupIdx;
}

void EditingComponent::OnPointerEnter(EventSystems::PointerEventData* eventData) {
    BasePointerEnter();
    GetEditingGroup()->highlightedComponents.emplace(this);
    GetEditingGroup()->UpdateColor();
}

void EditingComponent::OnPointerExit(EventSystems::PointerEventData* eventData) {
    BasePointerExit();
    GetEditingGroup()->highlightedComponents.erase(this);
    GetEditingGroup()->UpdateColor();
}

void EditingComponent::OnDrag(EventSystems::PointerEventData* eventData) {
    auto position = GetPointerPos(eventData);
    auto& component = Editor::GetSelectedComponent(dragActionId);

    if (!dragging) {
        grabOffset = Vector2::op_Subtraction(component.Position, position);
        if (GetGroup().Detached)
            Editor::EnableDetachedCanvas(true);
        else
            Editor::BeginDrag(GetGroup().Anchor, false);
        dragStart = Time::get_time();
    }
    dragging = true;

    component.Position = Vector2::op_Addition(position, grabOffset);
    Editor::UpdatePosition();
    OptionsViewController::GetInstance()->UpdateSimpleUI();
}

void EditingComponent::OnEndDrag(EventSystems::PointerEventData* eventData) {
    dragging = false;
    if (GetGroup().Detached)
        Editor::EnableDetachedCanvas(false);
    else
        Editor::EndDrag();
    UpdateColor();

    Editor::FinalizeAction();
    bool tooShort = Time::get_time() - dragStart < MAX_SECS_WITHOUT_DRAG;
    if (tooShort)
        Editor::Undo();
}

EditingGroup* EditingComponent::GetEditingGroup() {
    if (!group)
        group = GetComponentsInParent<EditingGroup*>(true)->First();
    return group;
}

int EditingComponent::GetComponentIdx() {
    return component;
}

Group& EditingComponent::GetGroup() {
    return Editor::GetGroup(GetEditingGroup()->group);
}

Qounters::Component& EditingComponent::GetComponent() {
    return GetGroup().Components[component];
}

void EditingComponent::Init(UnityEngine::UI::Graphic* comp, int componentIdx) {
    typeComponent = comp;
    component = componentIdx;
}
