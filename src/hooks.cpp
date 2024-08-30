#include "hooks.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapObjectExecutionRatingsRecorder.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/CoreGameHUDController.hpp"
#include "GlobalNamespace/CutScoreBuffer.hpp"
#include "GlobalNamespace/FlyingGameHUDRotation.hpp"
#include "GlobalNamespace/GameEnergyCounter.hpp"
#include "GlobalNamespace/LayerMasks.hpp"
#include "GlobalNamespace/MultiplayerIntroAnimationController.hpp"
#include "GlobalNamespace/MultiplayerLocalActivePlayerGameplayManager.hpp"
#include "GlobalNamespace/MultiplayerLocalActivePlayerInGameMenuViewController.hpp"
#include "GlobalNamespace/MultiplayerSessionManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/ScoreModel.hpp"
#include "GlobalNamespace/ScoreMultiplierCounter.hpp"
#include "GlobalNamespace/ScoringElement.hpp"
#include "GlobalNamespace/StandardLevelDetailView.hpp"
#include "GlobalNamespace/UIKeyboardManager.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/CurvedCanvasSettingsHelper.hpp"
#include "UnityEngine/Sprites/DataUtility.hpp"
#include "UnityEngine/Time.hpp"
#include "VRUIControls/VRGraphicRaycaster.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "config.hpp"
#include "customtypes/components.hpp"
#include "customtypes/settings.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "events.hpp"
#include "game.hpp"
#include "internals.hpp"
#include "main.hpp"
#include "pp.hpp"
#include "qounters.hpp"
#include "utils.hpp"

using namespace GlobalNamespace;
using namespace VRUIControls;
using namespace Qounters;

UnityEngine::Transform* rotationalAnchor = nullptr;
static bool wasHidden = false;
static bool initialized = false;

MAKE_HOOK_MATCH(
    ScoreController_DespawnScoringElement, &ScoreController::DespawnScoringElement, void, ScoreController* self, ScoringElement* scoringElement
) {
    ScoreController_DespawnScoringElement(self, scoringElement);

    int cutScore = scoringElement->cutScore * scoringElement->multiplier;
    int maxCutScore = scoringElement->maxPossibleCutScore * scoringElement->maxMultiplier;

    bool badCut = scoringElement->multiplierEventType == ScoreMultiplierCounter::MultiplierEventType::Negative &&
                  scoringElement->wouldBeCorrectCutBestPossibleMultiplierEventType == ScoreMultiplierCounter::MultiplierEventType::Positive &&
                  cutScore == 0 && maxCutScore > 0;

    // NoteScoreDefinition fixedCutScore, for now only this case
    bool isGoodScoreFixed = scoringElement->noteData->gameplayType == NoteData::GameplayType::BurstSliderElement;

    if (scoringElement->noteData->colorType == ColorType::ColorA) {
        leftScore += cutScore;
        leftMaxScore += maxCutScore;
        if (badCut) {
            if (isGoodScoreFixed)
                leftMissedFixedScore += maxCutScore;
            else
                leftMissedMaxScore += maxCutScore;
        } else
            leftMissedFixedScore += (scoringElement->cutScore * scoringElement->maxMultiplier) - cutScore;
    } else {
        rightScore += cutScore;
        rightMaxScore += maxCutScore;
        if (badCut) {
            if (isGoodScoreFixed)
                rightMissedFixedScore += maxCutScore;
            else
                rightMissedMaxScore += maxCutScore;
        } else
            rightMissedFixedScore += (scoringElement->cutScore * scoringElement->maxMultiplier) - cutScore;
    }
    BroadcastEvent((int) Events::ScoreChanged);
}

