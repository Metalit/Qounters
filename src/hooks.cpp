#include "events.hpp"
#include "game.hpp"
#include "main.hpp"
#include "hooks.hpp"
#include "config.hpp"
#include "internals.hpp"
#include "pp.hpp"
#include "qounters.hpp"
#include "environment.hpp"
#include "customtypes/settings.hpp"

using namespace GlobalNamespace;
using namespace VRUIControls;
using namespace Qounters;

#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/ScoringElement.hpp"
#include "GlobalNamespace/ScoreMultiplierCounter.hpp"

MAKE_HOOK_MATCH(ScoreController_DespawnScoringElement, &ScoreController::DespawnScoringElement, void, ScoreController* self, ScoringElement* scoringElement) {

    ScoreController_DespawnScoringElement(self, scoringElement);

    int cutScore = scoringElement->get_cutScore() * scoringElement->get_multiplier();
    int maxCutScore = scoringElement->get_maxPossibleCutScore() * scoringElement->get_maxMultiplier();

    bool badCut = scoringElement->get_multiplierEventType() == ScoreMultiplierCounter::MultiplierEventType::Negative
        && scoringElement->get_wouldBeCorrectCutBestPossibleMultiplierEventType() == ScoreMultiplierCounter::MultiplierEventType::Positive
        && cutScore == 0 && maxCutScore > 0;

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
            leftMissedFixedScore += (scoringElement->get_cutScore() * scoringElement->get_maxMultiplier()) - cutScore;
    } else {
        rightScore += cutScore;
        rightMaxScore += maxCutScore;
        if (badCut) {
            if (isGoodScoreFixed)
                rightMissedFixedScore += maxCutScore;
            else
                rightMissedMaxScore += maxCutScore;
        } else
            rightMissedFixedScore += (scoringElement->get_cutScore() * scoringElement->get_maxMultiplier()) - cutScore;
    }
    BroadcastEvent((int) Events::ScoreChanged);
}

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"

