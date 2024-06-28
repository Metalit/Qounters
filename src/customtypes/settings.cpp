#include "customtypes/settings.hpp"

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/EnvironmentType.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "HMUI/ScreenSystem.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "bsml/shared/Helpers/extension.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "config.hpp"
#include "custom-types/shared/coroutine.hpp"
#include "custom-types/shared/delegate.hpp"
#include "customtypes/components.hpp"
#include "customtypes/editing.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "main.hpp"
#include "playtest.hpp"
#include "sources.hpp"
#include "sourceui.hpp"
#include "templates.hpp"
#include "utils.hpp"

DEFINE_TYPE(Qounters, SettingsFlowCoordinator);
DEFINE_TYPE(Qounters, SettingsViewController);
DEFINE_TYPE(Qounters, TemplatesViewController);
DEFINE_TYPE(Qounters, OptionsViewController);
DEFINE_TYPE(Qounters, EndDragHandler);
DEFINE_TYPE(Qounters, KeyboardCloseHandler);
DEFINE_TYPE(Qounters, SpritesListCell);
DEFINE_TYPE(Qounters, SpritesListSource);

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace Qounters;

void Qounters::SettingsFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (addedToHierarchy) {
        auto presets = getConfig().Presets.GetValue();
        auto presetName = getConfig().Preset.GetValue();
        if (!presets.contains(presetName)) {
            presetName = presets.begin()->first;
            getConfig().Preset.SetValue(presetName);
        }
        auto& preset = presets[presetName];

        Editor::Initialize(preset);
    }

    if (!blankViewController)
        blankViewController = BSML::Helpers::CreateViewController();

    ProvideInitialViewControllers(
        blankViewController, SettingsViewController::GetInstance(), TemplatesViewController::GetInstance(), nullptr, nullptr
    );
    Qounters::OptionsViewController::GetInstance()->Deselect();
}

void Qounters::SettingsFlowCoordinator::Save() {
    auto presets = getConfig().Presets.GetValue();
    presets[getConfig().Preset.GetValue()] = Editor::GetPreset();
    getConfig().Presets.SetValue(presets);
}

bool Qounters::SettingsFlowCoordinator::IsSaved() {
    return getConfig().Presets.GetValue()[getConfig().Preset.GetValue()] == Editor::GetPreset();
}

void Qounters::SettingsFlowCoordinator::PresentTemplates() {
    auto instance = GetInstance();
    auto templates = TemplatesViewController::GetInstance();
    if (instance->rightScreenViewController != templates)
        instance->SetRightScreenViewController(templates, HMUI::ViewController::AnimationType::In);
}

void Qounters::SettingsFlowCoordinator::PresentOptions() {
    auto instance = GetInstance();
    auto options = Qounters::OptionsViewController::GetInstance();
    if (instance->rightScreenViewController != options)
        instance->SetRightScreenViewController(options, HMUI::ViewController::AnimationType::In);
}

void Qounters::SettingsFlowCoordinator::DismissScene() {
    ConfirmAction(DismissSettingsEnvironment);
}

void Qounters::SettingsFlowCoordinator::RefreshScene() {
    if (CurrentSettingsEnvironment() != getConfig().Environment.GetValue())
        ConfirmAction(RefreshSettingsEnvironment);
}

void Qounters::SettingsFlowCoordinator::OnModalConfirm() {
    if (nextModalAction)
        nextModalAction();
    SettingsViewController::GetInstance()->HideConfirmModal();
}

void Qounters::SettingsFlowCoordinator::OnModalCancel() {
    if (nextModalCancel)
        nextModalCancel();
    SettingsViewController::GetInstance()->HideConfirmModal();
}

void Qounters::SettingsFlowCoordinator::SelectPreset(StringW name) {
    ConfirmAction(
        [name = (std::string) name]() {
            auto presets = getConfig().Presets.GetValue();
            if (!presets.contains(name)) {
                SettingsViewController::GetInstance()->UpdateUI();
                return;
            }
            getConfig().Preset.SetValue(name);
            Editor::LoadPreset(presets[name]);
        },
        []() { SettingsViewController::GetInstance()->UpdateUI(); }
    );
}

