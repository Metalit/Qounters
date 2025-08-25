#include "utils.hpp"

#include <iomanip>
#include <sstream>
#include <string>

#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "HMUI/AnimatedSwitchView.hpp"
#include "HMUI/ButtonBinder.hpp"
#include "HMUI/ScrollView.hpp"
#include "System/Action.hpp"
#include "UnityEngine/CanvasGroup.hpp"
#include "UnityEngine/EventSystems/EventSystem.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/UI/LayoutRebuilder.hpp"
#include "UnityEngine/UI/Mask.hpp"
#include "assets.hpp"
#include "bsml/shared/BSML-Lite.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "copies.hpp"
#include "customtypes/editing.hpp"
#include "customtypes/settings.hpp"
#include "environment.hpp"
#include "main.hpp"
#include "metacore/shared/delegates.hpp"
#include "metacore/shared/stats.hpp"
#include "metacore/shared/ui.hpp"
#include "playtest.hpp"
#include "songcore/shared/SongCore.hpp"
#include "songcore/shared/SongLoader/CustomBeatmapLevel.hpp"
#include "types.hpp"

using namespace Qounters;
using namespace MetaCore;
using namespace GlobalNamespace;

UnityEngine::Vector3 Utils::GetFixedEuler(UnityEngine::Quaternion rotation) {
    auto ret = rotation.eulerAngles;
    for (auto val : {&ret.x, &ret.y, &ret.z}) {
        if (*val > 180)
            *val -= 360;
    }
    return ret;
}

UnityEngine::Color Utils::GetClampedColor(std::tuple<float, float, float> hsv) {
    auto [h, s, v] = hsv;
    h = fmod(h, 1);
    if (h < 0)
        h += 1;
    s = std::clamp(s, 0.f, 1.f);
    v = std::clamp(v, 0.f, 1.f);
    return UnityEngine::Color::HSVToRGB(h, s, v);
}

std::tuple<std::string, std::string, int> Utils::GetBeatmapDetails(BeatmapKey beatmap) {
    std::string id = beatmap.levelId;
    std::string characteristic = beatmap.beatmapCharacteristic->serializedName;
    int difficulty = (int) beatmap.difficulty;
    return {id, characteristic, difficulty};
}

std::string Utils::GetBeatmapIdentifier(BeatmapKey beatmap) {
    if (!beatmap.IsValid())
        return "Unknown";
    auto [id, characteristic, difficulty] = GetBeatmapDetails(beatmap);
    return fmt::format("{}_{}_{}", id, characteristic, difficulty);
}

std::map<std::string, std::string> const RequirementsMap = {
    {"Noodle Extensions", "<color=#ffb73b>+Noodle</color>"},
    {"Chroma", "<color=#45f3ff>+Chroma</color>"},
};

std::vector<std::string> Utils::GetSimplifiedRequirements(BeatmapKey beatmap) {
    auto level = SongCore::API::Loading::GetLevelByLevelID((std::string) beatmap.levelId);
    if (!level)
        return {};
    auto custom = level->CustomSaveDataInfo;
    if (!custom)
        return {};
    auto diff = custom->get().TryGetCharacteristicAndDifficulty(beatmap.beatmapCharacteristic->serializedName, beatmap.difficulty);
    if (!diff)
        return {};
    std::set<std::string> all;
    all.insert(diff->get().requirements.begin(), diff->get().requirements.end());
    all.insert(diff->get().suggestions.begin(), diff->get().suggestions.end());
    std::vector<std::string> ret;
    for (auto& req : all) {
        if (RequirementsMap.contains(req))
            ret.emplace_back(RequirementsMap.at(req));
    }
    return ret;
}

std::string Utils::FormatNumber(int value, int separator) {
    std::string seperatorString;
    switch ((Qounters::Types::Separators) separator) {
        case Qounters::Types::Separators::None:
            return std::to_string(value);
        case Qounters::Types::Separators::Gap:
            seperatorString = " ";
            break;
        case Qounters::Types::Separators::Comma:
            seperatorString = ",";
            break;
        case Qounters::Types::Separators::Period:
            seperatorString = ".";
            break;
    }

    std::string num = std::to_string(std::abs(value));
    int insertPosition = static_cast<int>(num.length()) - 3;
    while (insertPosition > 0) {
        num.insert(insertPosition, seperatorString);
        insertPosition -= 3;
    }
    return (value < 0 ? "-" + num : num);
}

double Utils::GetScoreRatio(bool includeModifiers, int saber) {
    int max = Stats::GetMaxScore((int) saber);
    if (max == 0)
        return 1;
    int current = Stats::GetScore((int) saber);
    if (includeModifiers)
        current *= Stats::GetModifierMultiplier(true, true);
    return current / (double) max;
}

