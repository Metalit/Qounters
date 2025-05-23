#include "hooks.hpp"

#include "GlobalNamespace/CoreGameHUDController.hpp"
#include "GlobalNamespace/EnvironmentOverrideSettingsPanelController.hpp"
#include "GlobalNamespace/FlyingGameHUDRotation.hpp"
#include "GlobalNamespace/LayerMasks.hpp"
#include "GlobalNamespace/MultiplayerIntroAnimationController.hpp"
#include "GlobalNamespace/MultiplayerLocalActivePlayerGameplayManager.hpp"
#include "GlobalNamespace/MultiplayerLocalActivePlayerInGameMenuViewController.hpp"
#include "GlobalNamespace/MultiplayerSessionManager.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/UIKeyboardManager.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/CurvedCanvasSettingsHelper.hpp"
#include "UnityEngine/Sprites/DataUtility.hpp"
#include "UnityEngine/Time.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"
#include "config.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "gameplay.hpp"
#include "main.hpp"
#include "metacore/shared/events.hpp"
#include "metacore/shared/internals.hpp"
#include "pp.hpp"
#include "qounters.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace GlobalNamespace;
using namespace VRUIControls;

static UnityEngine::Transform* rotationalAnchor = nullptr;
static bool hidden = false;
static bool initialized = false;

static void TryInitialize() {
    if (!getConfig().Enabled.GetValue() || Environment::InSettings() || hidden || initialized)
        return;
    if (!MetaCore::Internals::stateValid)
        MetaCore::Events::AddCallback(MetaCore::Events::GameplaySceneStarted, TryInitialize, true);
    else {
        logger.info("Qounters start");
        HUD::SetupObjects();
        HUD::CreateQounters();
        initialized = true;
    }
}

MAKE_AUTO_HOOK_MATCH(
    CoreGameHUDController_Initialize, &CoreGameHUDController::Initialize, void, CoreGameHUDController* self, CoreGameHUDController::InitData* initData
) {
    if (!getConfig().Enabled.GetValue()) {
        CoreGameHUDController_Initialize(self, initData);
        return;
    }

    initData->advancedHUD = true;
    hidden = initData->hide;
    TryInitialize();

    CoreGameHUDController_Initialize(self, initData);
}

MAKE_AUTO_HOOK_MATCH(
    MultiplayerIntroAnimationController_BindTimeline,
    &MultiplayerIntroAnimationController::BindTimeline,
    void,
    MultiplayerIntroAnimationController* self
) {
    MultiplayerIntroAnimationController_BindTimeline(self);

    TryInitialize();
}

MAKE_AUTO_HOOK_MATCH(FlyingGameHUDRotation_LateUpdate, &FlyingGameHUDRotation::LateUpdate, void, FlyingGameHUDRotation* self) {
    FlyingGameHUDRotation_LateUpdate(self);

    if (rotationalAnchor)
        rotationalAnchor->rotation = self->transform->rotation;
}

MAKE_AUTO_HOOK_MATCH(
    EnvironmentOverrideSettingsPanelController_HandleDropDownDidSelectCellWithIdx,
    &EnvironmentOverrideSettingsPanelController::HandleDropDownDidSelectCellWithIdx,
    void,
    EnvironmentOverrideSettingsPanelController* self,
    HMUI::DropdownWithTableView* dropdown,
    int idx
) {
    EnvironmentOverrideSettingsPanelController_HandleDropDownDidSelectCellWithIdx(self, dropdown, idx);

    Gameplay::UpdateEnvironment();
}

MAKE_AUTO_HOOK_MATCH(
    EnvironmentOverrideSettingsPanelController_HandleOverrideEnvironmentsToggleValueChanged,
    &EnvironmentOverrideSettingsPanelController::HandleOverrideEnvironmentsToggleValueChanged,
    void,
    EnvironmentOverrideSettingsPanelController* self,
    bool isOn
) {
    EnvironmentOverrideSettingsPanelController_HandleOverrideEnvironmentsToggleValueChanged(self, isOn);

    Gameplay::UpdateEnvironment();
}

MAKE_AUTO_HOOK_MATCH(PauseController_Pause, &PauseController::Pause, void, PauseController* self) {
    if (!Environment::InSettings())
        PauseController_Pause(self);
}

MAKE_AUTO_HOOK_MATCH(
    MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu,
    &MultiplayerLocalActivePlayerInGameMenuViewController::ShowMenu,
    void,
    MultiplayerLocalActivePlayerInGameMenuViewController* self
) {
    if (!Environment::InSettings())
        MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu(self);
}

