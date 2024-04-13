#include "config.hpp"
#include "environment.hpp"
#include "events.hpp"
#include "internals.hpp"
#include "main.hpp"
#include "playtest.hpp"
#include "qounters.hpp"
#include "utils.hpp"
#include "customtypes/settings.hpp"
#include "customtypes/components.hpp"

#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/MenuEnvironmentManager.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "GlobalNamespace/UIKeyboardManager.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace Qounters;

MenuEnvironmentManager* menuEnvironment;
SongPreviewPlayer* songPreview;
VRUIControls::VRInputModule* vrInput;
UIKeyboardManager* keyboardManager;
GameObject* menuEnv;
bool inSettings = false;
std::string currentEnvironment = "";
std::string currentColors = "";

#include "GlobalNamespace/SimpleLevelStarter.hpp"

SimpleLevelStarter* GetLevelStarter() {
    return Resources::FindObjectsOfTypeAll<SimpleLevelStarter*>()->Last();
}

#include "GlobalNamespace/GameScenesManager.hpp"

#include "System/Action_1.hpp"
#include "Zenject/DiContainer.hpp"
#include "custom-types/shared/delegate.hpp"

void Present(SimpleLevelStarter* levelStarter, bool refresh, ScenesTransitionSetupDataSO* setupData, EnvironmentInfoSO* environment = nullptr) {
    auto startDelegate = custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>((std::function<void (Zenject::DiContainer*)>) [environment](Zenject::DiContainer*) {
        Qounters::OnSceneStart(environment);
    });

    if (refresh)
        levelStarter->_gameScenesManager->ReplaceScenes(setupData, nullptr, 0.25, nullptr, startDelegate);
    else
        levelStarter->_gameScenesManager->PushScenes(setupData, 0.25, nullptr, startDelegate);
}

#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/MultiplayerResultsData.hpp"
#include "GlobalNamespace/MultiplayerPlayerResultsData.hpp"
#include "GlobalNamespace/MultiplayerLevelCompletionResults.hpp"
#include "GlobalNamespace/RecordingToolManager.hpp"
#include "GlobalNamespace/IBeatmapLevelLoader.hpp"
#include "GlobalNamespace/BeatmapLevelsModel.hpp"

#include "System/Threading/CancellationToken.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Action_2.hpp"
#include "System/Nullable_1.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"

void PresentMultiplayer(SimpleLevelStarter* levelStarter, bool refresh, BeatmapLevel* level, BeatmapKey diff, ColorScheme* colors) {
    auto setupData = levelStarter->_menuTransitionsHelper->_multiplayerLevelScenesTransitionSetupData;
    auto levelLoader = levelStarter->_menuTransitionsHelper->_beatmapLevelsModel->levelLoader;
    //TODO: In the future, make a custom type inheriting IBeatmapLevelData and return hardcoded beatmaps, this sucks -Future
    auto levelData = levelLoader->LoadBeatmapLevelDataAsync(level, System::Threading::CancellationToken::get_None()); // FUCK
    BSML::MainThreadScheduler::ScheduleUntil(std::function<bool ()>([&](){
        return levelData->GetAwaiter().IsCompleted;
    }), [&](){
        auto beatmapLevelData = levelData->Result.beatmapLevelData;
        setupData->Init("Settings", diff, level, beatmapLevelData, colors, levelStarter->_gameplayModifiers, levelStarter->_playerDataModel->playerData->playerSpecificSettings, nullptr, levelStarter->_menuTransitionsHelper->_audioClipAsyncLoader, levelStarter->_menuTransitionsHelper->_beatmapDataLoader, false);
        localFakeConnectedPlayer = nullptr;

        Present(levelStarter, refresh, setupData);
    });
}

#include "GlobalNamespace/PlayerDataFileManagerSO.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/BpmTimeProcessor.hpp"
#include "GlobalNamespace/EnvironmentKeywords.hpp"
#include "GlobalNamespace/ISortedList_1.hpp"
#include "GlobalNamespace/BeatmapDataSortedListForTypeAndIds_1.hpp"
#include "GlobalNamespace/ColorSchemeSO.hpp"
#include "UnityEngine/TextAsset.hpp"
#include "UnityEngine/JsonUtility.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"
#include "BeatmapSaveDataVersion4/LightshowSaveData.hpp"
#include "BeatmapSaveDataVersion3/BpmChangeEventData.hpp"
#include "BeatmapDataLoaderVersion4/BeatmapDataLoader.hpp"