double Utils::GetBestScoreRatio() {
    if (Environment::InSettings())
        return std::max(Playtest::GetOverridePBRatio(), (float) 0);
    int max = Stats::GetSongMaxScore();
    if (max == 0)
        return 1;
    int best = Stats::GetBestScore();
    if (best == -1)
        return 0;
    best *= Stats::GetModifierMultiplier(true, true);
    return best / (double) max;
}

BSML::ColorSetting* Utils::CreateColorPicker(
    UnityEngine::GameObject* parent,
    std::string name,
    UnityEngine::Color value,
    std::function<void(UnityEngine::Color)> onChange,
    std::function<void()> onClose
) {
    static UnityEngine::Color copied;
    static bool hasCopied = false;
    static UnityW<UnityEngine::Sprite> copySprite;
    if (!copySprite)
        copySprite = PNG_SPRITE(Copy);

    auto ret = BSML::Lite::CreateColorPicker(parent, name, value);
    ret->modalColorPicker->onChange = [ret, onChange](UnityEngine::Color val) {
        ret->set_currentColor(val);
        onChange(val);
    };
    auto modal = ret->modalColorPicker->GetComponent<UnityEngine::RectTransform*>();
    modal->Find("BSMLHSVPanel/ColorPickerButtonPrimary")->gameObject->active = false;
    modal->Find("BSMLHorizontalLayoutGroup")->gameObject->active = false;
    modal->sizeDelta = {50, 70};
    auto rgb = ret->modalColorPicker->rgbPanel->GetComponent<UnityEngine::RectTransform*>();
    auto wheel = ret->modalColorPicker->hsvPanel->GetComponent<UnityEngine::RectTransform*>();
    auto preview = ret->modalColorPicker->colorImage->GetComponent<UnityEngine::RectTransform*>();
    rgb->localScale = {0.75, 0.75, 0.75};
    rgb->anchorMin = {0.5, 0.5};
    rgb->anchorMax = {0.5, 0.5};
    rgb->anchoredPosition = {-24, 10};
    auto content = rgb->GetChild(0);
    for (int i = 0; i < 3; i++) {
        auto slider = content->GetChild(i).cast<UnityEngine::RectTransform>();
        slider->anchoredPosition = {4, slider->anchoredPosition.y};
        slider->sizeDelta = {56, 8};
    }
    wheel->localScale = {0.8, 0.8, 0.8};
    wheel->anchorMin = {0.5, 0.5};
    wheel->anchorMax = {0.5, 0.5};
    wheel->anchoredPosition = {0, -10};
    preview->anchoredPosition = {17, -27};
    auto modalView = ret->modalColorPicker->modalView;
    modalView->moveToCenter = true;
    modalView->add_blockerClickedEvent(Delegates::MakeSystemAction([ret, modalView, onClose]() {
        modalView->Hide();
        ret->currentColor = ret->modalColorPicker->currentColor;
        onClose();
    }));

    auto copyModal = BSML::Lite::CreateModal(ret);
    auto modalRect = copyModal->GetComponent<UnityEngine::RectTransform*>();
    modalRect->anchoredPosition = {10, 0};
    modalRect->sizeDelta = {35, 18};
    auto vertical = BSML::Lite::CreateVerticalLayoutGroup(copyModal);
    vertical->spacing = -2;
    auto copyButton = BSML::Lite::CreateUIButton(vertical, "       Copy Color", [copyModal, ret]() {
        copyModal->Hide();
        hasCopied = true;
        copied = ret->get_currentColor();
    });
    auto horizontal = BSML::Lite::CreateHorizontalLayoutGroup(copyButton);
    horizontal->childForceExpandWidth = false;
    auto currentColorImage = UnityEngine::Object::Instantiate(ret->colorImage, horizontal->transform, false);
    currentColorImage->name = "QountersCurrentColor";
    currentColorImage->preserveAspect = true;
    UI::SetLayoutSize(currentColorImage, 9, 4);
    auto pasteButton = BSML::Lite::CreateUIButton(vertical, "       Paste Color", [copyModal, ret, onChange, onClose]() {
        copyModal->Hide();
        if (hasCopied) {
            ret->set_currentColor(copied);
            onChange(copied);
            onClose();
        }
    });
    horizontal = BSML::Lite::CreateHorizontalLayoutGroup(pasteButton);
    horizontal->childForceExpandWidth = false;
    auto copiedColorImage = UnityEngine::Object::Instantiate(ret->colorImage, horizontal->transform, false);
    copiedColorImage->preserveAspect = true;
    copiedColorImage->name = "QountersCopiedColor";
    UI::SetLayoutSize(copiedColorImage, 9, 4);

    auto valuePicker = ret->transform->Find("ValuePicker").cast<UnityEngine::RectTransform>();
    valuePicker->anchoredPosition = {-9, 0};
    auto button = UI::CreateIconButton(ret->gameObject, copySprite, [copyModal, ret, currentColorImage, pasteButton, copiedColorImage]() {
        currentColorImage->color = ret->get_currentColor();
        copiedColorImage->color = hasCopied ? copied : UnityEngine::Color(1, 1, 1, 0.3);
        pasteButton->interactable = hasCopied;
        copyModal->Show();
    });
    BSML::Lite::AddHoverHint(button, "Copy or paste colors");
    auto buttonRect = button->GetComponent<UnityEngine::RectTransform*>();
    buttonRect->anchorMin = {1, 0};
    buttonRect->anchorMax = {1, 1};
    buttonRect->anchoredPosition = {-4, 0};

    return ret;
}

