#include "playtest.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/ComboController.hpp"
#include "GlobalNamespace/GameEnergyCounter.hpp"
#include "GlobalNamespace/GameEnergyUIPanel.hpp"
#include "GlobalNamespace/IBeatmapObjectController.hpp"
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
#include "main.hpp"
#include "metacore/shared/events.hpp"
#include "metacore/shared/internals.hpp"
#include "metacore/shared/stats.hpp"
#include "pp.hpp"
#include "qounters.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace MetaCore;
using namespace GlobalNamespace;
namespace MEvents = MetaCore::Events;

static GameEnergyUIPanel* energyBar;
static BeatmapObjectSpawnController* spawner;

static float lastPbValue = 1;
static SafePtr<ScoreMultiplierCounter> maxMultiplier;

static void FindObjects() {
    energyBar = UnityEngine::Object::FindObjectOfType<GameEnergyUIPanel*>(false);
    spawner = UnityEngine::Resources::FindObjectsOfTypeAll<BeatmapObjectSpawnController*>()->First();
}

static void DespawnObjects() {
    if (!spawner)
        return;
    if (auto cast = Utils::ptr_cast<BeatmapObjectManager>(spawner->_beatmapObjectSpawner)) {
        ListW<IBeatmapObjectController*> objects = cast->_allBeatmapObjects;
        std::vector<IBeatmapObjectController*> copy = {objects.begin(), objects.end()};
        for (auto& object : copy) {
            object->Pause(false);
            if (auto controller = il2cpp_utils::try_cast<NoteController>(object))
                cast->Despawn(*controller);
            else if (auto controller = il2cpp_utils::try_cast<ObstacleController>(object))
                cast->Despawn(*controller);
            else if (auto controller = il2cpp_utils::try_cast<SliderController>(object))
                cast->Despawn(*controller);
        }
    }
}

void Playtest::Update() {
    if (Internals::referencesValid)
        Internals::audioTimeSyncController->_songTime += UnityEngine::Time::get_deltaTime();
    Internals::DoSlowUpdate();
    MEvents::Broadcast(MEvents::Update);
}

void Playtest::SetEnabled(bool enabled) {
    Environment::SetPlayerActive(enabled);
    if (enabled)
        FindObjects();
    DespawnObjects();
    if (!enabled) {
        energyBar = nullptr;
        spawner = nullptr;
    }
}

static void ResetEnergyBar() {
    if (!energyBar || !energyBar->isActiveAndEnabled || energyBar->_energyBar->enabled)
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
    if (!Internals::referencesValid)
        return;
    Internals::scoreController->_modifiedScore = 0;
    Internals::scoreController->_multipliedScore = 0;
    Internals::scoreController->_immediateMaxPossibleModifiedScore = 1;
    Internals::scoreController->_immediateMaxPossibleMultipliedScore = 0;
    Internals::scoreController->_scoreMultiplierCounter->Reset();
    Internals::scoreController->_maxScoreMultiplierCounter->Reset();
    Internals::gameEnergyCounter->_didReach0Energy = false;
    Internals::gameEnergyCounter->energy = 0.5;
    if (energyBar)
        energyBar->RefreshEnergyUI(0.5);
    ResetEnergyBar();
    Internals::comboController->_combo = 0;
    Internals::comboController->_maxCombo = 0;
    // don't despawn elements as they might still be event recievers, so just make them do nothing to the score instead
    for (auto element : ListW<ScoringElement*>(Internals::scoreController->_sortedScoringElementsWithoutMultiplier))
        Internals::scoreController->_scoringElementsWithMultiplier->Add(element);
    Internals::scoreController->_sortedScoringElementsWithoutMultiplier->Clear();
    for (auto element : ListW<ScoringElement*>(Internals::scoreController->_scoringElementsWithMultiplier))
        element->SetMultipliers(0, 0);
    DespawnObjects();
}

static int PositiveMultiplier() {
    if (!maxMultiplier)
        maxMultiplier = ScoreMultiplierCounter::New_ctor();
    maxMultiplier->ProcessMultiplierEvent(ScoreMultiplierCounter::MultiplierEventType::Positive);
    return maxMultiplier->multiplier;
}

void Playtest::SpawnNote(bool left, bool chain) {
    if (!spawner || !Internals::referencesValid)
        return;

    static int const chainSegments = 3;
    static int const segmentMaxCut = ScoreModel::GetNoteScoreDefinition(NoteData::ScoringType::ChainLink)->maxCutScore;

    float time = Internals::audioTimeSyncController->songTime + 2;
    int index = left ? 1 : 2;
    auto layer = chain ? NoteLineLayer::Upper : NoteLineLayer::Base;
    auto color = left ? ColorType::ColorA : ColorType::ColorB;
    auto data = NoteData::CreateBasicNoteData(time, 0, 0, index, layer, color, NoteCutDirection::Down);

    if (chain) {
        data->ChangeToBurstSliderHead();
        spawner->HandleNoteDataCallback(data);
        auto baseLayer = NoteLineLayer::Base;
        spawner->HandleSliderDataCallback(
            SliderData::CreateBurstSliderData(
                color, time, 0, 0, index, layer, layer, NoteCutDirection::Down, time, 0, index, baseLayer, baseLayer, chainSegments, 1
            )
        );
    } else
        spawner->HandleNoteDataCallback(data);

    // chain links not counted
    if (left) {
        Internals::remainingNotesLeft++;
        Internals::songNotesLeft++;
    } else {
        Internals::remainingNotesRight++;
        Internals::songNotesRight++;
    }

    Internals::songMaxScore += ScoreModel::GetNoteScoreDefinition(data->scoringType)->maxCutScore * PositiveMultiplier();
    for (int i = 0; i < chainSegments; i++)
        Internals::songMaxScore += segmentMaxCut * PositiveMultiplier();
    if (Internals::personalBest != -1)
        SetPersonalBest(0);
    else
        Events::BroadcastQountersEvent(Events::MapInfo);
}