void PresentSingleplayer(SimpleLevelStarter* levelStarter, bool refresh, BeatmapLevel* level, BeatmapKey diff, ColorScheme* colors) {
    auto env = OverrideEnvironmentSettings::New_ctor();

    auto dataManager = levelStarter->_playerDataModel->playerDataFileModel;
    auto listModel = dataManager->_environmentsListModel;

    env->overrideEnvironments = true;
    EnvironmentInfoSO* environment = nullptr;
    ListW<UnityW<EnvironmentInfoSO>> environments = ListW<UnityW<EnvironmentInfoSO>>::New();
    environments->AddRange(listModel->GetAllEnvironmentInfosWithType(GlobalNamespace::EnvironmentType::Normal)->i___System__Collections__Generic__IEnumerable_1_T_());
    environments->AddRange(listModel->GetAllEnvironmentInfosWithType(GlobalNamespace::EnvironmentType::Circle)->i___System__Collections__Generic__IEnumerable_1_T_());
    for (auto& info : environments) {
        if (info->environmentName == getConfig().Environment.GetValue()) {
            environment = info;
            break;
        }
    }
    if (!environment) {
        environment = listModel->GetFirstEnvironmentInfoWithType(GlobalNamespace::EnvironmentType::Normal);
        getConfig().Environment.SetValue(environment->environmentName);
    }
    env->SetEnvironmentInfoForType(environment->environmentType, environment);

    if (colors == nullptr) {
        colors = environment->colorScheme->colorScheme;
    }

    auto setupData = levelStarter->_menuTransitionsHelper->_standardLevelScenesTransitionSetupData;
    setupData->Init("Settings", diff, level, env, colors, nullptr, levelStarter->_gameplayModifiers, levelStarter->_playerDataModel->playerData->playerSpecificSettings, nullptr, levelStarter->_environmentsListModel, levelStarter->_menuTransitionsHelper->_audioClipAsyncLoader, levelStarter->_menuTransitionsHelper->_beatmapDataLoader, "", levelStarter->_menuTransitionsHelper->_beatmapLevelsModel, false, false, System::Nullable_1<RecordingToolManager::SetupData>());

    Present(levelStarter, refresh, setupData, environment);
}

#include "GlobalNamespace/BeatmapLevelSO.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/BeatmapLevelExtensions.hpp"
#include "UnityEngine/AddressableAssets/AssetReferenceT_1.hpp"

void PresentScene(SimpleLevelStarter* levelStarter, bool refresh) {
    auto levelSO = !levelStarter->_beatmapLevel->IsValid() ? levelStarter->_beatmapLevel->LoadAssetAsync().WaitForCompletion().ptr() : reinterpret_cast<BeatmapLevelSO*>(levelStarter->_beatmapLevel->Asset.ptr()); // This is GROSS and I DO NOT care, maybe we can just make this ourselves instead of relying on the game. -Future
    auto level = BeatmapLevelExtensions::ToRuntime(levelSO);
    auto diff = BeatmapKey(levelStarter->_beatmapCharacteristic, levelStarter->_beatmapDifficulty, level->levelID);

    ColorScheme* colors = nullptr;
    currentColors = getConfig().ColorScheme.GetValue();
    auto colorSchemeSettings = levelStarter->_playerDataModel->playerData->colorSchemesSettings;
    if (currentColors != "Environment Default")
        colors = colorSchemeSettings->GetColorSchemeForId(currentColors);

    QountersLogger::Logger.debug("Presenting scene");
    QountersLogger::Logger.debug("level {}", fmt::ptr(level));
    QountersLogger::Logger.debug("level info {} {} {}", level->levelID, diff.difficulty.value__, diff.beatmapCharacteristic->serializedName);

    currentEnvironment = getConfig().Environment.GetValue();
    if (currentEnvironment == "Multiplayer")
        PresentMultiplayer(levelStarter, refresh, level, diff, colors);
    else
        PresentSingleplayer(levelStarter, refresh, level, diff, colors);
}

#include "System/Collections/Generic/HashSet_1.hpp"

void Qounters::PresentSettingsEnvironment() {
    QountersLogger::Logger.debug("Presenting environment");

    if (getConfig().Presets.GetValue().empty()) {
        QountersLogger::Logger.error("No presets!");
        return;
    }

    inSettings = true;

    menuEnvironment = Object::FindObjectOfType<MenuEnvironmentManager*>();
    songPreview = Object::FindObjectOfType<SongPreviewPlayer*>();
    vrInput = Object::FindObjectOfType<VRUIControls::VRInputModule*>();
    keyboardManager = Object::FindObjectOfType<UIKeyboardManager*>();
    menuEnv = GameObject::Find("MenuEnvironmentCore");

    auto levelStarter = GetLevelStarter();
    levelStarter->_gameScenesManager->_neverUnloadScenes->Add("MenuCore");
    menuEnvironment->ShowEnvironmentType(MenuEnvironmentManager::MenuEnvironmentType::None);
    PresentScene(levelStarter, false);
}

