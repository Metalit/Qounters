#include "environment.hpp"

#include "BeatmapDataLoaderVersion4/BeatmapDataLoader.hpp"
#include "BeatmapSaveDataVersion3/BpmChangeEventData.hpp"
#include "BeatmapSaveDataVersion4/LightshowSaveData.hpp"
#include "GlobalNamespace/BeatmapBasicData.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/BeatmapCallbacksUpdater.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDataSortedListForTypeAndIds_1.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapLevelExtensions.hpp"
#include "GlobalNamespace/BeatmapLevelSO.hpp"
#include "GlobalNamespace/BeatmapLightEventConverterNoConvert.hpp"
#include "GlobalNamespace/BpmTimeProcessor.hpp"
#include "GlobalNamespace/ColorSchemeSO.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/DefaultEnvironmentEventsFactory.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/EnvironmentKeywords.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/FadeInOutController.hpp"
#include "GlobalNamespace/GameScenesManager.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/IBeatmapLevelLoader.hpp"
#include "GlobalNamespace/ISortedList_1.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/MenuEnvironmentManager.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/MockPlayer.hpp"
#include "GlobalNamespace/MockPlayerSettings.hpp"
#include "GlobalNamespace/MultiplayerController.hpp"
#include "GlobalNamespace/MultiplayerIntroAnimationController.hpp"
#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/MultiplayerPlayersManager.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/SaberModelContainer.hpp"
#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/SimpleLevelStarter.hpp"
#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/UIKeyboardManager.hpp"
#include "GlobalNamespace/VRRenderingParamsSetup.hpp"
#include "HMUI/ScreenSystem.hpp"
#include "HMUI/ViewController.hpp"
#include "System/Action_1.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"
#include "System/Collections/Generic/LinkedListNode_1.hpp"
#include "System/Collections/Generic/LinkedList_1.hpp"
#include "System/Nullable_1.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "UnityEngine/AddressableAssets/AssetReferenceT_1.hpp"
#include "UnityEngine/JsonUtility.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/TextAsset.hpp"
#include "VRUIControls/VRInputModule.hpp"
#include "Zenject/DiContainer.hpp"
#include "config.hpp"
#include "custom-types/shared/delegate.hpp"
#include "customtypes/components.hpp"
#include "customtypes/settings.hpp"
#include "editor.hpp"
#include "main.hpp"
#include "metacore/shared/internals.hpp"
#include "metacore/shared/unity.hpp"
#include "qounters.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace GlobalNamespace;
using namespace UnityEngine;

static MenuEnvironmentManager* menuEnvironment;
static SongPreviewPlayer* songPreview;
static VRUIControls::VRInputModule* vrInput;
static UIKeyboardManager* keyboardManager;
static GameObject* menuEnv;
static bool wasNoTextsAndHuds = false;
static GameObject* localPlayer;
static bool inSettings = false;
static EnvironmentInfoSO* currentEnvironment = nullptr;
static std::string currentColors = "";

static std::map<std::string, Environment::HUDType> const HudTypes = {
    {"DefaultEnvironment", Environment::HUDType::Wide},
    {"TriangleEnvironment", Environment::HUDType::Wide},
    {"NiceEnvironment", Environment::HUDType::Wide},
    {"BigMirrorEnvironment", Environment::HUDType::Wide},
    {"DragonsEnvironment", Environment::HUDType::Wide},
    {"KDAEnvironment", Environment::HUDType::Wide},
    {"MonstercatEnvironment", Environment::HUDType::Wide},
    {"CrabRaveEnvironment", Environment::HUDType::Wide},
    {"PanicEnvironment", Environment::HUDType::Wide},
    {"TimbalandEnvironment", Environment::HUDType::Wide},
    {"SkrillexEnvironment", Environment::HUDType::Wide},
    {"TheSecondEnvironment", Environment::HUDType::Wide},
    {"LizzoEnvironment", Environment::HUDType::Wide},
    {"TheWeekndEnvironment", Environment::HUDType::Wide},
    {"Dragons2Environment", Environment::HUDType::Wide},
    {"Panic2Environment", Environment::HUDType::Wide},
    {"TheRollingStonesEnvironment", Environment::HUDType::Wide},
    {"LatticeEnvironment", Environment::HUDType::Wide},
    {"DaftPunkEnvironment", Environment::HUDType::Wide},
    {"HipHopEnvironment", Environment::HUDType::Wide},  // could be Close instead
    {"ColliderEnvironment", Environment::HUDType::Wide},
    {"OriginsEnvironment", Environment::HUDType::Narrow},
    {"RocketEnvironment", Environment::HUDType::Narrow},
    {"GreenDayGrenadeEnvironment", Environment::HUDType::Narrow},
    {"GreenDayEnvironment", Environment::HUDType::Narrow},
    {"FitBeatEnvironment", Environment::HUDType::Narrow},
    {"LinkinParkEnvironment", Environment::HUDType::Narrow},
    {"BTSEnvironment", Environment::HUDType::Narrow},
    {"KaleidoscopeEnvironment", Environment::HUDType::Narrow},
    {"InterscopeEnvironment", Environment::HUDType::Narrow},
    {"BillieEnvironment", Environment::HUDType::Narrow},
    {"HalloweenEnvironment", Environment::HUDType::Narrow},
    {"GagaEnvironment", Environment::HUDType::Narrow},
    {"LinkinPark2Environment", Environment::HUDType::Narrow},
    {"WeaveEnvironment", Environment::HUDType::Close},
    {"EDMEnvironment", Environment::HUDType::Close},
    {"PyroEnvironment", Environment::HUDType::Sunken},
    {"RockMixtapeEnvironment", Environment::HUDType::Sunken},
    {"GlassDesertEnvironment", Environment::HUDType::Circle},
    {"MultiplayerEnvironment", Environment::HUDType::Wide},
};

