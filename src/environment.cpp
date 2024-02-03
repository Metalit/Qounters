#include "config.hpp"
#include "environment.hpp"
#include "internals.hpp"
#include "main.hpp"
#include "qounters.hpp"
#include "utils.hpp"
#include "customtypes/settings.hpp"
#include "customtypes/components.hpp"

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

#include "GlobalNamespace/SimpleLevelStarter.hpp"

SimpleLevelStarter* GetLevelStarter() {
    return Resources::FindObjectsOfTypeAll<SimpleLevelStarter*>().First([](auto x) {return x->level->levelID == "PerformanceTest";});
}

#include "GlobalNamespace/GameScenesManager.hpp"

#include "System/Action_1.hpp"
#include "Zenject/DiContainer.hpp"
#include "custom-types/shared/delegate.hpp"

void Present(SimpleLevelStarter* levelStarter, bool refresh, ScenesTransitionSetupDataSO* setupData) {
    auto startDelegate = custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>((std::function<void (Zenject::DiContainer*)>) [](Zenject::DiContainer*) {
        Qounters::OnSceneStart();
    });

    if (refresh)
        levelStarter->gameScenesManager->ReplaceScenes(setupData, nullptr, 0.25, nullptr, startDelegate);
    else
        levelStarter->gameScenesManager->PushScenes(setupData, 0.25, nullptr, startDelegate);
}

#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/MultiplayerResultsData.hpp"
#include "GlobalNamespace/MultiplayerPlayerResultsData.hpp"
#include "GlobalNamespace/MultiplayerLevelCompletionResults.hpp"

#include "System/Action_2.hpp"

void PresentMultiplayer(SimpleLevelStarter* levelStarter, bool refresh, IDifficultyBeatmap* diff, ColorScheme* colors) {
    auto setupData = levelStarter->menuTransitionsHelper->multiplayerLevelScenesTransitionSetupData;
    setupData->Init("Settings", (IPreviewBeatmapLevel*) levelStarter->level, levelStarter->beatmapDifficulty, levelStarter->beatmapCharacteristic, diff, colors, levelStarter->gameplayModifiers, levelStarter->playerDataModel->playerData->playerSpecificSettings, nullptr, false);
    localFakeConnectedPlayer = nullptr;

    Present(levelStarter, refresh, setupData);
}

#include "GlobalNamespace/PlayerDataFileManagerSO.hpp"
#include "GlobalNamespace/EnvironmentsListSO.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"

void PresentSingleplayer(SimpleLevelStarter* levelStarter, bool refresh, IDifficultyBeatmap* diff, ColorScheme* colors) {
    auto env = OverrideEnvironmentSettings::New_ctor();
    auto dataManager = levelStarter->playerDataModel->playerDataFileManager;
    env->overrideEnvironments = true;
    EnvironmentInfoSO* environment = nullptr;
    for (auto& info : dataManager->allEnvironmentInfos->environmentInfos) {
        if (info->environmentName == getConfig().Environment.GetValue()) {
            environment = info;
            break;
        }
    }
    if (!environment) {
        environment = dataManager->allEnvironmentInfos->GetFirstEnvironmentInfoWithType(dataManager->normalEnvironmentType);
        getConfig().Environment.SetValue(environment->environmentName);
    }
    env->SetEnvironmentInfoForType(dataManager->normalEnvironmentType, environment);

    auto setupData = levelStarter->menuTransitionsHelper->standardLevelScenesTransitionSetupData;
    setupData->Init("Settings", diff, (IPreviewBeatmapLevel*) levelStarter->level, env, colors, levelStarter->gameplayModifiers, levelStarter->playerDataModel->playerData->playerSpecificSettings, nullptr, "", false, false, nullptr);

    Present(levelStarter, refresh, setupData);
}

#include "GlobalNamespace/BeatmapLevelSO.hpp"
#include "GlobalNamespace/BeatmapLevelDataExtensions.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"

void PresentScene(SimpleLevelStarter* levelStarter, bool refresh) {
    auto diff = BeatmapLevelDataExtensions::GetDifficultyBeatmap(levelStarter->level->beatmapLevelData, levelStarter->beatmapCharacteristic, levelStarter->beatmapDifficulty);
    auto colors = levelStarter->playerDataModel->playerData->colorSchemesSettings->GetOverrideColorScheme();

    currentEnvironment = getConfig().Environment.GetValue();
    if (currentEnvironment == "Multiplayer")
        PresentMultiplayer(levelStarter, refresh, diff, colors);
    else
        PresentSingleplayer(levelStarter, refresh, diff, colors);
}

#include "System/Collections/Generic/HashSet_1.hpp"

void Qounters::PresentSettingsEnvironment() {
    getLogger().debug("Presenting environment");

    if (getConfig().Presets.GetValue().empty()) {
        getLogger().error("No presets!");
        return;
    }

    inSettings = true;

    menuEnvironment = Object::FindObjectOfType<MenuEnvironmentManager*>();
    songPreview = Object::FindObjectOfType<SongPreviewPlayer*>();
    vrInput = Object::FindObjectOfType<VRUIControls::VRInputModule*>();
    keyboardManager = Object::FindObjectOfType<UIKeyboardManager*>();
    menuEnv = GameObject::Find("MenuEnvironmentCore");

    auto levelStarter = GetLevelStarter();
    levelStarter->gameScenesManager->neverUnloadScenes->Add("MenuCore");
    menuEnvironment->ShowEnvironmentType(MenuEnvironmentManager::MenuEnvironmentType::None);
    PresentScene(levelStarter, false);
}