#include "HMUI/ViewController.hpp"

void DismissFlowCoordinator() {
    auto mainFlow = GameObject::Find("MainFlowCoordinator")->GetComponent<HMUI::FlowCoordinator*>();
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    mainFlow->DismissFlowCoordinator(settingsFlow, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, true);
}

#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/MultiplayerController.hpp"

void Qounters::DismissSettingsEnvironment() {
    QountersLogger::Logger.debug("Dismissing environment");

    Reset();

    inSettings = false;

    DismissFlowCoordinator();

    auto endDelegate = custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>((std::function<void (Zenject::DiContainer*)>) [](Zenject::DiContainer*) {
        Qounters::OnSceneEnd();
    });
    GetLevelStarter()->_gameScenesManager->PopScenes(0.25, nullptr, endDelegate);
}

void Qounters::RefreshSettingsEnvironment() {
    QountersLogger::Logger.debug("Refreshing environment");

    Reset();

    DismissFlowCoordinator();

    PresentScene(GetLevelStarter(), true);
}

bool Qounters::InSettingsEnvironment() {
    return inSettings;
}

std::string Qounters::CurrentSettingsEnvironment() {
    return currentEnvironment;
}

std::string Qounters::CurrentColorScheme() {
    return currentColors;
}

#include "GlobalNamespace/MultiplayerController.hpp"
#include "GlobalNamespace/MockPlayer.hpp"
#include "GlobalNamespace/MockPlayerSettings.hpp"
#include "GlobalNamespace/MultiplayerPlayersManager.hpp"
#include "GlobalNamespace/MultiplayerIntroAnimationController.hpp"

void OnMultiplayerSceneStart(MultiplayerController* multiplayerController) {
    auto fakeConnectedPlayers = System::Collections::Generic::List_1<IConnectedPlayer*>::New_ctor(1);
    auto settings = MockPlayerSettings::New_ctor();
    settings->userId = "qounters_settings_player";
    settings->userName = settings->userId;
    auto player = MockPlayer::New_ctor(settings, true);
    player->set_isReady(true);
    player->set_wasActiveAtLevelStart(true);
    player->set_isActive(true);
    localFakeConnectedPlayer = player->i___GlobalNamespace__IConnectedPlayer();
    fakeConnectedPlayers->Add(localFakeConnectedPlayer);

    multiplayerController->_playersManager->SpawnPlayers(MultiplayerPlayerStartState::InSync, fakeConnectedPlayers->i___System__Collections__Generic__IReadOnlyList_1_T_());
    multiplayerController->_introAnimationController->TransitionToAfterIntroAnimationState();

    GameObject::Find("WaitingForOtherPlayersEnvironment")->SetActive(false);
    GameObject::Find("MultiplayerOtherPlayersScoreDiffTextManager")->SetActive(false);
    GameObject::Find("MultiplayerPositionHUD")->SetActive(false);

    auto activeObjects = GameObject::Find("IsActiveObjects")->get_transform();
    Utils::FindRecursive(activeObjects, "Origin")->get_gameObject()->SetActive(false);

    Utils::FindRecursive(activeObjects, "MenuControllers")->get_gameObject()->SetActive(true);
    auto wrapper = Utils::FindRecursive(activeObjects, "MenuWrapper");
    wrapper->get_gameObject()->SetActive(true);
    wrapper->Find("Canvas")->get_gameObject()->SetActive(false);
}

// #include "GlobalNamespace/CoreGameHUDController.hpp"
#include "GlobalNamespace/BeatmapCallbacksUpdater.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/BasicBeatmapEventData.hpp"
#include "GlobalNamespace/VRRenderingParamsSetup.hpp"
#include "GlobalNamespace/LightColorBeatmapEventData.hpp"
#include "GlobalNamespace/EnvironmentColorType.hpp"
#include "GlobalNamespace/LightColorGroupEffectManager.hpp"
#include "GlobalNamespace/LightColorGroup.hpp"
#include "GlobalNamespace/LightGroup.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/NoteData.hpp"


