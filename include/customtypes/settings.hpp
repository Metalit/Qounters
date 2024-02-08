#pragma once

#include "main.hpp"
#include "config.hpp"

#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/coroutine.hpp"

#include "bsml/shared/BSML/Components/Settings/IncrementSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/SliderSetting.hpp"
#include "bsml/shared/BSML/Components/ClickableImage.hpp"

#include "UnityEngine/UI/VerticalLayoutGroup.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "HMUI/SimpleTextDropdown.hpp"
#include "HMUI/ModalView.hpp"
#include "HMUI/FlowCoordinator.hpp"
#include "System/Action.hpp"

#define METHOD(...) il2cpp_utils::il2cpp_type_check::MetadataGetter<__VA_ARGS__>::methodInfo()
#define INTERFACES(...) std::vector<Il2CppClass*>({ __VA_ARGS__ })

DECLARE_CLASS_CODEGEN(Qounters, SettingsFlowCoordinator, HMUI::FlowCoordinator,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, METHOD(&HMUI::FlowCoordinator::DidActivate), bool, bool, bool);

    DECLARE_DEFAULT_CTOR();

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

    DECLARE_INSTANCE_FIELD_DEFAULT(HMUI::ViewController*, blankViewController, nullptr);

    DECLARE_STATIC_METHOD(Qounters::SettingsFlowCoordinator*, GetInstance);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);

    private:
    static inline Qounters::SettingsFlowCoordinator* instance = nullptr;

    static void ConfirmAction(std::function<void ()> action, std::function<void ()> cancel = nullptr);
    static inline std::function<void ()> nextModalAction = nullptr;
    static inline std::function<void ()> nextModalCancel = nullptr;

    static void MakeNewPreset(std::string name, bool removeOld);
)

#include "HMUI/ViewController.hpp"

DECLARE_CLASS_CODEGEN(Qounters, SettingsViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, METHOD(&HMUI::ViewController::DidActivate), bool, bool, bool);

    DECLARE_DEFAULT_CTOR();

    DECLARE_STATIC_METHOD(SettingsViewController*, GetInstance);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_METHOD(void, ShowConfirmModal);
    DECLARE_INSTANCE_METHOD(void, HideConfirmModal);
    DECLARE_INSTANCE_METHOD(void, UpdateUI);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, uiInitialized, false);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, undoButton);
    DECLARE_INSTANCE_FIELD(HMUI::SimpleTextDropdown*, presetDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, deleteButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Toggle*, previewToggle);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, confirmModal);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, nameModal);

    private:
    bool nameModalIsRename = false;

    static inline SettingsViewController* instance = nullptr;
)

#include "bsml/shared/BSML/Components/CustomListTableData.hpp"

DECLARE_CLASS_CODEGEN(Qounters, TemplatesViewController, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, METHOD(&HMUI::ViewController::DidActivate), bool, bool, bool);

    DECLARE_DEFAULT_CTOR();
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
    DECLARE_OVERRIDE_METHOD(void, DidActivate, METHOD(&HMUI::ViewController::DidActivate), bool, bool, bool);

    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(void, Deselect);
    DECLARE_INSTANCE_METHOD(void, GroupSelected);
    DECLARE_INSTANCE_METHOD(void, ComponentSelected);

    DECLARE_INSTANCE_METHOD(void, UpdateSimpleUI);
    DECLARE_INSTANCE_METHOD(void, UpdateUI);
    DECLARE_INSTANCE_METHOD(void, UpdateTypeOptions);
    DECLARE_INSTANCE_METHOD(void, UpdateColorSourceOptions);
    DECLARE_INSTANCE_METHOD(void, UpdateEnableSourceOptions);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, uiInitialized, false);

    DECLARE_INSTANCE_FIELD_DEFAULT(bool, group, false);
    DECLARE_INSTANCE_FIELD_DEFAULT(bool, component, false);

    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, groupParent);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, componentParent);

    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, gPosIncrementX);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, gPosIncrementY);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, gPosIncrementZ);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, gRotSlider);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, gRotSliderX);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, gRotSliderY);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, gRotSliderZ);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gComponentButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDetachButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDeleteButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, gDeselectButton);

    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, cPosIncrementX);
    DECLARE_INSTANCE_FIELD(BSML::IncrementSetting*, cPosIncrementY);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cRotSlider);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cScaleSliderX);
    DECLARE_INSTANCE_FIELD(BSML::SliderSetting*, cScaleSliderY);
    DECLARE_INSTANCE_FIELD(HMUI::SimpleTextDropdown*, cTypeDropdown);
    DECLARE_INSTANCE_FIELD(HMUI::SimpleTextDropdown*, cColorSourceDropdown);
    DECLARE_INSTANCE_FIELD(HMUI::SimpleTextDropdown*, cEnableSourceDropdown);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Toggle*, cInvertEnableToggle);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, cDeleteButton);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, cDeselectButton);

    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cTypeOptions);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cColorSourceOptions);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::VerticalLayoutGroup*, cEnableSourceOptions);

    DECLARE_STATIC_METHOD(Qounters::OptionsViewController*, GetInstance);

    DECLARE_INSTANCE_METHOD(void, OnDestroy);

    private:
    custom_types::Helpers::Coroutine UpdateScrollView();

    static inline Qounters::OptionsViewController* instance = nullptr;
)

