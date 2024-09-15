#include "customtypes/settings.hpp"

#include <numbers>

#include "GlobalNamespace/BloomPrePassBackgroundColorsGradientFromColorSchemeColors.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/ColorSchemeSO.hpp"
#include "GlobalNamespace/ColorsOverrideSettingsPanelController.hpp"
#include "GlobalNamespace/EnvironmentColorManager.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/EnvironmentType.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/LightColorGroupEffectManager.hpp"
#include "GlobalNamespace/NoteCutCoreEffectsSpawner.hpp"
#include "GlobalNamespace/PlayerData.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/ScreenSystem.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Single.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "assets.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/Components/Backgroundable.hpp"
#include "bsml/shared/BSML/Components/ScrollViewContent.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "bsml/shared/Helpers/delegates.hpp"
#include "bsml/shared/Helpers/extension.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "config.hpp"
#include "custom-types/shared/delegate.hpp"
#include "customtypes/components.hpp"
#include "customtypes/editing.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "events.hpp"
#include "game.hpp"
#include "internals.hpp"
#include "main.hpp"
#include "options.hpp"
#include "playtest.hpp"
#include "pp.hpp"
#include "sources.hpp"
#include "sourceui.hpp"
#include "templates.hpp"
#include "types.hpp"
#include "utils.hpp"

DEFINE_TYPE(Qounters, SettingsFlowCoordinator);
DEFINE_TYPE(Qounters, SettingsViewController);
DEFINE_TYPE(Qounters, PlaytestViewController);
DEFINE_TYPE(Qounters, TemplatesViewController);
DEFINE_TYPE(Qounters, OptionsViewController);
DEFINE_TYPE(Qounters, HSVGradientImage);
DEFINE_TYPE(Qounters, HSVController);
DEFINE_TYPE(Qounters, CollapseController);
DEFINE_TYPE(Qounters, MenuDragger);
DEFINE_TYPE(Qounters, EndDragHandler);
DEFINE_TYPE(Qounters, KeyboardCloseHandler);
DEFINE_TYPE(Qounters, SpritesListCell);
DEFINE_TYPE(Qounters, SpritesListSource);

using namespace Qounters;
using namespace UnityEngine;

float settingsStarsBL = 10;
float settingsStarsSS = 10;

static GameObject* AddBackground(HMUI::ViewController* self, Vector2 size) {
    auto object = GameObject::New_ctor("QountersBackground");
    object->transform->SetParent(self->transform, false);
    auto bg = object->AddComponent<BSML::Backgroundable*>();
    bg->ApplyBackground("round-rect-panel");
    bg->background->raycastTarget = true;
    auto rect = self->rectTransform;
    rect->anchorMin = {0, 0};
    rect->anchorMax = {1, 1};
    rect->sizeDelta = {0, 10};
    rect = object->GetComponent<RectTransform*>();
    rect->anchorMin = {0.5, 1};
    rect->anchorMax = {0.5, 1};
    rect->pivot = {0.5, 1};
    rect->sizeDelta = size;
    return object;
}

// exponential function f(x) where f(-1) = min, f(0) = 1, f(1) = max
// min < 1, max > 1
// https://math.stackexchange.com/a/2244310 + wolfram alpha
static constexpr float min = 0.1, max = 5;
static constexpr float coeff = (1 - max) / (min - 1);
static constexpr float CalculateScale(float input) {
    return (std::pow(coeff, input) - 1 / coeff) * (max - min) / (coeff - 1 / coeff) + min;
}
static float CalculateScaleInverse(float scale) {
    static float const coeffLog = std::log(coeff);  // no constexpr :(
    float const expr1 = (coeff * coeff * (min - scale) - max + scale) / (min - max);
    return (std::log(expr1) - coeffLog) / coeffLog;
}

static StringW ScaleFormat(float val) {
    return Utils::FormatDecimals(CalculateScale(val), 2);
}

void SettingsFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (addedToHierarchy) {
        auto presets = getConfig().Presets.GetValue();
        auto presetName = getConfig().SettingsPreset.GetValue();
        if (!presets.contains(presetName)) {
            presetName = presets.begin()->first;
            getConfig().SettingsPreset.SetValue(presetName);
        }
        auto& preset = presets[presetName];

        Editor::Initialize(preset);

        auto curveSettings = _screenSystem->mainScreen->GetComponentInParent<HMUI::CurvedCanvasSettings*>();
        oldRadius = curveSettings->_radius;
        curveSettings->SetRadius(GetRadius());
    }

    if (!blankViewController)
        blankViewController = BSML::Helpers::CreateViewController();

    ProvideInitialViewControllers(
        blankViewController, SettingsViewController::GetInstance(), TemplatesViewController::GetInstance(), nullptr, nullptr
    );
    OptionsViewController::GetInstance()->Deselect();

    if (!leftDragger)
        leftDragger = Utils::CreateMenuDragger(_screenSystem->_leftScreen->gameObject, true)->gameObject;
    else
        leftDragger->active = true;
    if (!rightDragger)
        rightDragger = Utils::CreateMenuDragger(_screenSystem->_rightScreen->gameObject, false)->gameObject;
    else
        rightDragger->active = true;
}

void SettingsFlowCoordinator::DidDeactivate(bool removedFromHierarchy, bool screenSystemDisabling) {
    if (oldRadius > 0)
        _screenSystem->mainScreen->GetComponentInParent<HMUI::CurvedCanvasSettings*>()->SetRadius(oldRadius);
    // idk why this stays active
    OptionsViewController::GetInstance()->gameObject->active = false;
    leftDragger->active = false;
    rightDragger->active = false;
}

void SettingsFlowCoordinator::Save() {
    auto presets = getConfig().Presets.GetValue();
    presets[getConfig().SettingsPreset.GetValue()] = Editor::GetPreset();
    getConfig().Presets.SetValue(presets);
}

bool SettingsFlowCoordinator::IsSaved() {
    return getConfig().Presets.GetValue()[getConfig().SettingsPreset.GetValue()] == Editor::GetPreset();
}

void SettingsFlowCoordinator::PresentPlaytest() {
    if (!instance)
        return;
    auto playtest = PlaytestViewController::GetInstance();
    if (instance->rightScreenViewController != playtest)
        instance->SetRightScreenViewController(playtest, HMUI::ViewController::AnimationType::None);
}

void SettingsFlowCoordinator::PresentTemplates() {
    if (!instance)
        return;
    auto templates = TemplatesViewController::GetInstance();
    if (instance->rightScreenViewController != templates)
        instance->SetRightScreenViewController(templates, HMUI::ViewController::AnimationType::None);
}

void SettingsFlowCoordinator::PresentOptions() {
    if (!instance)
        return;
    auto options = OptionsViewController::GetInstance();
    if (instance->rightScreenViewController != options)
        instance->SetRightScreenViewController(options, HMUI::ViewController::AnimationType::None);
}

void SettingsFlowCoordinator::DismissScene() {
    ConfirmAction(Environment::DismissSettings);
}

void SettingsFlowCoordinator::RefreshScene() {
    if (Environment::CurrentSettingsEnvironment()->serializedName != getConfig().Environment.GetValue())
        ConfirmAction(Environment::RefreshSettings);
}

void SettingsFlowCoordinator::OnModalConfirm() {
    if (nextModalAction)
        nextModalAction();
    SettingsViewController::GetInstance()->HideConfirmModal();
}

void SettingsFlowCoordinator::OnModalCancel() {
    if (nextModalCancel)
        nextModalCancel();
    SettingsViewController::GetInstance()->HideConfirmModal();
}

void SettingsFlowCoordinator::SelectPreset(StringW name) {
    ConfirmAction(
        [name = (std::string) name]() {
            auto presets = getConfig().Presets.GetValue();
            if (!presets.contains(name)) {
                SettingsViewController::GetInstance()->UpdateUI();
                return;
            }
            getConfig().SettingsPreset.SetValue(name);
            Editor::LoadPreset(presets[name]);
        },
        []() { SettingsViewController::GetInstance()->UpdateUI(); }
    );
}

void SettingsFlowCoordinator::RenamePreset(StringW name) {
    MakeNewPreset(name, true);
}

void SettingsFlowCoordinator::DuplicatePreset(StringW newName) {
    ConfirmAction([name = (std::string) newName]() { MakeNewPreset(name, false); });
}

