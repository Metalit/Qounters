#include "customtypes/editing.hpp"
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

const Color offColor = {0.7, 0.7, 0.7, 1};
const Color highlightColor = {0, 0.7, 0.8, 1};
const Color selectColor = {0.2, 0.5, 0.6, 1};

Material* Outline::material = nullptr;
int Outline::count = 0;

void Outline::Awake() {
    count++;
    if (!material)
        material = Resources::FindObjectsOfTypeAll<Material*>().First([](auto x) { return x->get_name() == "UINoGlow"; });
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

    auto rectTransform = get_rectTransform();
    rectTransform->set_anchorMin({0, 0});
    rectTransform->set_anchorMax({1, 1});
    UpdateSizeDelta();
}

void Outline::SetBaseSize(Vector2 value) {
    baseSize = value;
    UpdateSizeDelta();
}

void Outline::SetBorderWidth(float value) {
    borderWidth = value;
    UpdateSizeDelta();
}

void Outline::SetBorderGap(float value) {
    borderGap = value;
    UpdateSizeDelta();
}

void Outline::UpdateSizeDelta() {
    auto scale = get_transform()->get_lossyScale();
    auto pad = 2 * (borderGap + borderWidth);
    get_rectTransform()->set_sizeDelta({baseSize.x + (pad * 0.02f / scale.x), baseSize.y + (pad * 0.02f / scale.y)});
}

#include "UnityEngine/Rect.hpp"

void Outline::OnPopulateMesh(UI::VertexHelper *vh) {
    if (borderWidth <= 0)
        return;

    auto scale = get_transform()->get_lossyScale();

    float borderX = borderWidth * 0.02 / scale.x;
    float borderY = borderWidth * 0.02 / scale.y;

    auto rect = GetPixelAdjustedRect();
    float width = rect.get_width();
    float height = rect.get_height();
    float xMin = rect.get_xMin();
    float yMin = rect.get_yMin();
    float xMax = rect.get_xMax();
    float yMax = rect.get_yMax();

    float perimeter = width * 2 + height * 2;

    auto color = get_color();
    auto color32 = Color32(color.r * 255, color.g * 255, color.b * 255, color.a * 255);

    vh->Clear();

    float uv = height / perimeter;
    vh->AddVert({xMin, yMin, 0}, color32, {0, 0});
    vh->AddVert({xMin, yMax, 0}, color32, {uv, 0});
    vh->AddVert({xMin + borderX, yMax - borderY, 0}, color32, {uv, 1});
    vh->AddVert({xMin + borderX, yMin + borderY, 0}, color32, {0, 1});
    vh->AddTriangle(0, 1, 2);
    vh->AddTriangle(2, 3, 0);

    uv = (height + width) / perimeter;
    vh->AddVert({xMax, yMax, 0}, color32, {uv, 0});
    vh->AddVert({xMax - borderX, yMax - borderY, 0}, color32, {uv, 1});
    vh->AddTriangle(1, 4, 5);
    vh->AddTriangle(5, 2, 1);

    uv = (height + width + height) / perimeter;
    vh->AddVert({xMax, yMin, 0}, color32, {uv, 0});
    vh->AddVert({xMax - borderX, yMin + borderY, 0}, color32, {uv, 1});
    vh->AddTriangle(4, 6, 7);
    vh->AddTriangle(7, 5, 4);
    vh->AddTriangle(6, 0, 3);
    vh->AddTriangle(3, 7, 6);
}

void Outline::OnRectTransformDimensionsChange() {
    UpdateSizeDelta(); // TODO: doesn't seem to update consistently with scale changes
    SetVerticesDirty();
}

Outline* Outline::Create(Component* obj) {
    auto child = GameObject::New_ctor("QountersOutline");
    child->get_transform()->SetParent(obj->get_transform(), false);
    return child->AddComponent<Outline*>();
}


void CanvasHighlight::OnEnable() {
    static auto base = il2cpp_utils::FindMethodUnsafe(classof(UI::Graphic*), "OnEnable", 0);
    il2cpp_utils::RunMethod(this, base);

    auto mat = Resources::FindObjectsOfTypeAll<Material*>().First([](auto x) { return x->get_name() == "UINoGlow"; }); // TODO: cache
    set_material(mat);

    auto rectTransform = Utils::GetOrAddComponent<RectTransform*>(this);
    rectTransform->set_anchorMin({0, 0});
    rectTransform->set_anchorMax({1, 1});
}

void CanvasHighlight::SetHighlighted(bool value) {
    highlighted = value;
    SetVerticesDirty();
}

// rounded corners might look nice
void CanvasHighlight::OnPopulateMesh(UI::VertexHelper *vh) {
    vh->Clear();

    if (!highlighted)
        return;

    auto rect = GetPixelAdjustedRect();

    auto color = get_color();
    auto color32 = Color32(color.r * 255, color.g * 255, color.b * 255, color.a * 255);

	vh->AddVert({rect.m_XMin, rect.m_YMin, 0}, color32, {0, 0});
	vh->AddVert({rect.m_XMin, rect.m_YMin + rect.m_Height, 0}, color32, {0, 1});
	vh->AddVert({rect.m_XMin + rect.m_Width, rect.m_YMin + rect.m_Height, 0}, color32, {1, 1});
	vh->AddVert({rect.m_XMin + rect.m_Width, rect.m_YMin, 0}, color32, {1, 0});
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

    rectTransform->set_sizeDelta({0, 0});
    SetDirty();
}