#define UES UnityEngine::EventSystems

#include "UnityEngine/EventSystems/IEndDragHandler.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, EndDragHandler, UnityEngine::MonoBehaviour, INTERFACES( classof(UES::IEventSystemHandler*), classof(UES::IEndDragHandler*) ),
    DECLARE_OVERRIDE_METHOD(void, OnEndDrag, METHOD(&UES::IEndDragHandler::OnEndDrag), UES::PointerEventData* eventData);

    DECLARE_DEFAULT_CTOR();
    public:
    std::function<void ()> callback = nullptr;
)

DECLARE_CLASS_CODEGEN(Qounters, KeyboardCloseHandler,  UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();

    public:
    std::function<void ()> closeCallback = nullptr;
    std::function<void ()> okCallback = nullptr;
)

#include "HMUI/TableCell.hpp"

DECLARE_CLASS_CODEGEN(Qounters, SpritesListCell, HMUI::TableCell,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_FIELD(ArrayW<BSML::ClickableImage*>, images);
    DECLARE_INSTANCE_FIELD(UnityEngine::RectTransform*, layout);
    DECLARE_INSTANCE_FIELD(int, imageStartIdx);

    DECLARE_INSTANCE_METHOD(void, OnImageClicked, int imageIdx);

    // DECLARE_INSTANCE_METHOD(void, RefreshVisuals);
    DECLARE_INSTANCE_METHOD(void, SetImageStartIdx, int idx);
    // DECLARE_OVERRIDE_METHOD(void, SelectionDidChange, METHOD(&HMUI::SelectableCell::SelectionDidChange), HMUI::SelectableCell::TransitionType transitionType);
    // DECLARE_OVERRIDE_METHOD(void, HighlightDidChange, METHOD(&HMUI::SelectableCell::HighlightDidChange), HMUI::SelectableCell::TransitionType transitionType);

    DECLARE_STATIC_METHOD(SpritesListCell*, CreateNew, int imagesStartIdx, StringW reuseIdentifier);

    public:
    static constexpr int imagesPerCell = 4;
    static constexpr float imageSpacing = 2;
    static constexpr float cellSize = 10;
)

#include "HMUI/TableView.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(Qounters, SpritesListSource, UnityEngine::MonoBehaviour, INTERFACES( classof(HMUI::TableView::IDataSource*) ),
    DECLARE_INSTANCE_FIELD(HMUI::TableView*, tableView);

    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD(HMUI::TableCell*, CellForIdx, METHOD(&HMUI::TableView::IDataSource::CellForIdx), HMUI::TableView* tableView, int idx);

    DECLARE_OVERRIDE_METHOD(float, CellSize, METHOD(&HMUI::TableView::IDataSource::CellSize));
    DECLARE_OVERRIDE_METHOD(int, NumberOfCells, METHOD(&HMUI::TableView::IDataSource::NumberOfCells));

    DECLARE_INSTANCE_METHOD(void, OnImageClicked, int imageIdx);
    public:
    std::function<void (int)> imageClickedCallback;
    private:
    static const inline std::string reuseIdentifier = "QountersCustomListSource";
)

#undef INTERFACES