void SettingsFlowCoordinator::DeletePreset() {
    auto presets = getConfig().Presets.GetValue();
    if (presets.size() < 2)
        return;

    auto name = getConfig().SettingsPreset.GetValue();
    presets.erase(name);
    getConfig().Presets.SetValue(presets);

    name = presets.begin()->first;
    getConfig().SettingsPreset.SetValue(name);

    Editor::LoadPreset(presets[name]);
}

void SettingsFlowCoordinator::ResetPreset() {
    auto presets = getConfig().Presets.GetValue();
    auto name = getConfig().SettingsPreset.GetValue();
    if (!presets.contains(name))
        return;

    presets[name] = Options::GetDefaultHUDPreset();

    getConfig().Presets.SetValue(presets);
    Editor::LoadPreset(presets[name]);
}

SettingsFlowCoordinator* SettingsFlowCoordinator::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateFlowCoordinator<SettingsFlowCoordinator*>();
    return instance;
}

float SettingsFlowCoordinator::GetRadius() {
    return 110;
}

void SettingsFlowCoordinator::OnDestroy() {
    if (leftDragger)
        Object::Destroy(leftDragger);
    if (rightDragger)
        Object::Destroy(rightDragger);
    instance = nullptr;
}

void SettingsFlowCoordinator::ConfirmAction(std::function<void()> action, std::function<void()> cancel) {
    nextModalAction = action;
    nextModalCancel = cancel;
    if (IsSaved())
        nextModalAction();
    else
        SettingsViewController::GetInstance()->ShowConfirmModal();
}

void SettingsFlowCoordinator::MakeNewPreset(std::string name, bool removeOld) {
    auto presets = getConfig().Presets.GetValue();
    if (presets.contains(name))
        return;

    auto currentName = getConfig().SettingsPreset.GetValue();
    if (removeOld) {
        presets[name] = std::move(presets[currentName]);
        presets.erase(currentName);
    } else {
        presets[name] = presets[currentName];
        Editor::LoadPreset(presets[name]);
    }

    getConfig().SettingsPreset.SetValue(name);
    getConfig().Presets.SetValue(presets);
    SettingsViewController::GetInstance()->UpdateUI();
}

static GlobalNamespace::ColorsOverrideSettingsPanelController* GetColorSchemeTemplate() {
    static UnityW<GlobalNamespace::ColorsOverrideSettingsPanelController> overrideColorsPanel;
    if (!overrideColorsPanel)
        overrideColorsPanel = Object::FindObjectsOfType<GlobalNamespace::ColorsOverrideSettingsPanelController*>(true)->First([](auto x) {
            return x->name == StringW("ColorsOverrideSettings") && x->transform->parent->Find("PlayerOptions");
        });
    return overrideColorsPanel;
}

void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation) {
        UpdateUI();
        return;
    }

    using namespace GlobalNamespace;

    environments = BSML::Helpers::GetMainFlowCoordinator()->_playerDataModel->_playerDataFileModel->_environmentsListModel->_envInfos;

    auto background = AddBackground(this, {110, 82});
    Utils::SetCanvasSorting(gameObject, 4);

    auto vertical = BSML::Lite::CreateVerticalLayoutGroup(background);
    vertical->childControlHeight = false;
    vertical->childForceExpandHeight = false;
    vertical->spacing = 1;
    vertical->rectTransform->anchoredPosition = {0, -3};

    auto buttons1 = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    buttons1->spacing = 3;
    undoButton = BSML::Lite::CreateUIButton(buttons1, "Undo", Editor::Undo);
    BSML::Lite::CreateUIButton(buttons1, "Exit", SettingsFlowCoordinator::DismissScene);

    auto buttons2 = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    buttons2->spacing = 1;
    BSML::Lite::CreateUIButton(buttons2, "Save", SettingsFlowCoordinator::Save);
    BSML::Lite::CreateUIButton(buttons2, "Save And Exit", "ActionButton", []() {
        SettingsFlowCoordinator::Save();
        SettingsFlowCoordinator::DismissScene();
    });

    auto environment = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    environment->spacing = 1;
    environmentDropdown = BSML::Lite::CreateDropdown(environment, getConfig().Environment.GetName(), "", {}, [this](StringW value) {
        for (auto& env : environments) {
            if (env->environmentName == value)
                getConfig().Environment.SetValue(env->serializedName);
        }
    });
    auto parent = environmentDropdown->transform->parent;
    Utils::SetLayoutSize(parent, 87, 8);

    std::vector<std::string_view> append = {"Any"};
    append.insert(append.begin(), Environment::HUDTypeStrings.begin(), Environment::HUDTypeStrings.end());
    auto nested = Utils::CreateDropdownEnum(background, "", getConfig().EnvironmentType.GetValue(), append, [this](int value) {
        getConfig().EnvironmentType.SetValue(value);
        UpdateUI();
    });
    auto rect = nested->GetComponent<RectTransform*>();
    auto toDelete = rect->parent->gameObject;
    rect->SetParent(parent, false);
    rect->anchoredPosition = {-36, 0};
    rect->sizeDelta = {22, 0};
    Object::Destroy(toDelete);

    auto apply = BSML::Lite::CreateUIButton(environment, "Apply", SettingsFlowCoordinator::RefreshScene);
    Utils::SetLayoutSize(apply, 14, 8);

    auto overrideColorsPanel = GetColorSchemeTemplate();
    colorSchemeSettings = BSML::Helpers::GetMainFlowCoordinator()->_playerDataModel->playerData->colorSchemesSettings;
    ListW<GlobalNamespace::ColorScheme*> schemesList = colorSchemeSettings->_colorSchemesList;

    auto toggle = BSML::Lite::CreateToggle(vertical, "", getConfig().OverrideColor.GetValue(), [this](bool enabled) {
        getConfig().OverrideColor.SetValue(enabled);
        UpdateColors();
        UpdateUI();
    });
    colorToggleName = toggle->transform->Find("NameText")->GetComponent<TMPro::TextMeshProUGUI*>();

    auto transform = BSML::Helpers::GetDiContainer()
                         ->InstantiatePrefab(overrideColorsPanel->_colorSchemeDropDown->transform->parent)
                         ->GetComponent<RectTransform*>();
    transform->SetParent(toggle->transform, false);
    transform->anchoredPosition = {-20, 0.25};
    Object::Destroy(transform->Find("NameText")->gameObject);

    colorEditButton = transform->Find("EditButton")->GetComponent<UI::Button*>();
    colorDropdown = transform->Find("ColorSchemeDropDown")->GetComponent<GlobalNamespace::ColorSchemeDropdown*>();

    colorEditor = BSML::Helpers::GetDiContainer()
                      ->InstantiatePrefab(overrideColorsPanel->_editColorSchemeController)
                      ->GetComponent<GlobalNamespace::EditColorSchemeController*>();
    colorEditor->transform->SetParent(transform, false);
    auto modal = colorEditor->GetComponent<HMUI::ModalView*>();

    // so bsml doesn't grab them
    colorEditor->_rgbPanelController->gameObject->name = "QountersRGBPanel";
    colorEditor->_hsvPanelController->gameObject->name = "QountersHSVPanel";

    colorDropdown->SetData(colorSchemeSettings->_colorSchemesList->i___System__Collections__Generic__IReadOnlyList_1_T_());
    colorDropdown->add_didSelectCellWithIdxEvent(BSML::MakeSystemAction((std::function<void(UnityW<HMUI::DropdownWithTableView>, int)>) [this](
        UnityW<HMUI::DropdownWithTableView>, int cell
    ) { ColorCellSelected(cell); }));

    colorEditButton->onClick->AddListener(BSML::MakeUnityAction([modal]() { modal->Show(true, true, nullptr); }));

    colorEditor->add_didChangeColorSchemeEvent(BSML::MakeSystemAction((std::function<void(GlobalNamespace::ColorScheme*)>) [this](auto scheme) {
        colorSchemeSettings->SetColorSchemeForId(scheme);
    }));
    colorEditor->add_didFinishEvent(BSML::MakeSystemAction([this, modal]() {
        modal->Hide(true, nullptr);
        UpdateColors();
        UpdateUI();
    }));

    BSML::Lite::CreateText(vertical, "Changes to the preset list are always saved!", {0, 0}, {50, 8})->alignment =
        TMPro::TextAlignmentOptions::Center;

    presetDropdown = BSML::Lite::CreateDropdown(vertical, "Editing Preset", "", {}, SettingsFlowCoordinator::SelectPreset);

    auto buttons3 = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    buttons3->spacing = 1;
    auto renameButton = BSML::Lite::CreateUIButton(buttons3, "Rename", [this]() {
        nameModalIsRename = true;
        nameInput->text = getConfig().SettingsPreset.GetValue();
        nameModal->Show(true, true, nullptr);
    });
    Utils::SetLayoutSize(renameButton, 24, 8);
    auto dupeButton = BSML::Lite::CreateUIButton(buttons3, "Duplicate", [this]() {
        nameModalIsRename = false;
        nameInput->text = getConfig().SettingsPreset.GetValue();
        nameModal->Show(true, true, nullptr);
    });
    Utils::SetLayoutSize(dupeButton, 24, 8);
    deleteButton = BSML::Lite::CreateUIButton(buttons3, "Delete", SettingsFlowCoordinator::DeletePreset);
    Utils::SetLayoutSize(deleteButton, 24, 8);
    auto resetButton = BSML::Lite::CreateUIButton(buttons3, "Reset", SettingsFlowCoordinator::ResetPreset);
    Utils::SetLayoutSize(resetButton, 24, 8);

    auto snapIncrement = AddConfigValueIncrementFloat(vertical, getConfig().SnapStep, 1, 0.5, 0.5, 5);
    auto incrementObject = snapIncrement->transform->GetChild(1)->gameObject;
    incrementObject->active = getConfig().Snap.GetValue();
    incrementObject->GetComponent<RectTransform*>()->anchoredPosition = {-20, 0};

    auto snapToggle =
        BSML::Lite::CreateToggle(vertical, getConfig().Snap.GetName(), getConfig().Snap.GetValue(), [incrementObject](bool value) mutable {
            getConfig().Snap.SetValue(value);
            incrementObject->active = value;
        });
    snapToggle->toggle->transform->SetParent(snapIncrement->transform, false);
    snapToggle->transform->SetParent(snapIncrement->transform, false);
    Object::Destroy(snapToggle->text->gameObject);

    previewToggle = BSML::Lite::CreateToggle(vertical, "Preview Mode", false, [this](bool value) {
        Editor::SetPreviewMode(value);
        undoButton->interactable = !value && Editor::HasUndo();
    });

    confirmModal = BSML::Lite::CreateModal(this, {95, 25}, SettingsFlowCoordinator::OnModalCancel);
    auto modalLayout1 = BSML::Lite::CreateVerticalLayoutGroup(confirmModal);
    modalLayout1->childControlHeight = false;
    modalLayout1->childForceExpandHeight = true;
    modalLayout1->spacing = 1;

    auto warningString = "You have unsaved changes that will be lost.\nAre you sure you would like to continue? This action cannot be undone.";
    auto text1 = BSML::Lite::CreateText(modalLayout1, warningString, {0, 0}, {50, 13});
    text1->alignment = TMPro::TextAlignmentOptions::Bottom;

    auto modalButtons = BSML::Lite::CreateHorizontalLayoutGroup(modalLayout1);
    modalButtons->GetComponent<UI::LayoutElement*>()->preferredHeight = 9;
    modalButtons->spacing = 3;
    BSML::Lite::CreateUIButton(modalButtons, "Continue", SettingsFlowCoordinator::OnModalConfirm);
    BSML::Lite::CreateUIButton(modalButtons, "Save And Continue", []() {
        SettingsFlowCoordinator::Save();
        SettingsFlowCoordinator::OnModalConfirm();
    });
    BSML::Lite::CreateUIButton(modalButtons, "Cancel", SettingsFlowCoordinator::OnModalCancel);

    nameModal = BSML::Lite::CreateModal(this, {95, 20}, nullptr);
    auto modalLayout2 = BSML::Lite::CreateVerticalLayoutGroup(nameModal);
    modalLayout2->childControlHeight = false;
    modalLayout2->childForceExpandHeight = true;
    modalLayout2->spacing = 1;

    auto text2 = BSML::Lite::CreateText(modalLayout2, "Enter new preset name", {0, 0}, {50, 8});
    text2->alignment = TMPro::TextAlignmentOptions::Bottom;

    nameInput = BSML::Lite::CreateStringSetting(modalLayout2, "Name", "", {0, 0}, {0, 0, 0});
    Utils::GetOrAddComponent<KeyboardCloseHandler*>(nameInput)->okCallback = [this]() {
        std::string val = nameInput->text;
        if (val.empty())
            return;
        nameModal->Hide(true, nullptr);
        if (nameModalIsRename)
            SettingsFlowCoordinator::RenamePreset(val);
        else
            SettingsFlowCoordinator::DuplicatePreset(val);
    };

    uiInitialized = true;
    UpdateUI();
}

