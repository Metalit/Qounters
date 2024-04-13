#include "customtypes/settings.hpp"
#include "UnityEngine/zzzz__GameObject_def.hpp"
#include "customtypes/components.hpp"
#include "customtypes/editing.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "main.hpp"
#include "playtest.hpp"
#include "sources.hpp"
#include "sourceui.hpp"
#include "templates.hpp"
#include "utils.hpp"

#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "bsml/shared/Helpers/getters.hpp"

#include "UnityEngine/UI/ContentSizeFitter.hpp"
#include <vector>

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
using namespace BSML;
using namespace Qounters;

#include "HMUI/ViewController.hpp"
#include "HMUI/ScreenSystem.hpp"

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
        blankViewController = Helpers::CreateViewController();

    ProvideInitialViewControllers(blankViewController, SettingsViewController::GetInstance(), TemplatesViewController::GetInstance(), nullptr, nullptr);
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
    if (instance->_rightScreenViewController != templates)
        instance->SetRightScreenViewController(templates, HMUI::ViewController::AnimationType::In);
}

void Qounters::SettingsFlowCoordinator::PresentOptions() {
    auto instance = GetInstance();
    auto options = Qounters::OptionsViewController::GetInstance();
    if (instance->_rightScreenViewController != options)
        instance->SetRightScreenViewController(options, HMUI::ViewController::AnimationType::In);
}

void Qounters::SettingsFlowCoordinator::DismissScene() {
    ConfirmAction(DismissSettingsEnvironment);
}

void Qounters::SettingsFlowCoordinator::RefreshScene() {
    if (CurrentSettingsEnvironment() != getConfig().Environment.GetValue() || CurrentColorScheme() != getConfig().ColorScheme.GetValue())
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
    ConfirmAction([name = (std::string) name]() {
        auto presets = getConfig().Presets.GetValue();
        if (!presets.contains(name)) {
            SettingsViewController::GetInstance()->UpdateUI();
            return;
        }
        getConfig().Preset.SetValue(name);
        Editor::LoadPreset(presets[name]);
    }, []() {
        SettingsViewController::GetInstance()->UpdateUI();
    });
}

void Qounters::SettingsFlowCoordinator::RenamePreset(StringW name) {
    MakeNewPreset(name, true);
}

void Qounters::SettingsFlowCoordinator::DuplicatePreset(StringW newName) {
    ConfirmAction([name = (std::string) newName]() {
        MakeNewPreset(name, false);
    });
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
        instance = Helpers::CreateFlowCoordinator<Qounters::SettingsFlowCoordinator*>();
    return instance;
}

void Qounters::SettingsFlowCoordinator::OnDestroy() {
    instance = nullptr;
}

