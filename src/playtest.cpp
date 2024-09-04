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

static ScoreController* scoreController;
static AudioTimeSyncController* audioController;
static ComboController* comboController;
static GameEnergyCounter* gameEnergyCounter;
static GameEnergyUIPanel* energyBar;
static BeatmapObjectSpawnController* spawner;

static SafePtr<ScoreMultiplierCounter> maxMultiplier;

static void FindObjects() {
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
    Internals::DoSlowUpdate();
    Events::Broadcast((int) Events::Update);
}

void Playtest::SetEnabled(bool enabled) {
    Environment::SetPlayerActive(enabled);
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

static void ResetEnergyBar() {
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

static int PositiveMultiplier() {
    if (!maxMultiplier)
        maxMultiplier = ScoreMultiplierCounter::New_ctor();
    maxMultiplier->ProcessMultiplierEvent(ScoreMultiplierCounter::MultiplierEventType::Positive);
    return maxMultiplier->multiplier;
}

void Playtest::SpawnNote(bool left, bool chain) {
    if (!spawner)
        return;

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
        Internals::songNotesLeft++;
    else
        Internals::songNotesRight++;

    Internals::songMaxScore += ScoreModel::GetNoteScoreDefinition(data->scoringType)->maxCutScore * PositiveMultiplier();
    for (int i = 0; i < chainSegments; i++)
        Internals::songMaxScore += segmentMaxCut * PositiveMultiplier();
    if (Internals::personalBest != -1)
        SetPersonalBest(0);
    else
        Events::Broadcast((int) Qounters::Events::MapInfo);
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
    Internals::leftScore = 0;
    Internals::rightScore = 0;
    Internals::leftMaxScore = 0;
    Internals::rightMaxScore = 0;
    Internals::songMaxScore = 0;
    Internals::leftCombo = 0;
    Internals::rightCombo = 0;
    Internals::combo = 0;
    Internals::health = 0.5;
    Internals::notesLeftCut = 0;
    Internals::notesRightCut = 0;
    Internals::notesLeftBadCut = 0;
    Internals::notesRightBadCut = 0;
    Internals::notesLeftMissed = 0;
    Internals::notesRightMissed = 0;
    Internals::bombsLeftHit = 0;
    Internals::bombsRightHit = 0;
    Internals::wallsHit = 0;
    Internals::songNotesLeft = 0;
    Internals::songNotesRight = 0;
    Internals::leftPreSwing = 0;
    Internals::rightPreSwing = 0;
    Internals::leftPostSwing = 0;
    Internals::rightPostSwing = 0;
    Internals::leftAccuracy = 0;
    Internals::rightAccuracy = 0;
    Internals::leftTimeDependence = 0;
    Internals::rightTimeDependence = 0;
    Internals::leftMissedMaxScore = 0;
    Internals::rightMissedMaxScore = 0;
    Internals::leftMissedFixedScore = 0;
    Internals::rightMissedFixedScore = 0;
    Events::Broadcast((int) Qounters::Events::ScoreChanged);
    Events::Broadcast((int) Qounters::Events::NoteCut);
    Events::Broadcast((int) Qounters::Events::NoteMissed);
    Events::Broadcast((int) Qounters::Events::BombCut);
    Events::Broadcast((int) Qounters::Events::WallHit);
    Events::Broadcast((int) Qounters::Events::ComboChanged);
    Events::Broadcast((int) Qounters::Events::HealthChanged);
    if (Internals::personalBest != -1)
        SetPersonalBest(0);
    else
        Events::Broadcast((int) Qounters::Events::MapInfo);
    if (maxMultiplier)
        maxMultiplier->Reset();
    PlaytestViewController::GetInstance()->UpdateUI();
}

void Playtest::ResetAll() {
    ResetGameControllers();
    Internals::Initialize();
    PP::blSongValid = false;
    settingsStarsBL = 10;
    PP::ssSongValid = false;
    settingsStarsSS = 10;
    SetPersonalBest(-1);
    HUD::UpdateAllSources();
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
        Internals::personalBest = -1;
    else
        Internals::personalBest = Game::GetSongMaxScore() * value / 100;
    Events::Broadcast((int) Qounters::Events::MapInfo);
}

void Playtest::SetSongTime(float value) {
    Internals::songTime = value;
    Events::Broadcast((int) Qounters::Events::Update);
};

void Playtest::SetPositiveModifiers(float value) {
    Internals::positiveMods = 1 + (value * 0.01);
    Events::Broadcast((int) Qounters::Events::ScoreChanged);
};

void Playtest::SetNegativeModifiers(float value) {
    Internals::negativeMods = 1 + (value * 0.01);
    Events::Broadcast((int) Qounters::Events::ScoreChanged);
};

void Playtest::SetRankedBL(bool value) {
    PP::blSongValid = value;
    Events::Broadcast((int) Qounters::Events::MapInfo);
};

void Playtest::SetStarsBL(float value) {
    settingsStarsBL = value;
    Events::Broadcast((int) Qounters::Events::MapInfo);
};

void Playtest::SetRankedSS(bool value) {
    PP::ssSongValid = value;
    Events::Broadcast((int) Qounters::Events::MapInfo);
};

void Playtest::SetStarsSS(float value) {
    settingsStarsSS = value;
    Events::Broadcast((int) Qounters::Events::MapInfo);
};
