# Qounters++

Add endlessly customizable counters to your ingame UI!

## Custom Qounter Mods

It really wasn't enough for you? Just kidding.

A custom qounter mod can be anywhere from just a template to templates, premade components, sources, and events. Some terminology, first:

- A qounter or group is a set of components that can be moved, rotated, and deleted together.
- A component is an individual shape, image, bit of text, or premade object.
- A premade object is a fully custom object or set of objects that are effectively inseparable into separate components or sources, such as the base game multiplier ring due to its animation.
- A template is some UI that generally has some options and a "Create" button, allowing you to create a group of components with certain layouts and settings much faster than doing it one by one.
- A source is a function that drives either the text, fill level, color, or visibility of a component. It is registered to a set of MetaCore events and reevaluated each time any of the events occurs.

### Templates

To create a custom template, only a single function must be registered, which creates the UI when that premade is clicked, and then configures and adds the premade in response to that UI.

The function takes a `UnityEngine::GameObject*` parameter, which is a vertical layout group intended for you to use as the parent of your UI. For configuring your premade, all the types and names of the default sources are available in `sources.hpp`, and it's recommended to use those types instead of just parsing formatted JSON strings, as it avoids the possibility of typos.

Also note that template UI is shown on a modal, so if you don't use `CreateTemplateFinishButtons`, make sure to call `CloseTemplateModal()` when creating the template, or in a custom cancel button.

```cpp
using namespace Qounters;

void Templates::AddFails(int anchor, UnityEngine::Vector2 pos, bool restarts) {
    // Create the parent group for the premade.
    Options::Group group;
    group.Anchor = anchor;
    group.Position = pos;
    // Create a label for the counter.
    Sources::Text::Static labelText;
    labelText.Input = restarts ? "Restarts" : "Fails";
    Options::Text labelOptions;
    labelOptions.Align = (int) Options::Text::Aligns::Center;
    labelOptions.Size = 11;
    labelOptions.TextSource = Sources::Text::StaticName;
    labelOptions.SourceOptions = labelText;
    auto& label = group.Components.emplace_back();
    label.Position = UnityEngine::Vector2(0, 12.5);
    label.Type = (int) Options::Component::Types::Text;
    label.Options = labelOptions;
    // Create the fails text itself.
    Sources::Text::Fails failsText;
    failsText.Restarts = restarts;
    Options::Text failsOptions;
    failsOptions.Align = (int) Options::Text::Aligns::Center;
    failsOptions.Size = 11;
    failsOptions.TextSource = Sources::Text::FailsName;
    failsOptions.SourceOptions = failsText;
    auto& fails = group.Components.emplace_back();
    fails.Type = (int) Options::Component::Types::Text;
    fails.Options = failsOptions;
    // Add the finished premade to the current preset.
    API::AddGroup(group);
}

void CreateFailsUI(UnityEngine::GameObject* parent) {
    // Only one instance of this UI will ever be shown at a time, so static storage can be used.
    static int anchor = 0;
    static bool restarts = false;

    // Creates a dropdown for the parent anchor standard with the default templates.
    API::CreateTemplateAnchorDropdown(parent, anchor);
    // Adds an option to the template.
    BSML::Lite::AddHoverHint(
        BSML::Lite::CreateToggle(parent, "Count Restarts Instead", restarts, [](bool val) { restarts = val; }),
        "Count the number of consecutive map restarts instead of total fails"
    );
    // Creates "Create" and "Cancel" buttons standard with the default templates.
    API::CreateTemplateFinishButtons(parent, []() { AddFails(anchor, {0, 0}, restarts); });
}

// Registers the template function.
RegisterTemplate("ExtraQounters", "Fails", CreateFailsUI);
```

### Premades

Multiple functions may be necessary to create a custom premade component, depending on the configurability and optimization desired.

The first, and only required one, is to create the component itself. It has two parameters: a `UnityEngine::GameObject*`, which should be the parent of your premade, and a `UnparsedJSON`, which is all the options for your premade. It should return your premade object, which is required to be a `UnityEngine::UI::Graphic*` (to support coloring). The reported size of this graphic also determines the size of the outline and selectable area shown in the settings. This does not mean your premade is required to itself be only a single object - you are free to make any structure of child objects.

The second function is used to update your premade component with new options. Its parameters are a `UnityEngine::UI::Graphic*`, which is naturally your premade object, and a `UnparsedJSON`, which is the new options. If this function is not registered, the premade will be destroyed and recreated every time an option specific to it is changed, which may be undesirable for performance.

