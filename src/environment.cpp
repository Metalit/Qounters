#include "environment.hpp"

#include "BeatSaber/GameSettings/GraphicSettingsHandler.hpp"
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
#include "internals.hpp"
#include "main.hpp"
#include "qounters.hpp"
#include "utils.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace Qounters;

MenuEnvironmentManager* menuEnvironment;
SongPreviewPlayer* songPreview;
VRUIControls::VRInputModule* vrInput;
UIKeyboardManager* keyboardManager;
GameObject* menuEnv;
GameObject* localPlayer;
bool inSettings = false;
std::string currentEnvironment = "";
std::string currentColors = "";

std::map<std::string, EnvironmentHUDType> const hudTypes = {
    {"DefaultEnvironment", EnvironmentHUDType::Wide},
    {"TriangleEnvironment", EnvironmentHUDType::Wide},
    {"NiceEnvironment", EnvironmentHUDType::Wide},
    {"BigMirrorEnvironment", EnvironmentHUDType::Wide},
    {"DragonsEnvironment", EnvironmentHUDType::Wide},
    {"KDAEnvironment", EnvironmentHUDType::Wide},
    {"MonstercatEnvironment", EnvironmentHUDType::Wide},
    {"CrabRaveEnvironment", EnvironmentHUDType::Wide},
    {"PanicEnvironment", EnvironmentHUDType::Wide},
    {"TimbalandEnvironment", EnvironmentHUDType::Wide},
    {"SkrillexEnvironment", EnvironmentHUDType::Wide},
    {"TheSecondEnvironment", EnvironmentHUDType::Wide},
    {"LizzoEnvironment", EnvironmentHUDType::Wide},
    {"TheWeekndEnvironment", EnvironmentHUDType::Wide},
    {"Dragons2Environment", EnvironmentHUDType::Wide},
    {"Panic2Environment", EnvironmentHUDType::Wide},
    {"TheRollingStonesEnvironment", EnvironmentHUDType::Wide},
    {"LatticeEnvironment", EnvironmentHUDType::Wide},
    {"DaftPunkEnvironment", EnvironmentHUDType::Wide},
    {"HipHopEnvironment", EnvironmentHUDType::Wide},  // could be Close instead
    {"ColliderEnvironment", EnvironmentHUDType::Wide},
    {"OriginsEnvironment", EnvironmentHUDType::Narrow},
    {"RocketEnvironment", EnvironmentHUDType::Narrow},
    {"GreenDayGrenadeEnvironment", EnvironmentHUDType::Narrow},
    {"GreenDayEnvironment", EnvironmentHUDType::Narrow},
    {"FitBeatEnvironment", EnvironmentHUDType::Narrow},
    {"LinkinParkEnvironment", EnvironmentHUDType::Narrow},
    {"BTSEnvironment", EnvironmentHUDType::Narrow},
    {"KaleidoscopeEnvironment", EnvironmentHUDType::Narrow},
    {"InterscopeEnvironment", EnvironmentHUDType::Narrow},
    {"BillieEnvironment", EnvironmentHUDType::Narrow},
    {"HalloweenEnvironment", EnvironmentHUDType::Narrow},
    {"GagaEnvironment", EnvironmentHUDType::Narrow},
    {"LinkinPark2Environment", EnvironmentHUDType::Narrow},
    {"WeaveEnvironment", EnvironmentHUDType::Close},
    {"EDMEnvironment", EnvironmentHUDType::Close},
    {"PyroEnvironment", EnvironmentHUDType::Sunken},
    {"RockMixtapeEnvironment", EnvironmentHUDType::Sunken},
    {"GlassDesertEnvironment", EnvironmentHUDType::Circle},
    {"MultiplayerEnvironment", EnvironmentHUDType::Wide},
};

std::vector<std::string_view> Qounters::EnvironmentHUDTypeStrings = {
    "Wide",
    "Narrow",
    "Close",
    "Sunken",
    "360",
};

EnvironmentHUDType Qounters::GetHUDType(std::string serializedName) {
    auto itr = hudTypes.find(serializedName);
    if (itr != hudTypes.end())
        return itr->second;
    return EnvironmentHUDType::Wide;
}

SimpleLevelStarter* GetLevelStarter() {
    return Resources::FindObjectsOfTypeAll<SimpleLevelStarter*>()->Last();
}

EnvironmentInfoSO* GetEnvironment(SimpleLevelStarter* levelStarter) {
    currentEnvironment = getConfig().Environment.GetValue();

    auto listModel = levelStarter->_playerDataModel->playerDataFileModel->_environmentsListModel;
    for (auto& info : listModel->_envInfos) {
        if (info->serializedName == currentEnvironment)
            return info;
    }

    auto ret = listModel->GetFirstEnvironmentInfoWithType(EnvironmentType::Normal);
    logger.warn("Environment {} not found, resetting to {}", currentEnvironment, ret->serializedName);
    currentEnvironment = (std::string) ret->serializedName;
    getConfig().Environment.SetValue(currentEnvironment);
    return ret;
}