void SettingsViewController::OnDestroy() {
    instance = nullptr;
}

void SettingsViewController::ShowConfirmModal() {
    if (confirmModal && !confirmModal->_isShown)
        confirmModal->Show(true, true, nullptr);
}

void SettingsViewController::HideConfirmModal() {
    if (confirmModal && confirmModal->_isShown)
        confirmModal->Hide(true, nullptr);
}

void SettingsViewController::ColorCellSelected(int idx) {
    auto selectedScheme = colorSchemeSettings->_colorSchemesList->get_Item(idx);
    getConfig().ColorScheme.SetValue(selectedScheme->colorSchemeId);
    UpdateColors();
    UpdateUI();
}

void SettingsViewController::UpdateColors() {
    GetColorSchemeTemplate()->Refresh();
    if (getConfig().OverrideColor.GetValue())
        Internals::colors = colorSchemeSettings->GetColorSchemeForId(getConfig().ColorScheme.GetValue());
    else
        Internals::colors = Environment::CurrentSettingsEnvironment()->colorScheme->colorScheme;
    // doesn't update any already spawned notes/walls, but idc
    if (auto colorManager = Object::FindObjectOfType<GlobalNamespace::NoteCutCoreEffectsSpawner*>(true)->_colorManager)
        colorManager->SetColorScheme(Internals::colors);
    if (auto env = Object::FindObjectOfType<GlobalNamespace::EnvironmentColorManager*>(true))
        env->SetColorScheme(Internals::colors);
    if (auto bg = Object::FindObjectOfType<GlobalNamespace::BloomPrePassBackgroundColorsGradientFromColorSchemeColors*>(true))
        bg->Start();
    PlaytestViewController::GetInstance()->UpdateUI();
    Environment::UpdateSaberColors();
    Environment::RunLightingEvents();
    Events::Broadcast((int) Events::MapInfo);
}

void SettingsViewController::UpdateUI() {
    if (!uiInitialized)
        return;

    undoButton->interactable = !Editor::GetPreviewMode() && Editor::HasUndo();

    auto presets = getConfig().Presets.GetValue();
    auto preset = getConfig().SettingsPreset.GetValue();
    std::vector<std::string> names;
    for (auto& [name, _] : presets)
        names.emplace_back(name);
    Utils::SetDropdownValues(presetDropdown, names, preset);

    int selectedType = getConfig().EnvironmentType.GetValue();
    GlobalNamespace::EnvironmentInfoSO* first = nullptr;
    std::string selectedName;
    names.clear();
    for (auto& env : environments) {
        if (selectedType == (int) Environment::HUDType::Max + 1 || (int) Environment::GetHUDType(env->serializedName) == selectedType) {
            if (!first)
                first = env;
            if (env->serializedName == getConfig().Environment.GetValue())
                selectedName = (std::string) env->environmentName;
            names.emplace_back(env->environmentName);
        }
    }
    Utils::SetDropdownValues(environmentDropdown, names, selectedName, [first]() {
        // set to the first environment in the filtered type when it changes
        getConfig().Environment.SetValue(first->serializedName);
    });

    auto dict = colorSchemeSettings->_colorSchemesDict;
    if (!dict->ContainsKey(getConfig().ColorScheme.GetValue()))
        getConfig().ColorScheme.SetValue(colorSchemeSettings->GetColorSchemeForIdx(0)->colorSchemeId);
    auto selectedSchemeId = getConfig().ColorScheme.GetValue();
    auto selectedScheme = dict->get_Item(selectedSchemeId);
    colorEditButton->interactable = selectedScheme->isEditable;
    if (selectedScheme->isEditable)
        colorEditor->SetColorScheme(selectedScheme);

    colorToggleName->text = getConfig().OverrideColor.GetValue() ? "..." : "Override Environment Colors";

    colorDropdown->transform->parent->gameObject->active = getConfig().OverrideColor.GetValue();
    auto list = colorSchemeSettings->_colorSchemesList;
    int idx = 0;
    for (int i = 0; i < list->get_Count(); i++) {
        if (list->get_Item(i)->colorSchemeId == selectedSchemeId) {
            idx = i;
            break;
        }
    }
    colorDropdown->SelectCellWithIdx(idx);

    deleteButton->interactable = presets.size() > 1;

    Utils::InstantSetToggle(previewToggle, Editor::GetPreviewMode());
}