void Qounters::SettingsFlowCoordinator::RenamePreset(StringW name) {
    MakeNewPreset(name, true);
}

void Qounters::SettingsFlowCoordinator::DuplicatePreset(StringW newName) {
    ConfirmAction([name = (std::string) newName]() { MakeNewPreset(name, false); });
}

void Qounters::SettingsFlowCoordinator::DeletePreset() {
    auto presets = getConfig().Presets.GetValue();
    if (presets.size() < 2)
        return;

    auto name = getConfig().Preset.GetValue();
    presets.erase(name);
    getConfig().Presets.SetValue(presets);

    name = presets.begin()->first;
    getConfig().Preset.SetValue(name);

    Editor::LoadPreset(presets[name]);
}

Qounters::SettingsFlowCoordinator* Qounters::SettingsFlowCoordinator::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateFlowCoordinator<Qounters::SettingsFlowCoordinator*>();
    return instance;
}

void Qounters::SettingsFlowCoordinator::OnDestroy() {
    instance = nullptr;
}

void Qounters::SettingsFlowCoordinator::ConfirmAction(std::function<void()> action, std::function<void()> cancel) {
    nextModalAction = action;
    nextModalCancel = cancel;
    if (IsSaved())
        nextModalAction();
    else
        SettingsViewController::GetInstance()->ShowConfirmModal();
}

void Qounters::SettingsFlowCoordinator::MakeNewPreset(std::string name, bool removeOld) {
    auto presets = getConfig().Presets.GetValue();
    if (presets.contains(name))
        return;

    auto currentName = getConfig().Preset.GetValue();
    if (removeOld) {
        presets[name] = std::move(presets[currentName]);
        presets.erase(currentName);
    } else {
        presets[name] = presets[currentName];
        Editor::LoadPreset(presets[name]);
    }

    getConfig().Preset.SetValue(name);
    getConfig().Presets.SetValue(presets);
    SettingsViewController::GetInstance()->UpdateUI();
}

