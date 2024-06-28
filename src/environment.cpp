#include "environment.hpp"

#include "BeatSaber/GameSettings/GraphicSettingsHandler.hpp"
#include "BeatmapDataLoaderVersion4/BeatmapDataLoader.hpp"
#include "BeatmapSaveDataVersion3/BpmChangeEventData.hpp"
#include "BeatmapSaveDataVersion4/LightshowSaveData.hpp"
#include "GlobalNamespace/BasicBeatmapEventData.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/BeatmapCallbacksUpdater.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDataSortedListForTypeAndIds_1.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapLevelExtensions.hpp"
#include "GlobalNamespace/BeatmapLevelSO.hpp"
#include "GlobalNamespace/BeatmapLevelsModel.hpp"
#include "GlobalNamespace/BpmTimeProcessor.hpp"
#include "GlobalNamespace/ColorSchemeSO.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/EnvironmentColorType.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/EnvironmentKeywords.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/FadeInOutController.hpp"
#include "GlobalNamespace/GameScenesManager.hpp"
#include "GlobalNamespace/IBeatmapLevelLoader.hpp"
#include "GlobalNamespace/ISortedList_1.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/LightColorBeatmapEventData.hpp"
#include "GlobalNamespace/LightColorGroup.hpp"
#include "GlobalNamespace/LightColorGroupEffectManager.hpp"
#include "GlobalNamespace/LightGroup.hpp"
#include "GlobalNamespace/MenuEnvironmentManager.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/MockPlayer.hpp"
#include "GlobalNamespace/MockPlayerSettings.hpp"
#include "GlobalNamespace/MultiplayerController.hpp"
#include "GlobalNamespace/MultiplayerIntroAnimationController.hpp"
#include "GlobalNamespace/MultiplayerLevelCompletionResults.hpp"
#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/MultiplayerPlayerResultsData.hpp"
#include "GlobalNamespace/MultiplayerPlayersManager.hpp"
#include "GlobalNamespace/MultiplayerResultsData.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataFileManagerSO.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/RecordingToolManager.hpp"
#include "GlobalNamespace/SimpleLevelStarter.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/UIKeyboardManager.hpp"
#include "GlobalNamespace/VRRenderingParamsSetup.hpp"
#include "HMUI/ViewController.hpp"
#include "System/Action_1.hpp"
#include "System/Action_2.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Nullable_1.hpp"
#include "System/Threading/CancellationToken.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "UnityEngine/AddressableAssets/AssetReferenceT_1.hpp"
#include "UnityEngine/JsonUtility.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/TextAsset.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "Zenject/DiContainer.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "config.hpp"
#include "custom-types/shared/delegate.hpp"
#include "customtypes/components.hpp"
#include "customtypes/settings.hpp"
#include "internals.hpp"
#include "main.hpp"
#include "playtest.hpp"
#include "qounters.hpp"
#include "utils.hpp"

// #include "GlobalNamespace/CoreGameHUDController.hpp"

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

SimpleLevelStarter* GetLevelStarter() {
    return Resources::FindObjectsOfTypeAll<SimpleLevelStarter*>()->Last();
}

void Present(SimpleLevelStarter* levelStarter, bool refresh, ScenesTransitionSetupDataSO* setupData, EnvironmentInfoSO* environment = nullptr) {
    auto startDelegate = custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>(
        (std::function<void(Zenject::DiContainer*)>) [environment](Zenject::DiContainer*) { Qounters::OnSceneStart(environment); }
    );

    if (refresh)
        levelStarter->_gameScenesManager->ReplaceScenes(setupData, nullptr, 0.25, nullptr, startDelegate);
    else
        levelStarter->_gameScenesManager->PushScenes(setupData, 0.25, nullptr, startDelegate);
}

