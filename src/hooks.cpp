#include "hooks.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapLevelsModel.hpp"
#include "GlobalNamespace/BeatmapObjectExecutionRatingsRecorder.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/CoreGameHUDController.hpp"
#include "GlobalNamespace/CutScoreBuffer.hpp"
#include "GlobalNamespace/EnvironmentOverrideSettingsPanelController.hpp"
#include "GlobalNamespace/FlyingGameHUDRotation.hpp"
#include "GlobalNamespace/GameEnergyCounter.hpp"
#include "GlobalNamespace/LayerMasks.hpp"
#include "GlobalNamespace/LevelBar.hpp"
#include "GlobalNamespace/MissionDataSO.hpp"
#include "GlobalNamespace/MissionLevelDetailViewController.hpp"
#include "GlobalNamespace/MissionNode.hpp"
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
#include "gameplay.hpp"
#include "internals.hpp"
#include "main.hpp"
#include "pp.hpp"
#include "qounters.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace GlobalNamespace;
using namespace VRUIControls;

static UnityEngine::Transform* rotationalAnchor = nullptr;
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
        Internals::leftScore += cutScore;
        Internals::leftMaxScore += maxCutScore;
        if (badCut) {
            if (isGoodScoreFixed)
                Internals::leftMissedFixedScore += maxCutScore;
            else
                Internals::leftMissedMaxScore += maxCutScore;
        } else
            Internals::leftMissedFixedScore += (scoringElement->cutScore * scoringElement->maxMultiplier) - cutScore;
    } else {
        Internals::rightScore += cutScore;
        Internals::rightMaxScore += maxCutScore;
        if (badCut) {
            if (isGoodScoreFixed)
                Internals::rightMissedFixedScore += maxCutScore;
            else
                Internals::rightMissedMaxScore += maxCutScore;
        } else
            Internals::rightMissedFixedScore += (scoringElement->cutScore * scoringElement->maxMultiplier) - cutScore;
    }
    Events::Broadcast((int) Events::ScoreChanged);
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
    if (!bomb && Internals::IsFakeNote(noteController->noteData))
        return;

    if (info->allIsOK) {
        Internals::combo++;
        if (left)
            Internals::leftCombo++;
        else
            Internals::rightCombo++;
    } else {
        Internals::combo = 0;
        if (left) {
            if (bomb)
                Internals::bombsLeftHit++;
            else
                Internals::notesLeftBadCut++;
            Internals::leftCombo = 0;
        } else {
            if (bomb)
                Internals::bombsRightHit++;
            else
                Internals::notesRightBadCut++;
            Internals::rightCombo = 0;
        }
        if (bomb)
            Events::Broadcast((int) Events::BombCut);
        else
            Events::Broadcast((int) Events::NoteCut);
    }
    Events::Broadcast((int) Events::ComboChanged);
}

MAKE_HOOK_MATCH(
    BeatmapObjectManager_HandleNoteControllerNoteWasMissed,
    &BeatmapObjectManager::HandleNoteControllerNoteWasMissed,
    void,
    BeatmapObjectManager* self,
    NoteController* noteController
) {
    BeatmapObjectManager_HandleNoteControllerNoteWasMissed(self, noteController);

    if (noteController->noteData->gameplayType == NoteData::GameplayType::Bomb || Internals::IsFakeNote(noteController->noteData))
        return;

    Internals::combo = 0;
    if (noteController->noteData->colorType == ColorType::ColorA) {
        Internals::leftCombo = 0;
        Internals::notesLeftMissed++;
    } else {
        Internals::rightCombo = 0;
        Internals::notesRightMissed++;
    }
    Events::Broadcast((int) Events::NoteMissed);
    Events::Broadcast((int) Events::ComboChanged);
}