MAKE_HOOK_MATCH(
    BeatmapObjectManager_HandleNoteControllerNoteWasCut,
    &BeatmapObjectManager::HandleNoteControllerNoteWasCut,
    void,
    BeatmapObjectManager* self,
    NoteController* noteController,
    ByRef<NoteCutInfo> info
) {
    BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, info);

    bool left = info->saberType == SaberType::SaberA;
    bool bomb = noteController->noteData->gameplayType == NoteData::GameplayType::Bomb;
    if (info->allIsOK) {
        combo++;
        if (left)
            leftCombo++;
        else
            rightCombo++;
    } else {
        combo = 0;
        if (left) {
            if (bomb)
                bombsLeftHit++;
            else
                notesLeftBadCut++;
            leftCombo = 0;
        } else {
            if (bomb)
                bombsRightHit++;
            else
                notesRightBadCut++;
            rightCombo = 0;
        }
        if (bomb)
            BroadcastEvent((int) Events::BombCut);
        else
            BroadcastEvent((int) Events::NoteCut);
    }
    BroadcastEvent((int) Events::ComboChanged);
}

MAKE_HOOK_MATCH(
    BeatmapObjectManager_HandleNoteControllerNoteWasMissed,
    &BeatmapObjectManager::HandleNoteControllerNoteWasMissed,
    void,
    BeatmapObjectManager* self,
    NoteController* noteController
) {
    BeatmapObjectManager_HandleNoteControllerNoteWasMissed(self, noteController);

    if (noteController->noteData->gameplayType == NoteData::GameplayType::Bomb)
        return;

    combo = 0;
    if (noteController->noteData->colorType == ColorType::ColorA) {
        leftCombo = 0;
        notesLeftMissed++;
    } else {
        rightCombo = 0;
        notesRightMissed++;
    }
    BroadcastEvent((int) Events::NoteMissed);
    BroadcastEvent((int) Events::ComboChanged);
}

MAKE_HOOK_MATCH(
    CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish,
    &CutScoreBuffer::HandleSaberSwingRatingCounterDidFinish,
    void,
    CutScoreBuffer* self,
    ISaberSwingRatingCounter* swingRatingCounter
) {
    CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish(self, swingRatingCounter);

    if (self->noteCutInfo.allIsOK && ShouldProcessNote(self->noteCutInfo.noteData)) {
        int after = self->afterCutScore;
        if (self->noteScoreDefinition->maxAfterCutScore == 0)  // TODO: selectively exclude from averages?
            after = 30;
        if (self->noteCutInfo.saberType == SaberType::SaberA) {
            notesLeftCut++;
            leftPreSwing += self->beforeCutScore;
            leftPostSwing += after;
            leftAccuracy += self->centerDistanceCutScore;
            leftTimeDependence += std::abs(self->noteCutInfo.cutNormal.z);
        } else {
            notesRightCut++;
            rightPreSwing += self->beforeCutScore;
            rightPostSwing += after;
            rightAccuracy += self->centerDistanceCutScore;
            rightTimeDependence += std::abs(self->noteCutInfo.cutNormal.z);
        }
        BroadcastEvent((int) Events::NoteCut);
    }
}

MAKE_HOOK_MATCH(
    BeatmapObjectExecutionRatingsRecorder_HandlePlayerHeadDidEnterObstacle,
    &BeatmapObjectExecutionRatingsRecorder::HandlePlayerHeadDidEnterObstacle,
    void,
    BeatmapObjectExecutionRatingsRecorder* self,
    ObstacleController* obstacleController
) {
    BeatmapObjectExecutionRatingsRecorder_HandlePlayerHeadDidEnterObstacle(self, obstacleController);

    wallsHit++;
    combo = 0;
    // TODO: should left and right combos go to 0 as well? depending on location of wall (lol)?
    BroadcastEvent((int) Events::WallHit);
    BroadcastEvent((int) Events::ComboChanged);
}

MAKE_HOOK_MATCH(GameEnergyCounter_ProcessEnergyChange, &GameEnergyCounter::ProcessEnergyChange, void, GameEnergyCounter* self, float energyChange) {

    bool wasAbove0 = !self->_didReach0Energy;

    GameEnergyCounter_ProcessEnergyChange(self, energyChange);

    if (wasAbove0 && self->_didReach0Energy) {
        negativeMods -= 0.5;
        BroadcastEvent((int) Events::ScoreChanged);
    }
    health = self->energy;
    BroadcastEvent((int) Events::HealthChanged);
}

MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {

    AudioTimeSyncController_Update(self);

    DoSlowUpdate();
    songTime = self->songTime;
    BroadcastEvent((int) Events::Update);
}

MAKE_HOOK_MATCH(
    CoreGameHUDController_Initialize, &CoreGameHUDController::Initialize, void, CoreGameHUDController* self, CoreGameHUDController::InitData* initData
) {
    initData->advancedHUD = true;
    wasHidden = initData->hide;

    DestroySignal::Create([]() {
        logger.info("Qounters end");
        initialized = false;
        Reset();
        saberManager = nullptr;
        rotationalAnchor = nullptr;
    });

    if (!InSettingsEnvironment() && !wasHidden && self->isActiveAndEnabled) {
        logger.info("Qounters start");
        if (self->name == "FlyingGameHUD")
            rotationalAnchor = UnityEngine::GameObject::New_ctor("QountersRotationalAnchor")->transform;
        Initialize();
        SetupObjects();
        CreateQounters();
        initialized = true;
    }

    CoreGameHUDController_Initialize(self, initData);
}

MAKE_HOOK_MATCH(
    MultiplayerIntroAnimationController_BindTimeline,
    &MultiplayerIntroAnimationController::BindTimeline,
    void,
    MultiplayerIntroAnimationController* self
) {
    MultiplayerIntroAnimationController_BindTimeline(self);

    if (!InSettingsEnvironment() && !wasHidden && !initialized) {
        logger.info("Qounters start");
        Initialize();
        SetupObjects();
        CreateQounters();
        initialized = true;
    }
}

MAKE_HOOK_MATCH(FlyingGameHUDRotation_LateUpdate, &FlyingGameHUDRotation::LateUpdate, void, FlyingGameHUDRotation* self) {

    FlyingGameHUDRotation_LateUpdate(self);

    if (rotationalAnchor)
        rotationalAnchor->rotation = self->transform->rotation;
}

MAKE_HOOK_MATCH(
    StandardLevelDetailView_SetContentForBeatmapData, &StandardLevelDetailView::SetContentForBeatmapData, void, StandardLevelDetailView* self
) {
    if (self->beatmapKey.IsValid())
        PP::GetMapInfo(self->beatmapKey);

    StandardLevelDetailView_SetContentForBeatmapData(self);
}

MAKE_HOOK_MATCH(PauseController_Pause, &PauseController::Pause, void, PauseController* self) {

    if (!InSettingsEnvironment())
        PauseController_Pause(self);
}

MAKE_HOOK_MATCH(
    MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu,
    &MultiplayerLocalActivePlayerInGameMenuViewController::ShowMenu,
    void,
    MultiplayerLocalActivePlayerInGameMenuViewController* self
) {
    if (!InSettingsEnvironment())
        MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu(self);
}

MAKE_HOOK_MATCH(
    MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu,
    &MultiplayerLocalActivePlayerInGameMenuViewController::HideMenu,
    void,
    MultiplayerLocalActivePlayerInGameMenuViewController* self
) {
    if (!InSettingsEnvironment())
        MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu(self);
}

MAKE_HOOK_MATCH(
    MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail,
    &MultiplayerLocalActivePlayerGameplayManager::PerformPlayerFail,
    void,
    MultiplayerLocalActivePlayerGameplayManager* self
) {
    if (!InSettingsEnvironment())
        MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail(self);
}

MAKE_HOOK_MATCH(
    MultiplayerSessionManager_get_localPlayer, &MultiplayerSessionManager::get_localPlayer, IConnectedPlayer*, MultiplayerSessionManager* self
) {
    auto ret = MultiplayerSessionManager_get_localPlayer(self);

    if (!ret && localFakeConnectedPlayer)
        return localFakeConnectedPlayer;
    return ret;
}

