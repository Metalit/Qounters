#include "playtest.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/ComboController.hpp"
#include "GlobalNamespace/GameEnergyCounter.hpp"
#include "GlobalNamespace/GameEnergyUIPanel.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/PrepareLevelCompletionResults.hpp"
#include "GlobalNamespace/ScoreController.hpp"
#include "GlobalNamespace/ScoreModel.hpp"
#include "GlobalNamespace/ScoreMultiplierCounter.hpp"
#include "GlobalNamespace/ScoringElement.hpp"
#include "GlobalNamespace/SliderData.hpp"
#include "HMUI/ImageView.hpp"
#include "System/Action_1.hpp"
#include "UnityEngine/Playables/PlayableDirector.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Time.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "customtypes/settings.hpp"
#include "environment.hpp"
#include "events.hpp"
#include "game.hpp"
#include "internals.hpp"
#include "main.hpp"
#include "pp.hpp"
#include "qounters.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace GlobalNamespace;

ScoreController* scoreController;
AudioTimeSyncController* audioController;
ComboController* comboController;
GameEnergyCounter* gameEnergyCounter;
GameEnergyUIPanel* energyBar;
BeatmapObjectSpawnController* spawner;

SafePtr<ScoreMultiplierCounter> maxMultiplier;

void FindObjects() {
    auto hasOtherObjects = UnityEngine::Resources::FindObjectsOfTypeAll<PrepareLevelCompletionResults*>()->First();
    scoreController = Utils::ptr_cast<ScoreController>(hasOtherObjects->_scoreController);
    if (!scoreController) {
        logger.error(
            "score controller was wrong type! {}", il2cpp_utils::ClassStandardName(((Il2CppObject*) hasOtherObjects->_scoreController)->klass, true)
        );
        return;
    }
    audioController = scoreController->_audioTimeSyncController;
    comboController = hasOtherObjects->_comboController;
    gameEnergyCounter = hasOtherObjects->_gameEnergyCounter;
    energyBar = UnityEngine::Object::FindObjectOfType<GameEnergyUIPanel*>(false);
    spawner = UnityEngine::Resources::FindObjectsOfTypeAll<BeatmapObjectSpawnController*>()->First();
}

void Playtest::Update() {
    if (audioController)
        audioController->_songTime += UnityEngine::Time::get_deltaTime();
    DoSlowUpdate();
    BroadcastEvent((int) Events::Update);
}

void Playtest::SetEnabled(bool enabled) {
    SetPlayerActive(enabled);
    if (!enabled) {
        scoreController = nullptr;
        audioController = nullptr;
        comboController = nullptr;
        gameEnergyCounter = nullptr;
        energyBar = nullptr;
        spawner = nullptr;
        return;
    } else
        FindObjects();
    if (!spawner)
        return;
    if (auto cast = Utils::ptr_cast<BeatmapObjectManager>(spawner->_beatmapObjectSpawner))
        cast->DissolveAllObjects();
}

void ResetEnergyBar() {
    if (!energyBar->isActiveAndEnabled)
        return;
    static auto RebindPlayableGraphOutputs = il2cpp_utils::resolve_icall<void, UnityEngine::Playables::PlayableDirector*>(
        "UnityEngine.Playables.PlayableDirector::RebindPlayableGraphOutputs"
    );
    energyBar->_playableDirector->Stop();
    RebindPlayableGraphOutputs(energyBar->_playableDirector);
    energyBar->_playableDirector->Evaluate();
    energyBar->_energyBar->enabled = true;
    static auto opacityColor = UnityEngine::Color(1, 1, 1, 0.3);
    auto emptyIcon = energyBar->transform->Find("EnergyIconEmpty");
    emptyIcon->localPosition = {-59, 0, 0};
    emptyIcon->GetComponent<HMUI::ImageView*>()->color = opacityColor;
    auto fullIcon = energyBar->transform->Find("EnergyIconFull");
    fullIcon->localPosition = {59, 0, 0};
    fullIcon->GetComponent<HMUI::ImageView*>()->color = opacityColor;
    energyBar->transform->Find("Laser")->gameObject->active = false;
    // reregister HandleGameEnergyDidChange
    energyBar->Init();
}

