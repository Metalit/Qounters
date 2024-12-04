#include "copies.hpp"

#include "customtypes/settings.hpp"
#include "editor.hpp"
#include "main.hpp"

using namespace Qounters;

static std::set<enum Copies::Copy> hasCopy = {};

static Options::Group group;

static Options::Component component;

static ConfigUtils::Vector2 positionPos;
static float positionRot;
static ConfigUtils::Vector2 positionScale;

static int typeType;
static Options::Component::OptionsTypes typeOptions;

static std::string textSource;
static UnparsedJSON textOptions;

static int fillFill;
static std::string fillSource;
static UnparsedJSON fillSourceOptions;
static bool fillInverse;

static std::string colorSource;
static UnparsedJSON colorOptions;

static Options::Gradient gradient;

static std::string enableSource;
static bool enableInvert;
static UnparsedJSON enableOptions;

void Copies::Copy(enum Copy id) {
    hasCopy.emplace(id);

    auto& current = Editor::GetSelectedComponent(-1);

    switch (id) {
        case Position:
            positionPos = current.Position;
            positionRot = current.Rotation;
            positionScale = current.Scale;
            break;
        case Type:
            typeType = current.Type;
            typeOptions = current.Options;
            break;
        case TextSource:
            if (current.Type == (int) Options::Component::Types::Text) {
                if (auto text = current.Options.GetValue<Options::Text>()) {
                    textSource = text->TextSource;
                    textOptions = text->SourceOptions;
                }
            }
            break;
        case Fill:
            if (current.Type == (int) Options::Component::Types::Shape) {
                if (auto fill = current.Options.GetValue<Options::Shape>()) {
                    fillFill = fill->Fill;
                    fillSource = fill->FillSource;
                    fillSourceOptions = fill->SourceOptions;
                    fillInverse = fill->Inverse;
                }
            }
            break;
        case Color:
            colorSource = current.ColorSource;
            colorOptions = current.ColorOptions;
            break;
        case Gradient:
            gradient = current.GradientOptions;
            gradient.Enabled = true;
            break;
        case Enable:
            enableSource = current.EnableSource;
            enableInvert = current.InvertEnable;
            enableOptions = current.EnableOptions;
            break;
        default:
            break;
    }
}

void Copies::Paste(enum Copy id) {
    if (!HasCopy(id))
        return;

    auto& current = Editor::GetSelectedComponent(id);

    switch (id) {
        case Position:
            current.Position = positionPos;
            current.Rotation = positionRot;
            current.Scale = positionScale;
            Editor::UpdatePosition(true);
            OptionsViewController::GetInstance()->UpdateSimpleUI();
            break;
        case Type:
            current.Type = typeType;
            current.Options = typeOptions;
            Editor::UpdateType();
            break;
        case TextSource:
            if (current.Type == (int) Options::Component::Types::Text) {
                if (auto text = current.Options.GetValue<Options::Text>()) {
                    text->TextSource = textSource;
                    text->SourceOptions = textOptions;
                    current.Options = *text;
                    Editor::UpdateOptions();
                    OptionsViewController::GetInstance()->UpdateUI();
                    break;
                }
            }
            return;
        case Fill:
            if (current.Type == (int) Options::Component::Types::Shape) {
                if (auto fill = current.Options.GetValue<Options::Shape>()) {
                    fill->Fill = fillFill;
                    fill->FillSource = fillSource;
                    fill->SourceOptions = fillSourceOptions;
                    fill->Inverse = fillInverse;
                    current.Options = *fill;
                    Editor::UpdateOptions();
                    OptionsViewController::GetInstance()->UpdateUI();
                    break;
                }
            }
            return;
        case Color:
            current.ColorSource = colorSource;
            current.ColorOptions = colorOptions;
            OptionsViewController::GetInstance()->UpdateUI();
            Editor::UpdateColor();
            break;
        case Gradient:
            current.GradientOptions = gradient;
            OptionsViewController::GetInstance()->UpdateSimpleUI();
            Editor::UpdateColor();
            break;
        case Enable:
            current.EnableSource = enableSource;
            current.InvertEnable = enableInvert;
            current.EnableOptions = enableOptions;
            OptionsViewController::GetInstance()->UpdateUI();
            Editor::UpdateEnable();
            break;
        default:
            break;
    }
    Editor::FinalizeAction();
}

bool Copies::HasCopy(enum Copy id) {
    return hasCopy.contains(id);
}
