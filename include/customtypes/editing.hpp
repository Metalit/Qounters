#pragma once

#include "UnityEngine/EventSystems/IDragHandler.hpp"
#include "UnityEngine/EventSystems/IEndDragHandler.hpp"
#include "UnityEngine/EventSystems/IEventSystemHandler.hpp"
#include "UnityEngine/EventSystems/IInitializePotentialDragHandler.hpp"
#include "UnityEngine/EventSystems/IPointerEnterHandler.hpp"
#include "UnityEngine/EventSystems/IPointerExitHandler.hpp"
#include "UnityEngine/EventSystems/UIBehaviour.hpp"
#include "UnityEngine/UI/Graphic.hpp"
#include "UnityEngine/UI/ILayoutController.hpp"
#include "UnityEngine/UI/VertexHelper.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "config.hpp"
#include "custom-types/shared/macros.hpp"
#include "main.hpp"

#define UUI UnityEngine::UI
#define UES UnityEngine::EventSystems

#define CAST_METHOD(c, m, ...) static_cast<void (c::*)(__VA_ARGS__)>(&c::m)

DECLARE_CLASS_CODEGEN(Qounters, Outline, UUI::Graphic) {
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, SetBaseSize, UnityEngine::Vector2 value);
    DECLARE_INSTANCE_METHOD(void, SetBorderWidth, float value);
    DECLARE_INSTANCE_METHOD(void, SetBorderGap, float value);
    DECLARE_INSTANCE_METHOD(void, UpdateSize);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPopulateMesh, CAST_METHOD(UUI::Graphic, OnPopulateMesh, UUI::VertexHelper*), UUI::VertexHelper* vh);

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, awaitingPopulate, false);

    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::Vector2, baseSize, UnityEngine::Vector2(0, 0));
    DECLARE_INSTANCE_FIELD_DEFAULT(float, borderWidth, 1);
    DECLARE_INSTANCE_FIELD_DEFAULT(float, borderGap, 1);

   public:
    static UnityEngine::Material* material;
    static int count;
    static Outline* Create(UnityEngine::Component * obj);
};

DECLARE_CLASS_CODEGEN(Qounters, CanvasHighlight, UUI::Graphic) {
    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, SetHighlighted, bool value);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPopulateMesh, CAST_METHOD(UUI::Graphic, OnPopulateMesh, UUI::VertexHelper*), UUI::VertexHelper* vh);

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, highlighted, false);
};

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, TextOutlineSizer, UES::UIBehaviour, UUI::ILayoutController*) {
    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);
    DECLARE_OVERRIDE_METHOD_MATCH(void, SetLayoutHorizontal, &UUI::ILayoutController::SetLayoutHorizontal);
    DECLARE_OVERRIDE_METHOD_MATCH(void, SetLayoutVertical, &UUI::ILayoutController::SetLayoutVertical);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnRectTransformDimensionsChange, &UES::UIBehaviour::OnRectTransformDimensionsChange);
    DECLARE_INSTANCE_METHOD(void, SetDirty);
    DECLARE_INSTANCE_METHOD(void, SetLayout);

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(Qounters::Outline*, GetOutline);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, settingLayout, false);

    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::RectTransform*, rectTransform, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(TMPro::TextMeshProUGUI*, text, nullptr);

   private:
    Outline* outline = nullptr;
};

DECLARE_CLASS_CODEGEN(Qounters, GroupOutlineSizer, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, Update);

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(UnityEngine::Vector4, CalculateBounds);
    DECLARE_INSTANCE_METHOD(Qounters::Outline*, GetOutline);

    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::RectTransform*, rectTransform, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(float, padding, 5);

   private:
    Outline* outline = nullptr;
};