MAKE_AUTO_HOOK_MATCH(
    MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu,
    &MultiplayerLocalActivePlayerInGameMenuViewController::HideMenu,
    void,
    MultiplayerLocalActivePlayerInGameMenuViewController* self
) {
    if (!Environment::InSettings())
        MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu(self);
}

MAKE_AUTO_HOOK_MATCH(
    MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail,
    &MultiplayerLocalActivePlayerGameplayManager::PerformPlayerFail,
    void,
    MultiplayerLocalActivePlayerGameplayManager* self
) {
    if (!Environment::InSettings())
        MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail(self);
}

MAKE_AUTO_HOOK_MATCH(
    MultiplayerSessionManager_get_localPlayer, &MultiplayerSessionManager::get_localPlayer, IConnectedPlayer*, MultiplayerSessionManager* self
) {
    auto ret = MultiplayerSessionManager_get_localPlayer(self);

    if (!ret && localFakeConnectedPlayer)
        return localFakeConnectedPlayer;
    return ret;
}

MAKE_AUTO_HOOK_MATCH(
    VRGraphicRaycaster_Raycast,
    &VRGraphicRaycaster::Raycast,
    void,
    VRGraphicRaycaster* self,
    UnityEngine::EventSystems::PointerEventData* eventData,
    System::Collections::Generic::List_1<UnityEngine::EventSystems::RaycastResult>* resultAppendList
) {
    if (blockOtherRaycasts && !raycastCanvases.contains(self->_canvas))
        return;
    auto mask = self->_blockingMask;
    if (Environment::InSettings() && Editor::GetPreviewMode()) {
        if (self->_canvas->name == "QounterGroup")
            return;
        self->_blockingMask = mask.m_Mask & ~LayerMasks::getStaticF_saberLayerMask().m_Mask;
    }

    VRGraphicRaycaster_Raycast(self, eventData, resultAppendList);
    self->_blockingMask = mask;
}

MAKE_AUTO_HOOK_MATCH(
    UIKeyboardManager_OpenKeyboardFor, &UIKeyboardManager::OpenKeyboardFor, void, UIKeyboardManager* self, HMUI::InputFieldView* input
) {
    if (auto inputModal = input->GetComponentInParent<HMUI::ModalView*>()) {
        auto inputModalCanvas = inputModal->GetComponent<UnityEngine::Canvas*>();
        auto keyboardModalCanvas = self->_keyboardModalView->GetComponent<UnityEngine::Canvas*>();
        keyboardModalCanvas->sortingOrder = inputModalCanvas->sortingOrder;
    }

    UIKeyboardManager_OpenKeyboardFor(self, input);
}

