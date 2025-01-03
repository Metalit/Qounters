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
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/ScoreModel.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Collections/IEnumerator.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Time.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils-classes.hpp"
#include "environment.hpp"
#include "events.hpp"
#include "main.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace GlobalNamespace;

static std::string lastBeatmap;

static int GetNoteCount(BeatmapCallbacksUpdater* updater, bool left) {
    if (!updater)
        return 0;
    using LinkedList = System::Collections::Generic::LinkedList_1<NoteData*>;
    int noteCount = 0;
    auto bcc = updater->_beatmapCallbacksController;
    auto songTime = bcc->_startFilterTime;
    auto data = Utils::ptr_cast<BeatmapData>(bcc->_beatmapData);
    if (!data) {
        logger.warn("IReadonlyBeatmapData was {} not BeatmapData", il2cpp_functions::class_get_name(((Il2CppObject*) bcc->_beatmapData)->klass));
        return 0;
    }
    auto noteDataItemsList = (LinkedList*) data->_beatmapDataItemsPerTypeAndId->GetList(csTypeOf(NoteData*), 0)->items;
    auto enumerator = noteDataItemsList->GetEnumerator();
    while (enumerator.MoveNext()) {
        auto noteData = (NoteData*) enumerator.Current;
        if (Internals::ShouldCountNote(noteData) && noteData->time > songTime && (noteData->colorType == ColorType::ColorA) == left)
            noteCount++;
    }
    return noteCount;
}

static int GetMaxScore(BeatmapCallbacksUpdater* updater) {
    if (!updater)
        return 0;
    return ScoreModel::ComputeMaxMultipliedScoreForBeatmap(updater->_beatmapCallbacksController->_beatmapData);
}

static float GetSongLength(ScoreController* controller) {
    if (!controller)
        return 0;
    return controller->_audioTimeSyncController->_initData->audioClip->length;
}

static int GetFailCount(PlayerDataModel* data) {
    if (!data)
        return 0;
    return data->playerData->playerAllOverallStatsData->get_allOverallStatsData()->failedLevelsCount;
}