MAKE_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasCut, &BeatmapObjectManager::HandleNoteControllerNoteWasCut,
        void, BeatmapObjectManager* self, NoteController* noteController, ByRef<NoteCutInfo> info) {

    BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, info);

    bool left = info->saberType == SaberType::SaberA;
    bool bomb = noteController->noteData->gameplayType == NoteData::GameplayType::Bomb;
    if (info->get_allIsOK()) {
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

MAKE_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasMissed, &BeatmapObjectManager::HandleNoteControllerNoteWasMissed,
        void, BeatmapObjectManager* self, NoteController* noteController) {

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

#include "GlobalNamespace/CutScoreBuffer.hpp"
#include "GlobalNamespace/ScoreModel.hpp"

MAKE_HOOK_MATCH(CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish, &CutScoreBuffer::HandleSaberSwingRatingCounterDidFinish,
        void, CutScoreBuffer* self, ISaberSwingRatingCounter* swingRatingCounter) {

    CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish(self, swingRatingCounter);

    if (self->noteCutInfo.get_allIsOK() && ShouldProcessNote(self->noteCutInfo.noteData)) {
        int after = self->get_afterCutScore();
        if (self->noteScoreDefinition->maxAfterCutScore == 0) // TODO: selectively exclude from averages?
            after = 30;
        if (self->noteCutInfo.saberType == SaberType::SaberA) {
            notesLeftCut++;
            leftPreSwing += self->get_beforeCutScore();
            leftPostSwing += after;
            leftAccuracy += self->get_centerDistanceCutScore();
            leftTimeDependence += std::abs(self->noteCutInfo.cutNormal.z);
        } else {
            notesRightCut++;
            rightPreSwing += self->get_beforeCutScore();
            rightPostSwing += after;
            rightAccuracy += self->get_centerDistanceCutScore();
            rightTimeDependence += std::abs(self->noteCutInfo.cutNormal.z);
        }
        BroadcastEvent((int) Events::NoteCut);
    }
}

#include "GlobalNamespace/BeatmapObjectExecutionRatingsRecorder.hpp"

MAKE_HOOK_MATCH(BeatmapObjectExecutionRatingsRecorder_HandlePlayerHeadDidEnterObstacle, &BeatmapObjectExecutionRatingsRecorder::HandlePlayerHeadDidEnterObstacle,
        void, BeatmapObjectExecutionRatingsRecorder* self, ObstacleController* obstacleController) {

    BeatmapObjectExecutionRatingsRecorder_HandlePlayerHeadDidEnterObstacle(self, obstacleController);

    wallsHit++;
    combo = 0;
    // TODO: should left and right combos go to 0 as well? depending on location of wall (lol)?
    BroadcastEvent((int) Events::WallHit);
    BroadcastEvent((int) Events::ComboChanged);
}

#include "GlobalNamespace/GameEnergyCounter.hpp"

MAKE_HOOK_MATCH(GameEnergyCounter_ProcessEnergyChange, &GameEnergyCounter::ProcessEnergyChange, void, GameEnergyCounter* self, float energyChange) {

    bool wasAbove0 = !self->_didReach0Energy;

    GameEnergyCounter_ProcessEnergyChange(self, energyChange);

    if (wasAbove0 && self->_didReach0Energy) {
        negativeMods -= 0.5;
        BroadcastEvent((int) Events::ScoreChanged);
    }
    health = self->get_energy();
    BroadcastEvent((int) Events::HealthChanged);
}

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "UnityEngine/Time.hpp"
#include "GlobalNamespace/Saber.hpp"

SaberManager* saberManager = nullptr;
float lastUpdated = 0;

UnityEngine::Quaternion prevRotLeft;
UnityEngine::Quaternion prevRotRight;

MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {

    AudioTimeSyncController_Update(self);

    lastUpdated += UnityEngine::Time::get_deltaTime();
    if (lastUpdated > 1 / (float) SPEED_SAMPLES_PER_SEC) {
        if (saberManager) {
            leftSpeeds.emplace_back(saberManager->leftSaber->get_bladeSpeed());
            rightSpeeds.emplace_back(saberManager->rightSaber->get_bladeSpeed());

            auto rotLeft = saberManager->leftSaber->get_transform()->get_rotation();
            auto rotRight = saberManager->rightSaber->get_transform()->get_rotation();
            // use speeds array as tracker for if prevRots have accurate values
            if (leftSpeeds.size() > 1) {
                leftAngles.emplace_back(UnityEngine::Quaternion::Angle(rotLeft, prevRotLeft));
                rightAngles.emplace_back(UnityEngine::Quaternion::Angle(rotRight, prevRotRight));
            }
            prevRotLeft = rotLeft;
            prevRotRight = rotRight;
        }
        lastUpdated = 0;
        BroadcastEvent((int) Events::SlowUpdate);
    }

    songTime = self->get_songTime();
    BroadcastEvent((int) Events::Update);
}

#include "GlobalNamespace/CoreGameHUDController.hpp"

MAKE_HOOK_MATCH(CoreGameHUDController_Initialize, &CoreGameHUDController::Initialize, void, CoreGameHUDController* self, CoreGameHUDController::InitData* initData) {

    QountersLogger::Logger.info("Qounters start");
    initData->advancedHUD = true;
    saberManager = UnityEngine::Object::FindObjectOfType<SaberManager*>();

    if (!InSettingsEnvironment()) {
        Initialize();
        SetupObjects();
        CreateQounters();
    }

    CoreGameHUDController_Initialize(self, initData);
}

#include "GlobalNamespace/StandardLevelDetailView.hpp"

MAKE_HOOK_MATCH(StandardLevelDetailView_RefreshContent, &StandardLevelDetailView::RefreshContent,
        void, StandardLevelDetailView* self) {

    //if (selectedDifficultyBeatmap)
    //    PP::GetMapInfo(selectedDifficultyBeatmap);

    StandardLevelDetailView_RefreshContent(self);
}

#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"

MAKE_HOOK_MATCH(StandardLevelScenesTransitionSetupDataSO_Finish, &StandardLevelScenesTransitionSetupDataSO::Finish,
        void, StandardLevelScenesTransitionSetupDataSO* self, LevelCompletionResults* levelCompletionResults) {

    Reset();

    StandardLevelScenesTransitionSetupDataSO_Finish(self, levelCompletionResults);
}

#include "GlobalNamespace/PauseController.hpp"

MAKE_HOOK_MATCH(PauseController_Pause, &PauseController::Pause, void, PauseController* self) {

    if (!InSettingsEnvironment())
        PauseController_Pause(self);
}

#include "GlobalNamespace/MultiplayerLocalActivePlayerInGameMenuViewController.hpp"

MAKE_HOOK_MATCH(MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu, &MultiplayerLocalActivePlayerInGameMenuViewController::ShowMenu,
        void, MultiplayerLocalActivePlayerInGameMenuViewController* self) {

    if (!InSettingsEnvironment())
        MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu(self);
}

MAKE_HOOK_MATCH(MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu, &MultiplayerLocalActivePlayerInGameMenuViewController::HideMenu,
        void, MultiplayerLocalActivePlayerInGameMenuViewController* self) {

    if (!InSettingsEnvironment())
        MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu(self);
}

#include "GlobalNamespace/MultiplayerLocalActivePlayerGameplayManager.hpp"

MAKE_HOOK_MATCH(MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail, &MultiplayerLocalActivePlayerGameplayManager::PerformPlayerFail,
        void, MultiplayerLocalActivePlayerGameplayManager* self) {

    if (!InSettingsEnvironment())
        MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail(self);
}

#include "GlobalNamespace/MultiplayerSessionManager.hpp"

MAKE_HOOK_MATCH(MultiplayerSessionManager_get_localPlayer, &MultiplayerSessionManager::get_localPlayer, IConnectedPlayer*, MultiplayerSessionManager* self) {

    auto ret = MultiplayerSessionManager_get_localPlayer(self);

    if (!ret && localFakeConnectedPlayer)
        return localFakeConnectedPlayer;
    return ret;
}

#include "VRUIControls/VRGraphicRaycaster.hpp"

MAKE_HOOK_MATCH(VRGraphicRaycaster_Raycast, &VRGraphicRaycaster::Raycast,
        void, VRGraphicRaycaster* self, UnityEngine::EventSystems::PointerEventData* eventData, System::Collections::Generic::List_1<UnityEngine::EventSystems::RaycastResult>* resultAppendList) {

    if (blockOtherRaycasts && !raycastCanvases.contains(self->_canvas))
        return;

    VRGraphicRaycaster_Raycast(self, eventData, resultAppendList);
}

#include "HMUI/InputFieldView.hpp"
#include "HMUI/UIKeyboard.hpp"

MAKE_HOOK_MATCH(InputFieldView_DeactivateKeyboard, &HMUI::InputFieldView::DeactivateKeyboard, void, HMUI::InputFieldView* self, HMUI::UIKeyboard* keyboard) {

    InputFieldView_DeactivateKeyboard(self, keyboard);

    auto handler = self->GetComponent<KeyboardCloseHandler*>();
    if (handler && handler->closeCallback)
        handler->closeCallback();
}

#include "GlobalNamespace/UIKeyboardManager.hpp"

MAKE_HOOK_MATCH(UIKeyboardManager_OpenKeyboardFor, &UIKeyboardManager::OpenKeyboardFor, void, UIKeyboardManager* self, HMUI::InputFieldView* input) {

    if (auto inputModal = input->GetComponentInParent<HMUI::ModalView*>()) {
        auto inputModalCanvas = inputModal->GetComponent<UnityEngine::Canvas*>();
        auto keyboardModalCanvas = self->_keyboardModalView->GetComponent<UnityEngine::Canvas*>();
        keyboardModalCanvas->set_sortingOrder(inputModalCanvas->get_sortingOrder());
    }

    UIKeyboardManager_OpenKeyboardFor(self, input);
}

MAKE_HOOK_MATCH(UIKeyboardManager_HandleKeyboardOkButton, &UIKeyboardManager::HandleKeyboardOkButton, void, UIKeyboardManager* self) {

    auto handler = self->_selectedInput->GetComponent<KeyboardCloseHandler*>();
    if (handler && handler->okCallback)
        handler->okCallback();

    UIKeyboardManager_HandleKeyboardOkButton(self);
}

MAKE_HOOK(abort_hook, nullptr, void) {
    QountersLogger::Logger.info("abort called");
    QountersLogger::Logger.Backtrace(40);

    abort_hook();
}

void Qounters::InstallHooks() {
    auto logger = QountersLogger::Logger;
    INSTALL_HOOK(logger, ScoreController_DespawnScoringElement);
    INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasCut);
    INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasMissed);
    INSTALL_HOOK(logger, CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish);
    INSTALL_HOOK(logger, BeatmapObjectExecutionRatingsRecorder_HandlePlayerHeadDidEnterObstacle);
    INSTALL_HOOK(logger, GameEnergyCounter_ProcessEnergyChange);
    INSTALL_HOOK(logger, AudioTimeSyncController_Update);
    INSTALL_HOOK(logger, CoreGameHUDController_Initialize);
    INSTALL_HOOK(logger, StandardLevelDetailView_RefreshContent);
    INSTALL_HOOK(logger, StandardLevelScenesTransitionSetupDataSO_Finish);
    INSTALL_HOOK(logger, PauseController_Pause);
    INSTALL_HOOK(logger, MultiplayerLocalActivePlayerInGameMenuViewController_ShowMenu);
    INSTALL_HOOK(logger, MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu);
    INSTALL_HOOK(logger, MultiplayerLocalActivePlayerGameplayManager_PerformPlayerFail);
    INSTALL_HOOK(logger, MultiplayerSessionManager_get_localPlayer);
    INSTALL_HOOK(logger, VRGraphicRaycaster_Raycast);
    INSTALL_HOOK(logger, InputFieldView_DeactivateKeyboard);
    INSTALL_HOOK(logger, UIKeyboardManager_OpenKeyboardFor);
    INSTALL_HOOK(logger, UIKeyboardManager_HandleKeyboardOkButton);

    auto libc = dlopen("libc.so", RTLD_NOW);
    auto abort = dlsym(libc, "abort");

    INSTALL_HOOK_DIRECT(QountersLogger::Logger, abort_hook, abort)
}