#include "UnityEngine/UI/LayoutRebuilder.hpp"

void TextOutlineSizer::OnDisable() {
    static auto base = il2cpp_utils::FindMethodUnsafe(classof(EventSystems::UIBehaviour*), "OnDisable", 0);
    il2cpp_utils::RunMethod(this, base);

    UI::LayoutRebuilder::MarkLayoutForRebuild(rectTransform);
}

void TextOutlineSizer::SetLayoutHorizontal() {
    if (settingLayout)
        return;
    settingLayout = true;
    StartCoroutine(custom_types::Helpers::CoroutineHelper::New(SetLayout()));
}

void TextOutlineSizer::SetLayoutVertical() {
    if (settingLayout)
        return;
    settingLayout = true;
    StartCoroutine(custom_types::Helpers::CoroutineHelper::New(SetLayout()));
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

#include "UnityEngine/DrivenTransformProperties.hpp"
#include "UnityEngine/Bounds.hpp"

custom_types::Helpers::Coroutine TextOutlineSizer::SetLayout() {
    co_yield nullptr;
    settingLayout = false;

    if (!GetOutline())
        co_return;

    auto outline = GetOutline();

    auto bounds = text->get_bounds();

    if (text->get_text() == "")
        bounds = Bounds({}, {});

    outline->get_rectTransform()->set_anchoredPosition({bounds.m_Center.x, bounds.m_Center.y});
    outline->SetBaseSize({bounds.m_Extents.x * 2, bounds.m_Extents.y * 2});

    co_return;
}


void GroupOutlineSizer::OnEnable() {
    if (!rectTransform)
        rectTransform = GetComponent<RectTransform*>();
    rectTransform->set_sizeDelta({0, 0});
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
        childOutline->get_rectTransform()->GetWorldCorners(corners);
        for (auto& corner : corners) {
            auto point = rectTransform->InverseTransformPoint(corner);
            if (point.x < minX) minX = point.x;
            else if (point.x > maxX) maxX = point.x;
            if (point.y < minY) minY = point.y;
            else if (point.y > maxY) maxY = point.y;
        }
    }

    if (!hasChild)
        return {};
    return {minX, minY, maxX, maxY};
}

void GroupOutlineSizer::Update() {
    if (!GetOutline())
        return;

    auto bounds = CalculateBounds();
    auto outline = GetOutline();

    outline->get_rectTransform()->set_anchoredPosition({(bounds.x + bounds.z) / 2, (bounds.y + bounds.w) / 2});
    outline->SetBaseSize({bounds.z - bounds.x + padding * 2, bounds.w - bounds.y + padding * 2});

    outline->get_transform()->SetAsFirstSibling();

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
    auto pos = rectTransform->get_parent()->InverseTransformPoint({eventData->position.x, eventData->position.y, 0});
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
        outline->set_color(highlightColor);
    else if (selected)
        outline->set_color(selectColor);
    else
        outline->set_color(offColor);
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

#include "UnityEngine/Time.hpp"

void EditingGroup::OnDrag(EventSystems::PointerEventData* eventData) {
    auto position = GetPointerPos(eventData);
    auto& group = Editor::GetSelectedGroup(dragActionId);

    if (!dragging) {
        grabOffset = group.Position - position;
        Editor::BeginDrag(group.Anchor, true);
        dragStart = Time::get_time();

        outlineSizer->set_enabled(false);
    }
    dragging = true;

    if (!Editor::UpdateDrag(this)) {
        group.Position = position + grabOffset;
        Editor::UpdatePosition();
    }
    OptionsViewController::GetInstance()->UpdateSimpleUI();
}

void EditingGroup::OnEndDrag(EventSystems::PointerEventData* eventData) {
    dragging = false;
    Editor::EndDrag();
    UpdateColor();

    bool tooShort = Time::get_time() - dragStart < MAX_SECS_WITHOUT_DRAG;
    if (tooShort)
        Editor::Undo();

    outlineSizer->set_enabled(true);
}

void EditingGroup::UpdateColorChildren() {
    if (!highlightedComponents.empty()) {
        if (selected)
            outline->set_color(selectColor);
        else
            outline->set_color(offColor);
    }
}

void EditingGroup::UpdateDragAnchor(int anchor) {
    auto& group = Editor::GetSelectedGroup(dragActionId);
    group.Anchor = anchor;
    group.Position = group.Position + grabOffset;
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
        grabOffset = component.Position - position;
        Editor::BeginDrag(GetGroup().Anchor, false);
        dragStart = Time::get_time();
    }
    dragging = true;

    component.Position = position + grabOffset;
    Editor::UpdatePosition();
    OptionsViewController::GetInstance()->UpdateSimpleUI();
}

void EditingComponent::OnEndDrag(EventSystems::PointerEventData* eventData) {
    dragging = false;
    Editor::EndDrag();
    UpdateColor();

    bool tooShort = Time::get_time() - dragStart < MAX_SECS_WITHOUT_DRAG;
    if (tooShort)
        Editor::Undo();
}

EditingGroup* EditingComponent::GetEditingGroup() {
    if (!group)
        group = GetComponentInParent<EditingGroup*>();
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