void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation) {
        UpdateUI();
        return;
    }

    std::vector<std::string> dropdownStrings = {};
    auto environments = BSML::Helpers::GetMainFlowCoordinator()->_playerDataModel->_playerDataFileModel->_environmentsListModel;
    for (auto& env : ListW<EnvironmentInfoSO*>(environments->GetAllEnvironmentInfosWithType(EnvironmentType::Normal)))
        dropdownStrings.emplace_back(env->environmentName);
    for (auto& env : ListW<EnvironmentInfoSO*>(environments->GetAllEnvironmentInfosWithType(EnvironmentType::Circle)))
        dropdownStrings.emplace_back(env->environmentName);
    for (auto& env : environments->_envInfos) {
        std::string str = env->environmentName;
        bool add = true;
        for (auto& added : dropdownStrings) {
            if (added == str) {
                add = false;
                break;
            }
        }
        if (add)
            dropdownStrings.emplace_back(str);
    }
    std::vector<std::string_view> dropdownStringViews{dropdownStrings.begin(), dropdownStrings.end()};

    auto vertical = BSML::Lite::CreateVerticalLayoutGroup(this);
    vertical->childControlHeight = false;
    vertical->childForceExpandHeight = false;
    vertical->spacing = 1;

    auto buttons1 = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    buttons1->GetComponent<UI::LayoutElement*>()->preferredHeight = 9;
    buttons1->spacing = 3;
    undoButton = BSML::Lite::CreateUIButton(buttons1, "Undo", Editor::Undo);
    BSML::Lite::CreateUIButton(buttons1, "Exit", Qounters::SettingsFlowCoordinator::DismissScene);

    auto buttons2 = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    buttons2->GetComponent<UI::LayoutElement*>()->preferredHeight = 9;
    buttons2->spacing = 3;
    BSML::Lite::CreateUIButton(buttons2, "Save", Qounters::SettingsFlowCoordinator::Save);
    BSML::Lite::CreateUIButton(buttons2, "Save And Exit", "ActionButton", []() {
        Qounters::SettingsFlowCoordinator::Save();
        Qounters::SettingsFlowCoordinator::DismissScene();
    });

    auto environment = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    environment->spacing = 3;
    auto dropdown = AddConfigValueDropdownString(environment, getConfig().Environment, dropdownStringViews);
    dropdown->GetComponentsInParent<UI::LayoutElement*>(true)->First()->preferredWidth = 65;
    auto apply = BSML::Lite::CreateUIButton(environment, "Apply", Qounters::SettingsFlowCoordinator::RefreshScene);

    BSML::Lite::CreateText(vertical, "Changes to the preset list are always saved!", {0, 0}, {50, 8})->alignment =
        TMPro::TextAlignmentOptions::Center;

    presetDropdown = BSML::Lite::CreateDropdown(vertical, "Current Preset", "", {}, Qounters::SettingsFlowCoordinator::SelectPreset);

    auto buttons3 = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    buttons3->GetComponent<UI::LayoutElement*>()->preferredHeight = 9;
    buttons3->spacing = 3;
    BSML::Lite::CreateUIButton(buttons3, "Rename", [this]() {
        nameModalIsRename = true;
        nameInput->text = getConfig().Preset.GetValue();
        nameModal->Show(true, true, nullptr);
    });
    BSML::Lite::CreateUIButton(buttons3, "Duplicate", [this]() {
        nameModalIsRename = false;
        nameInput->text = getConfig().Preset.GetValue();
        nameModal->Show(true, true, nullptr);
    });
    deleteButton = BSML::Lite::CreateUIButton(buttons3, "Delete", Qounters::SettingsFlowCoordinator::DeletePreset);

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
    UnityEngine::Object::Destroy(snapToggle->text->gameObject);
    
    auto preview = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    preview->set_spacing(3);
    previewToggle = BSML::Lite::CreateToggle(preview, "Preview Mode", false, Editor::SetPreviewMode);
    previewToggle->GetComponentsInParent<UI::LayoutElement*>(true)->First()->set_preferredWidth(65);
    playtestButton = BSML::Lite::CreateUIButton(preview, "Playtest", []() {
        Qounters::PlayTest::SpawnSequence();
    });
    playtestButton->interactable = Editor::GetPreviewMode();

    confirmModal = BSML::Lite::CreateModal(this, {95, 25}, Qounters::SettingsFlowCoordinator::OnModalCancel);
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
    BSML::Lite::CreateUIButton(modalButtons, "Continue", Qounters::SettingsFlowCoordinator::OnModalConfirm);
    BSML::Lite::CreateUIButton(modalButtons, "Save And Continue", []() {
        Qounters::SettingsFlowCoordinator::Save();
        Qounters::SettingsFlowCoordinator::OnModalConfirm();
    });
    BSML::Lite::CreateUIButton(modalButtons, "Cancel", [this]() { Qounters::SettingsFlowCoordinator::OnModalCancel(); });

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
            Qounters::SettingsFlowCoordinator::RenamePreset(val);
        else
            Qounters::SettingsFlowCoordinator::DuplicatePreset(val);
    };

    uiInitialized = true;
    UpdateUI();
}

SettingsViewController* SettingsViewController::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<SettingsViewController*>();
    return instance;
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

void SettingsViewController::UpdateUI() {
    if (!uiInitialized)
        return;

    undoButton->interactable = Editor::HasUndo();

    auto presets = getConfig().Presets.GetValue();
    auto preset = getConfig().Preset.GetValue();
    auto texts = ListW<System::Object*>::New(presets.size());
    int selectedIdx = 0;
    int i = 0;
    for (auto& [name, _] : presets) {
        texts->Add((System::Object*) StringW(name).convert());
        if (name == preset)
            selectedIdx = i;
        i++;
    }
    presetDropdown->values = texts;
    presetDropdown->UpdateChoices();
    presetDropdown->set_Value(texts[selectedIdx]);

    deleteButton->interactable = presets.size() > 1;

    Utils::InstantSetToggle(previewToggle, Editor::GetPreviewMode());
}

void TemplatesViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation)
        return;

    list = BSML::Lite::CreateScrollableList(transform, {50, 80}, [this](int idx) {
        list->tableView->ClearSelection();
        ShowTemplateModal(idx);
    });
    list->listStyle = BSML::CustomListTableData::ListStyle::Simple;
    auto rect = list->transform->parent.cast<RectTransform>();
    rect->anchorMin = {0.5, 0.5};
    rect->anchorMax = {0.5, 0.5};

    for (auto templateName : Utils::GetKeys(templates))
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

custom_types::Helpers::Coroutine FitModalCoroutine(HMUI::ModalView* modal, RectTransform* modalLayout) {
    co_yield nullptr;

    modal->GetComponent<RectTransform*>()->sizeDelta = Vector2::op_Addition(modalLayout->sizeDelta, {5, 5});
}

void TemplatesViewController::ShowTemplateModal(int idx) {
    if (!uiInitialized)
        return;

    while (modalLayout->GetChildCount() > 0)
        UnityEngine::Object::DestroyImmediate(modalLayout->GetChild(0)->gameObject);

    templates[idx].second(modalLayout->gameObject);
    Utils::SetChildrenWidth(modalLayout, 75);

    modal->Show(true, true, nullptr);
    modal->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(FitModalCoroutine(modal, modalLayout)));
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

