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

    if (noteController->noteData->colorType == ColorType::ColorA)
        notesLeftMissed++;
    else
        notesRightMissed++;
    BroadcastEvent((int) Events::NoteMissed);
}

#include "GlobalNamespace/CutScoreBuffer.hpp"
#include "GlobalNamespace/ScoreModel_NoteScoreDefinition.hpp"

MAKE_HOOK_MATCH(CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish, &CutScoreBuffer::HandleSaberSwingRatingCounterDidFinish,
        void, CutScoreBuffer* self, ISaberSwingRatingCounter* swingRatingCounter) {

    CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish(self, swingRatingCounter);

    if (self->noteCutInfo.get_allIsOK() && ShouldProcessNote(self->noteCutInfo.noteData)) {
        float after = self->afterCutScore;
        if (self->noteScoreDefinition->maxAfterCutScore == 0) // TODO: selectively exclude from averages?
            after = 30;
        if (self->noteCutInfo.saberType == SaberType::SaberA) {
            notesLeftCut++;
            leftPreSwing += self->beforeCutScore;
            leftPostSwing += after;
            leftAccuracy += self->centerDistanceCutScore;
        } else {
            notesRightCut++;
            rightPreSwing += self->beforeCutScore;
            rightPostSwing += after;
            rightAccuracy += self->centerDistanceCutScore;
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
    Initialize();
    SetupObjects();

    if (!InSettingsEnvironment())
        CreateQounters();

    CoreGameHUDController_Start(self);
}

// #include "System/Action_2.hpp"

// using ScoreChangeDelegate = System::Action_2<int, int>*;

// MAKE_HOOK_MATCH(ScoreController_Start, &ScoreController::Start, void, ScoreController* self) {

//     ScoreController_Start(self);

//     self->add_scoreDidChangeEvent(il2cpp_utils::MakeDelegate<ScoreChangeDelegate>(
//         classof(ScoreChangeDelegate), self, +[](ScoreController* self, int rawScore, int modifiedScore) {
//             BroadcastEvent((int) Events::ScoreChanged);
//         }
//     ));
// }

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

#include "GlobalNamespace/UIKeyboardManager.hpp"

MAKE_HOOK_MATCH(log, &UIKeyboardManager::ProcessMousePress, void, UIKeyboardManager* self, UnityEngine::GameObject* currentOverGo) {
    log(self, currentOverGo);
    std::string name = currentOverGo->get_name();
    getLogger().debug("process mouse press, go: %s", name.c_str());
}

void Qounters::InstallHooks() {
    INSTALL_HOOK(getLogger(), ScoreController_DespawnScoringElement);
    INSTALL_HOOK(getLogger(), BeatmapObjectManager_HandleNoteControllerNoteWasCut);
    INSTALL_HOOK(getLogger(), BeatmapObjectManager_HandleNoteControllerNoteWasMissed);
    INSTALL_HOOK(getLogger(), CutScoreBuffer_HandleSaberSwingRatingCounterDidFinish);
    INSTALL_HOOK(getLogger(), BeatmapObjectExecutionRatingsRecorder_HandlePlayerHeadDidEnterObstacle);
    INSTALL_HOOK(getLogger(), GameEnergyCounter_ProcessEnergyChange);
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Update);
    INSTALL_HOOK(getLogger(), CoreGameHUDController_Start);
    // INSTALL_HOOK(getLogger(), ScoreController_Start);
    INSTALL_HOOK(getLogger(), StandardLevelDetailView_SetContentForBeatmapDataAsync);
    INSTALL_HOOK(getLogger(), StandardLevelScenesTransitionSetupDataSO_Finish);
    // INSTALL_HOOK(getLogger(), log);
}