HMUI::ColorGradientSlider* CreateGradientSlider(
    UnityEngine::Component* parent, UnityEngine::Vector2 anchoredPosition, std::function<void(float)> onChange, int modified, float modifier
) {
    static SafePtrUnity<HMUI::ColorGradientSlider> sliderTemplate;
    if (!sliderTemplate) {
        sliderTemplate = UnityEngine::Resources::FindObjectsOfTypeAll<HMUI::ColorGradientSlider*>()->FirstOrDefault([](auto x) {
            return x->name == StringW("GGradientSlider");
        });
    }
    auto ret = UnityEngine::Object::Instantiate(sliderTemplate.ptr(), parent->transform, false);
    ret->gameObject->name = "QountersGradientSlider";
    ret->normalizedValueDidChangeEvent = Delegates::MakeSystemAction([onChange](UnityW<HMUI::TextSlider>, float val) {
        onChange(val * 2 - 1);  // (0, 1) -> (-1, 1)
    });
    ret->SetColors({1, 1, 1, 1}, {1, 1, 1, 1});
    for (auto& img : ret->_gradientImages) {
        auto hsv = img->gameObject->AddComponent<HSVGradientImage*>();
        hsv->modified = modified;
        hsv->modifier = modifier;
    }
    auto rect = ret->GetComponent<UnityEngine::RectTransform*>();
    rect->anchorMin = {0.5, 0.5};
    rect->anchorMax = {0.5, 0.5};
    rect->pivot = {0.5, 0.5};
    rect->sizeDelta = {40, 10};
    rect->anchoredPosition = anchoredPosition;
    return ret;
}

HSVController* Utils::CreateHSVModifierPicker(
    UnityEngine::GameObject* parent, std::string name, std::function<void(UnityEngine::Vector3)> onChange, std::function<void()> onClose
) {
    auto layout = BSML::Lite::CreateHorizontalLayoutGroup(parent);
    layout->gameObject->name = "QountersHSVPicker";
    auto ret = layout->gameObject->AddComponent<HSVController*>();
    ret->onChange = onChange;
    ret->onClose = onClose;
    ret->nameText = BSML::Lite::CreateText(layout, name);
    UI::SetLayoutSize(ret->nameText, -1, -1, 999);
    ret->openButton = BSML::Lite::CreateUIButton(layout, "H+0 S+0 V+0", [ret]() { ret->Show(); });
    auto modal = BSML::Lite::CreateModal(parent, false);
    modal->add_blockerClickedEvent(Delegates::MakeSystemAction([ret]() { ret->Hide(); }));
    modal->GetComponent<UnityEngine::RectTransform*>()->sizeDelta = {50, 40};
    ret->modal = modal;
    // go a little extra on the sides because of the padding
    ret->hSlider = CreateGradientSlider(modal, {0, 12}, [ret](float val) { ret->SetHue(val); }, 0, 0.55);
    ret->sSlider = CreateGradientSlider(modal, {0, 0}, [ret](float val) { ret->SetSat(val); }, 1, 1.1);
    ret->vSlider = CreateGradientSlider(modal, {0, -12}, [ret](float val) { ret->SetVal(val); }, 2, 1.1);
    return ret;
}

