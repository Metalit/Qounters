#include "gameplay.hpp"

#include "GlobalNamespace/CampaignFlowCoordinator.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/GameServerLobbyFlowCoordinator.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "config.hpp"
#include "customtypes/components.hpp"
#include "environment.hpp"
#include "metacore/shared/events.hpp"
#include "metacore/shared/songs.hpp"
#include "metacore/shared/ui.hpp"
#include "metacore/shared/unity.hpp"
#include "utils.hpp"

using namespace Qounters;
using namespace MetaCore;

static bool initialized = false;

static UnityEngine::UI::VerticalLayoutGroup* vertical;
static TMPro::TextMeshProUGUI* environmentText;
static TMPro::TextMeshProUGUI* environmentTypeText;
static BSML::DropdownListSetting* presetDropdown;
static BSML::DropdownListSetting* typePresetDropdown;
static BSML::ToggleSetting* typeOverrideToggle;
static BSML::DropdownListSetting* specificPresetDropdown;
static BSML::ToggleSetting* specificOverrideToggle;
static UnityEngine::UI::Button* settingsButton;

static GlobalNamespace::EnvironmentInfoSO* lastEnvironment = nullptr;

static bool IsMultiplayer() {
    auto flowCoordinator = BSML::Helpers::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf();
    return Utils::ptr_cast<GlobalNamespace::GameServerLobbyFlowCoordinator>(flowCoordinator.ptr());
}

static bool IsCampaign() {
    auto flowCoordinator = BSML::Helpers::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf();
    return Utils::ptr_cast<GlobalNamespace::CampaignFlowCoordinator>(flowCoordinator.ptr());
}

static void SetNameText(UnityEngine::Component* setting, std::string text, float scale) {
    if (auto textObj = setting->transform->Find("NameText")) {
        textObj->GetComponent<TMPro::TextMeshProUGUI*>()->text = text;
        textObj.cast<UnityEngine::RectTransform>()->sizeDelta = {scale, 0};
    }
}

static void OnDestroy() {
    initialized = false;
    lastEnvironment = nullptr;
}

static void SelectPreset(StringW name) {
    auto presets = getConfig().Presets.GetValue();
    if (!presets.contains(name))
        Gameplay::UpdateUI();
    else
        getConfig().Preset.SetValue(name);
}

static void TypeOverrideToggled(bool enabled) {
    if (!lastEnvironment)
        return;
    auto typePresets = getConfig().TypePresets.GetValue();
    std::string hudTypeString = std::to_string((int) Environment::GetHUDType(lastEnvironment->serializedName));
    if (!typePresets.contains(hudTypeString))
        typePresets[hudTypeString].Preset = getConfig().Preset.GetValue();
    typePresets[hudTypeString].Enabled = enabled;
    getConfig().TypePresets.SetValue(typePresets);
    Gameplay::UpdateUI();
}

static void SelectTypePreset(StringW name) {
    if (!lastEnvironment)
        return;
    auto typePresets = getConfig().TypePresets.GetValue();
    std::string hudTypeString = std::to_string((int) Environment::GetHUDType(lastEnvironment->serializedName));
    typePresets[hudTypeString].Preset = std::string(name);
    getConfig().TypePresets.SetValue(typePresets);
}

static void SpecificOverrideToggled(bool enabled) {
    if (!lastEnvironment)
        return;
    auto specificPresets = getConfig().SpecificPresets.GetValue();
    std::string serializedName = lastEnvironment->serializedName;
    if (!specificPresets.contains(serializedName))
        specificPresets[serializedName].Preset = getConfig().Preset.GetValue();
    specificPresets[serializedName].Enabled = enabled;
    getConfig().SpecificPresets.SetValue(specificPresets);
    Gameplay::UpdateUI();
}

static void SelectSpecificPreset(StringW name) {
    if (!lastEnvironment)
        return;
    auto specificPresets = getConfig().SpecificPresets.GetValue();
    std::string serializedName = lastEnvironment->serializedName;
    specificPresets[serializedName].Preset = std::string(name);
    getConfig().SpecificPresets.SetValue(specificPresets);
}