void Qounters::SettingsFlowCoordinator::ConfirmAction(std::function<void ()> action, std::function<void ()> cancel) {
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

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerDataFileManagerSO.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/PlayerDataFileModel.hpp"

void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation) {
        UpdateUI();
        return;
    }

    auto fileModel = Helpers::GetMainFlowCoordinator()->_playerDataModel->_playerDataFileModel;
    auto listModel = fileModel->_environmentsListModel;
    auto colorSchemeSettings = fileModel->_colorSchemesSettings;

    //bsml requires string views for dropdowns. Needs a seperate vector to keep original strings from leaving scope until the dropdown is created.
    std::vector<std::string> environmentStrings = {};
    std::vector<std::string_view> environmentStringViews = {};
    for (auto& info : listModel->_envInfos) 
        environmentStrings.push_back(static_cast<std::string>(info->_environmentName));
    for (const auto& string : environmentStrings)
        environmentStringViews.push_back(string);

    std::vector<std::string> colorSchemeStrings = {};
    std::vector<std::string_view> colorSchemeStringViews = {};
    colorSchemeStrings.push_back("Environment Default");
    for (int i = 0; i < colorSchemeSettings->GetNumberOfColorSchemes(); i++)
        colorSchemeStrings.push_back(static_cast<std::string>(colorSchemeSettings->GetColorSchemeForIdx(i)->_colorSchemeId));
    for (const auto& string : colorSchemeStrings)
        colorSchemeStringViews.push_back(string);

    auto vertical = Lite::CreateVerticalLayoutGroup(this);
    vertical->set_childControlHeight(false);
    vertical->set_childForceExpandHeight(false);
    vertical->set_spacing(1);

    auto buttons1 = Lite::CreateHorizontalLayoutGroup(vertical);
    buttons1->GetComponent<UI::LayoutElement*>()->set_preferredHeight(9);
    buttons1->set_spacing(3);
    undoButton = Lite::CreateUIButton(buttons1, "Undo", Editor::Undo);
    Lite::CreateUIButton(buttons1, "Exit", Qounters::SettingsFlowCoordinator::DismissScene);

    auto buttons2 = Lite::CreateHorizontalLayoutGroup(vertical);
    buttons2->GetComponent<UI::LayoutElement*>()->set_preferredHeight(9);
    buttons2->set_spacing(3);
    Lite::CreateUIButton(buttons2, "Save", Qounters::SettingsFlowCoordinator::Save);
    Lite::CreateUIButton(buttons2, "Save And Exit", "ActionButton", []() {
        Qounters::SettingsFlowCoordinator::Save();
        Qounters::SettingsFlowCoordinator::DismissScene();
    });

    auto environment = Lite::CreateHorizontalLayoutGroup(vertical);
    environment->set_spacing(3);
    auto dropdown = AddConfigValueDropdownString(environment, getConfig().Environment, environmentStringViews);
    dropdown->GetComponentsInParent<UI::LayoutElement*>(true)->First()->set_preferredWidth(65);
    auto apply = Lite::CreateUIButton(environment, "Apply", Qounters::SettingsFlowCoordinator::RefreshScene);

    auto colorScheme = Lite::CreateHorizontalLayoutGroup(vertical);
    environment->set_spacing(3);
    auto colorSchemeDropdown = AddConfigValueDropdownString(colorScheme, getConfig().ColorScheme, colorSchemeStringViews);
    colorSchemeDropdown->GetComponentsInParent<UI::LayoutElement*>(true)->First()->set_preferredWidth(65);
    auto colorSchemeApply = Lite::CreateUIButton(colorScheme, "Apply", Qounters::SettingsFlowCoordinator::RefreshScene);

    GameObject::New_ctor("")->transform->SetParent(vertical->transform);

    Lite::CreateText(vertical, "Changes to the preset list are always saved!")->set_alignment(TMPro::TextAlignmentOptions::Center);

    std::string_view empty = "";
    std::vector<std::string_view> dropdownStrings2 = { empty };
    presetDropdown = Lite::CreateDropdown(vertical, "Current Preset", "", dropdownStrings2, Qounters::SettingsFlowCoordinator::SelectPreset)->dropdown;

    auto buttons3 = Lite::CreateHorizontalLayoutGroup(vertical);
    buttons3->GetComponent<UI::LayoutElement*>()->set_preferredHeight(9);
    buttons3->set_spacing(3);
    Lite::CreateUIButton(buttons3, "Rename", [this]() {
        nameModalIsRename = true;
        nameModal->Show(true, true, nullptr);
    });
    Lite::CreateUIButton(buttons3, "Duplicate", [this]() {
        nameModalIsRename = false;
        nameModal->Show(true, true, nullptr);
    });
    deleteButton = Lite::CreateUIButton(buttons3, "Delete", Qounters::SettingsFlowCoordinator::DeletePreset);

    auto snapIncrement = AddConfigValueIncrementFloat(vertical, getConfig().SnapStep, 1, 0.5, 0.5, 5)->get_transform()->GetChild(1);
    snapIncrement->get_gameObject()->SetActive(getConfig().Snap.GetValue());
    snapIncrement.try_cast<RectTransform>().value_or(nullptr)->set_anchoredPosition({-20, 0});

    auto snapPtr = snapIncrement.ptr();

    auto snapToggle = Lite::CreateToggle(vertical, getConfig().Snap.GetName(), getConfig().Snap.GetValue(), [snapPtr](bool value) {
        getConfig().Snap.SetValue(value);
        snapPtr->get_gameObject()->SetActive(value);
    })->toggle->get_transform();

    //TODO: Fix this
    auto oldParent = snapToggle->GetParent()->get_gameObject();
    snapToggle->SetParent(snapIncrement->GetParent(), false);
    UnityEngine::Object::Destroy(oldParent);

    auto preview = Lite::CreateHorizontalLayoutGroup(vertical);
    preview->set_spacing(3);
    previewToggle = Lite::CreateToggle(preview, "Preview Mode", false, Editor::SetPreviewMode)->toggle;
    previewToggle->GetComponentsInParent<UI::LayoutElement*>(true)->First()->set_preferredWidth(65);
    playtestButton = Lite::CreateUIButton(preview, "Playtest", []() {
        Qounters::PlayTest::SpawnSequence();
    });
    playtestButton->interactable = Editor::GetPreviewMode();

    confirmModal = Lite::CreateModal(this, Vector2(95, 25), []() { Qounters::SettingsFlowCoordinator::OnModalCancel(); });
    auto modalLayout1 = Lite::CreateVerticalLayoutGroup(confirmModal);
    modalLayout1->set_childControlHeight(false);
    modalLayout1->set_childForceExpandHeight(true);
    modalLayout1->set_spacing(1);

    auto warningString = "You have unsaved changes that will be lost.\nAre you sure you would like to continue? This action cannot be undone.";
    auto text1 = Lite::CreateText(modalLayout1, warningString, Vector2(), Vector2(0, 13));
    text1->set_alignment(TMPro::TextAlignmentOptions::Bottom);

    auto modalButtons = Lite::CreateHorizontalLayoutGroup(modalLayout1);
    modalButtons->GetComponent<UI::LayoutElement*>()->set_preferredHeight(9);
    modalButtons->set_spacing(3);
    Lite::CreateUIButton(modalButtons, "Continue", Qounters::SettingsFlowCoordinator::OnModalConfirm);
    Lite::CreateUIButton(modalButtons, "Save And Continue", []() {
        Qounters::SettingsFlowCoordinator::Save();
        Qounters::SettingsFlowCoordinator::OnModalConfirm();
    });
    Lite::CreateUIButton(modalButtons, "Cancel", [this]() {
        Qounters::SettingsFlowCoordinator::OnModalCancel();
    });

    nameModal = Lite::CreateModal(this, Vector2(95, 20), nullptr);
    auto modalLayout2 = Lite::CreateVerticalLayoutGroup(nameModal);
    modalLayout2->set_childControlHeight(false);
    modalLayout2->set_childForceExpandHeight(true);
    modalLayout2->set_spacing(1);

    auto text2 = Lite::CreateText(modalLayout2, "Enter new preset name", Vector2(), Vector2(0, 8));
    text2->set_alignment(TMPro::TextAlignmentOptions::Bottom);

    auto nameInput = Lite::CreateStringSetting(modalLayout2, "Name", "", Vector2(), Vector3());
    Utils::GetOrAddComponent<KeyboardCloseHandler*>(nameInput)->okCallback = [this, nameInput]() {
        std::string val = nameInput->get_text();
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
        instance = Helpers::CreateViewController<SettingsViewController*>();
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

    if(undoButton) {
        undoButton->set_interactable(Editor::HasUndo());
    }

    auto presets = getConfig().Presets.GetValue();
    auto preset = getConfig().Preset.GetValue();
    auto texts = System::Collections::Generic::List_1<StringW>::New_ctor(presets.size());
    int selectedIdx = 0;
    int i = 0;
    for (auto& [name, _] : presets) {
        texts->Add(name);
        if (name == preset)
            selectedIdx = i;
        i++;
    }
    presetDropdown->SetTexts(texts->i___System__Collections__Generic__IReadOnlyList_1_T_());
    presetDropdown->SelectCellWithIdx(selectedIdx);

    deleteButton->set_interactable(presets.size() > 1);

    Utils::InstantSetToggle(previewToggle, Editor::GetPreviewMode());
}

void TemplatesViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation)
        return;

    list = Lite::CreateScrollableList(get_transform(), {0, 0}, {50, 80}, [this](int idx) {
        list->tableView->ClearSelection();
        ShowTemplateModal(idx);
    });

    list->set_listStyle(CustomListTableData::ListStyle::Simple);

    list->data->Clear();
    for (auto templateName : Utils::GetKeys(templates))
        list->data.push_back(CustomCellInfo::construct(templateName));

    list->tableView->ReloadData();
    list->simpleTextTableCell = nullptr;

    auto listParent = list->transform->parent;

    auto layout = listParent->get_gameObject()->AddComponent<UnityEngine::UI::LayoutElement*>();
    listParent->GetComponent<UI::VerticalLayoutGroup*>()->set_childForceExpandHeight(false);
    listParent->GetComponent<UI::VerticalLayoutGroup*>()->set_childControlHeight(false);
    listParent->GetComponent<UI::VerticalLayoutGroup*>()->set_childScaleHeight(false);
    layout->set_preferredHeight(80);
    layout->set_preferredWidth(50);


    list->set_listStyle(CustomListTableData::ListStyle::Simple);
    auto rect = list->get_transform()->GetParent().try_cast<RectTransform>().value_or(nullptr);
    rect->set_anchorMin({0.5, 0.5});
    rect->set_anchorMax({0.5, 0.5});

    for (auto templateName : Utils::GetKeys(templates))
        list->data.push_back(CustomCellInfo::construct(templateName));

    list->tableView->ReloadData();
    list->simpleTextTableCell = nullptr;

    modal = Lite::CreateModal(this);
    modalLayout = Lite::CreateVerticalLayoutGroup(modal)->get_rectTransform();
    modalLayout->set_anchorMin({0.5, 0.5});
    modalLayout->set_anchorMax({0.5, 0.5});

    auto fitter = modalLayout->GetComponent<UI::ContentSizeFitter*>();
    fitter->set_verticalFit(UI::ContentSizeFitter::FitMode::PreferredSize);
    fitter->set_horizontalFit(UI::ContentSizeFitter::FitMode::PreferredSize);

    uiInitialized = true;
}