void PresentMultiplayer(SimpleLevelStarter* levelStarter, bool refresh, BeatmapLevel* level, BeatmapKey diff, ColorScheme* colors) {
    auto setupData = levelStarter->_menuTransitionsHelper->_multiplayerLevelScenesTransitionSetupData;
    auto levelLoader = levelStarter->_menuTransitionsHelper->_beatmapLevelsModel->levelLoader;
    auto levelData = levelLoader->LoadBeatmapLevelDataAsync(level, System::Threading::CancellationToken::get_None());
    BSML::MainThreadScheduler::ScheduleUntil(
        std::function<bool()>([levelData]() { return levelData->GetAwaiter().IsCompleted; }),
        [levelData, diff, level, colors, levelStarter, refresh, setupData]() mutable {
            auto beatmapLevelData = levelData->Result.beatmapLevelData;
            setupData->Init(
                "Settings",
                diff,
                level,
                beatmapLevelData,
                colors,
                levelStarter->_gameplayModifiers,
                levelStarter->_playerDataModel->playerData->playerSpecificSettings,
                nullptr,
                levelStarter->_menuTransitionsHelper->_audioClipAsyncLoader,
                levelStarter->_menuTransitionsHelper->_graphicSettingsHandler->_currentPreset,
                levelStarter->_menuTransitionsHelper->_beatmapDataLoader,
                false
            );
            localFakeConnectedPlayer = nullptr;

            Present(levelStarter, refresh, setupData);
        }
    );
}

void PresentSingleplayer(SimpleLevelStarter* levelStarter, bool refresh, BeatmapLevel* level, BeatmapKey diff, ColorScheme* colors) {
    auto env = OverrideEnvironmentSettings::New_ctor();

    auto dataManager = levelStarter->_playerDataModel->playerDataFileModel;
    auto listModel = dataManager->_environmentsListModel;

    env->overrideEnvironments = true;
    EnvironmentInfoSO* environment = nullptr;
    for (auto& info : listModel->_envInfos) {
        if (info->environmentName == getConfig().Environment.GetValue()) {
            environment = info;
            break;
        }
    }
    if (!environment) {
        environment = listModel->GetFirstEnvironmentInfoWithType(EnvironmentType::Normal);
        logger.warn("Environment {} not found, resetting to {}", getConfig().Environment.GetValue(), environment->environmentName);
        getConfig().Environment.SetValue(environment->environmentName);
    }
    env->SetEnvironmentInfoForType(environment->environmentType, environment);

    if (colors == nullptr) {
        colors = environment->colorScheme->colorScheme;
    }

    auto setupData = levelStarter->_menuTransitionsHelper->_standardLevelScenesTransitionSetupData;
    setupData->Init(
        "Settings",
        diff,
        level,
        env,
        colors,
        nullptr,
        levelStarter->_gameplayModifiers,
        levelStarter->_playerDataModel->playerData->playerSpecificSettings,
        nullptr,
        levelStarter->_environmentsListModel,
        levelStarter->_menuTransitionsHelper->_audioClipAsyncLoader,
        levelStarter->_menuTransitionsHelper->_beatmapDataLoader,
        levelStarter->_menuTransitionsHelper->_graphicSettingsHandler->_currentPreset,
        "",
        levelStarter->_menuTransitionsHelper->_beatmapLevelsModel,
        false,
        false,
        System::Nullable_1<RecordingToolManager::SetupData>()
    );

    Present(levelStarter, refresh, setupData, environment);
}

void PresentScene(SimpleLevelStarter* levelStarter, bool refresh) {
    // TODO: create custom beatmap level for cleaner code and fix 360 env override
    auto levelSO = !levelStarter->_beatmapLevel->IsValid() ? levelStarter->_beatmapLevel->LoadAssetAsync().WaitForCompletion().ptr()
                                                           : (BeatmapLevelSO*) levelStarter->_beatmapLevel->Asset.ptr();
    auto level = BeatmapLevelExtensions::ToRuntime(levelSO);
    auto diff = BeatmapKey(levelStarter->_beatmapCharacteristic, levelStarter->_beatmapDifficulty, level->levelID);

    ColorScheme* colors = nullptr;
    currentColors = getConfig().ColorScheme.GetValue();
    auto colorSchemeSettings = levelStarter->_playerDataModel->playerData->colorSchemesSettings;
    if (currentColors == "User Override / Environment" || !colorSchemeSettings->_colorSchemesDict->ContainsKey(currentColors)) {
        auto isOverride = colorSchemeSettings->overrideDefaultColors;
        colors = isOverride ? colorSchemeSettings->GetOverrideColorScheme() : nullptr;
    } else if (currentColors != "Environment Default")
        colors = colorSchemeSettings->GetColorSchemeForId(currentColors);

    logger.debug("Presenting scene");
    logger.debug("level {}", fmt::ptr(level));
    logger.debug("level info {} {} {}", level->levelID, (int) diff.difficulty, diff.beatmapCharacteristic->serializedName);

    currentEnvironment = getConfig().Environment.GetValue();
    if (currentEnvironment == "Multiplayer")
        PresentMultiplayer(levelStarter, refresh, level, diff, colors);
    else
        PresentSingleplayer(levelStarter, refresh, level, diff, colors);
}

