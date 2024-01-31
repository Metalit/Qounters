#include "customtypes/settings.hpp"
#include "customtypes/components.hpp"
#include "customtypes/editing.hpp"
#include "editor.hpp"
#include "environment.hpp"
#include "config.hpp"
#include "main.hpp"
#include "sources.hpp"
#include "sourceui.hpp"
#include "templates.hpp"
#include "utils.hpp"

#include "questui/shared/BeatSaberUI.hpp"

DEFINE_TYPE(Qounters, SettingsFlowCoordinator);
DEFINE_TYPE(Qounters, SettingsViewController);
DEFINE_TYPE(Qounters, TemplatesViewController);
DEFINE_TYPE(Qounters, OptionsViewController);
DEFINE_TYPE(Qounters, EndDragHandler);
DEFINE_TYPE(Qounters, SpritesListCell);
DEFINE_TYPE(Qounters, SpritesListSource);

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace QuestUI;
using namespace Qounters;

#include "HMUI/ViewController_AnimationType.hpp"
#include "HMUI/ScreenSystem.hpp"

void Qounters::SettingsFlowCoordinator::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (addedToHierarchy) {
        auto presets = getConfig().Presets.GetValue();
        if (!presets.contains(modifyingPresetName))
            modifyingPresetName = getConfig().Preset.GetValue();
        if (!presets.contains(modifyingPresetName))
            modifyingPresetName = getConfig().Preset.GetDefaultValue();
        auto& preset = presets[modifyingPresetName];

        // TODO: preset layouts

        Editor::Initialize(preset);
    }

    if (!blankViewController)
        blankViewController = BeatSaberUI::CreateViewController();

    ProvideInitialViewControllers(blankViewController, SettingsViewController::GetInstance(), TemplatesViewController::GetInstance(), nullptr, nullptr);
    Qounters::OptionsViewController::GetInstance()->Deselect();
}

void Qounters::SettingsFlowCoordinator::Save() {
    auto presets = getConfig().Presets.GetValue();
    presets[GetInstance()->modifyingPresetName] = Editor::GetPreset();
    getConfig().Presets.SetValue(presets);
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
    DismissSettingsEnvironment();
}

void Qounters::SettingsFlowCoordinator::RefreshScene() {
    if (CurrentSettingsEnvironment() != getConfig().Environment.GetValue())
        RefreshSettingsEnvironment();
}

Qounters::SettingsFlowCoordinator* Qounters::SettingsFlowCoordinator::GetInstance() {
    if (!instance)
        instance = BeatSaberUI::CreateFlowCoordinator<Qounters::SettingsFlowCoordinator*>();
    return instance;
}

void Qounters::SettingsFlowCoordinator::OnDestroy() {
    instance = nullptr;
}

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "GlobalNamespace/PlayerDataModel.hpp"
#include "GlobalNamespace/PlayerDataFileManagerSO.hpp"
#include "GlobalNamespace/EnvironmentsListSO.hpp"

void SettingsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation) {
        Utils::InstantSetToggle(previewToggle, false);
        return;
    }

    std::vector<std::string> dropdownStrings = {};
    auto manager = BeatSaberUI::GetMainFlowCoordinator()->playerDataModel->playerDataFileManager;
    for (auto& env : ListW<EnvironmentInfoSO*>(manager->allEnvironmentInfos->GetAllEnvironmentInfosWithType(manager->normalEnvironmentType)))
        dropdownStrings.emplace_back(env->environmentName);
    for (auto& env : ListW<EnvironmentInfoSO*>(manager->allEnvironmentInfos->GetAllEnvironmentInfosWithType(manager->a360DegreesEnvironmentType)))
        dropdownStrings.emplace_back(env->environmentName);
    for (auto& env : manager->allEnvironmentInfos->environmentInfos) {
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

    auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(this);
    vertical->set_childControlHeight(false);
    vertical->set_childForceExpandHeight(false);
    vertical->set_spacing(1);

    auto buttons1 = BeatSaberUI::CreateHorizontalLayoutGroup(vertical);
    buttons1->GetComponent<UI::LayoutElement*>()->set_preferredHeight(9);
    buttons1->set_spacing(3);
    BeatSaberUI::CreateUIButton(buttons1, "Undo", Editor::Undo);
    BeatSaberUI::CreateUIButton(buttons1, "Exit", Qounters::SettingsFlowCoordinator::DismissScene);

    auto buttons2 = BeatSaberUI::CreateHorizontalLayoutGroup(vertical);
    buttons2->GetComponent<UI::LayoutElement*>()->set_preferredHeight(9);
    buttons2->set_spacing(3);
    BeatSaberUI::CreateUIButton(buttons2, "Save", Qounters::SettingsFlowCoordinator::Save);
    BeatSaberUI::CreateUIButton(buttons2, "Save And Exit", "ActionButton", []() {
        Qounters::SettingsFlowCoordinator::Save();
        Qounters::SettingsFlowCoordinator::DismissScene();
    });

    auto environment = BeatSaberUI::CreateHorizontalLayoutGroup(vertical);
    environment->set_spacing(3);
    auto dropdown = AddConfigValueDropdownString(environment, getConfig().Environment, dropdownStrings);
    dropdown->GetComponentsInParent<UI::LayoutElement*>(true).First()->set_preferredWidth(65);
    auto apply = BeatSaberUI::CreateUIButton(environment, "Apply", Qounters::SettingsFlowCoordinator::RefreshScene);

    previewToggle = BeatSaberUI::CreateToggle(vertical, "Preview Mode", false, Editor::SetPreviewMode);
}

SettingsViewController* SettingsViewController::GetInstance() {
    if (!instance)
        instance = BeatSaberUI::CreateViewController<SettingsViewController*>();
    return instance;
}

void SettingsViewController::OnDestroy() {
    instance = nullptr;
}

void TemplatesViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation)
        return;

    list = BeatSaberUI::CreateScrollableList(get_transform(), {50, 80}, [this](int idx) {
        list->tableView->ClearSelection();
        ShowTemplateModal(idx);
    });
    list->set_listStyle(CustomListTableData::ListStyle::Simple);
    auto rect = (RectTransform*) list->get_transform()->GetParent();
    rect->set_anchorMin({0.5, 0.5});
    rect->set_anchorMax({0.5, 0.5});

    for (auto templateName : Utils::GetKeys(templates))
        list->data.emplace_back(templateName);

    list->tableView->ReloadData();

    modal = BeatSaberUI::CreateModal(this);
    modalLayout = BeatSaberUI::CreateVerticalLayoutGroup(modal)->get_rectTransform();
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

    modal->GetComponent<RectTransform*>()->set_sizeDelta(modalLayout->get_sizeDelta() + Vector2(5, 5));
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
        instance = BeatSaberUI::CreateViewController<TemplatesViewController*>();
    return instance;
}

void TemplatesViewController::OnDestroy() {
    instance = nullptr;
}