static std::set<std::string> const EnabledGameplayObjects = {
    "GameplayData", "BaseGameEffects", "InteropSabersManager", "GameplaySabersManager", "GameplayDriversManager"
};

std::vector<std::string_view> Environment::HUDTypeStrings = {
    "Wide",
    "Narrow",
    "Close",
    "Sunken",
    "360",
};

Environment::HUDType Environment::GetHUDType(std::string serializedName) {
    auto itr = HudTypes.find(serializedName);
    if (itr != HudTypes.end())
        return itr->second;
    return HUDType::Wide;
}

static SimpleLevelStarter* GetLevelStarter() {
    return Resources::FindObjectsOfTypeAll<SimpleLevelStarter*>()->Last();
}

static EnvironmentInfoSO* GetEnvironment(SimpleLevelStarter* levelStarter) {
    auto serializedName = getConfig().Environment.GetValue();

    auto listModel = levelStarter->_playerDataModel->playerDataFileModel->_environmentsListModel;
    for (auto& info : listModel->_envInfos) {
        if (info->serializedName == serializedName)
            return info;
    }

    auto ret = listModel->GetFirstEnvironmentInfoWithType(EnvironmentType::Normal);
    logger.warn("Environment {} not found, resetting to {}", serializedName, ret->serializedName);
    getConfig().Environment.SetValue(ret->serializedName);
    return ret;
}

static inline System::Action_1<Zenject::DiContainer*>* MakeAction(std::function<void(Zenject::DiContainer*)> callback) {
    return custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>(callback);
}

static void Present(SimpleLevelStarter* levelStarter, bool refresh, ScenesTransitionSetupDataSO* setupData) {
    auto startDelegate = MakeAction([](Zenject::DiContainer*) { Environment::OnSceneStart(); });

    if (refresh)
        levelStarter->_gameScenesManager->ReplaceScenes(setupData, nullptr, 0.25, nullptr, startDelegate);
    else
        levelStarter->_gameScenesManager->PushScenes(setupData, 0.25, nullptr, startDelegate);
}

static void PresentMultiplayer(SimpleLevelStarter* levelStarter, bool refresh, BeatmapLevel* level, BeatmapKey diff, ColorScheme* colors) {
    auto setupData = levelStarter->_menuTransitionsHelper->_multiplayerLevelScenesTransitionSetupData;
    setupData->Init(
        "Settings",
        diff,
        level,
        nullptr,
        colors,
        levelStarter->_gameplayModifiers,
        levelStarter->_playerDataModel->playerData->playerSpecificSettings,
        nullptr,
        levelStarter->_menuTransitionsHelper->_audioClipAsyncLoader,
        levelStarter->_menuTransitionsHelper->_settingsManager,
        levelStarter->_menuTransitionsHelper->_beatmapDataLoader,
        false
    );
    setupData->gameplayCoreSceneSetupData->_beatmapLevelsModel = levelStarter->_menuTransitionsHelper->_beatmapLevelsModel;
    setupData->gameplayCoreSceneSetupData->_allowNullBeatmapLevelData = false;
    localFakeConnectedPlayer = nullptr;

    Present(levelStarter, refresh, setupData);
}