void Qounters::OnSceneStart(EnvironmentInfoSO* environment) {
    QountersLogger::Logger.info("Settings scene start");

    if (auto multiplayerController = Object::FindObjectOfType<MultiplayerController*>())
        OnMultiplayerSceneStart(multiplayerController);

    // Resources::FindObjectsOfTypeAll<CoreGameHUDController*>().First()->get_gameObject()->SetActive(true);
    Initialize();
    SetupObjects();

    menuEnvironment->get_transform()->get_root()->get_gameObject()->SetActive(true);
    songPreview->CrossfadeToDefault();
    vrInput->get_gameObject()->SetActive(false);

    auto newInput = Resources::FindObjectsOfTypeAll<VRUIControls::VRInputModule*>()->FirstOrDefault([](auto x) { return x != vrInput; });

    if (newInput) {
        keyboardManager->_vrInputModule = newInput;
        keyboardManager->Start();
    }

    menuEnv->SetActive(false);

    auto mainFlow = GameObject::Find("MainFlowCoordinator")->GetComponent<HMUI::FlowCoordinator*>();
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    if (!mainFlow->IsFlowCoordinatorInHierarchy(settingsFlow))
        mainFlow->PresentFlowCoordinator(settingsFlow, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, true, false);

    QountersLogger::Logger.debug("Fixing environment lighting");

    // rendering order issues
    auto env = GameObject::Find("Environment");
    env->SetActive(false);
    env->SetActive(true);
    
    std::vector<BeatmapEventData*> eventData = std::vector<BeatmapEventData*>();
    if (environment) {
        if (environment->defaultLightshowAsset) {
            QountersLogger::Logger.debug("Loading v3 lightshow data");
            //Should we convert the save data ourselves? This is insane
            auto envKeywords = EnvironmentKeywords::New_ctor(environment->environmentKeywords);
            auto envLightGroups = environment->environmentLightGroups;
            auto lightShowJSON = environment->defaultLightshowAsset->text;
            auto lightShowSaveData = JsonUtility::FromJson<BeatmapSaveDataVersion4::LightshowSaveData*>(lightShowJSON);
            auto bpmTimeProcessor = BpmTimeProcessor::New_ctor(69.420f, ListW<BeatmapSaveDataVersion3::BpmChangeEventData*>::New()->i___System__Collections__Generic__IReadOnlyList_1_T_());

            auto lightShowBeatmapData = BeatmapData::New_ctor(4);
            lightShowBeatmapData->set_updateAllBeatmapDataOnInsert(true);

            BeatmapDataLoaderVersion4::BeatmapDataLoader::LoadLightshow(lightShowBeatmapData, lightShowSaveData, bpmTimeProcessor, envKeywords, envLightGroups);
            
            auto events = lightShowBeatmapData->_allBeatmapData;
            auto eventsLinkedList = events->items;
            auto currentEvent = eventsLinkedList->First;
            for (size_t i = 0; i < events->count; i++)
            {
                auto event = il2cpp_utils::try_cast<BeatmapEventData>(currentEvent->Value);
                if (event) {
                    auto eventDataObject = event.value();
                    eventData.push_back(eventDataObject);
                }
                if (currentEvent != eventsLinkedList->Last) 
                    currentEvent = currentEvent->Next;
                else
                    break;
            }

            QountersLogger::Logger.debug("Loaded defaults");
        }
    }

    if (auto bcu = Object::FindObjectOfType<BeatmapCallbacksUpdater*>()) {
        auto bcc = bcu->_beatmapCallbacksController;
        
        //V2
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event0, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event1, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event2, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event3, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event4, 1, 1));

        //V3
        if (eventData.size() > 0) {
            for (size_t i = 0; i < eventData.size(); i++)
            {
                auto event = eventData[i];
                if(event) {
                    bcc->TriggerBeatmapEvent(event);
                }
            }
        }
    }

    auto renderParams = Object::FindObjectOfType<VRRenderingParamsSetup*>();
    renderParams->_sceneType = VRRenderingParamsSetup::SceneType::Menu;
    renderParams->OnEnable();

    QountersLogger::Logger.debug("Disabling objects");

    if (auto gameplay = GameObject::Find("BeatmapCallbacksUpdater"))
        gameplay->SetActive(false);

    GameObject::Find("DisableGCWhileEnabled")->SetActive(false);

    if (auto bts = GameObject::Find("BTSEnvironmentCharacterSpawner"))
        bts->SetActive(false); // the game literally just freezes

    Qounters::PlayTest::Setup();
}

#include "GlobalNamespace/FadeInOutController.hpp"

void Qounters::OnSceneEnd() {
    QountersLogger::Logger.info("Settings scene end");

    auto levelStarter = GetLevelStarter();
    levelStarter->_menuTransitionsHelper->_standardLevelScenesTransitionSetupData->didFinishEvent = nullptr;
    levelStarter->_gameScenesManager->_neverUnloadScenes->Remove("MenuCore");
    menuEnvironment->ShowEnvironmentType(MenuEnvironmentManager::MenuEnvironmentType::Default);
    songPreview->CrossfadeToDefault();
    vrInput->get_gameObject()->SetActive(true);
    keyboardManager->OnDestroy();
    keyboardManager->_vrInputModule = vrInput;
    menuEnv->SetActive(true);
    Object::FindObjectOfType<FadeInOutController*>()->FadeIn();
    localFakeConnectedPlayer = nullptr;
}