void Qounters::OptionsViewController::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    UpdateUI();

    if (!firstActivation)
        return;

    groupParent = BeatSaberUI::CreateScrollView(this);

    gPosIncrementX = BeatSaberUI::CreateIncrementSetting(groupParent, "X Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        auto& group = Editor::GetSelectedGroup(id);
        if (group.Detached)
            group.DetachedPosition.x = val;
        else
            group.Position.x = val;
        Editor::UpdatePosition();
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementX, 5);
    gPosIncrementY = BeatSaberUI::CreateIncrementSetting(groupParent, "Y Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        auto& group = Editor::GetSelectedGroup(id);
        if (group.Detached)
            group.DetachedPosition.y = val;
        else
            group.Position.y = val;
        Editor::UpdatePosition();
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementY, 5);
    gPosIncrementZ = BeatSaberUI::CreateIncrementSetting(groupParent, "Z Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedPosition.z = val;
        Editor::UpdatePosition();
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(gPosIncrementZ, 5);

    gRotSlider = BeatSaberUI::CreateSliderSetting(groupParent, "Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSlider, []() {
        Editor::FinalizeAction();
    });
    gRotSlider->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
    AddSliderIncrement(gRotSlider, 10);

    gRotSliderX = BeatSaberUI::CreateSliderSetting(groupParent, "X Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderX, []() {
        Editor::FinalizeAction();
    });
    gRotSliderX->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
    AddSliderIncrement(gRotSliderX, 10);
    gRotSliderY = BeatSaberUI::CreateSliderSetting(groupParent, "Y Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderY, []() {
        Editor::FinalizeAction();
    });
    gRotSliderY->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
    AddSliderIncrement(gRotSliderY, 10);
    gRotSliderZ = BeatSaberUI::CreateSliderSetting(groupParent, "Z Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedGroup(id).DetachedRotation.z = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(gRotSliderZ, []() {
        Editor::FinalizeAction();
    });
    gRotSliderZ->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
    AddSliderIncrement(gRotSliderZ, 10);

    auto gButtonsParent1 = BeatSaberUI::CreateHorizontalLayoutGroup(groupParent);
    gButtonsParent1->set_spacing(3);

    gComponentButton = BeatSaberUI::CreateUIButton(gButtonsParent1, "Add Component", Editor::AddComponent);

    gDetachButton = BeatSaberUI::CreateUIButton(gButtonsParent1, "Detach", Editor::ToggleAttachment);

    auto gButtonsParent2 = BeatSaberUI::CreateHorizontalLayoutGroup(groupParent);
    gButtonsParent2->set_spacing(3);

    gDeleteButton = BeatSaberUI::CreateUIButton(gButtonsParent2, "Delete", Editor::Remove);

    gDeselectButton = BeatSaberUI::CreateUIButton(gButtonsParent2, "Deselect", Editor::Deselect);

    Utils::FixScrollView(groupParent, 75);

    componentParent = BeatSaberUI::CreateScrollView(this);

    cPosIncrementX = BeatSaberUI::CreateIncrementSetting(componentParent, "Rel. X Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Position.x = val;
        Editor::UpdatePosition();
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(cPosIncrementX, 5);
    cPosIncrementY = BeatSaberUI::CreateIncrementSetting(componentParent, "Rel. Y Position", 1, 0.5, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Position.y = val;
        Editor::UpdatePosition();
        Editor::FinalizeAction();
    });
    Utils::AddIncrementIncrement(cPosIncrementY, 5);

    cRotSlider = BeatSaberUI::CreateSliderSetting(componentParent, "Relative Rotation", 1, 0, -180, 180, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Rotation = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cRotSlider, []() {
        Editor::FinalizeAction();
    });
    cRotSlider->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
    AddSliderIncrement(cRotSlider, 10);

    cScaleSliderX = BeatSaberUI::CreateSliderSetting(componentParent, "X Scale", 0.01, 0, 0.01, 10, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.x = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderX, []() {
        Editor::FinalizeAction();
    });
    cScaleSliderX->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
    AddSliderIncrement(cScaleSliderX, 0.01);
    cScaleSliderY = BeatSaberUI::CreateSliderSetting(componentParent, "Y Scale", 0.01, 0, 0.01, 10, 0, [this](float val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Scale.y = val;
        Editor::UpdatePosition();
    });
    Utils::AddSliderEndDrag(cScaleSliderY, []() {
        Editor::FinalizeAction();
    });
    cScaleSliderY->GetComponent<RectTransform*>()->set_sizeDelta({0, 8});
    AddSliderIncrement(cScaleSliderY, 0.01);

    cTypeDropdown = Utils::CreateDropdownEnum(componentParent, "Type", 0, TypeStrings, [this](int val) {
        static int id = Editor::GetActionId();
        Editor::GetSelectedComponent(id).Type = val;
        Editor::UpdateType();
        Editor::FinalizeAction();
    });

    cTypeOptions = BeatSaberUI::CreateVerticalLayoutGroup(componentParent);
    cTypeOptions->GetComponent<UI::ContentSizeFitter*>()->set_verticalFit(UI::ContentSizeFitter::FitMode::PreferredSize);

    cColorSourceDropdown = Utils::CreateDropdown(componentParent, "Color Source", "", Utils::GetKeys(colorSources), [this](std::string val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        if (comp.ColorSource != val) {
            cColorSourceDropdown->Hide(false);
            comp.ColorSource = val;
            Editor::UpdateColorSource();
        }
        Editor::FinalizeAction();
    });

    cColorSourceOptions = BeatSaberUI::CreateVerticalLayoutGroup(componentParent);
    cColorSourceOptions->GetComponent<UI::ContentSizeFitter*>()->set_verticalFit(UI::ContentSizeFitter::FitMode::PreferredSize);

    cEnableSourceDropdown = Utils::CreateDropdown(componentParent, "Enabled If", "", Utils::GetKeys(enableSources), [this](std::string val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        if (comp.EnableSource != val) {
            cEnableSourceDropdown->Hide(false);
            comp.EnableSource = val;
            Editor::UpdateEnableSource();
        }
        Editor::FinalizeAction();
    });

    cInvertEnableToggle = BeatSaberUI::CreateToggle(componentParent, "Invert Enabled", false, [this](bool val) {
        static int id = Editor::GetActionId();
        auto& comp = Editor::GetSelectedComponent(id);
        comp.InvertEnable = val;
        Editor::UpdateInvertEnabled();
        Editor::FinalizeAction();
    });

    cEnableSourceOptions = BeatSaberUI::CreateVerticalLayoutGroup(componentParent);
    cEnableSourceOptions->GetComponent<UI::ContentSizeFitter*>()->set_verticalFit(UI::ContentSizeFitter::FitMode::PreferredSize);

    auto cButtonsParent = BeatSaberUI::CreateHorizontalLayoutGroup(componentParent);
    cButtonsParent->set_spacing(3);

    cDeleteButton = BeatSaberUI::CreateUIButton(cButtonsParent, "Delete", [this]() {
        Editor::Remove();
    });

    cDeselectButton = BeatSaberUI::CreateUIButton(cButtonsParent, "Deselect", Editor::Deselect);

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
            gPosIncrementX->CurrentValue = state.DetachedPosition.x;
            gPosIncrementX->UpdateValue();
            gPosIncrementY->CurrentValue = state.DetachedPosition.y;
            gPosIncrementY->UpdateValue();
            gPosIncrementZ->CurrentValue = state.DetachedPosition.z;
            gPosIncrementZ->UpdateValue();
            gRotSliderX->set_value(state.DetachedRotation.x);
            gRotSliderY->set_value(state.DetachedRotation.y);
            gRotSliderZ->set_value(state.DetachedRotation.z);
        } else {
            gPosIncrementX->CurrentValue = state.Position.x;
            gPosIncrementX->UpdateValue();
            gPosIncrementY->CurrentValue = state.Position.y;
            gPosIncrementY->UpdateValue();
            gRotSlider->set_value(state.Rotation);
        }
        gPosIncrementZ->get_gameObject()->SetActive(state.Detached);
        gRotSlider->get_gameObject()->SetActive(!state.Detached);
        gRotSliderX->get_gameObject()->SetActive(state.Detached);
        gRotSliderY->get_gameObject()->SetActive(state.Detached);
        gRotSliderZ->get_gameObject()->SetActive(state.Detached);
        BeatSaberUI::SetButtonText(gDetachButton, state.Detached ? "Attach" : "Detach");
    } else if (component) {
        auto& state = Editor::GetSelectedComponent(-1);
        cPosIncrementX->CurrentValue = state.Position.x;
        cPosIncrementX->UpdateValue();
        cPosIncrementY->CurrentValue = state.Position.y;
        cPosIncrementY->UpdateValue();
        cRotSlider->set_value(state.Rotation);
        cScaleSliderX->set_value(state.Scale.x);
        cScaleSliderY->set_value(state.Scale.y);
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
        instance = BeatSaberUI::CreateViewController<Qounters::OptionsViewController*>();
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


void Qounters::EndDragHandler::OnEndDrag(UES::PointerEventData *eventData) {
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
            images[i] = BeatSaberUI::CreateClickableImage(layout, ImageSpriteCache::GetSpriteIdx(i + idx), [this, i]() {
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

    auto layout = BeatSaberUI::CreateHorizontalLayoutGroup(ret);
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
    auto ret = (SpritesListCell*) tableView->DequeueReusableCellForIdentifier(reuseIdentifier);

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