void Playtest::ResetGameControllers() {
    if (!scoreController)
        return;
    scoreController->_modifiedScore = 0;
    scoreController->_multipliedScore = 0;
    scoreController->_immediateMaxPossibleModifiedScore = 1;
    scoreController->_immediateMaxPossibleMultipliedScore = 0;
    scoreController->_scoreMultiplierCounter->Reset();
    scoreController->_maxScoreMultiplierCounter->Reset();
    gameEnergyCounter->_didReach0Energy = false;
    gameEnergyCounter->energy = 0.5;
    gameEnergyCounter->gameEnergyDidChangeEvent->Invoke(0.5);
    if (energyBar && !energyBar->_energyBar->enabled)
        ResetEnergyBar();
    comboController->_combo = 0;
    comboController->_maxCombo = 0;
    // don't despawn elements as they might still be event recievers, so just make them do nothing to the score instead
    for (auto element : ListW<ScoringElement*>(scoreController->_sortedScoringElementsWithoutMultiplier))
        scoreController->_scoringElementsWithMultiplier->Add(element);
    scoreController->_sortedScoringElementsWithoutMultiplier->Clear();
    for (auto element : ListW<ScoringElement*>(scoreController->_scoringElementsWithMultiplier))
        element->SetMultipliers(0, 0);
}

int PositiveMultiplier() {
    if (!maxMultiplier)
        maxMultiplier = ScoreMultiplierCounter::New_ctor();
    maxMultiplier->ProcessMultiplierEvent(ScoreMultiplierCounter::MultiplierEventType::Positive);
    return maxMultiplier->multiplier;
}

void Playtest::SpawnNote(bool left, bool chain) {
    if (!spawner)
        return;

    using namespace GlobalNamespace;
    static int const chainSegments = 3;
    static int const segmentMaxCut = ScoreModel::GetNoteScoreDefinition(NoteData::ScoringType::BurstSliderElement)->maxCutScore;

    float time = audioController->songTime + 2;
    int index = left ? 1 : 2;
    auto layer = chain ? NoteLineLayer::Upper : NoteLineLayer::Base;
    auto color = left ? ColorType::ColorA : ColorType::ColorB;
    auto data = NoteData::CreateBasicNoteData(time, index, layer, color, NoteCutDirection::Down);

    if (chain) {
        data->ChangeToBurstSliderHead();
        spawner->HandleNoteDataCallback(data);
        auto baseLayer = NoteLineLayer::Base;
        spawner->HandleSliderDataCallback(SliderData::CreateBurstSliderData(
            color, time, index, layer, layer, NoteCutDirection::Down, time, index, baseLayer, baseLayer, chainSegments, 1
        ));
    } else
        spawner->HandleNoteDataCallback(data);

    // chain links not counted
    if (left)
        songNotesLeft++;
    else
        songNotesRight++;

    songMaxScore += ScoreModel::GetNoteScoreDefinition(data->scoringType)->maxCutScore * PositiveMultiplier();
    for (int i = 0; i < chainSegments; i++)
        songMaxScore += segmentMaxCut * PositiveMultiplier();
    if (personalBest != -1)
        SetPersonalBest(0);
    else
        BroadcastEvent((int) Qounters::Events::MapInfo);
}

void Playtest::SpawnWall() {
    if (!spawner)
        return;
    float time = audioController->songTime + 2;
    int index = PlaytestViewController::GetInstance()->transform->position.x < 0 ? 3 : 0;
    spawner->HandleObstacleDataCallback(GlobalNamespace::ObstacleData::New_ctor(time, index, GlobalNamespace::NoteLineLayer::Top, 0.25, 1, 2));
}

