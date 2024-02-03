#include "internals.hpp"
#include "main.hpp"
#include "utils.hpp"

using namespace Qounters;

std::string lastBeatmap;

#include "GlobalNamespace/BeatmapCallbacksUpdater.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataSortedListForTypeAndIds_1.hpp"
#include "GlobalNamespace/ISortedList_1.hpp"

#include "System/Collections/IEnumerator.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"

using namespace GlobalNamespace;

int GetNoteCount(BeatmapCallbacksUpdater* updater) {
    if (!updater)
        return 0;
    using LinkedList = System::Collections::Generic::LinkedList_1<NoteData*>;
    int noteCount = 0;
    auto bcc = updater->beatmapCallbacksController;
    auto songTime = bcc->startFilterTime;
    auto noteDataItemsList = (LinkedList*) ((BeatmapData*) bcc->beatmapData)->beatmapDataItemsPerTypeAndId->GetList(csTypeOf(NoteData*), 0)->get_items();
    auto enumerator = (LinkedList::Enumerator*) noteDataItemsList->System_Collections_IEnumerable_GetEnumerator();
    while(enumerator->MoveNext()) {
        auto noteData = (NoteData*) enumerator->System_Collections_IEnumerator_get_Current();
        if(ShouldProcessNote(noteData) && noteData->get_time() > songTime)
            noteCount++;
    }
    return noteCount;
}

#include "GlobalNamespace/ScoreModel.hpp"

int GetMaxScore(BeatmapCallbacksUpdater* updater) {
    if (!updater)
        return 0;
    return ScoreModel::ComputeMaxMultipliedScoreForBeatmap(updater->beatmapCallbacksController->beatmapData);
}

#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/AudioTimeSyncController_InitData.hpp"
#include "UnityEngine/AudioClip.hpp"

float GetSongLength(ScoreController* controller) {
    if (!controller)
        return 0;
    return controller->audioTimeSyncController->initData->audioClip->get_length();
}

#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerAllOverallStatsData.hpp"
#include "GlobalNamespace/PlayerAllOverallStatsData_PlayerOverallStatsData.hpp"

int GetFailCount(PlayerDataModel* data) {
    if (!data)
        return 0;
    return data->playerData->playerAllOverallStatsData->get_allOverallStatsData()->failedLevelsCount;
}

#include "GlobalNamespace/GameplayModifierParamsSO.hpp"
#include "beatsaber-hook/shared/utils/typedefs-list.hpp"

float GetPositiveMods(ScoreController* controller) {
    if (!controller || !controller->gameplayModifierParams)
        return 0;
    float ret = 0;
    auto mods = ListW<GameplayModifierParamsSO*>(controller->gameplayModifierParams);
    for (auto& mod : mods) {
        float mult = mod->multiplier;
        if (mult > 0)
            ret += mult;
    }
    return ret;
}

#include "GlobalNamespace/GameplayModifiersModelSO.hpp"

float GetNegativeMods(ScoreController* controller) {
    if (!controller || !controller->gameplayModifierParams)
        return 0;
    float ret = 0;
    auto mods = ListW<GameplayModifierParamsSO*>(controller->gameplayModifierParams);
    for (auto& mod : mods) {
        if (mod == controller->gameplayModifiersModel->noFailOn0Energy) {
            noFail = true;
            continue;
        }
        float mult = mod->multiplier;
        if (mult < 0)
            ret += mult;
    }
    return ret;
}

#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/PlayerLevelStatsData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

int GetHighScore(PlayerDataModel* data, GameplayCoreInstaller* installer) {
    if (!data || !installer || !installer->sceneSetupData)
        return 0;
    auto [id, characteristic, difficulty] = Utils::GetBeatmapDetails(installer->sceneSetupData->difficultyBeatmap);
    for (auto& stats : ListW<PlayerLevelStatsData*>(data->playerData->levelsStatsData)) {
        if (stats->levelID == id && stats->beatmapCharacteristic->serializedName == characteristic && stats->difficulty == difficulty)
            return stats->highScore;
    }
    return -1;
}

#include "GlobalNamespace/GameEnergyCounter.hpp"

float GetHealth(ScoreController* controller) {
    if (!controller)
        return 1;
    return controller->gameEnergyCounter->get_energy();
}

