#pragma once

#include "GlobalNamespace/ColorManager.hpp"
#include "GlobalNamespace/ColorSchemeDropdown.hpp"
#include "GlobalNamespace/ColorSchemesSettings.hpp"
#include "GlobalNamespace/EditColorSchemeController.hpp"
#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "HMUI/ColorGradientSlider.hpp"
#include "HMUI/FlowCoordinator.hpp"
#include "HMUI/TableCell.hpp"
#include "HMUI/TableView.hpp"
#include "HMUI/ViewController.hpp"
#include "System/Action.hpp"
#include "UnityEngine/EventSystems/IDragHandler.hpp"
#include "UnityEngine/EventSystems/IEndDragHandler.hpp"
#include "UnityEngine/EventSystems/IInitializePotentialDragHandler.hpp"
#include "UnityEngine/EventSystems/IPointerUpHandler.hpp"
#include "bsml/shared/BSML/Components/ClickableImage.hpp"
#include "bsml/shared/BSML/Components/CustomListTableData.hpp"
#include "bsml/shared/BSML/Components/Settings/DropdownListSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/IncrementSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/SliderSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/ToggleSetting.hpp"
#include "config.hpp"
#include "custom-types/shared/macros.hpp"
#include "main.hpp"

#define UES UnityEngine::EventSystems

DECLARE_CLASS_CODEGEN(Qounters, SettingsFlowCoordinator, HMUI::FlowCoordinator) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::FlowCoordinator::DidActivate, bool, bool, bool);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidDeactivate, &HMUI::FlowCoordinator::DidDeactivate, bool, bool);

    DECLARE_STATIC_METHOD(void, PresentPlaytest);
    DECLARE_STATIC_METHOD(void, PresentTemplates);
    DECLARE_STATIC_METHOD(void, PresentOptions);

    DECLARE_STATIC_METHOD(void, Save);
    DECLARE_STATIC_METHOD(bool, IsSaved);

    DECLARE_STATIC_METHOD(void, DismissScene);
    DECLARE_STATIC_METHOD(void, RefreshScene);
    DECLARE_STATIC_METHOD(void, OnModalConfirm);
    DECLARE_STATIC_METHOD(void, OnModalCancel);

    DECLARE_STATIC_METHOD(void, SelectPreset, StringW name);
    DECLARE_STATIC_METHOD(void, RenamePreset, StringW name);
    DECLARE_STATIC_METHOD(void, DuplicatePreset, StringW newName);
    DECLARE_STATIC_METHOD(void, DeletePreset);
    DECLARE_STATIC_METHOD(void, ResetPreset);

    DECLARE_INSTANCE_FIELD_DEFAULT(HMUI::ViewController*, blankViewController, nullptr);
    DECLARE_INSTANCE_FIELD_DEFAULT(float, oldRadius, -1);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, leftDragger);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, rightDragger);

    DECLARE_STATIC_METHOD(Qounters::SettingsFlowCoordinator*, GetInstance);
    DECLARE_STATIC_METHOD(float, GetRadius);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);

   private:
    static inline Qounters::SettingsFlowCoordinator* instance = nullptr;

    static void ConfirmAction(std::function<void()> action, std::function<void()> cancel = nullptr);
    static inline std::function<void()> nextModalAction = nullptr;
    static inline std::function<void()> nextModalCancel = nullptr;

    static void MakeNewPreset(std::string name, bool removeOld);
};

DECLARE_CLASS_CODEGEN(Qounters, SettingsViewController, HMUI::ViewController) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool, bool, bool);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, ShowConfirmModal);
    DECLARE_INSTANCE_METHOD(void, HideConfirmModal);
    DECLARE_INSTANCE_METHOD(void, ColorCellSelected, int idx);
    DECLARE_INSTANCE_METHOD(void, UpdateColors);
    DECLARE_INSTANCE_METHOD(void, UpdateUI);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, uiInitialized, false);
    DECLARE_INSTANCE_FIELD(ArrayW<UnityW<GlobalNamespace::EnvironmentInfoSO>>, environments);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::ColorSchemesSettings*, colorSchemeSettings);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, undoButton);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, environmentDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, applyButton);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, colorToggleName);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::ColorSchemeDropdown*, colorDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, colorEditButton);
    DECLARE_INSTANCE_FIELD(GlobalNamespace::EditColorSchemeController*, colorEditor);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, presetDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, deleteButton);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, previewToggle);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, confirmModal);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, nameModal);
    DECLARE_INSTANCE_FIELD(HMUI::InputFieldView*, nameInput);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, deleteModal);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, resetModal);

    DECLARE_STATIC_METHOD(SettingsViewController*, GetInstance);

   private:
    bool nameModalIsRename = false;

    static inline SettingsViewController* instance = nullptr;
};