#include "custom-types/shared/coroutine.hpp"

custom_types::Helpers::Coroutine FitModalCoroutine(HMUI::ModalView* modal, RectTransform* modalLayout) {
    co_yield nullptr;

    modal->GetComponent<RectTransform*>()->set_sizeDelta(Vector2::op_Addition(modalLayout->get_sizeDelta(), Vector2(5, 5)));
}

#include "custom-types/shared/delegate.hpp"

void TemplatesViewController::ShowTemplateModal(int idx) {
    if (!uiInitialized)
        return;

    while (modalLayout->GetChildCount() > 0)
        UnityEngine::Object::DestroyImmediate(modalLayout->GetChild(0)->get_gameObject());

    templates[idx].second(modalLayout->get_gameObject());
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
        instance = Helpers::CreateViewController<TemplatesViewController*>();
    return instance;
}

void TemplatesViewController::OnDestroy() {
    instance = nullptr;
}

void Qounters::OptionsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    UpdateUI();

    if (!firstActivation)
        return;

    groupParent = Lite::CreateScrollView(this);

    gPosIncrementX = Lite::CreateIncrementSetting(groupParent, "X Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        auto& group = Editor::GetSelectedGroup(id);
        if (group.Detached)
            group.DetachedPosition.x = val;
        else
            group.Position.x = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });

    gPosIncrementY = Lite::CreateIncrementSetting(groupParent, "Y Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        auto& group = Editor::GetSelectedGroup(id);
        if (group.Detached)
            group.DetachedPosition.y = val;
        else
            group.Position.y = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });

    gPosIncrementZ = Lite::CreateIncrementSetting(groupParent, "Z Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedPosition.z = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });

    gRotSlider = Lite::CreateSliderSetting(groupParent, "Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSlider, [](float _) {
        Editor::FinalizeAction();
    });
    gRotSlider->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});

    gRotSliderX = Lite::CreateSliderSetting(groupParent, "X Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderX, [](float _) {
        Editor::FinalizeAction();
    });
    gRotSliderX->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});

    gRotSliderY = Lite::CreateSliderSetting(groupParent, "Y Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderY, [](float _) {
        Editor::FinalizeAction();
    });
    gRotSliderY->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});

    gRotSliderZ = Lite::CreateSliderSetting(groupParent, "Z Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.z = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderZ, [](float _) {
        Editor::FinalizeAction();
    });
    gRotSliderZ->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});

    auto GroupAnchorStringViews = std::vector<std::string_view>({ "Left", "Right", "Top", "Bottom" });

    gAnchorDropdown = Utils::CreateDropdownEnum(groupParent, "Anchor", 0, GroupAnchorStringViews, [this](int val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).Anchor = val;
        Editor::UpdatePosition();
        Editor::FinalizeAction();
    });

    auto gButtonsParent1 = Lite::CreateHorizontalLayoutGroup(groupParent);
    gButtonsParent1->set_spacing(3);

    gComponentButton = Lite::CreateUIButton(gButtonsParent1, "Add Component", Editor::AddComponent);

    gDetachButton = Lite::CreateUIButton(gButtonsParent1, "Detach", Editor::ToggleAttachment);

    auto gButtonsParent2 = Lite::CreateHorizontalLayoutGroup(groupParent);
    gButtonsParent2->set_spacing(3);

    gDeleteButton = Lite::CreateUIButton(gButtonsParent2, "Delete", Editor::Remove);

    gDeselectButton = Lite::CreateUIButton(gButtonsParent2, "Deselect", Editor::Deselect);

    Utils::FixScrollView(groupParent, 75);

    componentParent = Lite::CreateScrollView(this);

    cPosIncrementX = Lite::CreateIncrementSetting(componentParent, "Rel. X Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Position.x = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });

    cPosIncrementY = Lite::CreateIncrementSetting(componentParent, "Rel. Y Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Position.y = val;
        Editor::UpdatePosition(true);
        Editor::FinalizeAction();
    });

    cRotSlider = Lite::CreateSliderSetting(componentParent, "Relative Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cRotSlider, [](float _) {
        Editor::FinalizeAction();
    });
    cRotSlider->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});

    cScaleSliderX = Lite::CreateSliderSetting(componentParent, "X Scale", 0.01, 0, 0.01, 10, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderX, [](float _) {
        Editor::FinalizeAction();
    });
    cScaleSliderX->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
    cScaleSliderY = Lite::CreateSliderSetting(componentParent, "Y Scale", 0.01, 0, 0.01, 10, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderY, [](float _) {
        Editor::FinalizeAction();
    });
    cScaleSliderY->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});

    cTypeDropdown = Utils::CreateDropdownEnum(componentParent, "Type", 0, TypeStrings, [this](int val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Type = val;
        Editor::UpdateType();
        Editor::FinalizeAction();
    });

    cTypeOptions = Lite::CreateVerticalLayoutGroup(componentParent);
    cTypeOptions->GetComponent<UI::ContentSizeFitter*>()->set_verticalFit(UI::ContentSizeFitter::FitMode::PreferredSize);

    auto colorSourceKeys = Utils::GetKeys(colorSources);
    auto colorSourceStringViews = std::vector<std::string_view>(colorSourceKeys.size());
    for (int i = 0; i < colorSourceKeys.size(); ++i)
        colorSourceStringViews[i] = colorSourceKeys[i];

    cColorSourceDropdown = Utils::CreateDropdown(componentParent, "Color Source", "", colorSourceStringViews, [this](std::string val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        if (comp.ColorSource != val) {
            cColorSourceDropdown->Hide(false);
            comp.ColorSource = val;
            Editor::UpdateColorSource();
        }
        Editor::FinalizeAction();
    });

    cColorSourceOptions = Lite::CreateVerticalLayoutGroup(componentParent);
    cColorSourceOptions->GetComponent<UI::ContentSizeFitter*>()->set_verticalFit(UI::ContentSizeFitter::FitMode::PreferredSize);

    auto enableSourceKeys = Utils::GetKeys(enableSources);
    auto enableSourceStringViews = std::vector<std::string_view>(enableSourceKeys.size());
    for (int i = 0; i < enableSourceKeys.size(); ++i)
        enableSourceStringViews[i] = enableSourceKeys[i];

    cEnableSourceDropdown = Utils::CreateDropdown(componentParent, "Enabled If", "", enableSourceStringViews, [this](std::string val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        if (comp.EnableSource != val) {
            cEnableSourceDropdown->Hide(false);
            comp.EnableSource = val;
            Editor::UpdateEnableSource();
        }
        Editor::FinalizeAction();
    });

    cInvertEnableToggle = Lite::CreateToggle(componentParent, "Invert Enabled", false, [this](bool val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        comp.InvertEnable = val;
        Editor::UpdateInvertEnabled();
        Editor::FinalizeAction();
    })->toggle;

    cEnableSourceOptions = Lite::CreateVerticalLayoutGroup(componentParent);
    cEnableSourceOptions->GetComponent<UI::ContentSizeFitter*>()->set_verticalFit(UI::ContentSizeFitter::FitMode::PreferredSize);

    auto cButtonsParent = Lite::CreateHorizontalLayoutGroup(componentParent);
    cButtonsParent->set_spacing(3);

    cDeleteButton = Lite::CreateUIButton(cButtonsParent, "Delete", [this]() {
        Editor::Remove();
    });

    cDeselectButton = Lite::CreateUIButton(cButtonsParent, "Deselect", Editor::Deselect);

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
            gAnchorDropdown->SelectCellWithIdx(state.Anchor);
        }
        gPosIncrementZ->get_gameObject()->SetActive(state.Detached);
        gRotSlider->get_gameObject()->SetActive(!state.Detached);
        gRotSliderX->get_gameObject()->SetActive(state.Detached);
        gRotSliderY->get_gameObject()->SetActive(state.Detached);
        gRotSliderZ->get_gameObject()->SetActive(state.Detached);
        gAnchorDropdown->get_gameObject()->SetActive(!state.Detached);
        Lite::SetButtonText(gDetachButton, state.Detached ? "Attach" : "Detach");
    } else if (component) {
        auto& state = Editor::GetSelectedComponent(-1);
        cPosIncrementX->currentValue = state.Position.x;
        cPosIncrementX->UpdateState();
        cPosIncrementY->currentValue = state.Position.y;
        cPosIncrementY->UpdateState();
        cRotSlider->set_Value(state.Rotation);
        cScaleSliderX->set_Value(state.Scale.x);
        cScaleSliderY->set_Value(state.Scale.y);
        cTypeDropdown->SelectCellWithIdx(state.Type);
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
    CreateTypeOptionsUI(cTypeOptions->get_transform(), state.Type, state.Options);
}

