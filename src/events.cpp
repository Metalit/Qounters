#include "events.hpp"

#include "config.hpp"
#include "qounters.hpp"
#include "sources.hpp"

using namespace Qounters;

static std::map<int, std::vector<std::pair<Types::Sources, std::string>>> eventSourceRegistry = {
    {(int) Events::ScoreChanged,
     {
         {Types::Sources::Text, Sources::Text::ScoreName},
         {Types::Sources::Text, Sources::Text::RankName},
         {Types::Sources::Text, Sources::Text::PPName},
         {Types::Sources::Text, Sources::Text::FCPercentName},
         {Types::Sources::Shape, Sources::Shape::ScoreName},
         {Types::Sources::Color, Sources::Color::RankName},
         {Types::Sources::Color, Sources::Color::PersonalBestName},
         {Types::Sources::Enable, Sources::Enable::PercentageName},
     }},
    {(int) Events::NoteCut,
     {
         {Types::Sources::Text, Sources::Text::AverageCutName},
         {Types::Sources::Text, Sources::Text::TimeDependenceName},
         {Types::Sources::Text, Sources::Text::MistakesName},
         {Types::Sources::Text, Sources::Text::NotesName},
         {Types::Sources::Text, Sources::Text::FCPercentName},
         {Types::Sources::Shape, Sources::Shape::AverageCutName},
         {Types::Sources::Shape, Sources::Shape::NotesName},
     }},
    {(int) Events::NoteMissed,
     {
         {Types::Sources::Text, Sources::Text::MistakesName},
         {Types::Sources::Text, Sources::Text::NotesName},
         {Types::Sources::Shape, Sources::Shape::NotesName},
     }},
    {(int) Events::BombCut,
     {
         {Types::Sources::Text, Sources::Text::MistakesName},
         {Types::Sources::Text, Sources::Text::NotesName},
         {Types::Sources::Shape, Sources::Shape::NotesName},
     }},
    {(int) Events::WallHit,
     {
         {Types::Sources::Text, Sources::Text::MistakesName},
     }},
    {(int) Events::ComboChanged,
     {
         {Types::Sources::Text, Sources::Text::ComboName},
         {Types::Sources::Text, Sources::Text::MultiplierName},
         {Types::Sources::Shape, Sources::Shape::MultiplierName},
         {Types::Sources::Color, Sources::Color::ComboName},
         {Types::Sources::Color, Sources::Color::MultiplierName},
         {Types::Sources::Enable, Sources::Enable::FullComboName},
     }},
    {(int) Events::HealthChanged,
     {
         {Types::Sources::Text, Sources::Text::HealthName},
         {Types::Sources::Text, Sources::Text::FailsName},
         {Types::Sources::Shape, Sources::Shape::HealthName},
         {Types::Sources::Color, Sources::Color::HealthName},
         {Types::Sources::Enable, Sources::Enable::FailedName},
     }},
    {(int) Events::Update,
     {
         {Types::Sources::Text, Sources::Text::TimeName},
         {Types::Sources::Shape, Sources::Shape::TimeName},
     }},
    {(int) Events::SlowUpdate,
     {
         {Types::Sources::Text, Sources::Text::SaberSpeedName},
         {Types::Sources::Text, Sources::Text::SpinometerName},
     }},
    {(int) Events::MapInfo,
     {
         {Types::Sources::Text, Sources::Text::RankName},
         {Types::Sources::Text, Sources::Text::PersonalBestName},
         {Types::Sources::Text, Sources::Text::NotesName},
         {Types::Sources::Text, Sources::Text::PPName},
         {Types::Sources::Color, Sources::Color::PlayerName},
         {Types::Sources::Color, Sources::Color::PersonalBestName},
         {Types::Sources::Enable, Sources::Enable::RankedName},
     }},
};

static std::map<std::string, std::map<int, int>> customEvents = {};

int RegisterCustomEvent(std::string mod, int event) {
    int maxEvent = (int) Events::EventMax;
    for (auto& [_, idMap] : customEvents) {
        for (auto& [_, realId] : idMap)
            maxEvent = std::max(realId, maxEvent);
    }
    if (!customEvents.contains(mod))
        customEvents[mod] = {};
    customEvents[mod][event] = maxEvent + 1;
    return maxEvent + 1;
}

void RegisterToEvent(Types::Sources sourceType, std::string source, int event) {
    if (!eventSourceRegistry.contains(event))
        eventSourceRegistry[event] = {};
    eventSourceRegistry[event].emplace_back(sourceType, source);
}

void Events::Broadcast(int event) {
    if (!eventSourceRegistry.contains(event))
        return;
    for (auto& [sourceType, source] : eventSourceRegistry[event])
        HUD::UpdateSource(sourceType, source);
}

void Events::Broadcast(std::string mod, int event) {
    if (!customEvents.contains(mod))
        return;
    if (!customEvents[mod].contains(event))
        return;
    Broadcast(customEvents[mod][event]);
}