// I hate having to do this
void PopulateMeshHSVGradient(HMUI::ImageView* self, UnityEngine::UI::VertexHelper* toFill, int modIdx, float modifier, int elementTarget) {
    using namespace UnityEngine;
    auto s_VertScratch = HMUI::ImageView::getStaticF_s_VertScratch();
    auto s_UVScratch = HMUI::ImageView::getStaticF_s_UVScratch();
    auto s_UV1Scratch = HMUI::ImageView::getStaticF_s_UV1Scratch();
    std::array<float, 4> hsvModScratch;
    auto curvedCanvasSettings = self->_curvedCanvasSettingsHelper->GetCurvedCanvasSettings(self->canvas);
    float curvedUIRadius = curvedCanvasSettings ? curvedCanvasSettings->radius : 0;
    auto vector = Sprites::DataUtility::GetOuterUV(self->overrideSprite);
    auto vector2 = Sprites::DataUtility::GetInnerUV(self->overrideSprite);
    auto vector3 = Sprites::DataUtility::GetPadding(self->overrideSprite);
    auto vector4 = self->overrideSprite->border;
    Rect pixelAdjustedRect = self->GetPixelAdjustedRect();
    vector4 = HMUI::ImageView::GetAdjustedBorders(Vector4::op_Division(vector4, self->pixelsPerUnit), pixelAdjustedRect);
    vector3 = Vector4::op_Division(vector3, self->pixelsPerUnit);
    s_VertScratch[0] = {vector3.x, vector3.y};
    s_VertScratch[3] = {pixelAdjustedRect.width - vector3.z, pixelAdjustedRect.height - vector3.w};
    s_VertScratch[1].x = vector4.x;
    s_VertScratch[1].y = vector4.y;
    s_VertScratch[2].x = pixelAdjustedRect.width - vector4.z;
    s_VertScratch[2].y = pixelAdjustedRect.height - vector4.w;
    Vector2 scale = {1 / pixelAdjustedRect.width, 1 / pixelAdjustedRect.height};
    for (int i = 0; i < 4; i++) {
        s_UV1Scratch[i] = s_VertScratch[i];
        s_UV1Scratch[i].Scale(scale);
    }
    for (int j = 0; j < 4; j++) {
        s_VertScratch[j].x += pixelAdjustedRect.x;
        s_VertScratch[j].y += pixelAdjustedRect.y;
    }
    s_UVScratch[0] = {vector.x, vector.y};
    s_UVScratch[1] = {vector2.x, vector2.y};
    s_UVScratch[2] = {vector2.z, vector2.w};
    s_UVScratch[3] = {vector.z, vector.w};
    float skewFactor = self->_skew * (s_VertScratch[3].y - s_VertScratch[0].y);
    float y = self->rectTransform->pivot.y;
    float h, s, v;
    Color::RGBToHSV(self->color, byref(h), byref(s), byref(v));
    float& modified = modIdx == 0 ? h : (modIdx == 1 ? s : v);
    float originalModified = modified;
    float x = s_VertScratch[0].x;
    float num = s_VertScratch[3].x - s_VertScratch[0].x;
    hsvModScratch[0] = -modifier;
    hsvModScratch[1] = std::lerp(-modifier, modifier, (s_VertScratch[1].x - x) / num);
    hsvModScratch[2] = std::lerp(-modifier, modifier, (s_VertScratch[2].x - x) / num);
    hsvModScratch[3] = modifier;
    float totalWidth = 0;
    for (int k = 0; k < 3; k++)
        totalWidth += std::abs(s_VertScratch[k].x - s_VertScratch[k + 1].x);
    toFill->Clear();
    for (int k = 0; k < 3; k++) {
        for (int l = 0; l < 3; l++) {
            if (self->fillCenter || k != 1 || l != 1) {
                // divide target elements by relative width and add 1 to make sure it's above 0
                int numberOfElements = 1 + elementTarget * (std::abs(s_VertScratch[k].x - s_VertScratch[k + 1].x) / totalWidth);
                int currentVertCount = toFill->currentVertCount;
                Vector4 vector = {curvedUIRadius, 0, 0, 0};
                for (int i = 0; i < numberOfElements + 1; i++) {
                    float t = i / (float) numberOfElements;
                    float hsvMod = std::lerp(hsvModScratch[k], hsvModScratch[k + 1], t);
                    modified = originalModified + hsvMod;
                    Color color = Utils::GetClampedColor({h, s, v});
                    float num = std::lerp(s_VertScratch[k].x, s_VertScratch[k + 1].x, t);
                    float x = std::lerp(s_UVScratch[k].x, s_UVScratch[k + 1].x, t);
                    float x2 = std::lerp(s_UV1Scratch[k].x, s_UV1Scratch[k + 1].x, t);
                    toFill->AddVert(
                        {num + skewFactor * (s_UV1Scratch[l].y - y), s_VertScratch[l].y, 0},
                        Color32::op_Implicit___UnityEngine__Color32(color),
                        {x, s_UVScratch[l].y, 0, 0},
                        {x2, s_UV1Scratch[l].y, 0, 0},
                        vector,
                        {0, 0, 0, 0},
                        {0, 0, 0},
                        {0, 0, 0, 0}
                    );
                    toFill->AddVert(
                        {num + skewFactor * (s_UV1Scratch[l + 1].y - y), s_VertScratch[l + 1].y, 0},
                        Color32::op_Implicit___UnityEngine__Color32(color),
                        {x, s_UVScratch[l + 1].y, 0, 0},
                        {x2, s_UV1Scratch[l + 1].y, 0, 0},
                        vector,
                        {0, 0, 0, 0},
                        {0, 0, 0},
                        {0, 0, 0, 0}
                    );
                }
                for (int j = 0; j < numberOfElements; j++) {
                    int num2 = j * 2 + currentVertCount;
                    toFill->AddTriangle(num2, 1 + num2, 2 + num2);
                    toFill->AddTriangle(2 + num2, 3 + num2, 1 + num2);
                }
            }
        }
    }
}

MAKE_AUTO_HOOK_MATCH(ImageView_OnPopulateMesh, &HMUI::ImageView::OnPopulateMesh, void, HMUI::ImageView* self, UnityEngine::UI::VertexHelper* toFill) {
    if (auto hsv = self->GetComponent<HSVGradientImage*>())
        PopulateMeshHSVGradient(self, toFill, hsv->modified, hsv->modifier, hsv->elements);
    else
        ImageView_OnPopulateMesh(self, toFill);
}
