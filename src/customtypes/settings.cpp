#include "customtypes/settings.hpp"

#include <numbers>

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/EnvironmentType.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "HMUI/CurvedCanvasSettings.hpp"
#include "HMUI/ScreenSystem.hpp"
#include "System/Single.hpp"
#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "bsml/shared/Helpers/extension.hpp"
#include "bsml/shared/Helpers/getters.hpp"
#include "config.hpp"
#include "custom-types/shared/delegate.hpp"
#include "customtypes/components.hpp"
#include "customtypes/editing.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "main.hpp"
#include "sources.hpp"
#include "sourceui.hpp"
#include "templates.hpp"
#include "utils.hpp"

DEFINE_TYPE(Qounters, SettingsFlowCoordinator);
DEFINE_TYPE(Qounters, SettingsViewController);
DEFINE_TYPE(Qounters, TemplatesViewController);
DEFINE_TYPE(Qounters, OptionsViewController);
DEFINE_TYPE(Qounters, CollapseController);
DEFINE_TYPE(Qounters, MenuDragger);
DEFINE_TYPE(Qounters, EndDragHandler);
DEFINE_TYPE(Qounters, KeyboardCloseHandler);
DEFINE_TYPE(Qounters, SpritesListCell);
DEFINE_TYPE(Qounters, SpritesListSource);

using namespace UnityEngine;
using namespace Qounters;

void SettingsFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (addedToHierarchy) {
        auto presets = getConfig().Presets.GetValue();
        auto presetName = getConfig().Preset.GetValue();
        if (!presets.contains(presetName)) {
            presetName = presets.begin()->first;
            getConfig().Preset.SetValue(presetName);
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
    leftDragger->active = false;
    rightDragger->active = false;
}

void SettingsFlowCoordinator::Save() {
    auto presets = getConfig().Presets.GetValue();
    presets[getConfig().Preset.GetValue()] = Editor::GetPreset();
    getConfig().Presets.SetValue(presets);
}

bool SettingsFlowCoordinator::IsSaved() {
    return getConfig().Presets.GetValue()[getConfig().Preset.GetValue()] == Editor::GetPreset();
}

void SettingsFlowCoordinator::PresentTemplates() {
    auto instance = GetInstance();
    auto templates = TemplatesViewController::GetInstance();
    if (instance->rightScreenViewController != templates)
        instance->SetRightScreenViewController(templates, HMUI::ViewController::AnimationType::In);
}

void SettingsFlowCoordinator::PresentOptions() {
    auto instance = GetInstance();
    auto options = OptionsViewController::GetInstance();
    if (instance->rightScreenViewController != options)
        instance->SetRightScreenViewController(options, HMUI::ViewController::AnimationType::In);
}

void SettingsFlowCoordinator::DismissScene() {
    ConfirmAction(DismissSettingsEnvironment);
}

void SettingsFlowCoordinator::RefreshScene() {
    if (CurrentSettingsEnvironment() != getConfig().Environment.GetValue())
        ConfirmAction(RefreshSettingsEnvironment);
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
            getConfig().Preset.SetValue(name);
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

    auto name = getConfig().Preset.GetValue();
    presets.erase(name);
    getConfig().Presets.SetValue(presets);

    name = presets.begin()->first;
    getConfig().Preset.SetValue(name);

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

    using namespace GlobalNamespace;

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
    BSML::Lite::CreateUIButton(buttons1, "Exit", SettingsFlowCoordinator::DismissScene);

    auto buttons2 = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    buttons2->GetComponent<UI::LayoutElement*>()->preferredHeight = 9;
    buttons2->spacing = 3;
    BSML::Lite::CreateUIButton(buttons2, "Save", SettingsFlowCoordinator::Save);
    BSML::Lite::CreateUIButton(buttons2, "Save And Exit", "ActionButton", []() {
        SettingsFlowCoordinator::Save();
        SettingsFlowCoordinator::DismissScene();
    });

    auto environment = BSML::Lite::CreateHorizontalLayoutGroup(vertical);
    environment->spacing = 3;
    auto dropdown = AddConfigValueDropdownString(environment, getConfig().Environment, dropdownStringViews);
    dropdown->GetComponentsInParent<UI::LayoutElement*>(true)->First()->preferredWidth = 65;
    auto apply = BSML::Lite::CreateUIButton(environment, "Apply", SettingsFlowCoordinator::RefreshScene);

    BSML::Lite::CreateText(vertical, "Changes to the preset list are always saved!", {0, 0}, {50, 8})->alignment =
        TMPro::TextAlignmentOptions::Center;

    presetDropdown = BSML::Lite::CreateDropdown(vertical, "Current Preset", "", {}, SettingsFlowCoordinator::SelectPreset);

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
    deleteButton = BSML::Lite::CreateUIButton(buttons3, "Delete", SettingsFlowCoordinator::DeletePreset);

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

    undoButton->interactable = !Editor::GetPreviewMode() && Editor::HasUndo();

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

void TemplatesViewController::ShowTemplateModal(int idx) {
    if (!uiInitialized)
        return;

    while (modalLayout->GetChildCount() > 0)
        Object::DestroyImmediate(modalLayout->GetChild(0)->gameObject);

    templates[idx].second(modalLayout->gameObject);
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

    groupParent = BSML::Lite::CreateScrollView(this);

    gPosIncrementX = BSML::Lite::CreateIncrementSetting(groupParent, "X Position", 1, 0.5, 0, [](float val) {
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
    gPosIncrementY = BSML::Lite::CreateIncrementSetting(groupParent, "Y Position", 1, 0.5, 0, [](float val) {
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
    gPosIncrementZ = BSML::Lite::CreateIncrementSetting(groupParent, "Z Position", 1, 0.5, 0, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedPosition.z = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementZ, 5);

    gRotSlider = BSML::Lite::CreateSliderSetting(groupParent, "Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSlider, [](float _) { Editor::FinalizeAction(); });
    gRotSlider->GetComponent<RectTransform*>()->sizeDelta = {0, 8};

    gRotSliderX = BSML::Lite::CreateSliderSetting(groupParent, "X Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderX, [](float _) { Editor::FinalizeAction(); });
    gRotSliderX->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    gRotSliderY = BSML::Lite::CreateSliderSetting(groupParent, "Y Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderY, [](float _) { Editor::FinalizeAction(); });
    gRotSliderY->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    gRotSliderZ = BSML::Lite::CreateSliderSetting(groupParent, "Z Rotation", 1, 0, -180, 180, 0, true, {0, 0}, [](float val) {
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

    cScaleSliderX = BSML::Lite::CreateSliderSetting(componentParent, "X Scale", 0.01, 0, 0.01, 10, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderX, [](float _) { Editor::FinalizeAction(); });
    cScaleSliderX->GetComponent<RectTransform*>()->sizeDelta = {0, 8};
    cScaleSliderY = BSML::Lite::CreateSliderSetting(componentParent, "Y Scale", 0.01, 0, 0.01, 10, 0, true, {0, 0}, [](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderY, [](float _) { Editor::FinalizeAction(); });
    cScaleSliderY->GetComponent<RectTransform*>()->sizeDelta = {0, 8};

    positionCollapse->SetContents({cPosIncrementX, cPosIncrementY, cRotSlider, cScaleSliderX, cScaleSliderY});
    positionCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    auto typeCollapse = Utils::CreateCollapseArea(componentParent, "Text Options", true);
    typeCollapseComponent = typeCollapse;

    cTypeDropdown = Utils::CreateDropdownEnum(componentParent, "Type", 0, TypeStrings, [typeCollapse](int val) {
        typeCollapse->title = std::string(TypeStrings[val]) + " Options";
        typeCollapse->UpdateOpen();
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Type = val;
        Editor::UpdateType();
        Editor::FinalizeAction();
    });

    cTypeOptions = BSML::Lite::CreateVerticalLayoutGroup(componentParent);
    cTypeOptions->GetComponent<UI::ContentSizeFitter*>()->verticalFit = UI::ContentSizeFitter::FitMode::PreferredSize;

    typeCollapse->SetContents({cTypeDropdown->transform->parent, cTypeOptions});
    typeCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    auto colorCollapse = Utils::CreateCollapseArea(componentParent, "Color Options", false);

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

    colorCollapse->SetContents({cColorSourceDropdown->transform->parent, cColorSourceOptions});
    colorCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    auto enableCollapse = Utils::CreateCollapseArea(componentParent, "Visibility Options", false);

    cEnableSourceDropdown = Utils::CreateDropdown(componentParent, "Visible If", "", Utils::GetKeys(enableSources), [this](std::string val) {
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

    enableCollapse->SetContents({cEnableSourceDropdown->transform->parent, cInvertEnableToggle, cEnableSourceOptions});
    enableCollapse->onUpdate = OptionsViewController::UpdateScrollViewStatic;

    cButtonsParent = BSML::Lite::CreateHorizontalLayoutGroup(this);
    cButtonsParent->spacing = 3;
    cButtonsParent->childControlWidth = false;
    cButtonsParent->childForceExpandWidth = false;
    cButtonsParent->childAlignment = TextAnchor::MiddleCenter;
    cButtonsParent->rectTransform->anchoredPosition = {0, -37};

    cDeleteButton = BSML::Lite::CreateUIButton(cButtonsParent, "Delete", Editor::Remove);

    cDeselectButton = BSML::Lite::CreateUIButton(cButtonsParent, "Deselect", Editor::Deselect);

    Utils::FixScrollView(componentParent, 75);

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

    Utils::SetScrollViewActive(groupParent, group);
    Utils::SetScrollViewActive(componentParent, component);
    cButtonsParent->gameObject->active = component;

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
        auto typeCollapse = (CollapseController*) typeCollapseComponent;
        typeCollapse->title = std::string(TypeStrings[state.Type]) + " Options";
        typeCollapse->UpdateOpen();
        cTypeDropdown->dropdown->SelectCellWithIdx(state.Type);
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
    CreateTypeOptionsUI(cTypeOptions->transform, state.Type, state.Options);
}

void OptionsViewController::UpdateColorSourceOptions() {
    auto& state = Editor::GetSelectedComponent(-1);
    ColorSource::CreateUI(cColorSourceOptions->gameObject, state.ColorSource, state.ColorOptions);
}

void OptionsViewController::UpdateEnableSourceOptions() {
    auto& state = Editor::GetSelectedComponent(-1);
    EnableSource::CreateUI(cEnableSourceOptions->gameObject, state.EnableSource, state.EnableOptions);
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

void CollapseController::UpdateOpen() {
    text->text = StringW(open ? u"▼  " : u"▶  ") + title;
    for (auto& obj : contents)
        obj->gameObject->active = open;
    if (onUpdate)
        onUpdate();
}

void CollapseController::SetContents(std::vector<Component*> const& newContents) {
    contents = newContents;
    UpdateOpen();
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
        rootCanvas = GetComponentInParent<UnityEngine::Canvas*>()->rootCanvas;
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

// void SpritesListCell::RefreshVisuals() {

// }

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