namespace Qounters {
    int leftScore;
    int rightScore;
    int leftMaxScore;
    int rightMaxScore;
    int songMaxScore;
    int leftCombo;
    int rightCombo;
    int combo;
    float health;
    float songTime;
    float songLength;
    int notesLeftCut;
    int notesRightCut;
    int notesLeftBadCut;
    int notesRightBadCut;
    int notesLeftMissed;
    int notesRightMissed;
    int bombsLeftHit;
    int bombsRightHit;
    int wallsHit;
    int songNotes;
    int leftPreSwing;
    int rightPreSwing;
    int leftPostSwing;
    int rightPostSwing;
    int leftAccuracy;
    int rightAccuracy;
    float leftTimeDependence;
    float rightTimeDependence;
    std::vector<float> leftSpeeds;
    std::vector<float> rightSpeeds;
    bool noFail;
    float positiveMods;
    float negativeMods;
    int personalBest;
    int fails;
    int restarts;
    ColorScheme* colors;
    BeatmapData* beatmapData;
    int leftMissedMaxScore;
    int rightMissedMaxScore;
    int leftMissedFixedScore;
    int rightMissedFixedScore;
}

#include "UnityEngine/Resources.hpp"

void Qounters::Initialize() {
    auto beatmapCallbacksUpdater = UnityEngine::Object::FindObjectOfType<BeatmapCallbacksUpdater*>();
    auto scoreController = UnityEngine::Object::FindObjectOfType<ScoreController*>();
    auto playerDataModel = UnityEngine::Object::FindObjectOfType<PlayerDataModel*>();

    auto gameplayCoreInstallers = UnityEngine::Resources::FindObjectsOfTypeAll<GameplayCoreInstaller*>();
    GameplayCoreInstaller* gameplayCoreInstaller;
    for (auto& installer : gameplayCoreInstallers) {
        if (installer->get_isActiveAndEnabled() && installer->sceneSetupData != nullptr) {
            gameplayCoreInstaller = installer;
            break;
        }
    }
    if (!gameplayCoreInstaller) gameplayCoreInstaller = gameplayCoreInstallers[0];


    std::string beatmap = "Unknown";
    if (gameplayCoreInstaller && gameplayCoreInstaller->sceneSetupData)
        beatmap = Utils::GetBeatmapIdentifier(gameplayCoreInstaller->sceneSetupData->difficultyBeatmap);

    leftScore = 0;
    rightScore = 0;
    leftMaxScore = 0;
    rightMaxScore = 0;
    songMaxScore = GetMaxScore(beatmapCallbacksUpdater);
    leftCombo = 0;
    rightCombo = 0;
    combo = 0;
    health = GetHealth(scoreController);
    songTime = 0;
    songLength = GetSongLength(scoreController);
    notesLeftCut = 0;
    notesRightCut = 0;
    notesLeftBadCut = 0;
    notesRightBadCut = 0;
    notesLeftMissed = 0;
    notesRightMissed = 0;
    bombsLeftHit = 0;
    bombsRightHit = 0;
    wallsHit = 0;
    songNotes = GetNoteCount(beatmapCallbacksUpdater);
    leftPreSwing = 0;
    rightPreSwing = 0;
    leftPostSwing = 0;
    rightPostSwing = 0;
    leftAccuracy = 0;
    rightAccuracy = 0;
    leftTimeDependence = 0;
    rightTimeDependence = 0;
    leftSpeeds = {};
    rightSpeeds = {};
    noFail = false;
    // GetNegativeMods sets noFail
    positiveMods = GetPositiveMods(scoreController);
    negativeMods = GetNegativeMods(scoreController);
    personalBest = GetHighScore(playerDataModel, gameplayCoreInstaller);
    fails = GetFailCount(playerDataModel);

    getLogger().debug("modifiers %.2f -%.2f", positiveMods, negativeMods);

    if (beatmap != lastBeatmap)
        restarts = 0;
    lastBeatmap = beatmap;

    colors = gameplayCoreInstaller && gameplayCoreInstaller->sceneSetupData ? gameplayCoreInstaller->sceneSetupData->colorScheme : nullptr;
    beatmapData = beatmapCallbacksUpdater ? (BeatmapData*) beatmapCallbacksUpdater->beatmapCallbacksController->beatmapData : nullptr;

    leftMissedMaxScore = 0;
    rightMissedMaxScore = 0;
    leftMissedFixedScore = 0;
    rightMissedFixedScore = 0;
}

bool Qounters::ShouldProcessNote(NoteData* data) {
    bool shouldProcess = false;
    switch (data->gameplayType) {
        case NoteData::GameplayType::Normal:
            shouldProcess = true;
            break;
        case NoteData::GameplayType::BurstSliderHead:
            shouldProcess = true;
            break;
        default:
            break;
    }
    if (!hasCJD)
        return shouldProcess;
    return shouldProcess; // TODO
}