SettingsViewController* SettingsViewController::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<SettingsViewController*>();
    return instance;
}

static void CreateSpacer(UI::HorizontalLayoutGroup* parent, float width) {
    auto obj = GameObject::New_ctor("QountersSpacer");
    auto layout = obj->AddComponent<UI::LayoutElement*>();
    layout->preferredWidth = width;
    obj->transform->SetParent(parent->transform, false);
}

static BSML::ClickableImage*
CreateLayeredImageButton(UI::HorizontalLayoutGroup* parent, Sprite* bg, Sprite* fg, Vector2 size, std::function<void()> onClick) {
    auto ret = BSML::Lite::CreateClickableImage(parent, bg, onClick);
    ret->preserveAspect = true;
    Utils::SetLayoutSize(ret, size.x, size.y);
    if (fg)
        BSML::Lite::CreateImage(ret, fg, {0, 0}, size)->preserveAspect = true;
    return ret;
}

static void SetClickableImageColor(BSML::ClickableImage* image, Color color, bool wall = false) {
    float h, s, v;
    Color::RGBToHSV(color, byref(h), byref(s), byref(v));
    if (wall)
        SetClickableImageColor(image, Color::HSVToRGB(h, s, 1));
    else if (v < 0.5) {
        v += 0.3;
        image->set_defaultColor(color);
        image->set_highlightColor(Color::HSVToRGB(h, s, v));
    } else {
        v -= 0.3;
        image->set_defaultColor(Color::HSVToRGB(h, s, v));
        image->set_highlightColor(color);
    }
}

void PlaytestViewController::Update() {
    Playtest::Update();
}

void PlaytestViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation) {
        UpdateUI();
        return;
    }

    static auto percentFormat = [](float value) {
        return fmt::format("{}%", (int) value);
    };

    auto background = AddBackground(this, {85, 75});

    auto parent = BSML::Lite::CreateVerticalLayoutGroup(background);
    parent->childForceExpandHeight = false;
    parent->childControlHeight = false;
    parent->rectTransform->anchoredPosition = {0, -4};

    auto spawnButtons = BSML::Lite::CreateHorizontalLayoutGroup(parent);
    spawnButtons->childForceExpandHeight = false;
    spawnButtons->childForceExpandWidth = false;
    spawnButtons->spacing = 1;
    spawnButtons->childAlignment = TextAnchor::MiddleCenter;

    auto note = PNG_SPRITE(Bloq);
    auto arrow = PNG_SPRITE(Arrow);
    auto chain = PNG_SPRITE(Chain);
    auto chainArrow = PNG_SPRITE(ChainArrow);

    lNote = CreateLayeredImageButton(spawnButtons, note, arrow, {8, 8}, []() { Playtest::SpawnNote(true, false); });
    rNote = CreateLayeredImageButton(spawnButtons, note, arrow, {8, 8}, []() { Playtest::SpawnNote(false, false); });

    CreateSpacer(spawnButtons, 3);

    lChain = CreateLayeredImageButton(spawnButtons, chain, chainArrow, {8, 8}, []() { Playtest::SpawnNote(true, true); });
    rChain = CreateLayeredImageButton(spawnButtons, chain, chainArrow, {8, 8}, []() { Playtest::SpawnNote(false, true); });

    CreateSpacer(spawnButtons, 3);

    wall = CreateLayeredImageButton(spawnButtons, PNG_SPRITE(Wall), PNG_SPRITE(Frame), {8, 16}, []() { Playtest::SpawnWall(); });

    CreateSpacer(spawnButtons, 3);

    auto bomb = CreateLayeredImageButton(spawnButtons, PNG_SPRITE(Bomb), nullptr, {8, 8}, []() { Playtest::SpawnBomb(); });
    SetClickableImageColor(bomb, {0.2, 0.2, 0.2, 1});

    pbToggle = BSML::Lite::CreateToggle(parent, "Personal Best", true, [this](bool enabled) {
        Playtest::SetPersonalBest(enabled ? 0 : -1);
        UpdateUI();
    });

    pbSlider = BSML::Lite::CreateSliderSetting(parent, "", 1, 1, 1, 120, 0, Playtest::SetPersonalBest);
    pbSlider = Utils::ReparentSlider(pbSlider, pbToggle, 30);
    pbSlider->GetComponent<RectTransform*>()->anchoredPosition = {-20, 0};
    pbSlider->formatter = percentFormat;

    // for now, the settings "song" is 24 seconds long
    timeSlider = BSML::Lite::CreateSliderSetting(parent, "Song Progress", 1, 0, 0, 24, 0, true, {0, 0}, Playtest::SetSongTime);

    posModsIncrement = BSML::Lite::CreateIncrementSetting(parent, "Positive Modifiers", 0, 5, 0, 0, 50, Playtest::SetPositiveModifiers);
    posModsIncrement->formatter = percentFormat;
    negModsIncrement = BSML::Lite::CreateIncrementSetting(parent, "Negative Modifiers", 0, 5, 0, -50, 0, Playtest::SetNegativeModifiers);
    negModsIncrement->formatter = percentFormat;

    blToggle = BSML::Lite::CreateToggle(parent, "BeatLeader Ranked", true, [this](bool enabled) {
        Playtest::SetRankedBL(enabled);
        blSlider->gameObject->active = enabled;
    });

    blSlider = BSML::Lite::CreateSliderSetting(parent, "", 0.1, 1, 1, 15, 0, Playtest::SetStarsBL);
    blSlider = Utils::ReparentSlider(blSlider, blToggle, 28);
    blSlider->GetComponent<RectTransform*>()->anchoredPosition = {-20, 0};

    ssToggle = BSML::Lite::CreateToggle(parent, "ScoreSaber Ranked", true, [this](bool enabled) {
        Playtest::SetRankedSS(enabled);
        ssSlider->gameObject->active = enabled;
    });

    ssSlider = BSML::Lite::CreateSliderSetting(parent, "", 0.1, 1, 1, 15, 0, Playtest::SetStarsSS);
    ssSlider = Utils::ReparentSlider(ssSlider, ssToggle, 28);
    ssSlider->GetComponent<RectTransform*>()->anchoredPosition = {-20, 0};

    auto resetLayout = BSML::Lite::CreateHorizontalLayoutGroup(parent);
    resetLayout->childForceExpandWidth = false;
    resetLayout->spacing = 3;
    resetLayout->childAlignment = TextAnchor::MiddleCenter;
    resetButton = BSML::Lite::CreateUIButton(resetLayout, "Reset Notes", Playtest::ResetNotes);
    Utils::SetLayoutSize(resetButton, 30, 8);
    auto resetAllButton = BSML::Lite::CreateUIButton(resetLayout, "Reset All", Playtest::ResetAll);
    Utils::SetLayoutSize(resetAllButton, 30, 8);

    Utils::SetChildrenWidth(parent->transform, 75);

    uiInitialized = true;
    UpdateUI();
}

void PlaytestViewController::UpdateUI() {
    if (!uiInitialized)
        return;

    auto left = Game::GetColor((int) Sources::Color::Player::ColorSettings::LeftSaber);
    auto right = Game::GetColor((int) Sources::Color::Player::ColorSettings::RightSaber);

    SetClickableImageColor(lNote, left);
    SetClickableImageColor(rNote, right);
    SetClickableImageColor(lChain, left);
    SetClickableImageColor(rChain, right);

    SetClickableImageColor(wall, Game::GetColor((int) Sources::Color::Player::ColorSettings::Walls));

    Utils::InstantSetToggle(pbToggle, Game::GetBestScore() > 0);
    pbSlider->gameObject->active = Game::GetBestScore() > 0;
    pbSlider->set_Value(100 * Game::GetBestScore() / (float) Game::GetSongMaxScore());

    timeSlider->set_Value(Game::GetSongTime());

    int pos = (Game::GetModifierMultiplier(true, false) - 1) * 100;
    Utils::SetIncrementValue(posModsIncrement, pos);
    int neg = (Game::GetModifierMultiplier(false, true) - 1) * 100;
    Utils::SetIncrementValue(negModsIncrement, neg);

    Utils::InstantSetToggle(blToggle, PP::IsRankedBL());
    blSlider->gameObject->active = PP::IsRankedBL();
    blSlider->set_Value(settingsStarsBL);

    Utils::InstantSetToggle(ssToggle, PP::IsRankedSS());
    ssSlider->gameObject->active = PP::IsRankedSS();
    ssSlider->set_Value(settingsStarsSS);
}

