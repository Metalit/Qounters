#include "gameplay.hpp"

#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/GameServerLobbyFlowCoordinator.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "config.hpp"
#include "customtypes/components.hpp"
#include "environment.hpp"
#include "utils.hpp"

using namespace Qounters;

bool initialized = false;

UnityEngine::UI::VerticalLayoutGroup* vertical;
TMPro::TextMeshProUGUI* environmentText;
TMPro::TextMeshProUGUI* environmentTypeText;
BSML::DropdownListSetting* presetDropdown;
BSML::DropdownListSetting* typePresetDropdown;
BSML::ToggleSetting* typeOverrideToggle;
BSML::DropdownListSetting* specificPresetDropdown;
BSML::ToggleSetting* specificOverrideToggle;
UnityEngine::UI::Button* settingsButton;

GlobalNamespace::BeatmapLevel* beatmapLevel = nullptr;
GlobalNamespace::BeatmapKey beatmapKey;
bool canOverrideEnvironment;
GlobalNamespace::EnvironmentInfoSO* lastEnvironment = nullptr;
bool environmentIsOverride;

bool IsMultiplayer() {
    auto flowCoordinator = BSML::Helpers::GetMainFlowCoordinator()->YoungestChildFlowCoordinatorOrSelf();
    return Utils::ptr_cast<GlobalNamespace::GameServerLobbyFlowCoordinator>(flowCoordinator.ptr());
}

void SetNameText(UnityEngine::Component* setting, std::string text) {
    if (auto textObj = setting->transform->Find("NameText"))
        textObj->GetComponent<TMPro::TextMeshProUGUI*>()->text = text;
}

void OnDestroy() {
    initialized = false;
    lastEnvironment = nullptr;
}

void SelectPreset(StringW name) {
    auto presets = getConfig().Presets.GetValue();
    if (!presets.contains(name))
        UpdateUI();
    else
        getConfig().Preset.SetValue(name);
}

void TypeOverrideToggled(bool enabled) {
    if (!lastEnvironment)
        return;
    auto typePresets = getConfig().TypePresets.GetValue();
    std::string hudTypeString = std::to_string((int) GetHUDType(lastEnvironment->serializedName));
    if (!typePresets.contains(hudTypeString))
        typePresets[hudTypeString].Preset = getConfig().Preset.GetValue();
    typePresets[hudTypeString].Enabled = enabled;
    getConfig().TypePresets.SetValue(typePresets);
    UpdateUI();
}

void SelectTypePreset(StringW name) {
    if (!lastEnvironment)
        return;
    auto typePresets = getConfig().TypePresets.GetValue();
    std::string hudTypeString = std::to_string((int) GetHUDType(lastEnvironment->serializedName));
    typePresets[hudTypeString].Preset = std::string(name);
    getConfig().TypePresets.SetValue(typePresets);
}

void SpecificOverrideToggled(bool enabled) {
    if (!lastEnvironment)
        return;
    auto specificPresets = getConfig().SpecificPresets.GetValue();
    std::string serializedName = lastEnvironment->serializedName;
    if (!specificPresets.contains(serializedName))
        specificPresets[serializedName].Preset = getConfig().Preset.GetValue();
    specificPresets[serializedName].Enabled = enabled;
    getConfig().SpecificPresets.SetValue(specificPresets);
    UpdateUI();
}

void SelectSpecificPreset(StringW name) {
    if (!lastEnvironment)
        return;
    auto specificPresets = getConfig().SpecificPresets.GetValue();
    std::string serializedName = lastEnvironment->serializedName;
    specificPresets[serializedName].Preset = std::string(name);
    getConfig().SpecificPresets.SetValue(specificPresets);
}

void Qounters::GameplaySetupMenu(UnityEngine::GameObject* parent, bool firstActivation) {
    if (!firstActivation) {
        UpdateUI();
        return;
    }

    parent->AddComponent<ObjectSignal*>()->onDestroy = OnDestroy;

    vertical = BSML::Lite::CreateVerticalLayoutGroup(parent);
    vertical->name = "QountersGameplaySetup";
    vertical->childControlHeight = false;
    vertical->childForceExpandHeight = false;
    vertical->childForceExpandWidth = true;
    vertical->spacing = 0.5;

    auto horizontal = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    Utils::SetLayoutSize(BSML::Lite::CreateText(horizontal, "Current Environment:"), -1, -1, 999);
    environmentText = BSML::Lite::CreateText(horizontal, "");

    horizontal = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    Utils::SetLayoutSize(BSML::Lite::CreateText(horizontal, "Environment Type:"), -1, -1, 999);
    environmentTypeText = BSML::Lite::CreateText(horizontal, "");

    // not toggleable elsewhere for now
    AddConfigValueToggle(vertical, getConfig().Enabled);
    AddConfigValueToggle(vertical, getConfig().Noodle);

    presetDropdown = BSML::Lite::CreateDropdown(vertical, "Default Preset", "", {}, SelectPreset);

    typeOverrideToggle = BSML::Lite::CreateToggle(vertical, "", false, TypeOverrideToggled);
    typePresetDropdown = BSML::Lite::CreateDropdown(vertical, "", "", {}, SelectTypePreset);
    auto rect = typePresetDropdown->GetComponent<UnityEngine::RectTransform*>();
    auto toDelete = rect->parent->gameObject;
    rect->SetParent(typeOverrideToggle->transform, false);
    rect->anchoredPosition = {-20, 0};
    UnityEngine::Object::Destroy(toDelete);

    specificOverrideToggle = BSML::Lite::CreateToggle(vertical, "", false, SpecificOverrideToggled);
    specificPresetDropdown = BSML::Lite::CreateDropdown(vertical, "", "", {}, SelectSpecificPreset);
    rect = specificPresetDropdown->GetComponent<UnityEngine::RectTransform*>();
    toDelete = rect->parent->gameObject;
    rect->SetParent(specificOverrideToggle->transform, false);
    rect->anchoredPosition = {-20, 0};
    UnityEngine::Object::Destroy(toDelete);

    Utils::SetChildrenWidth(vertical->transform, 85);

    settingsButton = BSML::Lite::CreateUIButton(vertical, "Open Settings", PresentSettingsEnvironment);

    initialized = true;
    UpdateUI();
}