DECLARE_CLASS_CODEGEN(Qounters, PlaytestViewController, HMUI::ViewController) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool, bool, bool);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, Update);

    DECLARE_INSTANCE_METHOD(void, UpdateUI);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, uiInitialized, false);

    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, lNote);
    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, rNote);
    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, lChain);
    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, rChain);
    DECLARE_INSTANCE_FIELD(BSML::ClickableImage*, wall);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, resetButton);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, pbToggle);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, pbSlider);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, timeSlider);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, posModsIncrement);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, negModsIncrement);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, blToggle);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, blSlider);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, ssToggle);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, ssSlider);

    DECLARE_STATIC_METHOD(PlaytestViewController*, GetInstance);

   private:
    static inline PlaytestViewController* instance = nullptr;
};

DECLARE_CLASS_CODEGEN(Qounters, TemplatesViewController, HMUI::ViewController) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool, bool, bool);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidDeactivate, &HMUI::ViewController::DidDeactivate, bool, bool);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

    DECLARE_INSTANCE_METHOD(void, ShowTemplateModal, int idx);
    DECLARE_INSTANCE_METHOD(void, HideModal);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, uiInitialized, false);

    DECLARE_INSTANCE_FIELD(BSML::CustomListTableData*, list);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, modal);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, modalLayout);

    DECLARE_STATIC_METHOD(TemplatesViewController*, GetInstance);

   private:
    static inline TemplatesViewController* instance = nullptr;
};

DECLARE_CLASS_CODEGEN(Qounters, OptionsViewController, HMUI::ViewController) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool, bool, bool);

    DECLARE_INSTANCE_METHOD(void, Deselect);
    DECLARE_INSTANCE_METHOD(void, GroupSelected);
    DECLARE_INSTANCE_METHOD(void, ComponentSelected);

    DECLARE_INSTANCE_METHOD(void, UpdateScrollView);
    DECLARE_INSTANCE_METHOD(void, UpdateSimpleUI);
    DECLARE_INSTANCE_METHOD(void, UpdateUI);
    DECLARE_INSTANCE_METHOD(void, UpdateTypeOptions);
    DECLARE_INSTANCE_METHOD(void, UpdateColorSourceOptions);
    DECLARE_INSTANCE_METHOD(void, UpdateEnableSourceOptions);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, uiInitialized, false);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, group, false);
    DECLARE_INSTANCE_FIELD_DEFAULT(bool, component, false);

    DECLARE_INSTANCE_FIELD(UnityEngine::Sprite*, lockSprite);
    DECLARE_INSTANCE_FIELD(UnityEngine::Sprite*, unlockSprite);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, groupParent);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, componentParent);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::HorizontalLayoutGroup*, cButtonsParent);

    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, gPosIncrementX);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, gPosIncrementY);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, gDetPosIncrementX);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetPosLockX);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, gDetPosIncrementY);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetPosLockY);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, gDetPosIncrementZ);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetPosLockZ);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, gRotSlider);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, gDetRotSliderX);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetRotLockX);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, gDetRotSliderY);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetRotLockY);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, gDetRotSliderZ);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetRotLockZ);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gComponentButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gPasteButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDuplicateButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDeleteButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDeselectButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetachButton);

    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, cPosIncrementX);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, cPosIncrementY);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cRotSlider);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cScaleSliderX);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cScaleSliderY);
    DECLARE_INSTANCE_FIELD(UnityEngine::Component*, typeCollapseComponent);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, cTypeDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::Component*, colorCollapseComponent);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, cColorSourceDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::Component*, gradientCollapseComponent);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, cGradientToggle);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, cGradientDirectionDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::Component*, startHsvComponent);
    DECLARE_INSTANCE_FIELD(UnityEngine::Component*, endHsvComponent);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, cEnableSourceDropdown);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, cInvertEnableToggle);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, cDeleteButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, cDeselectButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, cCopyButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, cDuplicateButton);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cTypeOptions);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cColorSourceOptions);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cEnableSourceOptions);

    DECLARE_STATIC_METHOD(Qounters::OptionsViewController*, GetInstance);
    DECLARE_STATIC_METHOD(void, UpdateScrollViewStatic);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);

   private:
    static inline Qounters::OptionsViewController* instance = nullptr;
};

DECLARE_CLASS_CODEGEN(Qounters, HSVGradientImage, UnityEngine::MonoBehaviour) {
    DECLARE_DEFAULT_CTOR();
    DECLARE_INSTANCE_FIELD(int, modified);
    DECLARE_INSTANCE_FIELD(float, modifier);
    DECLARE_INSTANCE_FIELD_DEFAULT(int, elements, 10);
};