PlaytestViewController* PlaytestViewController::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<PlaytestViewController*>();
    return instance;
}

void PlaytestViewController::OnDestroy() {
    instance = nullptr;
}

void TemplatesViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation)
        return;

    auto background = AddBackground(this, {50, 88});

    list = BSML::Lite::CreateScrollableList(background, {50, 80}, [this](int idx) {
        list->tableView->ClearSelection();
        ShowTemplateModal(idx);
    });
    list->listStyle = BSML::CustomListTableData::ListStyle::Simple;
    auto rect = list->transform->parent.cast<RectTransform>();
    rect->anchorMin = {0.5, 0.5};
    rect->anchorMax = {0.5, 0.5};

    for (auto templateName : Utils::GetKeys(Templates::registration))
        list->data->Add(BSML::CustomCellInfo::construct(templateName));

    list->tableView->ReloadData();
    list->simpleTextTableCell = nullptr;

    modal = BSML::Lite::CreateModal(this);
    modalLayout = BSML::Lite::CreateVerticalLayoutGroup(modal)->rectTransform;
    modalLayout->anchorMin = {0.5, 0.5};
    modalLayout->anchorMax = {0.5, 0.5};

    auto fitter = modalLayout->GetComponent<UI::ContentSizeFitter*>();
    fitter->verticalFit = UI::ContentSizeFitter::FitMode::PreferredSize;
    fitter->horizontalFit = UI::ContentSizeFitter::FitMode::PreferredSize;

    uiInitialized = true;
}

void TemplatesViewController::ShowTemplateModal(int idx) {
    if (!uiInitialized)
        return;

    while (modalLayout->GetChildCount() > 0)
        Object::DestroyImmediate(modalLayout->GetChild(0)->gameObject);

    Templates::registration[idx].second(modalLayout->gameObject);
    Utils::SetChildrenWidth(modalLayout, 75);

    modal->Show(true, true, nullptr);
    BSML::MainThreadScheduler::ScheduleNextFrame([this]() {
        modal->GetComponent<RectTransform*>()->sizeDelta = Vector2::op_Addition(modalLayout->sizeDelta, {5, 5});
    });
}

void TemplatesViewController::HideModal() {
    if (!uiInitialized)
        return;

    modal->Hide(true, nullptr);
}

TemplatesViewController* TemplatesViewController::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<TemplatesViewController*>();
    return instance;
}

void TemplatesViewController::OnDestroy() {
    instance = nullptr;
}

void OptionsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation) {
        UpdateUI();
        return;
    }

    auto background = AddBackground(this, {95, 88});

    lockSprite = PNG_SPRITE(Lock);
    unlockSprite = PNG_SPRITE(Unlock);

    groupParent = BSML::Lite::CreateVerticalLayoutGroup(background);
    groupParent->childForceExpandHeight = false;
    groupParent->childControlHeight = false;
    groupParent->rectTransform->anchoredPosition = {0, -4};

    gPosIncrementX = BSML::Lite::CreateIncrementSetting(groupParent, "X Position", 1, 0.5, 0, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).Position.x = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementX, 5);
    gPosIncrementY = BSML::Lite::CreateIncrementSetting(groupParent, "Y Position", 1, 0.5, 0, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).Position.y = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementY, 5);

    auto xPosLayout = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    xPosLayout->spacing = 2;
    gDetPosIncrementX = BSML::Lite::CreateIncrementSetting(xPosLayout, "X Position", 2, 0.01, 0, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedPosition.x = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gDetPosIncrementX, 0.25);
    gDetPosLockX = Utils::CreateIconButton(xPosLayout->gameObject, nullptr, [this]() {
        auto& group = Editor::GetSelectedGroup(-1);
        group.LockPosX = !group.LockPosX;
        UpdateSimpleUI();
    });

    auto yPosLayout = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    yPosLayout->spacing = 2;
    gDetPosIncrementY = BSML::Lite::CreateIncrementSetting(yPosLayout, "Y Position", 2, 0.01, 0, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedPosition.y = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gDetPosIncrementY, 0.25);
    gDetPosLockY = Utils::CreateIconButton(yPosLayout->gameObject, nullptr, [this]() {
        auto& group = Editor::GetSelectedGroup(-1);
        group.LockPosY = !group.LockPosY;
        UpdateSimpleUI();
    });

    auto zPosLayout = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    zPosLayout->spacing = 2;
    gDetPosIncrementZ = BSML::Lite::CreateIncrementSetting(zPosLayout, "Z Position", 2, 0.01, 0, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedPosition.z = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gDetPosIncrementZ, 0.25);
    gDetPosLockZ = Utils::CreateIconButton(zPosLayout->gameObject, nullptr, [this]() {
        auto& group = Editor::GetSelectedGroup(-1);
        group.LockPosZ = !group.LockPosZ;
        UpdateSimpleUI();
    });

    gRotSlider = BSML::Lite::CreateSliderSetting(groupParent, "Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSlider, [](float _) { Editor::FinalizeAction(); });

    auto xRotLayout = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    xRotLayout->spacing = 1;
    gDetRotSliderX = BSML::Lite::CreateSliderSetting(xRotLayout, "X Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gDetRotSliderX, [](float _) { Editor::FinalizeAction(); });
    gDetRotLockX = Utils::CreateIconButton(xRotLayout->gameObject, nullptr, [this]() {
        auto& group = Editor::GetSelectedGroup(-1);
        group.LockRotX = !group.LockRotX;
        UpdateSimpleUI();
    });

    auto yRotLayout = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    yRotLayout->spacing = 1;
    gDetRotSliderY = BSML::Lite::CreateSliderSetting(yRotLayout, "Y Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gDetRotSliderY, [](float _) { Editor::FinalizeAction(); });
    gDetRotLockY = Utils::CreateIconButton(yRotLayout->gameObject, nullptr, [this]() {
        auto& group = Editor::GetSelectedGroup(-1);
        group.LockRotY = !group.LockRotY;
        UpdateSimpleUI();
    });

    auto zRotLayout = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    zRotLayout->spacing = 1;
    gDetRotSliderZ = BSML::Lite::CreateSliderSetting(zRotLayout, "Z Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.z = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gDetRotSliderZ, [](float _) { Editor::FinalizeAction(); });
    gDetRotLockZ = Utils::CreateIconButton(zRotLayout->gameObject, nullptr, [this]() {
        auto& group = Editor::GetSelectedGroup(-1);
        group.LockRotZ = !group.LockRotZ;
        UpdateSimpleUI();
    });

    auto gButtonsParent1 = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    gButtonsParent1->spacing = 3;

    gComponentButton = BSML::Lite::CreateUIButton(gButtonsParent1, "Add Component", Editor::AddComponent);

    gDetachButton = BSML::Lite::CreateUIButton(gButtonsParent1, "Detach", Editor::ToggleAttachment);

    auto gButtonsParent2 = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    gButtonsParent2->spacing = 3;

    gDeleteButton = BSML::Lite::CreateUIButton(gButtonsParent2, "Delete", Editor::Remove);

    gDeselectButton = BSML::Lite::CreateUIButton(gButtonsParent2, "Deselect", Editor::Deselect);

    Utils::SetChildrenWidth(groupParent->transform, 85);

    componentParent = BSML::Lite::CreateScrollView(background);

    auto positionCollapse = Utils::CreateCollapseArea(componentParent, "Position Options", true);

    cPosIncrementX = BSML::Lite::CreateIncrementSetting(componentParent, "Rel. X Position", 1, 0.5, 0, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Position.x = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(cPosIncrementX, 5);
    cPosIncrementY = BSML::Lite::CreateIncrementSetting(componentParent, "Rel. Y Position", 1, 0.5, 0, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Position.y = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(cPosIncrementY, 5);

    cRotSlider = BSML::Lite::CreateSliderSetting(componentParent, "Relative Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cRotSlider, [](float _) { Editor::FinalizeAction(); });
    cRotSlider->GetComponent<RectTransform*>()->sizeDelta = {0, 8};

    cScaleSliderX = BSML::Lite::CreateSliderSetting(componentParent, "X Scale", 0.01, 0, -1, 1, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.x = CalculateScale(val);
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderX, [](float _) { Editor::FinalizeAction(); });
    cScaleSliderX->formatter = ScaleFormat;
    cScaleSliderX->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    cScaleSliderY = BSML::Lite::CreateSliderSetting(componentParent, "Y Scale", 0.01, 0, -1, 1, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.y = CalculateScale(val);
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderY, [](float _) { Editor::FinalizeAction(); });
    cScaleSliderY->formatter = ScaleFormat;
    cScaleSliderY->GetComponent<RectTransform*>()->sizeDelta = {0, 8};

    positionCollapse->AddContents({cPosIncrementX, cPosIncrementY, cRotSlider, cScaleSliderX, cScaleSliderY});
    positionCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    auto typeCollapse = Utils::CreateCollapseArea(componentParent, "Text Options", true);
    typeCollapseComponent = typeCollapse;

    cTypeDropdown = Utils::CreateDropdownEnum(componentParent, "Type", 0, Options::TypeStrings, [typeCollapse](int val) {
        typeCollapse->title = std::string(Options::TypeStrings[val]) + " Options";
        typeCollapse->UpdateOpen();
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Type = val;
        Editor::UpdateType();
        Editor::FinalizeAction();
    });

    cTypeOptions = BSML::Lite::CreateVerticalLayoutGroup(componentParent);
    cTypeOptions->GetComponent<UI::ContentSizeFitter*>()->verticalFit = UI::ContentSizeFitter::FitMode::PreferredSize;

    typeCollapse->AddContents({cTypeDropdown->transform->parent, cTypeOptions});
    typeCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    auto colorCollapse = Utils::CreateCollapseArea(componentParent, "Color Options", false);
    colorCollapseComponent = colorCollapse;

    cColorSourceDropdown = Utils::CreateDropdown(componentParent, "Color Source", "", Utils::GetKeys(Sources::colors), [this](std::string val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        if (comp.ColorSource != val) {
            cColorSourceDropdown->dropdown->Hide(false);
            comp.ColorSource = val;
            Editor::UpdateColorSource();
        }
        Editor::FinalizeAction();
    });

    cColorSourceOptions = BSML::Lite::CreateVerticalLayoutGroup(componentParent);
    cColorSourceOptions->GetComponent<UI::ContentSizeFitter*>()->verticalFit = UI::ContentSizeFitter::FitMode::PreferredSize;

    cGradientToggle = BSML::Lite::CreateToggle(componentParent, "Gradient", false, [](bool value) {
        static int id = Editor::GetActionId();
        auto& gradient = Editor::GetSelectedComponent(id).GradientOptions;
        if (value != gradient.Enabled) {
            gradient.Enabled = value;
            Editor::UpdateGradientOptions();
        }
        Editor::FinalizeAction();
        GetInstance()->UpdateUI();
    });

    auto gradientCollapse = Utils::CreateCollapseArea(componentParent, "Gradient Options", true);
    gradientCollapseComponent = gradientCollapse;

    auto directionAndSwap = BSML::Lite::CreateHorizontalLayoutGroup(componentParent);
    directionAndSwap->spacing = 1;

    cGradientDirectionDropdown = Utils::CreateDropdownEnum(directionAndSwap->gameObject, "Direction", 0, Options::DirectionStrings, [](int value) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).GradientOptions.Direction = value;
        Editor::UpdateGradientOptions();
        Editor::FinalizeAction();
    });
    auto parent = cGradientDirectionDropdown->transform->parent;
    Utils::SetLayoutSize(parent, 50, 8);

    auto swap = BSML::Lite::CreateUIButton(directionAndSwap, "Swap Colors", Editor::SwapGradientColors);
    Utils::SetLayoutSize(swap, 24, 8);

    auto startHsvController = Utils::CreateHSVModifierPicker(
        componentParent,
        "Start Modifier",
        [](Vector3 hsv) {
            static int id = Editor::GetActionId();
            Editor::GetSelectedComponent(id).GradientOptions.StartModifierHSV = hsv;
            Editor::UpdateGradientOptions();
        },
        Editor::FinalizeAction
    );
    startHsvComponent = startHsvController;

    auto endHsvController = Utils::CreateHSVModifierPicker(
        componentParent,
        "End Modifier",
        [](Vector3 hsv) {
            static int id = Editor::GetActionId();
            Editor::GetSelectedComponent(id).GradientOptions.EndModifierHSV = hsv;
            Editor::UpdateGradientOptions();
        },
        Editor::FinalizeAction
    );
    endHsvComponent = endHsvController;

    gradientCollapse->AddContents({directionAndSwap, startHsvController, endHsvController});
    gradientCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    colorCollapse->AddContents({cGradientToggle, gradientCollapse, cColorSourceDropdown->transform->parent, cColorSourceOptions});
    colorCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    auto enableCollapse = Utils::CreateCollapseArea(componentParent, "Visibility Options", false);

    cEnableSourceDropdown = Utils::CreateDropdown(componentParent, "Visible If", "", Utils::GetKeys(Sources::enables), [this](std::string val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        if (comp.EnableSource != val) {
            cEnableSourceDropdown->dropdown->Hide(false);
            comp.EnableSource = val;
            Editor::UpdateEnableSource();
        }
        Editor::FinalizeAction();
    });

    cInvertEnableToggle = BSML::Lite::CreateToggle(componentParent, "Invert Visibility", false, [](bool val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).InvertEnable = val;
        Editor::UpdateEnableOptions();
        Editor::FinalizeAction();
    });

    cEnableSourceOptions = BSML::Lite::CreateVerticalLayoutGroup(componentParent);
    cEnableSourceOptions->GetComponent<UI::ContentSizeFitter*>()->verticalFit = UI::ContentSizeFitter::FitMode::PreferredSize;

    enableCollapse->AddContents({cEnableSourceDropdown->transform->parent, cInvertEnableToggle, cEnableSourceOptions});
    enableCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    cButtonsParent = BSML::Lite::CreateHorizontalLayoutGroup(background);
    cButtonsParent->spacing = 3;
    cButtonsParent->childControlWidth = false;
    cButtonsParent->childForceExpandWidth = false;
    cButtonsParent->childAlignment = TextAnchor::MiddleCenter;
    cButtonsParent->rectTransform->anchoredPosition = {0, -37};

    cDeleteButton = BSML::Lite::CreateUIButton(cButtonsParent, "Delete", Editor::Remove);

    cDeselectButton = BSML::Lite::CreateUIButton(cButtonsParent, "Deselect", Editor::Deselect);

    Object::Destroy(componentParent->GetComponentInParent<BSML::ScrollViewContent*>(true));
    auto componentTop = Utils::GetScrollViewTop(componentParent);
    // kinda janky spacing for the delete/deselect buttons
    componentTop->sizeDelta = {-5, -10};
    componentTop->anchoredPosition = {0, 4};
    componentParent->GetComponent<UI::VerticalLayoutGroup*>()->spacing = 0;
    float width = background->GetComponent<RectTransform*>()->sizeDelta.x;
    componentParent->GetComponent<RectTransform*>()->sizeDelta = {width - 20, 0};
    Utils::SetChildrenWidth(componentParent->transform, width - 20);

    uiInitialized = true;
    UpdateUI();
    BSML::MainThreadScheduler::ScheduleNextFrame(UpdateScrollViewStatic);
}

void OptionsViewController::Deselect() {
    component = false;
    group = false;
    UpdateUI();
}

void OptionsViewController::GroupSelected() {
    component = false;
    group = true;
    UpdateUI();
}

void OptionsViewController::ComponentSelected() {
    component = true;
    group = false;
    UpdateUI();
}

void OptionsViewController::UpdateScrollView() {
    if (uiInitialized && component)
        Utils::RebuildWithScrollPosition(componentParent);
}

void OptionsViewController::UpdateSimpleUI() {
    if (!uiInitialized)
        return;

    float height = 88;
    if (group)
        height = Editor::GetSelectedGroup(-1).Detached ? 70 : 47;
    auto background = transform->GetChild(0)->GetComponent<RectTransform*>();
    background->sizeDelta = {background->sizeDelta.x, height};
    groupParent->gameObject->active = group;
    Utils::GetScrollViewTop(componentParent)->gameObject->active = component;
    cButtonsParent->gameObject->active = component;

    Editor::DisableActions();

    if (group) {
        auto& state = Editor::GetSelectedGroup(-1);
        if (state.Detached) {
            Utils::SetIncrementValue(gDetPosIncrementX, state.DetachedPosition.x);
            Utils::SetIconButtonSprite(gDetPosLockX, state.LockPosX ? lockSprite : unlockSprite);
            Utils::SetIncrementValue(gDetPosIncrementY, state.DetachedPosition.y);
            Utils::SetIconButtonSprite(gDetPosLockY, state.LockPosY ? lockSprite : unlockSprite);
            Utils::SetIncrementValue(gDetPosIncrementZ, state.DetachedPosition.z);
            Utils::SetIconButtonSprite(gDetPosLockZ, state.LockPosZ ? lockSprite : unlockSprite);
            gDetRotSliderX->set_Value(state.DetachedRotation.x);
            Utils::SetIconButtonSprite(gDetRotLockX, state.LockRotX ? lockSprite : unlockSprite);
            gDetRotSliderY->set_Value(state.DetachedRotation.y);
            Utils::SetIconButtonSprite(gDetRotLockY, state.LockRotY ? lockSprite : unlockSprite);
            gDetRotSliderZ->set_Value(state.DetachedRotation.z);
            Utils::SetIconButtonSprite(gDetRotLockZ, state.LockRotZ ? lockSprite : unlockSprite);
        } else {
            Utils::SetIncrementValue(gPosIncrementX, state.Position.x);
            Utils::SetIncrementValue(gPosIncrementY, state.Position.y);
            gRotSlider->set_Value(state.Rotation);
        }
        gPosIncrementX->gameObject->active = !state.Detached;
        gPosIncrementY->gameObject->active = !state.Detached;
        gDetPosIncrementX->transform->parent->gameObject->active = state.Detached;
        gDetPosIncrementY->transform->parent->gameObject->active = state.Detached;
        gDetPosIncrementZ->transform->parent->gameObject->active = state.Detached;
        gRotSlider->gameObject->active = !state.Detached;
        gDetRotSliderX->transform->parent->gameObject->active = state.Detached;
        gDetRotSliderY->transform->parent->gameObject->active = state.Detached;
        gDetRotSliderZ->transform->parent->gameObject->active = state.Detached;
        BSML::Lite::SetButtonText(gDetachButton, state.Detached ? "Attach" : "Detach");
    } else if (component) {
        auto& state = Editor::GetSelectedComponent(-1);
        Utils::SetIncrementValue(cPosIncrementX, state.Position.x);
        Utils::SetIncrementValue(cPosIncrementY, state.Position.y);
        cRotSlider->set_Value(state.Rotation);
        cScaleSliderX->set_Value(CalculateScaleInverse(state.Scale.x));
        cScaleSliderY->set_Value(CalculateScaleInverse(state.Scale.y));
        auto typeCollapse = (CollapseController*) typeCollapseComponent;
        typeCollapse->title = std::string(Options::TypeStrings[state.Type]) + " Options";
        typeCollapse->UpdateOpen();
        cTypeDropdown->dropdown->SelectCellWithIdx(state.Type);
        bool supportsGradient = state.Type != (int) Options::Component::Types::Premade;
        auto colorCollapse = (CollapseController*) colorCollapseComponent;
        colorCollapse->SetContentActive(cGradientToggle, supportsGradient);
        if (supportsGradient) {
            Utils::InstantSetToggle(cGradientToggle, state.GradientOptions.Enabled);
            colorCollapse->SetContentActive(gradientCollapseComponent, state.GradientOptions.Enabled);
            cGradientDirectionDropdown->dropdown->SelectCellWithIdx(state.GradientOptions.Direction);
            // update hsv panels
            auto startHsvController = (HSVController*) startHsvComponent;
            startHsvController->SetHSV(state.GradientOptions.StartModifierHSV);
            auto endHsvController = (HSVController*) endHsvComponent;
            endHsvController->SetHSV(state.GradientOptions.EndModifierHSV);
            auto sourceFn = Sources::GetSource(Sources::colors, state.ColorSource).first;
            if (sourceFn) {
                auto color = sourceFn(state.ColorOptions);
                startHsvController->baseColor = color;
                endHsvController->baseColor = color;
            }
        } else
            colorCollapse->SetContentActive(gradientCollapseComponent, false);
        Utils::SetDropdownValue(cColorSourceDropdown, state.ColorSource);
        Utils::SetDropdownValue(cEnableSourceDropdown, state.EnableSource);
        Utils::InstantSetToggle(cInvertEnableToggle, state.InvertEnable);
    } else
        SettingsFlowCoordinator::PresentTemplates();

    Editor::EnableActions();
}

void OptionsViewController::UpdateUI() {
    UpdateSimpleUI();

    for (auto modal : GetComponentsInChildren<HMUI::ModalView*>())
        modal->Hide(false, nullptr);

    if (!uiInitialized || !component)
        return;

    Editor::DisableActions();

    UpdateTypeOptions();
    UpdateColorSourceOptions();
    UpdateEnableSourceOptions();
    Utils::RebuildWithScrollPosition(componentParent);

    Editor::EnableActions();
}

void OptionsViewController::UpdateTypeOptions() {
    auto& state = Editor::GetSelectedComponent(-1);
    Options::CreateTypeUI(cTypeOptions->transform, state.Type, state.Options);
}

void OptionsViewController::UpdateColorSourceOptions() {
    auto& state = Editor::GetSelectedComponent(-1);
    Sources::Color::CreateUI(cColorSourceOptions->gameObject, state.ColorSource, state.ColorOptions);
}

void OptionsViewController::UpdateEnableSourceOptions() {
    auto& state = Editor::GetSelectedComponent(-1);
    Sources::Enable::CreateUI(cEnableSourceOptions->gameObject, state.EnableSource, state.EnableOptions);
}

OptionsViewController* OptionsViewController::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<OptionsViewController*>();
    return instance;
}

void OptionsViewController::UpdateScrollViewStatic() {
    if (instance)
        instance->UpdateScrollView();
}

void OptionsViewController::OnDestroy() {
    instance = nullptr;
}

void HSVController::Show() {
    if (!modal)
        return;
    modal->Show(true, true, nullptr);
    UpdateVisuals();
}

void HSVController::Hide() {
    if (onClose)
        onClose();
    if (!modal)
        return;
    modal->Hide(true, nullptr);
}

void HSVController::SetHue(float hue) {
    hsv.x = hue / 2;
    if (onChange)
        onChange(hsv);
    UpdateVisuals();
}

void HSVController::SetSat(float saturation) {
    hsv.y = saturation;
    if (onChange)
        onChange(hsv);
    UpdateVisuals();
}

void HSVController::SetVal(float value) {
    hsv.z = value;
    if (onChange)
        onChange(hsv);
    UpdateVisuals();
}

void HSVController::UpdateVisuals() {
    UpdateButton();
    if (!hSlider || !sSlider || !vSlider)
        return;
    float h, s, v;
    Color::RGBToHSV(baseColor, byref(h), byref(s), byref(v));
    Color color = Utils::GetClampedColor({h + hsv.x, s + hsv.y, v + hsv.z});
    hSlider->SetColors(color, color);
    sSlider->SetColors(color, color);
    vSlider->SetColors(color, color);
    color = Utils::GetClampedColor({h, s + hsv.y, v + hsv.z});
    for (auto img : hSlider->_gradientImages)
        img->color = color;
    // prevent losing hue at 0 sat
    color = Utils::GetClampedColor({h + hsv.x, std::max(s, 0.01f), v + hsv.z});
    for (auto img : sSlider->_gradientImages)
        img->color = color;
    color = Utils::GetClampedColor({h + hsv.x, s + hsv.y, v});
    for (auto img : vSlider->_gradientImages)
        img->color = color;
    // (-0.5, 0.5) -> (0, 1)
    hSlider->normalizedValue = hsv.x + 0.5;
    // (-1, 1) -> (0, 1)
    sSlider->normalizedValue = (hsv.y + 1) / 2;
    vSlider->normalizedValue = (hsv.z + 1) / 2;
    hSlider->_valueText->text = fmt::format("H: {:+.1f}", hsv.x);
    sSlider->_valueText->text = fmt::format("S: {:+.1f}", hsv.y);
    vSlider->_valueText->text = fmt::format("V: {:+.1f}", hsv.z);
}

void HSVController::UpdateButton() {
    if (openButton)
        BSML::Lite::SetButtonText(openButton, fmt::format("H{:+.1f} S{:+.1f} V{:+.1f}", hsv.x, hsv.y, hsv.z));
}

void HSVController::SetHSV(Vector3 value) {
    hsv = value;
    UpdateButton();
}

void CollapseController::OnPointerEnter(EventSystems::PointerEventData* eventData) {
    text->color = {0.6, 0.6, 0.6, 1};
    line->color = {0.5, 0.5, 0.5, 1};
}

void CollapseController::OnPointerExit(EventSystems::PointerEventData* eventData) {
    text->color = {1, 1, 1, 1};
    line->color = {0.8, 0.8, 0.8, 1};
}

void CollapseController::OnPointerClick(EventSystems::PointerEventData* eventData) {
    open = !open;
    UpdateOpen();
}

void CollapseController::OnEnable() {
    UpdateOpen();
    if (text && line)
        OnPointerExit(nullptr);
}

void CollapseController::OnDisable() {
    UpdateOpen();
}

void CollapseController::UpdateOpen() {
    if (text)
        text->text = StringW(open ? u"▼  " : u"▶  ") + title;
    for (auto& [obj, active] : contents) {
        if (wasOpen)
            active = obj->gameObject->activeSelf;
        if (!open || !isActiveAndEnabled)
            obj->gameObject->active = false;
        else
            obj->gameObject->active = active;
    }
    if (onUpdate && isActiveAndEnabled)
        onUpdate();
    wasOpen = open && isActiveAndEnabled;
}

void CollapseController::AddContents(std::set<Component*> const& add) {
    for (auto comp : add) {
        if (contents.contains(comp))
            continue;
        contents.emplace(comp, comp->gameObject->activeSelf);
    }
    UpdateOpen();
}

void CollapseController::RemoveContents(std::set<Component*> const& remove) {
    for (auto comp : remove) {
        if (!contents.contains(comp))
            continue;
        comp->gameObject->active = contents[comp];
        contents.erase(comp);
    }
    if (onUpdate && isActiveAndEnabled)
        onUpdate();
}

void CollapseController::SetContentActive(Component* component, bool active) {
    if (open || !contents.contains(component))
        component->gameObject->active = active;
    else
        contents[component] = active;
}

inline float ClampInRadius(float pos) {
    float radius = SettingsFlowCoordinator::GetRadius();
    float circumference = 2 * radius * std::numbers::pi;
    float quarter = circumference * 0.25;
    return std::clamp(pos, -quarter, quarter);
}

void MenuDragger::OnEnable() {
    if (line)
        line->color = {0.8, 0.8, 0.8, 1};
    if (menu) {
        auto offset = (isLeftMenu ? getConfig().LeftOffset : getConfig().RightOffset).GetValue();
        auto pos = menu->anchoredPosition;
        originalPosition = pos.x;
        menu->anchoredPosition = {ClampInRadius(pos.x + offset), pos.y};
    }
}

void MenuDragger::OnDisable() {
    if (menu)
        menu->anchoredPosition = {originalPosition, menu->anchoredPosition.y};
    if (dragCanvas)
        dragCanvas->active = false;
}

void MenuDragger::OnPointerEnter(EventSystems::PointerEventData* eventData) {
    line->color = {0.5, 0.5, 0.5, 1};
}

void MenuDragger::OnPointerExit(EventSystems::PointerEventData* eventData) {
    line->color = {0.8, 0.8, 0.8, 1};
}

void MenuDragger::OnInitializePotentialDrag(EventSystems::PointerEventData* eventData) {
    eventData->useDragThreshold = false;
    float offset = (isLeftMenu ? getConfig().LeftOffset : getConfig().RightOffset).GetValue();
    offset = ClampInRadius(originalPosition + offset) - originalPosition;
    dragPosition = GetPointerPosX(eventData) - offset;
}

void MenuDragger::OnDrag(EventSystems::PointerEventData* eventData) {
    if (!menu || !IsPointerPosValid(eventData))
        return;
    if (dragCanvas)
        dragCanvas->active = true;
    float offset = GetPointerPosX(eventData) - dragPosition;
    (isLeftMenu ? getConfig().LeftOffset : getConfig().RightOffset).SetValue(offset);
    menu->anchoredPosition = {ClampInRadius(originalPosition + offset), menu->anchoredPosition.y};
}

void MenuDragger::OnEndDrag(EventSystems::PointerEventData* eventData) {
    if (dragCanvas)
        dragCanvas->active = false;
}

bool MenuDragger::IsPointerPosValid(EventSystems::PointerEventData* eventData) {
    auto raycast = eventData->pointerCurrentRaycast;
    if (!raycast.isValid)
        return false;
    return raycast.screenPosition.x < System::Single::MaxValue || raycast.screenPosition.y < System::Single::MaxValue;
}

float MenuDragger::GetPointerPosX(EventSystems::PointerEventData* eventData) {
    if (!rootCanvas)
        rootCanvas = GetComponentInParent<Canvas*>()->rootCanvas;
    auto screenPos = eventData->pointerCurrentRaycast.screenPosition;
    return rootCanvas->transform->InverseTransformPoint({screenPos.x, screenPos.y, 0}).x;
}

void EndDragHandler::OnEndDrag(EventSystems::PointerEventData* eventData) {
    callback();
}

void SpritesListCell::OnImageClicked(int idx) {
    int realIdx = idx + imageStartIdx;
    auto source = (SpritesListSource*) ((HMUI::TableView*) tableCellOwner)->dataSource;
    source->OnImageClicked(realIdx);
}

void SpritesListCell::SetImageStartIdx(int idx) {
    imageStartIdx = idx;

    while (layout->GetChildCount() > 0)
        Object::DestroyImmediate(layout->GetChild(0)->gameObject);

    for (int i = 0; i < imagesPerCell; i++) {
        if (i + idx < ImageSpriteCache::NumberOfSprites()) {
            images[i] = BSML::Lite::CreateClickableImage(layout, ImageSpriteCache::GetSpriteIdx(i + idx), [this, i]() { OnImageClicked(i); });
            images[i]->preserveAspect = true;
            images[i]->GetComponent<UI::LayoutElement*>()->preferredWidth = cellSize;
        } else
            images[i] = nullptr;
    }
}

SpritesListCell* SpritesListCell::CreateNew(int imagesStartIdx, StringW reuseIdentifier) {
    auto object = GameObject::New_ctor("QountersSpritesListCell");

    object->AddComponent<RectTransform*>()->sizeDelta = {0, cellSize};

    auto ret = object->AddComponent<SpritesListCell*>();
    ret->reuseIdentifier = reuseIdentifier;

    auto layout = BSML::Lite::CreateHorizontalLayoutGroup(ret);
    layout->spacing = imageSpacing;
    layout->childForceExpandWidth = false;
    layout->childAlignment = TextAnchor::MiddleCenter;
    layout->GetComponent<UI::ContentSizeFitter*>()->verticalFit = UI::ContentSizeFitter::FitMode::Unconstrained;

    ret->images = ArrayW<BSML::ClickableImage*>(imagesPerCell);
    ret->layout = layout->rectTransform;
    ret->SetImageStartIdx(imagesStartIdx);

    return ret;
}

HMUI::TableCell* SpritesListSource::CellForIdx(HMUI::TableView* tableView, int idx) {
    auto ret = tableView->DequeueReusableCellForIdentifier(reuseIdentifier).try_cast<SpritesListCell>().value_or(nullptr);

    if (!ret)
        ret = SpritesListCell::CreateNew(idx * SpritesListCell::imagesPerCell, reuseIdentifier);
    else
        ret->SetImageStartIdx(idx * SpritesListCell::imagesPerCell);

    return ret;
}

float SpritesListSource::CellSize() {
    return SpritesListCell::cellSize;
}

int SpritesListSource::NumberOfCells() {
    return (ImageSpriteCache::NumberOfSprites() + SpritesListCell::imagesPerCell - 1) / SpritesListCell::imagesPerCell;
}

void SpritesListSource::OnImageClicked(int imageIdx) {
    imageClickedCallback(imageIdx);
}