void Qounters::OptionsViewController::UpdateColorSourceOptions() {
    auto& state = Editor::GetSelectedComponent(-1, false);
    ColorSource::CreateUI(cColorSourceOptions->get_gameObject(), state.ColorSource, state.ColorOptions);
}

void Qounters::OptionsViewController::UpdateEnableSourceOptions() {
    auto& state = Editor::GetSelectedComponent(-1, false);
    EnableSource::CreateUI(cEnableSourceOptions->get_gameObject(), state.EnableSource, state.EnableOptions);
}

Qounters::OptionsViewController* Qounters::OptionsViewController::GetInstance() {
    if (!instance)
        instance = Helpers::CreateViewController<Qounters::OptionsViewController*>();
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


void Qounters::EndDragHandler::OnEndDrag(UnityEngine::EventSystems::PointerEventData *eventData) {
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
        UnityEngine::Object::DestroyImmediate(layout->GetChild(0)->get_gameObject());

    for (int i = 0; i < imagesPerCell; i++) {
        if (i + idx < ImageSpriteCache::NumberOfSprites()) {
            images[i] = Lite::CreateClickableImage(layout, ImageSpriteCache::GetSpriteIdx(i + idx), [this, i]() {
                OnImageClicked(i);
            });
            images[i]->set_preserveAspect(true);
            images[i]->GetComponent<UI::LayoutElement*>()->set_preferredWidth(cellSize);
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

    object->AddComponent<RectTransform*>()->set_sizeDelta({0, cellSize});

    auto ret = object->AddComponent<SpritesListCell*>();
    ret->set_reuseIdentifier(reuseIdentifier);

    auto layout = Lite::CreateHorizontalLayoutGroup(ret);
    layout->set_spacing(imageSpacing);
    layout->set_childForceExpandWidth(false);
    layout->set_childAlignment(TextAnchor::MiddleCenter);
    layout->GetComponent<UI::ContentSizeFitter*>()->set_verticalFit(UI::ContentSizeFitter::FitMode::Unconstrained);

    ret->images = ArrayW<ClickableImage*>(imagesPerCell);
    ret->layout = layout->get_rectTransform();
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

// IConnectedPlayer* MultiplayerSessionManager::get_localPlayer() {
//     return localPlayer;
// }