MAKE_HOOK_MATCH(
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
    if (InSettingsEnvironment() && Editor::GetPreviewMode()) {
        if (self->_canvas->name == "QounterGroup")
            return;
        self->_blockingMask = mask.m_Mask & ~LayerMasks::getStaticF_saberLayerMask().m_Mask;
    }

    VRGraphicRaycaster_Raycast(self, eventData, resultAppendList);
    self->_blockingMask = mask;
}

MAKE_HOOK_MATCH(
    InputFieldView_DeactivateKeyboard, &HMUI::InputFieldView::DeactivateKeyboard, void, HMUI::InputFieldView* self, HMUI::UIKeyboard* keyboard
) {
    InputFieldView_DeactivateKeyboard(self, keyboard);

    auto handler = self->GetComponent<KeyboardCloseHandler*>();
    if (handler && handler->closeCallback)
        handler->closeCallback();
}

MAKE_HOOK_MATCH(UIKeyboardManager_OpenKeyboardFor, &UIKeyboardManager::OpenKeyboardFor, void, UIKeyboardManager* self, HMUI::InputFieldView* input) {

    if (auto inputModal = input->GetComponentInParent<HMUI::ModalView*>()) {
        auto inputModalCanvas = inputModal->GetComponent<UnityEngine::Canvas*>();
        auto keyboardModalCanvas = self->_keyboardModalView->GetComponent<UnityEngine::Canvas*>();
        keyboardModalCanvas->sortingOrder = inputModalCanvas->sortingOrder;
    }

    UIKeyboardManager_OpenKeyboardFor(self, input);
}

MAKE_HOOK_MATCH(UIKeyboardManager_HandleKeyboardOkButton, &UIKeyboardManager::HandleKeyboardOkButton, void, UIKeyboardManager* self) {

    auto handler = self->_selectedInput->GetComponent<KeyboardCloseHandler*>();
    if (handler && handler->okCallback)
        handler->okCallback();

    UIKeyboardManager_HandleKeyboardOkButton(self);
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

MAKE_HOOK_MATCH(ImageView_OnPopulateMesh, &HMUI::ImageView::OnPopulateMesh, void, HMUI::ImageView* self, UnityEngine::UI::VertexHelper* toFill) {
    if (auto hsv = self->GetComponent<HSVGradientImage*>())
        PopulateMeshHSVGradient(self, toFill, hsv->modified, hsv->modifier, hsv->elements);
    else
        ImageView_OnPopulateMesh(self, toFill);
}

void Qounters::InstallHooks() {
    INSTALL_HOOK(logger, ScoreController_DespawnScoringElement);
    INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasCut);
    INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasMissed);
    INSTALL_HOOK(logger, CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish);
    INSTALL_HOOK(logger, BeatmapObjectExecutionRatingsRecorder_HandlePlayerHeadDidEnterObstacle);
    INSTALL_HOOK(logger, GameEnergyCounter_ProcessEnergyChange);
    INSTALL_HOOK(logger, AudioTimeSyncController_Update);
    INSTALL_HOOK(logger, CoreGameHUDController_Initialize);
    INSTALL_HOOK(logger, MultiplayerIntroAnimationController_BindTimeline);
    INSTALL_HOOK(logger, FlyingGameHUDRotation_LateUpdate);
    INSTALL_HOOK(logger, StandardLevelDetailView_SetContentForBeatmapData);
    INSTALL_HOOK(logger, PauseController_Pause);
    INSTALL_HOOK(logger, MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu);
    INSTALL_HOOK(logger, MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu);
    INSTALL_HOOK(logger, MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail);
    INSTALL_HOOK(logger, MultiplayerSessionManager_get_localPlayer);
    INSTALL_HOOK(logger, VRGraphicRaycaster_Raycast);
    INSTALL_HOOK(logger, InputFieldView_DeactivateKeyboard);
    INSTALL_HOOK(logger, UIKeyboardManager_OpenKeyboardFor);
    INSTALL_HOOK(logger, UIKeyboardManager_HandleKeyboardOkButton);
    INSTALL_HOOK(logger, ImageView_OnPopulateMesh);
}
