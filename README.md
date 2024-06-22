# Qounters++

Add endlessly customizable counters to your ingame UI!

## Custom Qounter Mods

It really wasn't enough for you? Just kidding.

A custom qounter mod can be anywhere from just a template to templates, sources, and events. Some terminology, first:

- A qounter or group is a set of components that can be moved, rotated, and deleted together.
- A component is an individual shape, image, bit of text, or "base game" object (such as the multiplier due to its animation).
- A template is some UI that generally has some options and a "Create" button, allowing you to create a group of components with certain layouts and settings much faster than doing it one by one.
- A source is a function that drives either the text, fill level, color, or visibility of a component. It is registered to a set of events and reevaluated each time any of the events occurs.
- An event is something that is triggered from a source outside the qounters, such as the score changing. It is represented as a unique numerical ID and can be broadcast at any time.

To make a custom template in your mod, create a function of the signature `void (UnityEngine::GameObject*)`. The object passed in is a vertical layout group for you to use as the parent for all your UI. All the types for the default available sources are available for you to use in your template in `sources.hpp`. To create your template, create a group with your options and use `AddGroup`. Templates are shown on a modal, so when you create the template or if you have a cancel button, make sure to call `CloseTemplateModal()`. Register the function with a title and you're done. Titles can be duplicated but it would likely be confusing to the user.

To make a custom source in your mod, you'll need to create two functions. One takes in an `UnparsedJSON` and should return either a string, float, Color, or bool depending on what type of source it is. The JSON object is the current state of your options, as set from your custom UI. Make sure to catch any potential errors deserializing it. It will be called whenever any of the events it is registered to is called along with when the component is created. A significant number of functions to query the current state of the game are available in `game.hpp`, since events do not pass any arguments into your function.

The other function has the signature `void (UnityEngine::GameObject*, UnparsedJSON)` with the same vertical layout passed in as for a template UI function, but also your current options again for you to use as the initial state of your UI. To make your UI components actually modify the component, you'll need to get a unique ID for the current action with `GetActionID()` (make sure to keep this around, potentially as a static variable), then use that along with your up-to-date options to update the component with `SetSourceOptions` for text and shape sources and `SetColorOptions`/`SetEnabledOptions` for the other sources, then finally use `FinalizeAction` to let the editor know when it should add another undo. The action ID part may seem like unnecessary boilerplate, but it's necessary for realtime updating from components such as color pickers and sliders, where you wouldn't want to have to undo every frame of the drag. A useful function for sliders is also provided, since they have no event for the drag ending themselves.

Register your source to the necessary events (in `events.hpp`) and register those two functions to the correct source type and you're done. Make sure the source has a unique string name, even among other custom qounters.

To make a custom event in your mod, simply use `RegisterCustomEvent()`. Put in your unique mod name and any event ID, and you can then broadcast the name with those parameters again, or use the return value of the function to call the other `BroadcastEvent` overload. You can also broadcast any other events, including those from other mods, if you find it necessary.

Events can be used for another type of integration with `internals.hpp`. The variables in that file represent the state of the game, and you should only change them if you modify that yourself in a way that Qounters++ might not pick up on. Make sure to broadcast any necessary events if you do. I'm not sure where this might be used outside of my replay mod, but it's available.

## Credits

- [danrouse](https://github.com/danrouse) for the original port to quest, Qounters-
- [FutureMapper](https://github.com/Futuremappermydud) for a bunch of help and general encouragement around the Unity update