MAKE_HOOK_MATCH(
    CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish,
    &CutScoreBuffer::HandleSaberSwingRatingCounterDidFinish,
    void,
    CutScoreBuffer* self,
    ISaberSwingRatingCounter* swingRatingCounter
) {
    CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish(self, swingRatingCounter);

    if (self->noteCutInfo.allIsOK && Internals::ShouldCountNote(self->noteCutInfo.noteData)) {
        int after = self->afterCutScore;
        if (self->noteScoreDefinition->maxAfterCutScore == 0)  // TODO: selectively exclude from averages?
            after = 30;
        if (self->noteCutInfo.saberType == SaberType::SaberA) {
            Internals::notesLeftCut++;
            Internals::leftPreSwing += self->beforeCutScore;
            Internals::leftPostSwing += after;
            Internals::leftAccuracy += self->centerDistanceCutScore;
            Internals::leftTimeDependence += std::abs(self->noteCutInfo.cutNormal.z);
        } else {
            Internals::notesRightCut++;
            Internals::rightPreSwing += self->beforeCutScore;
            Internals::rightPostSwing += after;
            Internals::rightAccuracy += self->centerDistanceCutScore;
            Internals::rightTimeDependence += std::abs(self->noteCutInfo.cutNormal.z);
        }
        Events::Broadcast((int) Events::NoteCut);
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

    Internals::wallsHit++;
    Internals::combo = 0;
    // TODO: should left and right combos go to 0 as well? depending on location of wall (lol)?
    Events::Broadcast((int) Events::WallHit);
    Events::Broadcast((int) Events::ComboChanged);
}

MAKE_HOOK_MATCH(GameEnergyCounter_ProcessEnergyChange, &GameEnergyCounter::ProcessEnergyChange, void, GameEnergyCounter* self, float energyChange) {

    bool wasAbove0 = !self->_didReach0Energy;

    GameEnergyCounter_ProcessEnergyChange(self, energyChange);

    if (wasAbove0 && self->_didReach0Energy) {
        Internals::negativeMods -= 0.5;
        Events::Broadcast((int) Events::ScoreChanged);
    }
    Internals::health = self->energy;
    Events::Broadcast((int) Events::HealthChanged);
}

MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {

    AudioTimeSyncController_Update(self);

    Internals::DoSlowUpdate();
    Internals::songTime = self->songTime;
    Events::Broadcast((int) Events::Update);
}

MAKE_HOOK_MATCH(
    CoreGameHUDController_Initialize, &CoreGameHUDController::Initialize, void, CoreGameHUDController* self, CoreGameHUDController::InitData* initData
) {
    initData->advancedHUD = true;
    wasHidden = initData->hide;

    ObjectSignal::CreateDestroySignal([]() {
        logger.info("Qounters end");
        initialized = false;
        HUD::Reset();
        Internals::saberManager = nullptr;
        rotationalAnchor = nullptr;
    });

    if (!Environment::InSettings() && !wasHidden && self->isActiveAndEnabled) {
        logger.info("Qounters start");
        if (self->name == "FlyingGameHUD")
            rotationalAnchor = UnityEngine::GameObject::New_ctor("QountersRotationalAnchor")->transform;
        Internals::Initialize();
        HUD::SetupObjects();
        HUD::CreateQounters();
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

    if (!Environment::InSettings() && !wasHidden && !initialized) {
        logger.info("Qounters start");
        Internals::Initialize();
        HUD::SetupObjects();
        HUD::CreateQounters();
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
    if (self->beatmapKey.IsValid()) {
        PP::GetMapInfo(self->beatmapKey);
        Gameplay::SetLevel(self->_beatmapLevel, self->beatmapKey, true);
    }
    if (auto signal = self->gameObject->AddComponent<ObjectSignal*>()) {
        signal->onEnable = [self]() {
            Gameplay::SetLevel(self->_beatmapLevel, self->beatmapKey, true);
        };
        signal->onDisable = Gameplay::ClearLevel;
    }

    StandardLevelDetailView_SetContentForBeatmapData(self);
}

MAKE_HOOK_MATCH(
    MissionLevelDetailViewController_RefreshContent, &MissionLevelDetailViewController::RefreshContent, void, MissionLevelDetailViewController* self
) {
    auto key = self->missionNode->missionData->beatmapKey;
    if (key.IsValid()) {
        PP::GetMapInfo(key);
        if (auto level = self->_levelBar->_beatmapLevelsModel->GetBeatmapLevel(key.levelId))
            Gameplay::SetLevel(level, key, false);
    }
    if (auto signal = self->gameObject->AddComponent<ObjectSignal*>()) {
        signal->onEnable = [model = self->_levelBar->_beatmapLevelsModel, key]() {
            if (auto level = model->GetBeatmapLevel(key.levelId))
                Gameplay::SetLevel(level, key, false);
        };
        signal->onDisable = Gameplay::ClearLevel;
    }

    MissionLevelDetailViewController_RefreshContent(self);
}

MAKE_HOOK_MATCH(
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

MAKE_HOOK_MATCH(
    EnvironmentOverrideSettingsPanelController_HandleOverrideEnvironmentsToggleValueChanged,
    &EnvironmentOverrideSettingsPanelController::HandleOverrideEnvironmentsToggleValueChanged,
    void,
    EnvironmentOverrideSettingsPanelController* self,
    bool isOn
) {
    EnvironmentOverrideSettingsPanelController_HandleOverrideEnvironmentsToggleValueChanged(self, isOn);

    Gameplay::UpdateEnvironment();
}

MAKE_HOOK_MATCH(PauseController_Pause, &PauseController::Pause, void, PauseController* self) {
    if (!Environment::InSettings())
        PauseController_Pause(self);
}

MAKE_HOOK_MATCH(
    MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu,
    &MultiplayerLocalActivePlayerInGameMenuViewController::ShowMenu,
    void,
    MultiplayerLocalActivePlayerInGameMenuViewController* self
) {
    if (!Environment::InSettings())
        MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu(self);
}

MAKE_HOOK_MATCH(
    MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu,
    &MultiplayerLocalActivePlayerInGameMenuViewController::HideMenu,
    void,
    MultiplayerLocalActivePlayerInGameMenuViewController* self
) {
    if (!Environment::InSettings())
        MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu(self);
}

MAKE_HOOK_MATCH(
    MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail,
    &MultiplayerLocalActivePlayerGameplayManager::PerformPlayerFail,
    void,
    MultiplayerLocalActivePlayerGameplayManager* self
) {
    if (!Environment::InSettings())
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
    if (Environment::InSettings() && Editor::GetPreviewMode()) {
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

void Hooks::Install() {
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
    INSTALL_HOOK(logger, MissionLevelDetailViewController_RefreshContent);
    INSTALL_HOOK(logger, EnvironmentOverrideSettingsPanelController_HandleDropDownDidSelectCellWithIdx);
    INSTALL_HOOK(logger, EnvironmentOverrideSettingsPanelController_HandleOverrideEnvironmentsToggleValueChanged);
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