CollapseController* Utils::CreateCollapseArea(UnityEngine::GameObject* parent, std::string title, bool open, int copyId) {
    static UnityW<UnityEngine::Sprite> copySprite;
    if (!copySprite)
        copySprite = PNG_SPRITE(Copy);

    auto layout = BSML::Lite::CreateHorizontalLayoutGroup(parent);
    layout->gameObject->name = "QountersCollapseArea";
    layout->spacing = 2;
    layout->childAlignment = UnityEngine::TextAnchor::MiddleCenter;
    layout->childForceExpandHeight = false;
    // makes the whole area clickable
    layout->gameObject->AddComponent<CanvasHighlight*>();
    auto ret = layout->gameObject->AddComponent<CollapseController*>();
    ret->open = open;
    // assume that the current state is what it should remember when open
    ret->wasOpen = true;
    ret->title = title;
    ret->text = BSML::Lite::CreateText(layout, "", TMPro::FontStyles::Normal, 3.5);
    ret->line = BSML::Lite::CreateImage(layout, BSML::Utilities::ImageResources::GetWhitePixel());
    UI::SetLayoutSize(ret->line, 0, 0.4, 999);
    ret->UpdateOpen();
    // update colors
    ret->OnPointerExit(nullptr);

    if (copyId >= 0) {
        auto copyImage = BSML::Lite::CreateClickableImage(layout, copySprite);
        UI::SetLayoutSize(copyImage, 3, 3);
        BSML::Lite::AddHoverHint(copyImage, "Copy and paste these options");
        auto copyModal = BSML::Lite::CreateModal(copyImage);
        auto modalRect = copyModal->GetComponent<UnityEngine::RectTransform*>();
        modalRect->anchoredPosition = {-16, 0};
        modalRect->sizeDelta = {24, 18};
        auto vertical = BSML::Lite::CreateVerticalLayoutGroup(copyModal);
        vertical->spacing = -2;
        auto copyEnum = (enum Copies::Copy) copyId;
        auto copyButton = BSML::Lite::CreateUIButton(vertical, "Copy", [copyModal, copyEnum]() {
            copyModal->Hide();
            Copies::Copy(copyEnum);
        });
        UI::SetLayoutSize(copyButton, 20, -1);
        auto pasteButton = BSML::Lite::CreateUIButton(vertical, "Paste", [copyModal, copyEnum]() {
            copyModal->HMUI::ModalView::Hide(false, nullptr);
            Copies::Paste(copyEnum);
        });
        UI::SetLayoutSize(pasteButton, 20, -1);
        copyImage->onClick += [copyModal, pasteButton, copyEnum]() {
            pasteButton->interactable = Copies::HasCopy(copyEnum);
            copyModal->Show();
        };
    }
    return ret;
}

MenuDragger* Utils::CreateMenuDragger(UnityEngine::GameObject* parent, bool isLeftMenu) {
    auto padding = BSML::Lite::CreateCanvas();
    padding->active = false;
    padding->name = "QountersMenuDragger";
    padding->AddComponent<CanvasHighlight*>();
    auto rect = padding->GetComponent<UnityEngine::RectTransform*>();
    rect->SetParent(parent->transform, false);
    rect->localScale = {1, 1, 1};
    rect->anchorMin = {0.5, 1};
    rect->anchorMax = {0.5, 1};
    rect->anchoredPosition = {0, 5};
    rect->sizeDelta = {42, 3};
    auto drag = BSML::Lite::CreateCanvas();
    drag->active = false;
    drag->name = "QountersMenuDragCanvas";
    drag->AddComponent<CanvasHighlight*>();
    auto dragRect = drag->GetComponent<UnityEngine::RectTransform*>();
    dragRect->SetParent(rect, false);
    dragRect->localScale = {1, 1, 1};
    dragRect->sizeDelta = {1000, 1000};
    auto ret = padding->AddComponent<MenuDragger*>();
    ret->dragCanvas = drag;
    ret->menu = parent->GetComponent<UnityEngine::RectTransform*>();
    ret->line = BSML::Lite::CreateImage(padding, BSML::Utilities::ImageResources::GetWhitePixel());
    auto img = ret->line->rectTransform;
    img->anchorMin = {0.5, 0.5};
    img->anchorMax = {0.5, 0.5};
    img->sizeDelta = {40, 1};
    ret->isLeftMenu = isLeftMenu;
    padding->active = true;
    UI::SetCanvasSorting(padding, 6);
    return ret;
}

void AnimateModal(HMUI::ModalView* modal, bool out) {
    auto bg = modal->transform->Find("BG")->GetComponent<UnityEngine::UI::Image*>();
    auto canvas = modal->GetComponent<UnityEngine::CanvasGroup*>();

    if (out) {
        bg->color = {0.2, 0.2, 0.2, 1};
        canvas->alpha = 0.9;
    } else {
        bg->color = {1, 1, 1, 1};
        canvas->alpha = 1;
    }
}

void Utils::RebuildWithScrollPosition(UnityEngine::GameObject* scrollView) {
    auto scrollComponent = GetScrollViewTop(scrollView)->GetComponent<HMUI::ScrollView*>();
    auto scroll = scrollComponent->position;
    // ew
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(scrollComponent->_contentRectTransform);
    UnityEngine::UI::LayoutRebuilder::ForceRebuildLayoutImmediate(scrollComponent->_contentRectTransform);
    scrollComponent->UpdateContentSize();
    scrollComponent->ScrollTo(std::min(scroll, scrollComponent->scrollableSize), false);
}

UnityEngine::RectTransform* Utils::GetScrollViewTop(UnityEngine::GameObject* scrollView) {
    return scrollView->transform->parent->parent->parent->GetComponent<UnityEngine::RectTransform*>();
}
