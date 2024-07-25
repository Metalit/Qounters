#pragma once

#include "GlobalNamespace/EnvironmentInfoSO.hpp"
#include "HMUI/FlowCoordinator.hpp"
#include "HMUI/TableCell.hpp"
#include "HMUI/TableView.hpp"
#include "HMUI/ViewController.hpp"
#include "System/Action.hpp"
#include "UnityEngine/EventSystems/IInitializePotentialDragHandler.hpp"
#include "UnityEngine/EventSystems/IDragHandler.hpp"
#include "UnityEngine/EventSystems/IEndDragHandler.hpp"
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

DECLARE_CLASS_CODEGEN(Qounters, SettingsFlowCoordinator, HMUI::FlowCoordinator,
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

    static void ConfirmAction(std::function<void ()> action, std::function<void ()> cancel = nullptr);
    static inline std::function<void ()> nextModalAction = nullptr;
    static inline std::function<void ()> nextModalCancel = nullptr;

    static void MakeNewPreset(std::string name, bool removeOld);
)

DECLARE_CLASS_CODEGEN(Qounters, SettingsViewController, HMUI::ViewController,
    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool, bool, bool);

    DECLARE_STATIC_METHOD(SettingsViewController*, GetInstance);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, ShowConfirmModal);
    DECLARE_INSTANCE_METHOD(void, HideConfirmModal);
    DECLARE_INSTANCE_METHOD(void, UpdateUI);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, uiInitialized, false);
    DECLARE_INSTANCE_FIELD(ArrayW<UnityW<GlobalNamespace::EnvironmentInfoSO>>, environments);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, undoButton);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, environmentDropdown);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, presetDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, deleteButton);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, previewToggle);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, confirmModal);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, nameModal);
    DECLARE_INSTANCE_FIELD(HMUI::InputFieldView*, nameInput);

   private:
    bool nameModalIsRename = false;

    static inline SettingsViewController* instance = nullptr;
)

DECLARE_CLASS_CODEGEN(Qounters, PlaytestViewController, HMUI::ViewController,
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
)

DECLARE_CLASS_CODEGEN(Qounters, TemplatesViewController, HMUI::ViewController,
    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool, bool, bool);

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
)

DECLARE_CLASS_CODEGEN(Qounters, OptionsViewController, HMUI::ViewController,
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
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetachButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDeleteButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDeselectButton);

    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, cPosIncrementX);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, cPosIncrementY);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cRotSlider);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cScaleSliderX);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cScaleSliderY);
    DECLARE_INSTANCE_FIELD(UnityEngine::Component*, typeCollapseComponent);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, cTypeDropdown);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, cColorSourceDropdown);
    DECLARE_INSTANCE_FIELD(BSML::DropdownListSetting*, cEnableSourceDropdown);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, cInvertEnableToggle);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, cDeleteButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, cDeselectButton);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cTypeOptions);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cColorSourceOptions);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cEnableSourceOptions);

    DECLARE_STATIC_METHOD(Qounters::OptionsViewController*, GetInstance);
    DECLARE_STATIC_METHOD(void, UpdateScrollViewStatic);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);

   private:
    static inline Qounters::OptionsViewController* instance = nullptr;
)

#define INTERFACES std::vector<Il2CppClass*>({ \
    classof(UES::IEventSystemHandler*), \
    classof(UES::IPointerEnterHandler*), \
    classof(UES::IPointerExitHandler*), \
    classof(UES::IPointerClickHandler*), \
})

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, CollapseController, UnityEngine::MonoBehaviour, INTERFACES,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(StringW, title);
    DECLARE_INSTANCE_FIELD(bool, open);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, text);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, line);

    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerEnter, &UES::IPointerEnterHandler::OnPointerEnter, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerExit, &UES::IPointerExitHandler::OnPointerExit, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerClick, &UES::IPointerClickHandler::OnPointerClick, UES::PointerEventData* eventData);

    DECLARE_INSTANCE_METHOD(void, UpdateOpen);

   public:
    std::vector<UnityEngine::Component*> contents;
    void SetContents(std::vector<UnityEngine::Component*> const& newContents);

    std::function<void ()> onUpdate = nullptr;
)