static void PresentSingleplayer(SimpleLevelStarter* levelStarter, bool refresh, BeatmapLevel* level, BeatmapKey diff, ColorScheme* colors) {
    auto setupData = levelStarter->_menuTransitionsHelper->_standardLevelScenesTransitionSetupData;
    setupData->Init(
        "Settings",
        byref(diff),
        level,
        nullptr,
        colors,
        true,
        nullptr,
        levelStarter->_gameplayModifiers,
        levelStarter->_playerDataModel->playerData->playerSpecificSettings,
        nullptr,
        levelStarter->_environmentsListModel,
        levelStarter->_menuTransitionsHelper->_audioClipAsyncLoader,
        levelStarter->_menuTransitionsHelper->_beatmapDataLoader,
        levelStarter->_menuTransitionsHelper->_settingsManager,
        "",
        levelStarter->_menuTransitionsHelper->_beatmapLevelsModel,
        levelStarter->_menuTransitionsHelper->_beatmapLevelsEntitlementModel,
        false,
        false,
        System::Nullable_1<RecordingToolManager::SetupData>()
    );

    Present(levelStarter, refresh, setupData);
}

static void PresentScene(SimpleLevelStarter* levelStarter, bool refresh) {
    auto levelSO = !levelStarter->_beatmapLevel->IsValid() ? levelStarter->_beatmapLevel->LoadAssetAsync().WaitForCompletion().ptr()
                                                           : (BeatmapLevelSO*) levelStarter->_beatmapLevel->Asset.ptr();
    auto level = BeatmapLevelExtensions::ToRuntime(levelSO);
    auto diff = BeatmapKey(levelStarter->_beatmapCharacteristic, levelStarter->_beatmapDifficulty, level->levelID);

    currentEnvironment = GetEnvironment(levelStarter);
    level->GetDifficultyBeatmapData(diff.beatmapCharacteristic, diff.difficulty)->environmentName = currentEnvironment->serializedName;

    auto playerData = levelStarter->_playerDataModel->playerData;
    wasNoTextsAndHuds = playerData->get_playerSpecificSettings()->noTextsAndHuds;
    playerData->get_playerSpecificSettings()->_noTextsAndHuds = false;

    ColorScheme* colors;
    currentColors = getConfig().ColorScheme.GetValue();
    if (getConfig().OverrideColor.GetValue() && playerData->colorSchemesSettings->_colorSchemesDict->ContainsKey(currentColors))
        colors = playerData->colorSchemesSettings->GetColorSchemeForId(currentColors);
    else
        colors = currentEnvironment->colorScheme->colorScheme;

    logger.debug("Presenting scene");

    if (currentEnvironment->serializedName == "MultiplayerEnvironment")
        PresentMultiplayer(levelStarter, refresh, level, diff, colors);
    else
        PresentSingleplayer(levelStarter, refresh, level, diff, colors);
}

void Environment::PresentSettings() {
    logger.debug("Presenting environment");

    if (getConfig().Presets.GetValue().empty()) {
        logger.error("No presets!");
        return;
    }

    inSettings = true;

    // idk if this is beatleader's fault or what
    auto mainFlow = GameObject::Find("MainFlowCoordinator")->GetComponent<HMUI::FlowCoordinator*>();
    if (auto rightView = mainFlow->_screenSystem->rightScreen->_rootViewController)
        rightView->gameObject->active = false;

    menuEnvironment = Object::FindObjectOfType<MenuEnvironmentManager*>();
    songPreview = Object::FindObjectOfType<SongPreviewPlayer*>();
    vrInput = Utils::GetCurrentInputModule();
    keyboardManager = Object::FindObjectOfType<UIKeyboardManager*>();
    menuEnv = GameObject::Find("MenuEnvironmentCore");

    auto levelStarter = GetLevelStarter();
    levelStarter->_gameScenesManager->_neverUnloadScenes->Add("MenuCore");
    menuEnvironment->ShowEnvironmentType(MenuEnvironmentManager::MenuEnvironmentType::None);
    PresentScene(levelStarter, false);
}

static void DismissFlowCoordinator() {
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    settingsFlow->_parentFlowCoordinator->DismissFlowCoordinator(settingsFlow, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, true);
}