Finally, the last function creates the UI for any options specific to your premade, its parameters being a `UnityEngine::GameObject*` as the parent and a `UnparsedJSON` as the current options of the component. The next section has more detail on the contents of UI functions. This should only be skipped if your premade has no options.

Also note that premades should, for now, be registered with a completely unique name, regardless of the mod adding it. This will be improved in the future.

```cpp
using namespace Qounters;

DECLARE_JSON_STRUCT(MultiplierOptions) {
    VALUE_DEFAULT(bool, Animated, true);
};

UnityEngine::UI::Graphic* CreateMultiplier(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    // Since our only value has a default, we don't need to worry about errors when parsing.
    auto options = unparsed.Parse<MultiplierOptions>();
    // Creation omitted due to complexity. See BaseGameGraphic in customtypes/components for an example (albeit with no options).
}

void UpdateMultiplier(UnityEngine::UI::Graphic* parent, UnparsedJSON unparsed) {
    auto options = unparsed.Parse<MultiplierOptions>();
    // Somehow set the animation to be enabled or disabled. This would also need to be done in CreateMultiplier.
}

void CreateMultiplierUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    // Only one instance of this UI will ever be shown at a time, so static storage can be used.
    static MultiplierOptions options;
    options = unparsed.Parse<MultiplierOptions>();

    // See the Sources section for more detail on UI functions.
    BSML::Lite::CreateToggle(parent, "Animated", opts.Animated, [](bool value) {
        static int id = API::GetActionId();
        opts.Animated = value;
        API::SetSourceOptions(id, opts);
        API::FinalizeAction();
    });
}

API::RegisterPremade("ExtraQounters", "Multiplier", CreateMultiplier, CreateMultiplierUI, UpdateMultiplier);
```

### Sources

Sources in Qounters++ require two functions: one to get the current value of the source based on its options, and one to create the UI to change the source's options. It can then be registered to events that determine when the value is recalculated.

The function to get the current value of your source has only one parameter, a `UnparsedJSON` that holds all of your options for the source. It will only be called when new components using the source are created, or when an event the source is registered to is fired, but its result is not cached so don't do anything too crazy. Since it has no parameters other than its options, it's generally required to query state stored elsewhere, and MetaCore can be useful for values related to the game such as score, combo, and more. Its return value depends on the type of your source: for text, `std::string`, for shape, `float`, for color, `UnityEngine::Color`, and for enable, `bool`.

The other function has two parameters: a `UnityEngine::GameObject*`, a vertical layout that should be the parent of your UI, and a `UnparsedJSON`, which is the initial state of your options. Importantly, any UI elements that modify the component require an action ID and finalization when the action is completed. This is for UI elements such as sliders or color pickers, where many changes will be made in a short amount of time, but all need to be grouped together into a single undo. A utility function to add a callback when sliders are released can be found in MetaCore, and a customized color picker with copy/paste support is available in `api.hpp`.

Note that sources also must have a unique string name, even among other custom qounter mods.

```cpp
using namespace Qounters;

DECLARE_JSON_STRUCT(FPSOptions) {
    VALUE_DEFAULT(bool, Label, true);
};

std::string GetFPS(UnparsedJSON unparsed) {
    // Since our only value has a default, we don't need to worry about parsing errors.
    bool label = unparsed.Parse<FPSOptions>().Label;
    // Get the actual FPS text to be displayed. (This is just an example and won't be very accurate.)
    int fps = std::round(1 / UnityEngine::Time::get_deltaTime());
    if (label)
        return fmt::format("FPS: {}", fps);
    return fmt::format("{}", fps);
};

void CreateFPSUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    // Only one instance of this UI will ever be shown at a time, so static storage can be used.
    static FPSOptions opts;
    opts = unparsed.Parse<FPSOptions>();

    BSML::Lite::CreateToggle(parent, "Label", opts.Label, [](bool value) {
        // Get the action ID associated with this toggle.
        static int id = API::GetActionId();
        opts.Label = value;
        // Tell Qounters to store the updated changes and update the component.
        API::SetSourceOptions(id, opts);
        // Finalize the action so that every flip of this toggle will add an "undo".
        API::FinalizeAction();
    });
}

Sources::RegisterText("FPS", GetFPS, CreateFPSUI);
// Make the source update on every song frame.
Events::RegisterToEvent(Types::Sources::Text, "FPS", MetaCore::Events::Update);
```

## Credits

- [danrouse](https://github.com/danrouse) for the original port to quest, Qounters-
- [FutureMapper](https://github.com/Futuremappermydud) for a bunch of help and general encouragement around the Unity update
- [frto027](https://github.com/frto027) for a lot of help with and (questionably voluntary) debugging of the API