DECLARE_CLASS_CODEGEN(Qounters, HSVController, UnityEngine::MonoBehaviour) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(UnityEngine::Color, baseColor);
    DECLARE_INSTANCE_FIELD(UnityEngine::Vector3, hsv);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, nameText);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, openButton);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, modal);
    DECLARE_INSTANCE_FIELD(HMUI::ColorGradientSlider*, hSlider);
    DECLARE_INSTANCE_FIELD(HMUI::ColorGradientSlider*, sSlider);
    DECLARE_INSTANCE_FIELD(HMUI::ColorGradientSlider*, vSlider);

    DECLARE_INSTANCE_METHOD(void, Show);
    DECLARE_INSTANCE_METHOD(void, Hide);
    DECLARE_INSTANCE_METHOD(void, SetHue, float hue);
    DECLARE_INSTANCE_METHOD(void, SetSat, float saturation);
    DECLARE_INSTANCE_METHOD(void, SetVal, float value);
    DECLARE_INSTANCE_METHOD(void, UpdateVisuals);
    DECLARE_INSTANCE_METHOD(void, UpdateButton);
    DECLARE_INSTANCE_METHOD(void, SetHSV, UnityEngine::Vector3 value);

   public:
    std::function<void(UnityEngine::Vector3)> onChange = nullptr;
    std::function<void()> onClose = nullptr;
};

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, CollapseController, UnityEngine::MonoBehaviour, UES::IEventSystemHandler*, UES::IPointerEnterHandler*, UES::IPointerExitHandler*, UES::IPointerClickHandler*) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(StringW, title);
    DECLARE_INSTANCE_FIELD(bool, open);
    DECLARE_INSTANCE_FIELD(bool, wasOpen);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, text);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, line);

    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerEnter, &UES::IPointerEnterHandler::OnPointerEnter, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerExit, &UES::IPointerExitHandler::OnPointerExit, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerClick, &UES::IPointerClickHandler::OnPointerClick, UES::PointerEventData* eventData);

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);

    DECLARE_INSTANCE_METHOD(void, UpdateOpen);

   public:
    std::map<UnityEngine::Component*, bool> contents;
    void AddContents(std::set<UnityEngine::Component*> const& add);
    void RemoveContents(std::set<UnityEngine::Component*> const& remove);
    void SetContentActive(UnityEngine::Component * content, bool active);

    std::function<void()> onUpdate = nullptr;
};

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, MenuDragger, UnityEngine::MonoBehaviour, UES::IEventSystemHandler*, UES::IPointerEnterHandler*, UES::IPointerExitHandler*, UES::IInitializePotentialDragHandler*, UES::IDragHandler*, UES::IEndDragHandler*) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);

    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerEnter, &UES::IPointerEnterHandler::OnPointerEnter, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerExit, &UES::IPointerExitHandler::OnPointerExit, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(
        void, OnInitializePotentialDrag, &UES::IInitializePotentialDragHandler::OnInitializePotentialDrag, UES::PointerEventData* eventData
    );
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnDrag, &UES::IDragHandler::OnDrag, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnEndDrag, &UES::IEndDragHandler::OnEndDrag, UES::PointerEventData* eventData);

    DECLARE_INSTANCE_METHOD(bool, IsPointerPosValid, UES::PointerEventData* eventData);
    DECLARE_INSTANCE_METHOD(float, GetPointerPosX, UES::PointerEventData* eventData);

    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, line);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, menu);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, dragCanvas);
    DECLARE_INSTANCE_FIELD(UnityEngine::Canvas*, rootCanvas);
    DECLARE_INSTANCE_FIELD(bool, isLeftMenu);
    DECLARE_INSTANCE_FIELD(float, originalPosition);
    DECLARE_INSTANCE_FIELD(float, dragPosition);
};

DECLARE_CLASS_CODEGEN(Qounters, SpritesListCell, HMUI::TableCell) {
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(ArrayW<BSML::ClickableImage*>, images);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, layout);
    DECLARE_INSTANCE_FIELD(int, imageStartIdx);

    DECLARE_INSTANCE_METHOD(void, OnImageClicked, int imageIdx);

    DECLARE_INSTANCE_METHOD(void, SetImageStartIdx, int idx);

    DECLARE_STATIC_METHOD(SpritesListCell*, CreateNew, int imagesStartIdx, StringW reuseIdentifier);

   public:
    static constexpr int ImagesPerCell = 4;
    static constexpr float ImageSpacing = 2;
    static constexpr float CellSize = 10;
};

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, SpritesListSource, UnityEngine::MonoBehaviour, HMUI::TableView::IDataSource*) {
    DECLARE_INSTANCE_FIELD(HMUI::TableView*, tableView);

    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(HMUI::TableCell*, CellForIdx, &HMUI::TableView::IDataSource::CellForIdx, HMUI::TableView * tableView, int idx);

    DECLARE_OVERRIDE_METHOD_MATCH(float, CellSize, &HMUI::TableView::IDataSource::CellSize);
    DECLARE_OVERRIDE_METHOD_MATCH(int, NumberOfCells, &HMUI::TableView::IDataSource::NumberOfCells);

    DECLARE_INSTANCE_METHOD(void, OnImageClicked, int imageIdx);

   public:
    std::function<void(int)> imageClickedCallback;

   private:
    static inline std::string const ReuseIdentifier = "QountersCustomListSource";
};

#undef UES