void Environment::DismissSettings() {
    logger.debug("Dismissing environment");

    MetaCore::Internals::Finish(true, false);
    HUD::Reset();

    Editor::SetPreviewMode(false);
    inSettings = false;

    DismissFlowCoordinator();

    auto endDelegate = MakeAction([](Zenject::DiContainer*) { OnSceneEnd(); });
    GetLevelStarter()->_gameScenesManager->PopScenes(0.25, nullptr, endDelegate);
}

void Environment::RefreshSettings() {
    logger.debug("Refreshing environment");

    HUD::Reset();

    DismissFlowCoordinator();

    PresentScene(GetLevelStarter(), true);
}

bool Environment::InSettings() {
    return inSettings;
}

EnvironmentInfoSO* Environment::CurrentSettingsEnvironment() {
    return currentEnvironment;
}

void Environment::SetPlayerActive(bool active) {
    localPlayer->active = active;
    // reesabers doesn't seem to use the right color scheme even initially
    UpdateSaberColors();
}

void Environment::UpdateSaberColors() {
    auto sabers = localPlayer->GetComponentsInChildren<SaberModelController*>();
    for (auto& saber : sabers) {
        // lapiz just adds custom sabers as children instead of replacing the normal one
        if (saber->GetComponentInChildren<SaberModelController*>())
            continue;
        if (auto container = saber->GetComponentInParent<SaberModelContainer*>()) {
            saber->Init(container->transform, container->_saber, container->_initData->trailTintColor);
            if (auto ree = container->transform->Find("ReeSaber")) {
                ree->gameObject->active = false;
                ree->gameObject->active = true;
            }
        }
    }
}

void Environment::RunLightingEvents() {
    auto bcu = Object::FindObjectOfType<BeatmapCallbacksUpdater*>(true);
    if (currentEnvironment && bcu) {
        // kinda lame beatgames
        auto lightShowBeatmapData = BeatmapData::New_ctor(4);
        lightShowBeatmapData->updateAllBeatmapDataOnInsert = true;

        if (currentEnvironment->defaultLightshowAsset) {
            logger.debug("loading lightshow data from asset");
            // logger.debug("{}", currentEnvironment->defaultLightshowAsset->text);
            auto lightShowSaveData =
                JsonUtility::FromJson<BeatmapSaveDataVersion4::LightshowSaveData*>(currentEnvironment->defaultLightshowAsset->text);

            auto bpmTimeProcessor = BpmTimeProcessor::New_ctor(
                69.420, ListW<BeatmapSaveDataVersion3::BpmChangeEventData*>::New()->i___System__Collections__Generic__IReadOnlyList_1_T_()
            );
            auto envKeywords = EnvironmentKeywords::New_ctor(currentEnvironment->environmentKeywords);
            auto envLightGroups = currentEnvironment->environmentLightGroups;
            auto converter = BeatmapLightEventConverterNoConvert::New_ctor()->i___GlobalNamespace__IBeatmapLightEventConverter();

            BeatmapDataLoaderVersion4::BeatmapDataLoader::LoadLightshow(
                lightShowBeatmapData, lightShowSaveData, bpmTimeProcessor, envKeywords, envLightGroups, converter
            );
        } else
            DefaultEnvironmentEventsFactory::InsertDefaultEvents(lightShowBeatmapData);

        logger.debug("running default lightshow");
        auto events = lightShowBeatmapData->_allBeatmapData->items;
        auto currentEvent = events->First;
        auto bcc = bcu->_beatmapCallbacksController;
        while (currentEvent != events->Last) {
            if (auto event = Utils::ptr_cast<BeatmapEventData>(currentEvent->Value))
                bcc->TriggerBeatmapEvent(event);
            currentEvent = currentEvent->Next;
        }
    }
}

static void OnMultiplayerSceneStart(MultiplayerController* multiplayerController) {
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

    auto objects = GameObject::Find("MultiplayerLocalActivePlayerController(Clone)/IsActiveObjects")->transform;

    objects->Find("MultiplayerLocalActivePlayerInGameMenuViewController")->gameObject->active = false;
    auto enabled = EnabledGameplayObjects;
    enabled.emplace("BeatmapObjectSpawnController");
    MetaCore::Engine::DisableAllBut(objects->Find("GameplayCore"), enabled);

    localPlayer = objects->Find("LocalPlayerGameCore")->gameObject;
    localPlayer->active = false;
    MetaCore::Engine::FindRecursive(localPlayer->transform, "MainCamera")->gameObject->active = false;
}