static float GetPositiveMods(ScoreController* controller) {
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

static float GetNegativeMods(ScoreController* controller) {
    if (!controller || !controller->_gameplayModifierParams)
        return 0;
    float ret = 0;
    auto mods = ListW<GameplayModifierParamsSO*>(controller->_gameplayModifierParams);
    for (auto& mod : mods) {
        if (mod == controller->_gameplayModifiersModel->_noFailOn0Energy.ptr()) {
            Internals::noFail = true;
            continue;
        }
        float mult = mod->multiplier;
        if (mult < 0)
            ret += mult;
    }
    return ret;
}

static int GetHighScore(PlayerDataModel* data, GameplayCoreInstaller* installer) {
    if (!data || !installer || !installer->_sceneSetupData)
        return -1;
    auto beatmap = installer->_sceneSetupData->beatmapKey;
    if (!data->playerData->levelsStatsData->ContainsKey(beatmap))
        return -1;
    return data->playerData->levelsStatsData->get_Item(beatmap)->highScore;
}

static float GetHealth(ScoreController* controller) {
    if (!controller)
        return 1;
    return controller->_gameEnergyCounter->energy;
}

namespace Qounters::Internals {
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
    int songNotesLeft;
    int songNotesRight;
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
    GameplayModifiers* modifiers;
    float positiveMods;
    float negativeMods;
    double personalBest;
    int fails;
    int restarts;
    ColorScheme* colors;
    BeatmapLevel* beatmapLevel;
    BeatmapKey beatmapKey;
    BeatmapData* beatmapData;
    EnvironmentInfoSO* environment;
    int leftMissedMaxScore;
    int rightMissedMaxScore;
    int leftMissedFixedScore;
    int rightMissedFixedScore;
    float timeSinceSlowUpdate;
    UnityEngine::Quaternion prevRotLeft;
    UnityEngine::Quaternion prevRotRight;
    UnityW<SaberManager> saberManager;
    UnorderedEventCallback<> onInitialize;
}

void Internals::Initialize() {
    auto beatmapCallbacksUpdater = UnityEngine::Object::FindObjectOfType<BeatmapCallbacksUpdater*>(true);
    auto scoreController = UnityEngine::Object::FindObjectOfType<ScoreController*>(true);
    auto playerDataModel = UnityEngine::Object::FindObjectOfType<PlayerDataModel*>(true);

    auto gameplayCoreInstallers = UnityEngine::Resources::FindObjectsOfTypeAll<GameplayCoreInstaller*>();
    GameplayCoreInstaller* gameplayCoreInstaller;
    for (auto& installer : gameplayCoreInstallers) {
        if (installer->isActiveAndEnabled && installer->_sceneSetupData != nullptr) {
            gameplayCoreInstaller = installer;
            break;
        }
    }
    if (!gameplayCoreInstaller)
        gameplayCoreInstaller = gameplayCoreInstallers->FirstOrDefault();
    auto setupData = gameplayCoreInstaller ? gameplayCoreInstaller->_sceneSetupData : nullptr;

    leftScore = 0;
    rightScore = 0;
    leftMaxScore = 0;
    rightMaxScore = 0;
    songMaxScore = Environment::InSettings() ? 1 : GetMaxScore(beatmapCallbacksUpdater);
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
    songNotesLeft = Environment::InSettings() ? 0 : GetNoteCount(beatmapCallbacksUpdater, true);
    songNotesRight = Environment::InSettings() ? 0 : GetNoteCount(beatmapCallbacksUpdater, false);
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
    modifiers = scoreController ? scoreController->_gameplayModifiers : nullptr;
    // GetNegativeMods sets noFail
    positiveMods = GetPositiveMods(scoreController);
    negativeMods = GetNegativeMods(scoreController);
    personalBest = GetHighScore(playerDataModel, gameplayCoreInstaller);
    fails = GetFailCount(playerDataModel);

    logger.debug("modifiers {} -{}", positiveMods, negativeMods);

    std::string beatmap = "Unknown";
    if (setupData)
        beatmap = Utils::GetBeatmapIdentifier(setupData->beatmapKey);
    if (beatmap != lastBeatmap || Environment::InSettings())
        restarts = 0;
    else
        restarts++;
    lastBeatmap = beatmap;

    colors = setupData ? setupData->colorScheme : nullptr;
    beatmapLevel = setupData ? setupData->beatmapLevel : nullptr;
    beatmapKey = setupData ? setupData->beatmapKey : BeatmapKey();
    beatmapData = beatmapCallbacksUpdater ? (BeatmapData*) beatmapCallbacksUpdater->_beatmapCallbacksController->_beatmapData : nullptr;
    environment = setupData ? setupData->environmentInfo : nullptr;

    leftMissedMaxScore = 0;
    rightMissedMaxScore = 0;
    leftMissedFixedScore = 0;
    rightMissedFixedScore = 0;

    timeSinceSlowUpdate = 0;
    prevRotLeft = UnityEngine::Quaternion::get_identity();
    prevRotRight = UnityEngine::Quaternion::get_identity();
    saberManager = UnityEngine::Object::FindObjectOfType<SaberManager*>();

    onInitialize.invoke();
}

bool Internals::IsFakeNote(NoteData* data) {
    return data->scoringType == NoteData::ScoringType::NoScore || data->scoringType == NoteData::ScoringType::Ignore;
}

bool Internals::ShouldCountNote(NoteData* data) {
    if (IsFakeNote(data))
        return false;
    return data->gameplayType == NoteData::GameplayType::Normal || data->gameplayType == NoteData::GameplayType::BurstSliderHead;
}

void Internals::DoSlowUpdate() {
    timeSinceSlowUpdate += UnityEngine::Time::get_deltaTime();
    if (timeSinceSlowUpdate > 1 / (float) SPEED_SAMPLES_PER_SEC) {
        if (saberManager) {
            leftSpeeds.emplace_back(saberManager->leftSaber->bladeSpeed);
            rightSpeeds.emplace_back(saberManager->rightSaber->bladeSpeed);

            auto rotLeft = saberManager->leftSaber->transform->rotation;
            auto rotRight = saberManager->rightSaber->transform->rotation;
            // use speeds array as tracker for if prevRots have accurate values
            if (leftSpeeds.size() > 1) {
                leftAngles.emplace_back(UnityEngine::Quaternion::Angle(rotLeft, prevRotLeft));
                rightAngles.emplace_back(UnityEngine::Quaternion::Angle(rotRight, prevRotRight));
            }
            prevRotLeft = rotLeft;
            prevRotRight = rotRight;
        }
        timeSinceSlowUpdate = 0;
        Events::Broadcast((int) Events::SlowUpdate);
    }
}