#include "HMUI/ViewController_AnimationDirection.hpp"

void DismissFlowCoordinator() {
    auto mainFlow = GameObject::Find("MainFlowCoordinator")->GetComponent<HMUI::FlowCoordinator*>();
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    mainFlow->DismissFlowCoordinator(settingsFlow, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, true);
}

#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/MultiplayerController.hpp"

void Qounters::DismissSettingsEnvironment() {
    getLogger().debug("Dismissing environment");

    Reset();

    inSettings = false;

    DismissFlowCoordinator();

    auto endDelegate = custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>((std::function<void (Zenject::DiContainer*)>) [](Zenject::DiContainer*) {
        Qounters::OnSceneEnd();
    });
    GetLevelStarter()->gameScenesManager->PopScenes(0.25, nullptr, endDelegate);
}

void Qounters::RefreshSettingsEnvironment() {
    getLogger().debug("Refreshing environment");

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

#include "GlobalNamespace/MultiplayerController.hpp"
#include "GlobalNamespace/MockPlayer.hpp"
#include "GlobalNamespace/MockPlayerSettings.hpp"
#include "GlobalNamespace/MultiplayerPlayersManager.hpp"
#include "GlobalNamespace/MultiplayerIntroAnimationController.hpp"

void OnMultiplayerSceneStart(MultiplayerController* multiplayerController) {
    auto fakeConnectedPlayers = List<IConnectedPlayer*>::New_ctor(1);
    auto settings = MockPlayerSettings::New_ctor();
    settings->userId = "qounters_settings_player";
    settings->userName = settings->userId;
    auto player = MockPlayer::New_ctor(settings, true);
    player->set_isReady(true);
    player->set_wasActiveAtLevelStart(true);
    player->set_isActive(true);
    localFakeConnectedPlayer = player->i_IConnectedPlayer();
    fakeConnectedPlayers->Add(localFakeConnectedPlayer);

    multiplayerController->playersManager->SpawnPlayers(MultiplayerPlayerStartState::InSync, fakeConnectedPlayers->i_IReadOnlyList_1_T());
    multiplayerController->introAnimationController->TransitionToAfterIntroAnimationState();

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

void Qounters::OnSceneStart() {
    getLogger().info("Settings scene start");

    if (auto multiplayerController = Object::FindObjectOfType<MultiplayerController*>())
        OnMultiplayerSceneStart(multiplayerController);

    // Resources::FindObjectsOfTypeAll<CoreGameHUDController*>().First()->get_gameObject()->SetActive(true);
    Initialize();
    SetupObjects();

    menuEnvironment->get_transform()->get_root()->get_gameObject()->SetActive(true);
    songPreview->CrossfadeToDefault();
    vrInput->get_gameObject()->SetActive(false);

    auto newInput = Resources::FindObjectsOfTypeAll<VRUIControls::VRInputModule*>().FirstOrDefault([](auto x) { return x != vrInput; });
    if (newInput) {
        keyboardManager->vrInputModule = newInput;
        keyboardManager->Start();
    }

    menuEnv->SetActive(false);

    auto mainFlow = GameObject::Find("MainFlowCoordinator")->GetComponent<HMUI::FlowCoordinator*>();
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    if (!mainFlow->IsFlowCoordinatorInHierarchy(settingsFlow))
        mainFlow->PresentFlowCoordinator(settingsFlow, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, true, false);

    getLogger().debug("Fixing environment lighting");

    // rendering order issues
    auto env = GameObject::Find("Environment");
    env->SetActive(false);
    env->SetActive(true);

    // TODO: v3 (weave is pitch black)
    if (auto bcu = Object::FindObjectOfType<BeatmapCallbacksUpdater*>()) {
        auto bcc = bcu->beatmapCallbacksController;
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event0, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event1, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event2, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event3, 1, 1));
        bcc->TriggerBeatmapEvent(BasicBeatmapEventData::New_ctor(0, BasicBeatmapEventType::Event4, 1, 1));
    }

    auto renderParams = Object::FindObjectOfType<VRRenderingParamsSetup*>();
    renderParams->sceneType = VRRenderingParamsSetup::SceneType::Menu;
    renderParams->OnEnable();

    getLogger().debug("Disabling objects");

    if (auto gameplay = GameObject::Find("StandardGameplay"))
        Utils::DisableAllBut(gameplay->get_transform(), {"EventSystem", "ControllerLeft", "ControllerRight"});

    GameObject::Find("DisableGCWhileEnabled")->SetActive(false);

    if (auto bts = GameObject::Find("BTSEnvironmentCharacterSpawner"))
        bts->SetActive(false); // the game literally just freezes
}

#include "GlobalNamespace/FadeInOutController.hpp"

void Qounters::OnSceneEnd() {
    getLogger().info("Settings scene end");

    auto levelStarter = GetLevelStarter();
    levelStarter->menuTransitionsHelper->standardLevelScenesTransitionSetupData->didFinishEvent = nullptr;
    levelStarter->gameScenesManager->neverUnloadScenes->Remove("MenuCore");
    menuEnvironment->ShowEnvironmentType(MenuEnvironmentManager::MenuEnvironmentType::Default);
    songPreview->CrossfadeToDefault();
    vrInput->get_gameObject()->SetActive(true);
    keyboardManager->OnDestroy();
    keyboardManager->vrInputModule = vrInput;
    menuEnv->SetActive(true);
    Object::FindObjectOfType<FadeInOutController*>()->FadeIn();
    localFakeConnectedPlayer = nullptr;
}