void Gameplay::GameplaySetupMenu(UnityEngine::GameObject* parent, bool firstActivation) {
    if (!firstActivation) {
        UpdateUI();
        return;
    }

    Engine::SetOnDestroy(parent, OnDestroy);

    vertical = BSML::Lite::CreateVerticalLayoutGroup(parent);
    vertical->name = "QountersGameplaySetup";
    vertical->childControlHeight = false;
    vertical->childForceExpandHeight = false;
    vertical->childForceExpandWidth = true;
    vertical->spacing = 0.5;

    auto horizontal = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    UI::SetLayoutSize(BSML::Lite::CreateText(horizontal, "Current Environment:"), -1, -1, 999);
    environmentText = BSML::Lite::CreateText(horizontal, "");
    BSML::Lite::AddHoverHint(environmentText, "The environment that will be used for the currently selected level");

    horizontal = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    UI::SetLayoutSize(BSML::Lite::CreateText(horizontal, "Environment Type:"), -1, -1, 999);
    environmentTypeText = BSML::Lite::CreateText(horizontal, "");
    BSML::Lite::AddHoverHint(environmentText, "The HUD type of the current environment");

    // not toggleable elsewhere for now
    AddConfigValueToggle(vertical, getConfig().Enabled);
    AddConfigValueToggle(vertical, getConfig().Noodle);

    presetDropdown = BSML::Lite::CreateDropdown(vertical, "Default Preset", "", {}, SelectPreset);
    BSML::Lite::AddHoverHint(presetDropdown, "The preset to use in levels");

    typeOverrideToggle = BSML::Lite::CreateToggle(vertical, "", false, TypeOverrideToggled);
    typePresetDropdown = BSML::Lite::CreateDropdown(vertical, "", "", {}, SelectTypePreset);
    auto rect = typePresetDropdown->GetComponent<UnityEngine::RectTransform*>();
    auto toDelete = rect->parent->gameObject;
    rect->SetParent(typeOverrideToggle->transform, false);
    rect->anchoredPosition = {-20, 0};
    UnityEngine::Object::Destroy(toDelete);
    typeOverrideToggle->transform->Find("NameText")->GetComponent<TMPro::TextMeshProUGUI*>()->fontSizeMin = 4;
    BSML::Lite::AddHoverHint(typeOverrideToggle, "Override the preset for this HUD type");

    specificOverrideToggle = BSML::Lite::CreateToggle(vertical, "", false, SpecificOverrideToggled);
    specificPresetDropdown = BSML::Lite::CreateDropdown(vertical, "", "", {}, SelectSpecificPreset);
    rect = specificPresetDropdown->GetComponent<UnityEngine::RectTransform*>();
    toDelete = rect->parent->gameObject;
    rect->SetParent(specificOverrideToggle->transform, false);
    rect->anchoredPosition = {-20, 0};
    UnityEngine::Object::Destroy(toDelete);
    specificOverrideToggle->transform->Find("NameText")->GetComponent<TMPro::TextMeshProUGUI*>()->fontSizeMin = 4;
    BSML::Lite::AddHoverHint(specificOverrideToggle, "Override the preset for this specific environment");

    UI::SetChildrenWidth(vertical->transform, 85);

    settingsButton = BSML::Lite::CreateUIButton(vertical, "Open Settings", Environment::PresentSettings);
    BSML::Lite::AddHoverHint(settingsButton, "Open the preset modification environment");

    initialized = true;
    UpdateUI();
}

