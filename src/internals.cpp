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
    auto bcc = updater->_beatmapCallbacksController;
    auto songTime = bcc->_startFilterTime;
    //TODO: fix this
    /*auto noteDataItemsList = (LinkedList*) ((BeatmapData*) bcc->_beatmapData)->_beatmapDataItemsPerTypeAndId->GetList(csTypeOf(NoteData*), 0)->get_items();
    auto enumerator = (LinkedList::Enumerator*) noteDataItemsList->System_Collections_IEnumerable_GetEnumerator();
    while(enumerator->MoveNext()) {
        auto noteData = (NoteData*) enumerator->System_Collections_IEnumerator_get_Current();
        if(ShouldProcessNote(noteData) && noteData->get_time() > songTime)
            noteCount++;
    }*/
    return noteCount;
}

#include "GlobalNamespace/ScoreModel.hpp"

int GetMaxScore(BeatmapCallbacksUpdater* updater) {
    if (!updater || !updater->_beatmapCallbacksController)
        return 0;
    return ScoreModel::ComputeMaxMultipliedScoreForBeatmap(updater->_beatmapCallbacksController->_beatmapData);
}

#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "UnityEngine/AudioClip.hpp"

float GetSongLength(ScoreController* controller) {
    if (!controller)
        return 0;
    return controller->_audioTimeSyncController->_initData->audioClip->get_length();
}

#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerAllOverallStatsData.hpp"

int GetFailCount(PlayerDataModel* data) {
    return 0;
    if (!data)
        return 0;
    return data->playerData->playerAllOverallStatsData->get_allOverallStatsData()->failedLevelsCount;
}

#include "GlobalNamespace/GameplayModifierParamsSO.hpp"
#include "beatsaber-hook/shared/utils/typedefs-list.hpp"

float GetPositiveMods(ScoreController* controller) {
    return 0;
    if (!controller || !controller->_gameplayModifierParams)
        return 0;
    float ret = 0;
    auto mods = ListW<GameplayModifierParamsSO*>(controller->_gameplayModifierParams);
    for (auto& mod : mods) {
        float mult = mod->get_multiplier();
        if (mult > 0)
            ret += mult;
    }
    return ret;
}

#include "GlobalNamespace/GameplayModifiersModelSO.hpp"

float GetNegativeMods(ScoreController* controller) {
    return 0;
    if (!controller || !controller->_gameplayModifierParams)
        return 0;
    float ret = 0;
    auto mods = ListW<GameplayModifierParamsSO*>(controller->_gameplayModifierParams);
    for (auto& mod : mods) {
        if (mod == controller->_gameplayModifiersModel->_noFailOn0Energy.ptr()) {
            noFail = true;
            continue;
        }
        float mult = mod->get_multiplier();
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
    return 0;
    if (!data || !installer || !installer->_sceneSetupData)
        return 0;
    auto [id, characteristic, difficulty] = Utils::GetBeatmapDetails(installer->_sceneSetupData->difficultyBeatmap);
    for (auto& stats : ListW<PlayerLevelStatsData*>(data->playerData->levelsStatsData)) {
        getLogger().info("ptr %p", stats);
        if(stats != nullptr) {
            if (stats->_levelID == id && stats->beatmapCharacteristic->serializedName == characteristic && stats->difficulty.value__ == difficulty)
                return stats->get_highScore();
        }
    }
    return -1;
}

#include "GlobalNamespace/IGameEnergyCounter.hpp"
#include "GlobalNamespace/GameEnergyCounter.hpp"

float GetHealth(ScoreController* controller) {
    if (!controller)
        return 1;
    return controller->_gameEnergyCounter->get_energy();
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
    std::vector<float> leftAngles;
    std::vector<float> rightAngles;
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
        if (installer->get_isActiveAndEnabled() && installer->_sceneSetupData != nullptr) {
            gameplayCoreInstaller = installer;
            break;
        }
    }

    if (!gameplayCoreInstaller) gameplayCoreInstaller = gameplayCoreInstallers[0];

    std::string beatmap = "Unknown";
    if (gameplayCoreInstaller && gameplayCoreInstaller->_sceneSetupData)
        beatmap = Utils::GetBeatmapIdentifier(gameplayCoreInstaller->_sceneSetupData->difficultyBeatmap);

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
    leftAngles = {};
    rightAngles = {};
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

    colors = gameplayCoreInstaller && gameplayCoreInstaller->_sceneSetupData ? gameplayCoreInstaller->_sceneSetupData->colorScheme : nullptr;
    beatmapData = beatmapCallbacksUpdater && beatmapCallbacksUpdater->_beatmapCallbacksController ? (BeatmapData*) beatmapCallbacksUpdater->_beatmapCallbacksController->_beatmapData : nullptr;

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