void Environment::OnSceneStart() {
    logger.info("Settings scene start");

    if (auto multiplayerController = Object::FindObjectOfType<MultiplayerController*>())
        OnMultiplayerSceneStart(multiplayerController);

    HUD::SetupObjects();

    menuEnvironment->transform->root->gameObject->active = true;
    songPreview->CrossfadeToDefault();

    if (auto gameplay = GameObject::Find("StandardGameplay")) {
        // disable all children so that we can easily reenable LocalPlayerGameCore later
        auto transform = gameplay->transform;
        auto enabled = EnabledGameplayObjects;
        enabled.emplace("LocalPlayerGameCore");
        MetaCore::Engine::DisableAllBut(transform, enabled);
        // disable local player, but not recursively
        localPlayer = transform->Find("LocalPlayerGameCore")->gameObject;
        localPlayer->active = false;
        MetaCore::Engine::FindRecursive(localPlayer->transform, "MainCamera")->gameObject->active = false;
    }

    auto newInput = Utils::GetCurrentInputModule();
    logger.debug("found input module {} (old was {})", fmt::ptr(newInput), fmt::ptr(vrInput));
    if (newInput && newInput != vrInput) {
        keyboardManager->_vrInputModule = newInput->i___GlobalNamespace__IVRInputModule();
        keyboardManager->Start();
        vrInput->gameObject->active = false;
        vrInput->eventSystem->gameObject->active = false;
        vrInput->_vrPointer->_leftVRController->gameObject->active = false;
        vrInput->_vrPointer->_rightVRController->gameObject->active = false;
    }

    menuEnv->active = false;

    auto currentFlow = GameObject::Find("MainFlowCoordinator")->GetComponent<HMUI::FlowCoordinator*>()->YoungestChildFlowCoordinatorOrSelf();
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    if (!currentFlow->IsFlowCoordinatorInHierarchy(settingsFlow))
        currentFlow->PresentFlowCoordinator(settingsFlow, nullptr, HMUI::ViewController::AnimationDirection::Horizontal, true, false);

    float height = 0;
    if (currentEnvironment) {
        if (currentEnvironment->serializedName == "LinkinPark2Environment")
            height = 0.65;
        else if (currentEnvironment->serializedName == "LizzoEnvironment")
            height = 0.1;
    }
    GameObject::Find("MenuCore/UI/ScreenSystem")->transform->localPosition = {0, height, 0};

    logger.debug("Fixing environment lighting");

    // rendering order issues
    if (auto env = GameObject::Find("Environment")) {
        env->active = false;
        env->active = true;
    }

    RunLightingEvents();

    auto renderParams = Object::FindObjectOfType<VRRenderingParamsSetup*>();
    renderParams->_sceneType = SceneType::Menu;
    renderParams->OnEnable();

    GameObject::Find("DisableGCWhileEnabled")->active = false;

    if (auto bts = GameObject::Find("BTSEnvironmentCharacterSpawner"))
        bts->active = false;  // the game literally just freezes
}

void Environment::OnSceneEnd() {
    logger.info("Settings scene end");

    GameObject::Find("MenuCore/UI/ScreenSystem")->transform->localPosition = {0, 0, 0};
    auto levelStarter = GetLevelStarter();
    levelStarter->_menuTransitionsHelper->_standardLevelScenesTransitionSetupData->didFinishEvent = nullptr;
    levelStarter->_gameScenesManager->_neverUnloadScenes->Remove("MenuCore");
    levelStarter->_playerDataModel->playerData->get_playerSpecificSettings()->_noTextsAndHuds = wasNoTextsAndHuds;
    menuEnvironment->ShowEnvironmentType(MenuEnvironmentManager::MenuEnvironmentType::Default);
    songPreview->CrossfadeToDefault();
    vrInput->gameObject->active = true;
    vrInput->eventSystem->gameObject->active = true;
    vrInput->_vrPointer->_leftVRController->gameObject->active = true;
    vrInput->_vrPointer->_rightVRController->gameObject->active = true;
    if (keyboardManager->_vrInputModule != vrInput->i___GlobalNamespace__IVRInputModule()) {
        keyboardManager->OnDestroy();
        keyboardManager->_vrInputModule = vrInput->i___GlobalNamespace__IVRInputModule();
    }
    menuEnv->active = true;
    Object::FindObjectOfType<FadeInOutController*>()->FadeIn();
    localPlayer = nullptr;
    localFakeConnectedPlayer = nullptr;
}