void Playtest::SpawnBomb() {
    if (!spawner)
        return;
    float time = audioController->songTime + 2;
    for (int index = 0; index < 4; index++)
        spawner->HandleNoteDataCallback(GlobalNamespace::NoteData::CreateBombNoteData(time, index, GlobalNamespace::NoteLineLayer::Base));
}

void Playtest::ResetNotes() {
    ResetGameControllers();
    leftScore = 0;
    rightScore = 0;
    leftMaxScore = 0;
    rightMaxScore = 0;
    songMaxScore = 0;
    leftCombo = 0;
    rightCombo = 0;
    combo = 0;
    health = 0.5;
    notesLeftCut = 0;
    notesRightCut = 0;
    notesLeftBadCut = 0;
    notesRightBadCut = 0;
    notesLeftMissed = 0;
    notesRightMissed = 0;
    bombsLeftHit = 0;
    bombsRightHit = 0;
    wallsHit = 0;
    songNotesLeft = 0;
    songNotesRight = 0;
    leftPreSwing = 0;
    rightPreSwing = 0;
    leftPostSwing = 0;
    rightPostSwing = 0;
    leftAccuracy = 0;
    rightAccuracy = 0;
    leftTimeDependence = 0;
    rightTimeDependence = 0;
    leftMissedMaxScore = 0;
    rightMissedMaxScore = 0;
    leftMissedFixedScore = 0;
    rightMissedFixedScore = 0;
    BroadcastEvent((int) Qounters::Events::ScoreChanged);
    BroadcastEvent((int) Qounters::Events::NoteCut);
    BroadcastEvent((int) Qounters::Events::NoteMissed);
    BroadcastEvent((int) Qounters::Events::BombCut);
    BroadcastEvent((int) Qounters::Events::WallHit);
    BroadcastEvent((int) Qounters::Events::ComboChanged);
    BroadcastEvent((int) Qounters::Events::HealthChanged);
    if (personalBest != -1)
        SetPersonalBest(0);
    else
        BroadcastEvent((int) Qounters::Events::MapInfo);
    if (maxMultiplier)
        maxMultiplier->Reset();
    PlaytestViewController::GetInstance()->UpdateUI();
}

void Playtest::ResetAll() {
    ResetGameControllers();
    Initialize();
    PP::blSongValid = false;
    settingsStarsBL = 10;
    PP::ssSongValid = false;
    settingsStarsSS = 10;
    SetPersonalBest(-1);
    UpdateAllSources();
    if (maxMultiplier)
        maxMultiplier->Reset();
    PlaytestViewController::GetInstance()->UpdateUI();
}

// -1: no pb, 0: use last pb, >0: set pb percent
void Playtest::SetPersonalBest(float value) {
    static float lastValue = -1;
    if (value == 0)
        value = lastValue;
    else if (value > 0)
        lastValue = value;
    if (value == -1)
        personalBest = -1;
    else
        personalBest = Game::GetSongMaxScore() * value / 100;
    BroadcastEvent((int) Qounters::Events::MapInfo);
}

void Playtest::SetSongTime(float value) {
    songTime = value;
    BroadcastEvent((int) Qounters::Events::Update);
};

void Playtest::SetPositiveModifiers(float value) {
    positiveMods = 1 + (value * 0.01);
    BroadcastEvent((int) Qounters::Events::ScoreChanged);
};

void Playtest::SetNegativeModifiers(float value) {
    negativeMods = 1 + (value * 0.01);
    BroadcastEvent((int) Qounters::Events::ScoreChanged);
};

void Playtest::SetRankedBL(bool value) {
    PP::blSongValid = value;
    BroadcastEvent((int) Qounters::Events::MapInfo);
};

void Playtest::SetStarsBL(float value) {
    settingsStarsBL = value;
    BroadcastEvent((int) Qounters::Events::MapInfo);
};

void Playtest::SetRankedSS(bool value) {
    PP::ssSongValid = value;
    BroadcastEvent((int) Qounters::Events::MapInfo);
};

void Playtest::SetStarsSS(float value) {
    settingsStarsSS = value;
    BroadcastEvent((int) Qounters::Events::MapInfo);
};
