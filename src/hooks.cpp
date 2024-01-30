#include "events.hpp"
#include "game.hpp"
#include "main.hpp"
#include "hooks.hpp"
#include "config.hpp"
#include "internals.hpp"
#include "pp.hpp"
#include "qounters.hpp"
#include "environment.hpp"

using namespace GlobalNamespace;
using namespace Qounters;

#include "GlobalNamespace/ScoreController.hpp"

MAKE_HOOK_MATCH(ScoreController_DespawnScoringElement, &ScoreController::DespawnScoringElement, void, ScoreController* self, ScoringElement* scoringElement) {

    ScoreController_DespawnScoringElement(self, scoringElement);

    if (scoringElement->noteData->colorType == ColorType::ColorA) {
        leftScore += scoringElement->get_cutScore() * scoringElement->multiplier;
        leftMaxScore += scoringElement->get_maxPossibleCutScore() * scoringElement->maxMultiplier;
    } else {
        rightScore += scoringElement->get_cutScore() * scoringElement->multiplier;
        rightMaxScore += scoringElement->get_maxPossibleCutScore() * scoringElement->maxMultiplier;
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
#include "GlobalNamespace/ScoreModel_NoteScoreDefinition.hpp"

MAKE_HOOK_MATCH(CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish, &CutScoreBuffer::HandleSaberSwingRatingCounterDidFinish,
        void, CutScoreBuffer* self, ISaberSwingRatingCounter* swingRatingCounter) {

    CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish(self, swingRatingCounter);

    if (self->noteCutInfo.get_allIsOK() && ShouldProcessNote(self->noteCutInfo.noteData)) {
        int after = self->afterCutScore;
        if (self->noteScoreDefinition->maxAfterCutScore == 0) // TODO: selectively exclude from averages?
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

    bool wasAbove0 = !self->didReach0Energy;

    GameEnergyCounter_ProcessEnergyChange(self, energyChange);

    if (wasAbove0 && self->didReach0Energy) {
        negativeMods -= 0.5;
        BroadcastEvent((int) Events::ScoreChanged);
    }
    health = self->energy;
    BroadcastEvent((int) Events::HealthChanged);
}

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "UnityEngine/Time.hpp"
#include "GlobalNamespace/Saber.hpp"

SaberManager* saberManager = nullptr;
float lastUpdated = 0;

MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {

    AudioTimeSyncController_Update(self);

    lastUpdated += UnityEngine::Time::get_deltaTime();
    if (lastUpdated > 1 / (float) SPEED_SAMPLES_PER_SEC) {
        if (saberManager) {
            leftSpeeds.emplace_back(saberManager->leftSaber->get_bladeSpeed());
            rightSpeeds.emplace_back(saberManager->rightSaber->get_bladeSpeed());
        }
        lastUpdated = 0;
        BroadcastEvent((int) Events::SlowUpdate);
    }

    songTime = self->songTime;
    BroadcastEvent((int) Events::Update);
}

#include "GlobalNamespace/CoreGameHUDController.hpp"
#include "GlobalNamespace/CoreGameHUDController_InitData.hpp"

MAKE_HOOK_MATCH(CoreGameHUDController_Start, &CoreGameHUDController::Start, void, CoreGameHUDController* self) {

    getLogger().info("Qounters start");
    self->initData->advancedHUD = true;
    saberManager = UnityEngine::Object::FindObjectOfType<SaberManager*>();

    if (!InSettingsEnvironment()) {
        Initialize();
        SetupObjects();
        CreateQounters();
    }

    CoreGameHUDController_Start(self);
}

#include "GlobalNamespace/StandardLevelDetailView.hpp"

MAKE_HOOK_MATCH(StandardLevelDetailView_SetContentForBeatmapDataAsync, &StandardLevelDetailView::SetContentForBeatmapDataAsync,
        void, StandardLevelDetailView* self, IDifficultyBeatmap* selectedDifficultyBeatmap) {

    if (selectedDifficultyBeatmap)
        PP::GetMapInfo(selectedDifficultyBeatmap);

    StandardLevelDetailView_SetContentForBeatmapDataAsync(self, selectedDifficultyBeatmap);
}

#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"

MAKE_HOOK_MATCH(StandardLevelScenesTransitionSetupDataSO_Finish, &StandardLevelScenesTransitionSetupDataSO::Finish,
        void, StandardLevelScenesTransitionSetupDataSO* self, LevelCompletionResults* levelCompletionResults) {

    Reset();

    StandardLevelScenesTransitionSetupDataSO_Finish(self, levelCompletionResults);
}

#include "GlobalNamespace/MultiplayerSessionManager.hpp"

MAKE_HOOK_MATCH(MultiplayerSessionManager_get_localPlayer, &MultiplayerSessionManager::get_localPlayer, IConnectedPlayer*, MultiplayerSessionManager* self) {

    auto ret = MultiplayerSessionManager_get_localPlayer(self);

    if (!ret && localFakeConnectedPlayer)
        return localFakeConnectedPlayer;
    return ret;
}

#include "GlobalNamespace/MultiplayerLocalActivePlayerInGameMenuViewController.hpp"

MAKE_HOOK_MATCH(MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu, &MultiplayerLocalActivePlayerInGameMenuViewController::HideMenu,
        void, MultiplayerLocalActivePlayerInGameMenuViewController* self) {

    if (!InSettingsEnvironment())
        MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu(self);
}

void Qounters::InstallHooks() {
    auto logger = getLogger().WithContext("Hooks");
    INSTALL_HOOK(logger, ScoreController_DespawnScoringElement);
    INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasCut);
    INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasMissed);
    INSTALL_HOOK(logger, CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish);
    INSTALL_HOOK(logger, BeatmapObjectExecutionRatingsRecorder_HandlePlayerHeadDidEnterObstacle);
    INSTALL_HOOK(logger, GameEnergyCounter_ProcessEnergyChange);
    INSTALL_HOOK(logger, AudioTimeSyncController_Update);
    INSTALL_HOOK(logger, CoreGameHUDController_Start);
    INSTALL_HOOK(logger, StandardLevelDetailView_SetContentForBeatmapDataAsync);
    INSTALL_HOOK(logger, StandardLevelScenesTransitionSetupDataSO_Finish);
    INSTALL_HOOK(logger, MultiplayerSessionManager_get_localPlayer);
    INSTALL_HOOK(logger, MultiplayerLocalActivePlayerInGameMenuViewController_HideMenu);
}