void Qounters::SetLevel(GlobalNamespace::BeatmapLevel* level, GlobalNamespace::BeatmapKey key, bool enableOverride) {
    beatmapLevel = level;
    beatmapKey = key;
    canOverrideEnvironment = enableOverride;
    UpdateEnvironment();
}

void Qounters::ClearLevel() {
    beatmapLevel = nullptr;
    beatmapKey = {};
    lastEnvironment = nullptr;
    UpdateUI();
}

void Qounters::UpdateEnvironment() {
    if (!beatmapLevel)
        return;
    auto beatmapEnv = beatmapLevel->GetEnvironmentName(beatmapKey.beatmapCharacteristic, beatmapKey.difficulty);
    auto data = BSML::Helpers::GetMainFlowCoordinator()->_playerDataModel;
    if (IsMultiplayer()) {
        lastEnvironment =
            data->_playerDataFileModel->_environmentsListModel->GetFirstEnvironmentInfoWithType(GlobalNamespace::EnvironmentType::Multiplayer);
        environmentIsOverride = true;
    } else {
        lastEnvironment = data->_playerDataFileModel->_environmentsListModel->GetEnvironmentInfoBySerializedName(beatmapEnv._environmentName);
        environmentIsOverride = false;
        auto override = data->_playerData->overrideEnvironmentSettings;
        if (canOverrideEnvironment && lastEnvironment && override && override->overrideEnvironments) {
            lastEnvironment = override->GetOverrideEnvironmentInfoForType(lastEnvironment->environmentType);
            environmentIsOverride = true;
        }
    }
    UpdateUI();
}

void Qounters::UpdateUI() {
    if (!initialized)
        return;

    auto presets = getConfig().Presets.GetValue();
    std::vector<std::string> names;
    for (auto& [name, _] : presets)
        names.emplace_back(name);
    Utils::SetDropdownValues(presetDropdown, names, getConfig().Preset.GetValue());

    typeOverrideToggle->gameObject->active = lastEnvironment;
    specificOverrideToggle->gameObject->active = lastEnvironment;

    environmentText->transform->parent->gameObject->active = lastEnvironment;
    environmentTypeText->transform->parent->gameObject->active = lastEnvironment;

    if (lastEnvironment) {
        int hudType = (int) GetHUDType(lastEnvironment->serializedName);
        auto type = EnvironmentHUDTypeStrings[hudType];
        std::string name = lastEnvironment->environmentName;
        auto reqs = Utils::GetSimplifiedRequirements(beatmapKey);

        std::string hudTypeString = std::to_string(hudType);
        std::string serializedName = lastEnvironment->serializedName;

        auto typePresets = getConfig().TypePresets.GetValue();
        bool hasTypeOverride = typePresets.contains(hudTypeString) && typePresets[hudTypeString].Enabled;
        Utils::InstantSetToggle(typeOverrideToggle, hasTypeOverride);
        typePresetDropdown->gameObject->active = hasTypeOverride;
        if (hasTypeOverride)
            Utils::SetDropdownValues(typePresetDropdown, names, typePresets[hudTypeString].Preset);

        auto specificPresets = getConfig().SpecificPresets.GetValue();
        bool hasSpecificOverride = specificPresets.contains(serializedName) && specificPresets[serializedName].Enabled;
        Utils::InstantSetToggle(specificOverrideToggle, hasSpecificOverride);
        specificPresetDropdown->gameObject->active = hasSpecificOverride;
        if (hasSpecificOverride)
            Utils::SetDropdownValues(specificPresetDropdown, names, specificPresets[serializedName].Preset);

        environmentText->text = name;
        environmentTypeText->text = reqs.empty() ? type : fmt::format("{}    <size=66%>{}", type, fmt::join(reqs, ", "));;
        if (hasTypeOverride)
            SetNameText(typeOverrideToggle, fmt::format("Override for {}...", type));
        else
            SetNameText(typeOverrideToggle, fmt::format("Override for {} Environments", type));
        SetNameText(specificOverrideToggle, fmt::format("Override for {}", name));

        presetDropdown->set_interactable(!hasTypeOverride && !hasSpecificOverride);
        typePresetDropdown->set_interactable(!hasSpecificOverride);
        typeOverrideToggle->set_interactable(!hasSpecificOverride);
    } else
        presetDropdown->set_interactable(true);

    settingsButton->gameObject->active = !IsMultiplayer();

    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(vertical->rectTransform);
}