#undef INTERFACES

#define INTERFACES std::vector<Il2CppClass*>({ \
    classof(UES::IEventSystemHandler*), \
    classof(UES::IPointerEnterHandler*), \
    classof(UES::IPointerExitHandler*), \
    classof(UES::IInitializePotentialDragHandler*), \
    classof(UES::IDragHandler*), \
    classof(UES::IEndDragHandler*), \
})

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, MenuDragger, UnityEngine::MonoBehaviour, INTERFACES,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);

    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerEnter, &UES::IPointerEnterHandler::OnPointerEnter, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnPointerExit, &UES::IPointerExitHandler::OnPointerExit, UES::PointerEventData* eventData);
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnInitializePotentialDrag, &UES::IInitializePotentialDragHandler::OnInitializePotentialDrag, UES::PointerEventData* eventData);
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
)

#undef INTERFACES

#define INTERFACES std::vector<Il2CppClass*>({ \
    classof(UES::IEventSystemHandler*), \
    classof(UES::IEndDragHandler*), \
})

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, EndDragHandler, UnityEngine::MonoBehaviour, INTERFACES,
    DECLARE_OVERRIDE_METHOD_MATCH(void, OnEndDrag, &UES::IEndDragHandler::OnEndDrag, UES::PointerEventData* eventData);

    DECLARE_DEFAULT_CTOR();
   public:
    std::function<void ()> callback = nullptr;
)

#undef INTERFACES

DECLARE_CLASS_CODEGEN(Qounters, KeyboardCloseHandler,  UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();

   public:
    std::function<void ()> closeCallback = nullptr;
    std::function<void ()> okCallback = nullptr;
)

DECLARE_CLASS_CODEGEN(Qounters, SpritesListCell, HMUI::TableCell,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(ArrayW<BSML::ClickableImage*>, images);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, layout);
    DECLARE_INSTANCE_FIELD(int, imageStartIdx);

    DECLARE_INSTANCE_METHOD(void, OnImageClicked, int imageIdx);

    // DECLARE_INSTANCE_METHOD(void, RefreshVisuals);
    DECLARE_INSTANCE_METHOD(void, SetImageStartIdx, int idx);
    // DECLARE_OVERRIDE_METHOD_MATCH(void, SelectionDidChange, &HMUI::SelectableCell::SelectionDidChange, HMUI::SelectableCell::TransitionType transitionType);
    // DECLARE_OVERRIDE_METHOD_MATCH(void, HighlightDidChange, &HMUI::SelectableCell::HighlightDidChange, HMUI::SelectableCell::TransitionType transitionType);

    DECLARE_STATIC_METHOD(SpritesListCell*, CreateNew, int imagesStartIdx, StringW reuseIdentifier);

   public:
    static constexpr int imagesPerCell = 4;
    static constexpr float imageSpacing = 2;
    static constexpr float cellSize = 10;
)

#define INTERFACES std::vector<Il2CppClass*>({ \
    classof(HMUI::TableView::IDataSource*), \
})

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, SpritesListSource, UnityEngine::MonoBehaviour, INTERFACES,
    DECLARE_INSTANCE_FIELD(HMUI::TableView*, tableView);

    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(HMUI::TableCell*, CellForIdx, &HMUI::TableView::IDataSource::CellForIdx, HMUI::TableView* tableView, int idx);

    DECLARE_OVERRIDE_METHOD_MATCH(float, CellSize, &HMUI::TableView::IDataSource::CellSize);
    DECLARE_OVERRIDE_METHOD_MATCH(int, NumberOfCells, &HMUI::TableView::IDataSource::NumberOfCells);

    DECLARE_INSTANCE_METHOD(void, OnImageClicked, int imageIdx);
   public:
    std::function<void (int)> imageClickedCallback;
   private:
    static const inline std::string reuseIdentifier = "QountersCustomListSource";
)

#undef INTERFACES
#undef UES