inline System::Action_1<Zenject::DiContainer*>* MakeAction(std::function<void(Zenject::DiContainer*)> callback) {
    return custom_types::MakeDelegate<System::Action_1<Zenject::DiContainer*>*>(callback);
}

void Present(SimpleLevelStarter* levelStarter, bool refresh, ScenesTransitionSetupDataSO* setupData, EnvironmentInfoSO* environment = nullptr) {
    auto startDelegate = MakeAction([environment](Zenject::DiContainer*) { Qounters::OnSceneStart(environment); });

    if (refresh)
        levelStarter->_gameScenesManager->ReplaceScenes(setupData, nullptr, 0.25, nullptr, startDelegate);
    else
        levelStarter->_gameScenesManager->PushScenes(setupData, 0.25, nullptr, startDelegate);
}

void PresentMultiplayer(SimpleLevelStarter* levelStarter, bool refresh, BeatmapLevel* level, BeatmapKey diff, ColorScheme* colors) {
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
        levelStarter->_menuTransitionsHelper->_graphicSettingsHandler->_currentPreset,
        levelStarter->_menuTransitionsHelper->_beatmapDataLoader,
        false
    );
    setupData->gameplayCoreSceneSetupData->_beatmapLevelsModel = levelStarter->_menuTransitionsHelper->_beatmapLevelsModel;
    setupData->gameplayCoreSceneSetupData->_allowNullBeatmapLevelData = false;
    localFakeConnectedPlayer = nullptr;

    Present(levelStarter, refresh, setupData);
}