// IEventSystemHandler is needed for pointer handlers
// and IInitializePotentialDragHandler for dragging

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, EditingBase, UnityEngine::MonoBehaviour, UES::IEventSystemHandler*, UES::IInitializePotentialDragHandler*) {
    DECLARE_INSTANCE_METHOD(void, Awake);

    DECLARE_INSTANCE_METHOD(void, BasePointerEnter);
    DECLARE_INSTANCE_METHOD(void, BasePointerExit);

    DECLARE_OVERRIDE_METHOD_MATCH(
        void, OnInitializePotentialDrag, &UES::IInitializePotentialDragHandler::OnInitializePotentialDrag, UES::PointerEventData* eventData
    );

    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::RectTransform*, rectTransform, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(Qounters::Outline*, outline, nullptr);

    DECLARE_INSTANCE_METHOD(UnityEngine::Vector2, GetPointerPos, UES::PointerEventData * eventData);

    DECLARE_INSTANCE_METHOD(void, Select);
    DECLARE_INSTANCE_METHOD(void, Deselect);
    DECLARE_INSTANCE_METHOD(void, UpdateColor);

   protected:
    bool dragging = false;
    bool pointer = false;
    bool selected = false;
    UnityEngine::Vector2 grabOffset = {0, 0};
    float dragStart = 0;
    int dragActionId;
};

DECLARE_CLASS_CUSTOM_INTERFACES(Qounters, EditingGroup, EditingBase, UES::IPointerEnterHandler*, UES::IPointerExitHandler*, UES::IDragHandler*, UES::IEndDragHandler*) {
    DECLARE_INSTANCE_METHOD(void, OnEnable);

    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerEnter, &UES::IPointerEnterHandler::OnPointerEnter, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerExit, &UES::IPointerExitHandler::OnPointerExit, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnDrag, &UES::IDragHandler::OnDrag, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnEndDrag, &UES::IEndDragHandler::OnEndDrag, UES::PointerEventData* eventData);

    DECLARE_INSTANCE_METHOD(void, UpdateColorChildren);
    DECLARE_INSTANCE_METHOD(void, UpdateDragAnchor, int anchor);

    DECLARE_INSTANCE_FIELD_DEFAULT(Qounters::GroupOutlineSizer*, outlineSizer, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(VRUIControls::VRInputModule*, cachedInputModule, nullptr);

    DECLARE_INSTANCE_METHOD(int, GetGroupIdx);

   private:
    int group = -1;
    std::set<class EditingComponent*> highlightedComponents = {};

    friend class EditingComponent;

    void OnDragNormal(UES::PointerEventData * eventData);
    void OnEndDragNormal(UES::PointerEventData * eventData);
    void OnDragDetached(UES::PointerEventData * eventData);
    void OnEndDragDetached(UES::PointerEventData * eventData);
    Options::Group& GetGroup();

    UnityEngine::Vector3 detachedGrabPos;
    UnityEngine::Quaternion detachedGrabRot;

   public:
    void Init(int groupIdx);
};

DECLARE_CLASS_CUSTOM_INTERFACES(Qounters, EditingComponent, EditingBase, UES::IPointerEnterHandler*, UES::IPointerExitHandler*, UES::IDragHandler*, UES::IEndDragHandler*) {

    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerEnter, &UES::IPointerEnterHandler::OnPointerEnter, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerExit, &UES::IPointerExitHandler::OnPointerExit, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnDrag, &UES::IDragHandler::OnDrag, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnEndDrag, &UES::IEndDragHandler::OnEndDrag, UES::PointerEventData* eventData);

    DECLARE_INSTANCE_METHOD(Qounters::EditingGroup*, GetEditingGroup);

    DECLARE_INSTANCE_FIELD_DEFAULT(Qounters::EditingGroup*, group, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(UnityEngine::UI::Graphic*, typeComponent, nullptr);

    DECLARE_INSTANCE_METHOD(int, GetComponentIdx);

   private:
    int component = -1;

    Options::Group& GetGroup();
    Options::Component& GetComponent();

   public:
    void Init(UUI::Graphic * typeComponent, int componentIdx);
};

#undef UUI
#undef UES
#undef CAST_METHOD
