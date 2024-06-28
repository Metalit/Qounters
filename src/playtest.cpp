#include "playtest.hpp"

#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/SaberManager.hpp"
#include "UnityEngine/EventSystems/zzzz__EventSystem_def.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/WaitForSeconds.hpp"
#include "UnityEngine/WaitForEndOfFrame.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Transform.hpp"
#include "custom-types/shared/coroutine.hpp"
#include "customtypes/settings.hpp"
#include "editor.hpp"

namespace Qounters::PlayTest {
    ListW<GlobalNamespace::NoteData*> sequenceData;

    GlobalNamespace::BeatmapObjectSpawnController* beatmapObjectSpawnController;
    GlobalNamespace::AudioTimeSyncController* audioTimeSyncController;

    UnityEngine::GameObject* eventSystem;
    UnityEngine::GameObject* menuControllers;
    GlobalNamespace::SaberManager* sabers;

    custom_types::Helpers::Coroutine WaitFrame() {
        co_yield nullptr;
        co_yield nullptr;
        co_yield nullptr;

        audioTimeSyncController->StopSong();
    }

    void Setup() {
        beatmapObjectSpawnController = UnityEngine::Object::FindObjectOfType<GlobalNamespace::BeatmapObjectSpawnController*>();
        audioTimeSyncController = UnityEngine::Object::FindObjectOfType<GlobalNamespace::AudioTimeSyncController*>();

        eventSystem = UnityEngine::Object::FindObjectOfType<UnityEngine::EventSystems::EventSystem*>(true)->gameObject;
        eventSystem->SetActive(true);
        menuControllers = UnityEngine::GameObject::Find("MenuControllers");
        sabers = UnityEngine::Object::FindObjectOfType<GlobalNamespace::SaberManager*>();

        menuControllers->SetActive(true);
        menuControllers->transform->GetChild(0)->gameObject->SetActive(true);
        menuControllers->transform->GetChild(1)->gameObject->SetActive(true);
        sabers->disableSabers = true;

        sabers->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(WaitFrame()));
    }

    custom_types::Helpers::Coroutine WaitForSequence() {
        co_yield reinterpret_cast<System::Collections::IEnumerator*>(UnityEngine::WaitForSeconds::New_ctor(7.0f));

        audioTimeSyncController->StopSong();

        menuControllers->SetActive(true);
        sabers->disableSabers = true;
        
        Qounters::SettingsViewController::GetInstance()->playtestButton->interactable = Qounters::Editor::GetPreviewMode();
    }

    void SpawnSequence() {
        if(!beatmapObjectSpawnController || !audioTimeSyncController) 
            Setup();

        menuControllers->SetActive(false);
        sabers->disableSabers = false;

        audioTimeSyncController->StartSong(0.0f);
        audioTimeSyncController->HandleAudioConfigurationChanged(false);

        //We have to remake the sequence each time as the game will not respawn NoteDatas that have been spawned already.
        if(!sequenceData) {
            sequenceData = ListW<GlobalNamespace::NoteData*>::New();

            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(1.5f, 1, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorA, GlobalNamespace::NoteCutDirection::Down));
            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(2.0f, 1, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorA, GlobalNamespace::NoteCutDirection::Up));
            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(2.5f, 1, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorA, GlobalNamespace::NoteCutDirection::Down));
            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(3.0f, 1, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorA, GlobalNamespace::NoteCutDirection::Up));
            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(3.5f, 1, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorA, GlobalNamespace::NoteCutDirection::Down));

            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(4.0f, 2, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorB, GlobalNamespace::NoteCutDirection::Down));
            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(4.5f, 2, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorB, GlobalNamespace::NoteCutDirection::Up));
            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(5.0f, 2, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorB, GlobalNamespace::NoteCutDirection::Down));
            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(5.5f, 2, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorB, GlobalNamespace::NoteCutDirection::Up));
            sequenceData.push_back(GlobalNamespace::NoteData::CreateBasicNoteData(6.0f, 2, GlobalNamespace::NoteLineLayer::Base, GlobalNamespace::ColorType::ColorB, GlobalNamespace::NoteCutDirection::Down));
        }

        for(auto noteData : sequenceData) {
            beatmapObjectSpawnController->HandleNoteDataCallback(noteData);
        }

        sabers->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(WaitForSequence()));

        Qounters::SettingsViewController::GetInstance()->playtestButton->interactable = false;
    }
}