void PresentSingleplayer(
    SimpleLevelStarter* levelStarter, bool refresh, BeatmapLevel* level, BeatmapKey diff, ColorScheme* colors, EnvironmentInfoSO* environment
) {
    if (colors == nullptr)
        colors = environment->colorScheme->colorScheme;

    auto setupData = levelStarter->_menuTransitionsHelper->_standardLevelScenesTransitionSetupData;
    setupData->Init(
        "Settings",
        diff,
        level,
        nullptr,
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
    auto levelSO = !levelStarter->_beatmapLevel->IsValid() ? levelStarter->_beatmapLevel->LoadAssetAsync().WaitForCompletion().ptr()
                                                           : (BeatmapLevelSO*) levelStarter->_beatmapLevel->Asset.ptr();
    auto level = BeatmapLevelExtensions::ToRuntime(levelSO);
    auto diff = BeatmapKey(levelStarter->_beatmapCharacteristic, levelStarter->_beatmapDifficulty, level->levelID);

    auto environment = GetEnvironment(levelStarter);
    level->GetDifficultyBeatmapData(diff.beatmapCharacteristic, diff.difficulty)->environmentName = environment->serializedName;

    ColorScheme* colors = nullptr;
    currentColors = getConfig().ColorScheme.GetValue();
    auto colorSchemeSettings = levelStarter->_playerDataModel->playerData->colorSchemesSettings;
    if (currentColors == "User Override / Environment" || !colorSchemeSettings->_colorSchemesDict->ContainsKey(currentColors)) {
        auto isOverride = colorSchemeSettings->overrideDefaultColors;
        colors = isOverride ? colorSchemeSettings->GetOverrideColorScheme() : nullptr;
    } else if (currentColors != "Environment Default")
        colors = colorSchemeSettings->GetColorSchemeForId(currentColors);

    logger.debug("Presenting scene");

    if (currentEnvironment == "MultiplayerEnvironment")
        PresentMultiplayer(levelStarter, refresh, level, diff, colors);
    else
        PresentSingleplayer(levelStarter, refresh, level, diff, colors, environment);
}

void Qounters::PresentSettingsEnvironment() {
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

void DismissFlowCoordinator() {
    auto settingsFlow = Qounters::SettingsFlowCoordinator::GetInstance();
    settingsFlow->_parentFlowCoordinator->DismissFlowCoordinator(settingsFlow, HMUI::ViewController::AnimationDirection::Horizontal, nullptr, true);
}

void Qounters::DismissSettingsEnvironment() {
    logger.debug("Dismissing environment");

    Reset();

    Editor::SetPreviewMode(false);
    inSettings = false;

    DismissFlowCoordinator();

    auto endDelegate = MakeAction([](Zenject::DiContainer*) { Qounters::OnSceneEnd(); });
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

void Qounters::SetPlayerActive(bool active) {
    localPlayer->active = active;
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

    auto objects = GameObject::Find("MultiplayerLocalActivePlayerController(Clone)/IsActiveObjects")->transform;

    objects->Find("MultiplayerLocalActivePlayerInGameMenuViewController")->gameObject->active = false;
    Utils::DisableAllBut(objects->Find("GameplayCore"), {"GameplayData", "BaseGameEffects", "BeatmapObjectSpawnController"});

    localPlayer = objects->Find("LocalPlayerGameCore")->gameObject;
    localPlayer->active = false;
    Utils::FindRecursive(localPlayer->transform, "MainCamera")->gameObject->active = false;
}

void Qounters::OnSceneStart(EnvironmentInfoSO* environment) {
    logger.info("Settings scene start");

    if (auto multiplayerController = Object::FindObjectOfType<MultiplayerController*>())
        OnMultiplayerSceneStart(multiplayerController);

    Initialize();
    SetupObjects();

    menuEnvironment->transform->root->gameObject->active = true;
    songPreview->CrossfadeToDefault();

    if (auto gameplay = GameObject::Find("StandardGameplay")) {
        // disable all children so that we can easily reenable LocalPlayerGameCore later
        auto transform = gameplay->transform;
        Utils::DisableAllBut(transform, {"LocalPlayerGameCore", "GameplayData", "BaseGameEffects"});
        // disable local player, but not recursively
        localPlayer = transform->Find("LocalPlayerGameCore")->gameObject;
        localPlayer->active = false;
        Utils::FindRecursive(localPlayer->transform, "MainCamera")->gameObject->active = false;
    }

    auto newInput = Utils::GetCurrentInputModule();
    logger.debug("found input module {} (old was {})", fmt::ptr(newInput), fmt::ptr(vrInput));
    if (newInput && newInput != vrInput) {
        keyboardManager->_vrInputModule = newInput;
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

    float height = environment && environment->serializedName == "LinkinPark2Environment" ? 0.8 : 0.1;
    GameObject::Find("MenuCore/UI/ScreenSystem")->transform->localPosition = {0, height, 0};

    logger.debug("Fixing environment lighting");

    // rendering order issues
    if (auto env = GameObject::Find("Environment")) {
        env->active = false;
        env->active = true;
    }

    auto bcu = Object::FindObjectOfType<BeatmapCallbacksUpdater*>(true);
    if (environment && bcu) {
        // kinda lame beatgames
        auto lightShowBeatmapData = BeatmapData::New_ctor(4);
        lightShowBeatmapData->updateAllBeatmapDataOnInsert = true;

        if (environment->defaultLightshowAsset) {
            logger.debug("loading lightshow data from asset");
            auto lightShowSaveData = JsonUtility::FromJson<BeatmapSaveDataVersion4::LightshowSaveData*>(environment->defaultLightshowAsset->text);

            auto bpmTimeProcessor = BpmTimeProcessor::New_ctor(
                69.420, ListW<BeatmapSaveDataVersion3::BpmChangeEventData*>::New()->i___System__Collections__Generic__IReadOnlyList_1_T_()
            );
            auto envKeywords = EnvironmentKeywords::New_ctor(environment->environmentKeywords);
            auto envLightGroups = environment->environmentLightGroups;

            BeatmapDataLoaderVersion4::BeatmapDataLoader::LoadLightshow(
                lightShowBeatmapData, lightShowSaveData, bpmTimeProcessor, envKeywords, envLightGroups
            );
        } else
            GlobalNamespace::DefaultEnvironmentEventsFactory::InsertDefaultEvents(lightShowBeatmapData);

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

    auto renderParams = Object::FindObjectOfType<VRRenderingParamsSetup*>();
    renderParams->_sceneType = SceneType::Menu;
    renderParams->OnEnable();

    GameObject::Find("DisableGCWhileEnabled")->active = false;

    if (auto bts = GameObject::Find("BTSEnvironmentCharacterSpawner"))
        bts->active = false;  // the game literally just freezes
}

void Qounters::OnSceneEnd() {
    logger.info("Settings scene end");

    GameObject::Find("MenuCore/UI/ScreenSystem")->transform->localPosition = {0, 0, 0};
    auto levelStarter = GetLevelStarter();
    levelStarter->_menuTransitionsHelper->_standardLevelScenesTransitionSetupData->didFinishEvent = nullptr;
    levelStarter->_gameScenesManager->_neverUnloadScenes->Remove("MenuCore");
    menuEnvironment->ShowEnvironmentType(MenuEnvironmentManager::MenuEnvironmentType::Default);
    songPreview->CrossfadeToDefault();
    vrInput->gameObject->active = true;
    vrInput->eventSystem->gameObject->active = true;
    vrInput->_vrPointer->_leftVRController->gameObject->active = true;
    vrInput->_vrPointer->_rightVRController->gameObject->active = true;
    if (keyboardManager->_vrInputModule.ptr() != vrInput) {
        keyboardManager->OnDestroy();
        keyboardManager->_vrInputModule = vrInput;
    }
    menuEnv->active = true;
    Object::FindObjectOfType<FadeInOutController*>()->FadeIn();
    localPlayer = nullptr;
    localFakeConnectedPlayer = nullptr;
}