void Qounters::PresentSettingsEnvironment() {
    logger.debug("Presenting environment");

    if (getConfig().Presets.GetValue().empty()) {
        logger.error("No presets!");
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

void DismissFlowCoordinator() {
    auto mainFlow = GameObject::Find("MainFlowCoordinator")->GetComponent<HMUI::FlowCoordinator*>();
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    mainFlow->DismissFlowCoordinator(settingsFlow, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, true);
}

void Qounters::DismissSettingsEnvironment() {
    logger.debug("Dismissing environment");

    Reset();

    inSettings = false;

    DismissFlowCoordinator();

    auto endDelegate =
        custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>((std::function<void(Zenject::DiContainer*)>) [](Zenject::DiContainer*) {
            Qounters::OnSceneEnd();
        });
    GetLevelStarter()->_gameScenesManager->PopScenes(0.25, nullptr, endDelegate);
}

void Qounters::RefreshSettingsEnvironment() {
    logger.debug("Refreshing environment");

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

void OnMultiplayerSceneStart(MultiplayerController* multiplayerController) {
    auto fakeConnectedPlayers = ListW<IConnectedPlayer*>::New(1);
    auto settings = MockPlayerSettings::New_ctor();
    settings->userId = "qounters_settings_player";
    settings->userName = settings->userId;
    auto player = MockPlayer::New_ctor(settings, true);
    player->isReady = true;
    player->wasActiveAtLevelStart = true;
    player->isActive = true;
    localFakeConnectedPlayer = player->i___GlobalNamespace__IConnectedPlayer();
    fakeConnectedPlayers->Add(localFakeConnectedPlayer);

    multiplayerController->_playersManager->SpawnPlayers(
        MultiplayerPlayerStartState::InSync, fakeConnectedPlayers->i___System__Collections__Generic__IReadOnlyList_1_T_()
    );
    multiplayerController->_introAnimationController->TransitionToAfterIntroAnimationState();

    GameObject::Find("WaitingForOtherPlayersEnvironment")->active = false;
    GameObject::Find("MultiplayerOtherPlayersScoreDiffTextManager")->active = false;
    GameObject::Find("MultiplayerPositionHUD")->active = false;

    auto activeObjects = GameObject::Find("IsActiveObjects")->transform;
    Utils::FindRecursive(activeObjects, "Origin")->gameObject->active = false;

    Utils::FindRecursive(activeObjects, "MenuControllers")->gameObject->active = true;
    auto wrapper = Utils::FindRecursive(activeObjects, "MenuWrapper");
    wrapper->gameObject->active = true;
    wrapper->Find("Canvas")->gameObject->active = false;
}

void Qounters::OnSceneStart(EnvironmentInfoSO* environment) {
    logger.info("Settings scene start");

    if (auto multiplayerController = Object::FindObjectOfType<MultiplayerController*>())
        OnMultiplayerSceneStart(multiplayerController);

    // Resources::FindObjectsOfTypeAll<CoreGameHUDController*>().First()->gameObject->active = true;
    Initialize();
    SetupObjects();

    menuEnvironment->transform->root->gameObject->active = true;
    songPreview->CrossfadeToDefault();
    vrInput->gameObject->active = false;

    auto newInput = Resources::FindObjectsOfTypeAll<VRUIControls::VRInputModule*>()->FirstOrDefault([](auto x) { return x != vrInput; });

    if (newInput) {
        keyboardManager->_vrInputModule = newInput;
        keyboardManager->Start();
        vrInput->_vrPointer->_leftVRController->gameObject->active = false;
        vrInput->_vrPointer->_rightVRController->gameObject->active = false;
    }

    menuEnv->active = false;

    auto mainFlow = GameObject::Find("MainFlowCoordinator")->GetComponent<HMUI::FlowCoordinator*>();
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    if (!mainFlow->IsFlowCoordinatorInHierarchy(settingsFlow))
        mainFlow->PresentFlowCoordinator(settingsFlow, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, true, false);

    logger.debug("Fixing environment lighting");

    // rendering order issues
    auto env = GameObject::Find("Environment");
    env->active = false;
    env->active = true;

    std::vector<BeatmapEventData*> eventData = std::vector<BeatmapEventData*>();
    if (environment) {
        if (environment->defaultLightshowAsset) {
            logger.debug("Loading v3 lightshow data");
            // Should we convert the save data ourselves? This is insane
            auto envKeywords = EnvironmentKeywords::New_ctor(environment->environmentKeywords);
            auto envLightGroups = environment->environmentLightGroups;
            auto lightShowJSON = environment->defaultLightshowAsset->text;
            auto lightShowSaveData = JsonUtility::FromJson<BeatmapSaveDataVersion4::LightshowSaveData*>(lightShowJSON);
            auto bpmTimeProcessor = BpmTimeProcessor::New_ctor(
                69.420, ListW<BeatmapSaveDataVersion3::BpmChangeEventData*>::New()->i___System__Collections__Generic__IReadOnlyList_1_T_()
            );

            auto lightShowBeatmapData = BeatmapData::New_ctor(4);
            lightShowBeatmapData->updateAllBeatmapDataOnInsert = true;

            BeatmapDataLoaderVersion4::BeatmapDataLoader::LoadLightshow(
                lightShowBeatmapData, lightShowSaveData, bpmTimeProcessor, envKeywords, envLightGroups
            );

            auto events = lightShowBeatmapData->_allBeatmapData;
            auto eventsLinkedList = events->items;
            auto currentEvent = eventsLinkedList->First;
            for (int i = 0; i < events->count; i++) {
                auto event = il2cpp_utils::try_cast<BeatmapEventData>(currentEvent->Value);
                if (event)
                    eventData.push_back(*event);
                if (currentEvent != eventsLinkedList->Last)
                    currentEvent = currentEvent->Next;
                else
                    break;
            }

            logger.debug("Loaded defaults");
        }
    }

    if (auto bcu = Object::FindObjectOfType<BeatmapCallbacksUpdater*>()) {
        auto bcc = bcu->_beatmapCallbacksController;

        // V2
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event0, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event1, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event2, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event3, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event4, 1, 1));

        // V3
        for (auto& event : eventData) {
            if (event)
                bcc->TriggerBeatmapEvent(event);
        }
    }

    auto renderParams = Object::FindObjectOfType<VRRenderingParamsSetup*>();
    renderParams->_sceneType = SceneType::Menu;
    renderParams->OnEnable();

    logger.debug("Disabling objects");

    if (auto gameplay = GameObject::Find("BeatmapCallbacksUpdater"))
        gameplay->SetActive(false);

    GameObject::Find("DisableGCWhileEnabled")->active = false;

    if (auto bts = GameObject::Find("BTSEnvironmentCharacterSpawner"))
        bts->SetActive(false); // the game literally just freezes

    Qounters::PlayTest::Setup();
}

void Qounters::OnSceneEnd() {
    logger.info("Settings scene end");

    auto levelStarter = GetLevelStarter();
    levelStarter->_menuTransitionsHelper->_standardLevelScenesTransitionSetupData->didFinishEvent = nullptr;
    levelStarter->_gameScenesManager->_neverUnloadScenes->Remove("MenuCore");
    menuEnvironment->ShowEnvironmentType(MenuEnvironmentManager::MenuEnvironmentType::Default);
    songPreview->CrossfadeToDefault();
    vrInput->gameObject->active = true;
    vrInput->_vrPointer->_leftVRController->gameObject->active = true;
    vrInput->_vrPointer->_rightVRController->gameObject->active = true;
    keyboardManager->OnDestroy();
    keyboardManager->_vrInputModule = vrInput;
    menuEnv->active = true;
    Object::FindObjectOfType<FadeInOutController*>()->FadeIn();
    localFakeConnectedPlayer = nullptr;
}
