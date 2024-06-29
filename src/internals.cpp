#include "internals.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/BeatmapCallbacksUpdater.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataSortedListForTypeAndIds_1.hpp"
#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/GameplayModifierParamsSO.hpp"
#include "GlobalNamespace/GameplayModifiersModelSO.hpp"
#include "GlobalNamespace/IGameEnergyCounter.hpp"
#include "GlobalNamespace/ISortedList_1.hpp"
#include "GlobalNamespace/PlayerAllOverallStatsData.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerLevelStatsData.hpp"
#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/ScoreModel.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Collections/IEnumerator.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/Resources.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils-classes.hpp"
#include "main.hpp"
#include "utils.hpp"

using namespace Qounters;

std::string lastBeatmap;

using namespace GlobalNamespace;

int GetNoteCount(BeatmapCallbacksUpdater* updater) {
    if (!updater)
        return 0;
    using LinkedList = System::Collections::Generic::LinkedList_1<NoteData*>;
    int noteCount = 0;
    auto bcc = updater->_beatmapCallbacksController;
    auto songTime = bcc->_startFilterTime;
    auto data = il2cpp_utils::try_cast<BeatmapData>(bcc->_beatmapData).value_or(nullptr);
    if (!data) {
        logger.warn("IReadonlyBeatmapData was {} not BeatmapData", il2cpp_functions::class_get_name(((Il2CppObject*) bcc->_beatmapData)->klass));
        return 0;
    }
    auto noteDataItemsList = (LinkedList*) data->_beatmapDataItemsPerTypeAndId->GetList(csTypeOf(NoteData*), 0)->items;
    auto enumerator = noteDataItemsList->GetEnumerator();
    while (enumerator.MoveNext()) {
        auto noteData = (NoteData*) enumerator.Current;
        if (ShouldProcessNote(noteData) && noteData->time > songTime)
            noteCount++;
    }
    return noteCount;
}

int GetMaxScore(BeatmapCallbacksUpdater* updater) {
    if (!updater)
        return 0;
    return ScoreModel::ComputeMaxMultipliedScoreForBeatmap(updater->_beatmapCallbacksController->_beatmapData);
}

float GetSongLength(ScoreController* controller) {
    if (!controller)
        return 0;
    return controller->_audioTimeSyncController->_initData->audioClip->length;
}

int GetFailCount(PlayerDataModel* data) {
    if (!data)
        return 0;
    return data->playerData->playerAllOverallStatsData->get_allOverallStatsData()->failedLevelsCount;
}

float GetPositiveMods(ScoreController* controller) {
    if (!controller || !controller->_gameplayModifierParams)
        return 0;
    float ret = 0;
    auto mods = ListW<GameplayModifierParamsSO*>(controller->_gameplayModifierParams);
    for (auto& mod : mods) {
        float mult = mod->multiplier;
        if (mult > 0)
            ret += mult;
    }
    return ret;
}

float GetNegativeMods(ScoreController* controller) {
    if (!controller || !controller->_gameplayModifierParams)
        return 0;
    float ret = 0;
    auto mods = ListW<GameplayModifierParamsSO*>(controller->_gameplayModifierParams);
    for (auto& mod : mods) {
        if (mod == controller->_gameplayModifiersModel->_noFailOn0Energy.ptr()) {
            noFail = true;
            continue;
        }
        float mult = mod->multiplier;
        if (mult < 0)
            ret += mult;
    }
    return ret;
}

int GetHighScore(PlayerDataModel* data, GameplayCoreInstaller* installer) {
    if (!data || !installer || !installer->_sceneSetupData)
        return 0;
    auto beatmap = installer->_sceneSetupData->beatmapKey;
    if (!data->playerData->levelsStatsData->ContainsKey(beatmap))
        return -1;
    return data->playerData->levelsStatsData->get_Item(beatmap)->highScore;
}

float GetHealth(ScoreController* controller) {
    if (!controller)
        return 1;
    return controller->_gameEnergyCounter->energy;
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
    if (!gameplayCoreInstaller)
        gameplayCoreInstaller = gameplayCoreInstallers[0];

    std::string beatmap = "Unknown";
    if (gameplayCoreInstaller && gameplayCoreInstaller->_sceneSetupData)
        beatmap = Utils::GetBeatmapIdentifier(gameplayCoreInstaller->_sceneSetupData->beatmapKey);

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

    logger.debug("modifiers {} -{}", positiveMods, negativeMods);

    if (beatmap != lastBeatmap)
        restarts = 0;
    lastBeatmap = beatmap;

    colors = gameplayCoreInstaller && gameplayCoreInstaller->_sceneSetupData ? gameplayCoreInstaller->_sceneSetupData->colorScheme : nullptr;
    beatmapData = beatmapCallbacksUpdater ? (BeatmapData*) beatmapCallbacksUpdater->_beatmapCallbacksController->_beatmapData : nullptr;

    leftMissedMaxScore = 0;
    rightMissedMaxScore = 0;
    leftMissedFixedScore = 0;
    rightMissedFixedScore = 0;
}

bool Qounters::ShouldProcessNote(NoteData* data) {
    // check first for noodle
    if (data->scoringType == NoteData::ScoringType::NoScore || data->scoringType == NoteData::ScoringType::Ignore)
        return false;
    if (data->gameplayType == NoteData::GameplayType::Normal || data->gameplayType == NoteData::GameplayType::BurstSliderHead)
        return true;
    return false;
}