void Playtest::SpawnWall() {
    if (!spawner || !Internals::referencesValid)
        return;
    float time = Internals::audioTimeSyncController->songTime + 2;
    int index = PlaytestViewController::GetInstance()->transform->position.x < 0 ? 3 : 0;
    spawner->HandleObstacleDataCallback(
        GlobalNamespace::ObstacleData::New_ctor(time, 0, 0, 0, index, GlobalNamespace::NoteLineLayer::Top, 0.25, 1, 2)
    );
}

void Playtest::SpawnBomb() {
    if (!spawner || !Internals::referencesValid)
        return;
    float time = Internals::audioTimeSyncController->songTime + 2;
    for (int index = 0; index < 4; index++)
        spawner->HandleNoteDataCallback(GlobalNamespace::NoteData::CreateBombNoteData(time, 0, 0, index, GlobalNamespace::NoteLineLayer::Base));
}

static void ResetNoteTrackers() {
    Internals::leftScore = 0;
    Internals::rightScore = 0;
    Internals::leftMaxScore = 0;
    Internals::rightMaxScore = 0;
    Internals::songMaxScore = 0;
    Internals::leftCombo = 0;
    Internals::rightCombo = 0;
    Internals::combo = 0;
    Internals::highestLeftCombo = 0;
    Internals::highestRightCombo = 0;
    Internals::highestCombo = 0;
    Internals::multiplier = 1;
    Internals::multiplierProgress = 0;
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
    Internals::remainingNotesLeft = 0;
    Internals::remainingNotesRight = 0;
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
}

void Playtest::ResetNotes() {
    ResetGameControllers();
    ResetNoteTrackers();
    MEvents::Broadcast(MEvents::ScoreChanged);
    MEvents::Broadcast(MEvents::NoteCut);
    MEvents::Broadcast(MEvents::NoteMissed);
    MEvents::Broadcast(MEvents::BombCut);
    MEvents::Broadcast(MEvents::WallHit);
    MEvents::Broadcast(MEvents::ComboChanged);
    MEvents::Broadcast(MEvents::HealthChanged);
    if (Internals::personalBest != -1)
        SetPersonalBest(0);
    else
        Events::BroadcastQountersEvent(Events::MapInfo);
    if (maxMultiplier)
        maxMultiplier->Reset();
    PlaytestViewController::GetInstance()->UpdateUI();
}

void Playtest::ResetAll() {
    ResetNoteTrackers();
    Internals::songTime = 0;
    Internals::noFail = false;
    Internals::positiveMods = 0;
    Internals::negativeMods = 0;
    PP::blSongValid = false;
    settingsStarsBL = 10;
    PP::ssSongValid = false;
    settingsStarsSS = 10;
    SetPersonalBest(-1);
    lastPbValue = 1;
    HUD::UpdateAllSources();
    if (maxMultiplier)
        maxMultiplier->Reset();
    PlaytestViewController::GetInstance()->UpdateUI();
}

// -1: no pb, 0: use last pb, >0: set pb percent
void Playtest::SetPersonalBest(float value) {
    if (value == 0)
        value = lastPbValue;
    else if (value > 0)
        lastPbValue = value;
    if (value == -1)
        Internals::personalBest = -1;
    else
        Internals::personalBest = Stats::GetSongMaxScore() * value / 100;
    Events::BroadcastQountersEvent(Events::MapInfo);
}

void Playtest::SetSongTime(float value) {
    Internals::songTime = value;
    MEvents::Broadcast(MEvents::Update);
};

void Playtest::SetPositiveModifiers(float value) {
    Internals::positiveMods = value * 0.01;
    MEvents::Broadcast(MEvents::ScoreChanged);
};

void Playtest::SetNegativeModifiers(float value) {
    Internals::negativeMods = value * 0.01;
    MEvents::Broadcast(MEvents::ScoreChanged);
};

void Playtest::SetRankedBL(bool value) {
    PP::blSongValid = value;
    Events::BroadcastQountersEvent(Events::MapInfo);
};

void Playtest::SetStarsBL(float value) {
    settingsStarsBL = value;
    Events::BroadcastQountersEvent(Events::MapInfo);
};

void Playtest::SetRankedSS(bool value) {
    PP::ssSongValid = value;
    Events::BroadcastQountersEvent(Events::MapInfo);
};

void Playtest::SetStarsSS(float value) {
    settingsStarsSS = value;
    Events::BroadcastQountersEvent(Events::MapInfo);
};

float Playtest::GetOverridePBRatio() {
    return Internals::personalBest == -1 ? -1 : lastPbValue / 100;
}