void Qounters::OptionsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    UpdateUI();

    if (!firstActivation)
        return;

    groupParent = BSML::Lite::CreateScrollView(this);

    gPosIncrementX = BSML::Lite::CreateIncrementSetting(groupParent, "X Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        auto& group = Editor::GetSelectedGroup(id);
        if (group.Detached)
            group.DetachedPosition.x = val;
        else
            group.Position.x = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementX, 5);
    gPosIncrementY = BSML::Lite::CreateIncrementSetting(groupParent, "Y Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        auto& group = Editor::GetSelectedGroup(id);
        if (group.Detached)
            group.DetachedPosition.y = val;
        else
            group.Position.y = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementY, 5);
    gPosIncrementZ = BSML::Lite::CreateIncrementSetting(groupParent, "Z Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedPosition.z = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementZ, 5);

    gRotSlider = BSML::Lite::CreateSliderSetting(groupParent, "Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSlider, [](float _) { Editor::FinalizeAction(); });
    gRotSlider->GetComponent<RectTransform*>()->sizeDelta = {0, 8};

    gRotSliderX = BSML::Lite::CreateSliderSetting(groupParent, "X Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderX, [](float _) { Editor::FinalizeAction(); });
    gRotSliderX->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    gRotSliderY = BSML::Lite::CreateSliderSetting(groupParent, "Y Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderY, [](float _) { Editor::FinalizeAction(); });
    gRotSliderY->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    gRotSliderZ = BSML::Lite::CreateSliderSetting(groupParent, "Z Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.z = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderZ, [](float _) { Editor::FinalizeAction(); });
    gRotSliderZ->GetComponent<RectTransform*>()->sizeDelta = {0, 8};

    auto gButtonsParent1 = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    gButtonsParent1->spacing = 3;

    gComponentButton = BSML::Lite::CreateUIButton(gButtonsParent1, "Add Component", Editor::AddComponent);

    gDetachButton = BSML::Lite::CreateUIButton(gButtonsParent1, "Detach", Editor::ToggleAttachment);

    auto gButtonsParent2 = BSML::Lite::CreateHorizontalLayoutGroup(groupParent);
    gButtonsParent2->spacing = 3;

    gDeleteButton = BSML::Lite::CreateUIButton(gButtonsParent2, "Delete", Editor::Remove);

    gDeselectButton = BSML::Lite::CreateUIButton(gButtonsParent2, "Deselect", Editor::Deselect);

    Utils::FixScrollView(groupParent, 75);

    componentParent = BSML::Lite::CreateScrollView(this);

    cPosIncrementX = BSML::Lite::CreateIncrementSetting(componentParent, "Rel. X Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Position.x = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(cPosIncrementX, 5);
    cPosIncrementY = BSML::Lite::CreateIncrementSetting(componentParent, "Rel. Y Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Position.y = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(cPosIncrementY, 5);

    cRotSlider = BSML::Lite::CreateSliderSetting(componentParent, "Relative Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cRotSlider, [](float _) { Editor::FinalizeAction(); });
    cRotSlider->GetComponent<RectTransform*>()->sizeDelta = {0, 8};

    cScaleSliderX = BSML::Lite::CreateSliderSetting(componentParent, "X Scale", 0.01, 0, 0.01, 10, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderX, [](float _) { Editor::FinalizeAction(); });
    cScaleSliderX->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    cScaleSliderY = BSML::Lite::CreateSliderSetting(componentParent, "Y Scale", 0.01, 0, 0.01, 10, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderY, [](float _) { Editor::FinalizeAction(); });
    cScaleSliderY->GetComponent<RectTransform*>()->sizeDelta = {0, 8};

    cTypeDropdown = Utils::CreateDropdownEnum(componentParent, "Type", 0, TypeStrings, [this](int val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Type = val;
        Editor::UpdateType();
        Editor::FinalizeAction();
    });

    cTypeOptions = BSML::Lite::CreateVerticalLayoutGroup(componentParent);
    cTypeOptions->GetComponent<UI::ContentSizeFitter*>()->verticalFit = UI::ContentSizeFitter::FitMode::PreferredSize;

    cColorSourceDropdown = Utils::CreateDropdown(componentParent, "Color Source", "", Utils::GetKeys(colorSources), [this](std::string val) {
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

    cEnableSourceDropdown = Utils::CreateDropdown(componentParent, "Enabled If", "", Utils::GetKeys(enableSources), [this](std::string val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        if (comp.EnableSource != val) {
            cEnableSourceDropdown->dropdown->Hide(false);
            comp.EnableSource = val;
            Editor::UpdateEnableSource();
        }
        Editor::FinalizeAction();
    });

    cInvertEnableToggle = BSML::Lite::CreateToggle(componentParent, "Invert Enabled", false, [this](bool val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        comp.InvertEnable = val;
        Editor::UpdateInvertEnabled();
        Editor::FinalizeAction();
    });

    cEnableSourceOptions = BSML::Lite::CreateVerticalLayoutGroup(componentParent);
    cEnableSourceOptions->GetComponent<UI::ContentSizeFitter*>()->verticalFit = UI::ContentSizeFitter::FitMode::PreferredSize;

    auto cButtonsParent = BSML::Lite::CreateHorizontalLayoutGroup(componentParent);
    cButtonsParent->spacing = 3;

    cDeleteButton = BSML::Lite::CreateUIButton(cButtonsParent, "Delete", [this]() { Editor::Remove(); });

    cDeselectButton = BSML::Lite::CreateUIButton(cButtonsParent, "Deselect", Editor::Deselect);

    Utils::FixScrollView(componentParent, 75);

    uiInitialized = true;
    UpdateUI();
    StartCoroutine(custom_types::Helpers::CoroutineHelper::New(UpdateScrollView()));
}

void Qounters::OptionsViewController::Deselect() {
    component = false;
    group = false;
    UpdateUI();
}

void Qounters::OptionsViewController::GroupSelected() {
    component = false;
    group = true;
    UpdateUI();
}

void Qounters::OptionsViewController::ComponentSelected() {
    component = true;
    group = false;
    UpdateUI();
}

void Qounters::OptionsViewController::UpdateSimpleUI() {
    if (!uiInitialized)
        return;

    Utils::SetScrollViewActive(groupParent, group);
    Utils::SetScrollViewActive(componentParent, component);

    Editor::DisableActions();

    if (group) {
        auto& state = Editor::GetSelectedGroup(-1);
        if (state.Detached) {
            gPosIncrementX->currentValue = state.DetachedPosition.x;
            gPosIncrementX->UpdateState();
            gPosIncrementY->currentValue = state.DetachedPosition.y;
            gPosIncrementY->UpdateState();
            gPosIncrementZ->currentValue = state.DetachedPosition.z;
            gPosIncrementZ->UpdateState();
            gRotSliderX->set_Value(state.DetachedRotation.x);
            gRotSliderY->set_Value(state.DetachedRotation.y);
            gRotSliderZ->set_Value(state.DetachedRotation.z);
        } else {
            gPosIncrementX->currentValue = state.Position.x;
            gPosIncrementX->UpdateState();
            gPosIncrementY->currentValue = state.Position.y;
            gPosIncrementY->UpdateState();
            gRotSlider->set_Value(state.Rotation);
        }
        gPosIncrementZ->gameObject->active = state.Detached;
        gRotSlider->gameObject->active = !state.Detached;
        gRotSliderX->gameObject->active = state.Detached;
        gRotSliderY->gameObject->active = state.Detached;
        gRotSliderZ->gameObject->active = state.Detached;
        BSML::Lite::SetButtonText(gDetachButton, state.Detached ? "Attach" : "Detach");
    } else if (component) {
        auto& state = Editor::GetSelectedComponent(-1);
        cPosIncrementX->currentValue = state.Position.x;
        cPosIncrementX->UpdateState();
        cPosIncrementY->currentValue = state.Position.y;
        cPosIncrementY->UpdateState();
        cRotSlider->set_Value(state.Rotation);
        cScaleSliderX->set_Value(state.Scale.x);
        cScaleSliderY->set_Value(state.Scale.y);
        cTypeDropdown->dropdown->SelectCellWithIdx(state.Type);
        Utils::SetDropdownValue(cColorSourceDropdown, state.ColorSource);
        Utils::SetDropdownValue(cEnableSourceDropdown, state.EnableSource);
        Utils::InstantSetToggle(cInvertEnableToggle, state.InvertEnable);
    } else
        SettingsFlowCoordinator::PresentTemplates();

    Editor::EnableActions();
}

void Qounters::OptionsViewController::UpdateUI() {
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

void Qounters::OptionsViewController::UpdateTypeOptions() {
    auto& state = Editor::GetSelectedComponent(-1, false);
    CreateTypeOptionsUI(cTypeOptions->transform, state.Type, state.Options);
}

void Qounters::OptionsViewController::UpdateColorSourceOptions() {
    auto& state = Editor::GetSelectedComponent(-1, false);
    ColorSource::CreateUI(cColorSourceOptions->gameObject, state.ColorSource, state.ColorOptions);
}

void Qounters::OptionsViewController::UpdateEnableSourceOptions() {
    auto& state = Editor::GetSelectedComponent(-1, false);
    EnableSource::CreateUI(cEnableSourceOptions->gameObject, state.EnableSource, state.EnableOptions);
}

Qounters::OptionsViewController* Qounters::OptionsViewController::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<Qounters::OptionsViewController*>();
    return instance;
}

void Qounters::OptionsViewController::OnDestroy() {
    instance = nullptr;
}

custom_types::Helpers::Coroutine Qounters::OptionsViewController::UpdateScrollView() {
    co_yield nullptr;
    if (uiInitialized && component)
        Utils::RebuildWithScrollPosition(componentParent);
    co_return;
}

void Qounters::EndDragHandler::OnEndDrag(UnityEngine::EventSystems::PointerEventData* eventData) {
    callback();
}

void SpritesListCell::OnImageClicked(int idx) {
    int realIdx = idx + imageStartIdx;
    auto source = (SpritesListSource*) ((HMUI::TableView*) tableCellOwner)->dataSource;
    source->OnImageClicked(realIdx);
}

// void SpritesListCell::RefreshVisuals() {

// }

void SpritesListCell::SetImageStartIdx(int idx) {
    imageStartIdx = idx;

    while (layout->GetChildCount() > 0)
        UnityEngine::Object::DestroyImmediate(layout->GetChild(0)->gameObject);

    for (int i = 0; i < imagesPerCell; i++) {
        if (i + idx < ImageSpriteCache::NumberOfSprites()) {
            images[i] = BSML::Lite::CreateClickableImage(layout, ImageSpriteCache::GetSpriteIdx(i + idx), [this, i]() { OnImageClicked(i); });
            images[i]->preserveAspect = true;
            images[i]->GetComponent<UI::LayoutElement*>()->preferredWidth = cellSize;
        } else
            images[i] = nullptr;
    }
}

// void SpritesListCell::SelectionDidChange(HMUI::SelectableCell::TransitionType transitionType) {

// }

// void SpritesListCell::HighlightDidChange(HMUI::SelectableCell::TransitionType transitionType) {

// }

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