void Gameplay::UpdateEnvironment() {
    auto beatmapLevel = MetaCore::Songs::GetSelectedLevel(false);
    if (!beatmapLevel)
        return;
    auto beatmapKey = MetaCore::Songs::GetSelectedKey(false);
    auto beatmapEnv = beatmapLevel->GetEnvironmentName(beatmapKey.beatmapCharacteristic, beatmapKey.difficulty);
    auto data = BSML::Helpers::GetMainFlowCoordinator()->_playerDataModel;
    if (!IsMultiplayer()) {
        lastEnvironment = data->_playerDataFileModel->_environmentsListModel->GetEnvironmentInfoBySerializedName(beatmapEnv._environmentName);
        auto override = data->_playerData->overrideEnvironmentSettings;
        if (!IsCampaign() && lastEnvironment && override && override->overrideEnvironments)
            lastEnvironment = override->GetOverrideEnvironmentInfoForType(lastEnvironment->environmentType);
    } else
        lastEnvironment =
            data->_playerDataFileModel->_environmentsListModel->GetFirstEnvironmentInfoWithType(GlobalNamespace::EnvironmentType::Multiplayer);
    UpdateUI();
}

void Gameplay::UpdateUI() {
    if (!initialized)
        return;

    auto presets = getConfig().Presets.GetValue();
    std::vector<std::string> names;
    for (auto& [name, _] : presets)
        names.emplace_back(name);
    UI::SetDropdownValues(presetDropdown, names, getConfig().Preset.GetValue());

    typeOverrideToggle->gameObject->active = lastEnvironment;
    specificOverrideToggle->gameObject->active = lastEnvironment;

    environmentText->transform->parent->gameObject->active = lastEnvironment;
    environmentTypeText->transform->parent->gameObject->active = lastEnvironment;

    if (lastEnvironment) {
        int hudType = (int) Environment::GetHUDType(lastEnvironment->serializedName);
        auto type = Environment::HUDTypeStrings[hudType];
        std::string name = lastEnvironment->environmentName;
        auto reqs = Utils::GetSimplifiedRequirements(MetaCore::Songs::GetSelectedKey(false));

        std::string hudTypeString = std::to_string(hudType);
        std::string serializedName = lastEnvironment->serializedName;

        auto typePresets = getConfig().TypePresets.GetValue();
        bool hasTypeOverride = typePresets.contains(hudTypeString) && typePresets[hudTypeString].Enabled;
        UI::InstantSetToggle(typeOverrideToggle, hasTypeOverride);
        typePresetDropdown->gameObject->active = hasTypeOverride;
        if (hasTypeOverride)
            UI::SetDropdownValues(typePresetDropdown, names, typePresets[hudTypeString].Preset);

        auto specificPresets = getConfig().SpecificPresets.GetValue();
        bool hasSpecificOverride = specificPresets.contains(serializedName) && specificPresets[serializedName].Enabled;
        UI::InstantSetToggle(specificOverrideToggle, hasSpecificOverride);
        specificPresetDropdown->gameObject->active = hasSpecificOverride;
        if (hasSpecificOverride)
            UI::SetDropdownValues(specificPresetDropdown, names, specificPresets[serializedName].Preset);

        environmentText->text = name;
        environmentTypeText->text = reqs.empty() ? type : fmt::format("{}    <size=66%>{}", type, fmt::join(reqs, ", "));
        SetNameText(typeOverrideToggle, fmt::format("Override for {} Environments", type), hasTypeOverride ? -57 : 0);
        SetNameText(specificOverrideToggle, fmt::format("Override for {}", name), hasSpecificOverride ? -57 : 0);

        presetDropdown->set_interactable(!hasTypeOverride && !hasSpecificOverride);
        typePresetDropdown->set_interactable(!hasSpecificOverride);
        typeOverrideToggle->set_interactable(!hasSpecificOverride);
    } else
        presetDropdown->set_interactable(true);

    settingsButton->gameObject->active = !IsMultiplayer();

    // why unity why
    if (vertical->gameObject->active) {
        UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(vertical->rectTransform);
        BSML::MainThreadScheduler::ScheduleNextFrame([]() { vertical->SetDirty(); });
    }
}

ON_EVENT(MetaCore::Events::MapSelected) {
    Gameplay::UpdateEnvironment();
}

ON_EVENT(MetaCore::Events::MapDeselected) {
    lastEnvironment = nullptr;
    Gameplay::UpdateUI();
}
