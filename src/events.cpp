#include "metacore/shared/events.hpp"

#include "config.hpp"
#include "events.hpp"
#include "qounters.hpp"
#include "sources.hpp"

using namespace Qounters;

static int infoEvent;

static std::map<int, std::vector<std::pair<Types::Sources, std::string>>> eventSourceRegistry = {
    {(int) MetaCore::Events::ScoreChanged,
     {
         {Types::Sources::Text, Sources::Text::ScoreName},
         {Types::Sources::Text, Sources::Text::RankName},
         {Types::Sources::Text, Sources::Text::PPName},
         {Types::Sources::Text, Sources::Text::FCPercentName},
         {Types::Sources::Shape, Sources::Shape::ScoreName},
         {Types::Sources::Color, Sources::Color::RankName},
         {Types::Sources::Color, Sources::Color::PersonalBestName},
         {Types::Sources::Text, Sources::Text::PBGapName},
         {Types::Sources::Enable, Sources::Enable::PercentageName},
     }},
    {(int) MetaCore::Events::NoteCut,
     {
         {Types::Sources::Text, Sources::Text::AverageCutName},
         {Types::Sources::Text, Sources::Text::TimeDependenceName},
         {Types::Sources::Text, Sources::Text::MistakesName},
         {Types::Sources::Text, Sources::Text::NotesName},
         {Types::Sources::Text, Sources::Text::FCPercentName},
         {Types::Sources::Shape, Sources::Shape::AverageCutName},
         {Types::Sources::Shape, Sources::Shape::NotesName},
     }},
    {(int) MetaCore::Events::NoteMissed,
     {
         {Types::Sources::Text, Sources::Text::MistakesName},
         {Types::Sources::Text, Sources::Text::NotesName},
         {Types::Sources::Shape, Sources::Shape::NotesName},
     }},
    {(int) MetaCore::Events::BombCut,
     {
         {Types::Sources::Text, Sources::Text::MistakesName},
         {Types::Sources::Text, Sources::Text::NotesName},
         {Types::Sources::Shape, Sources::Shape::NotesName},
     }},
    {(int) MetaCore::Events::WallHit,
     {
         {Types::Sources::Text, Sources::Text::MistakesName},
     }},
    {(int) MetaCore::Events::ComboChanged,
     {
         {Types::Sources::Text, Sources::Text::ComboName},
         {Types::Sources::Text, Sources::Text::MultiplierName},
         {Types::Sources::Shape, Sources::Shape::MultiplierName},
         {Types::Sources::Color, Sources::Color::ComboName},
         {Types::Sources::Color, Sources::Color::MultiplierName},
         {Types::Sources::Enable, Sources::Enable::FullComboName},
     }},
    {(int) MetaCore::Events::HealthChanged,
     {
         {Types::Sources::Text, Sources::Text::HealthName},
         {Types::Sources::Text, Sources::Text::FailsName},
         {Types::Sources::Shape, Sources::Shape::HealthName},
         {Types::Sources::Color, Sources::Color::HealthName},
         {Types::Sources::Enable, Sources::Enable::FailedName},
     }},
    {(int) MetaCore::Events::Update,
     {
         {Types::Sources::Text, Sources::Text::TimeName},
         {Types::Sources::Shape, Sources::Shape::TimeName},
     }},
    {(int) MetaCore::Events::SlowUpdate,
     {
         {Types::Sources::Text, Sources::Text::SaberSpeedName},
         {Types::Sources::Text, Sources::Text::SpinometerName},
     }},
    {infoEvent,
     {
         {Types::Sources::Text, Sources::Text::PPName},
         {Types::Sources::Text, Sources::Text::PersonalBestName},
         {Types::Sources::Text, Sources::Text::PBGapName},
         {Types::Sources::Text, Sources::Text::NotesName},
         {Types::Sources::Color, Sources::Color::PlayerName},
         {Types::Sources::Color, Sources::Color::PersonalBestName},
         {Types::Sources::Enable, Sources::Enable::RankedName},
     }},
};

void Events::RegisterToEvent(Types::Sources sourceType, std::string source, int event) {
    if (event < 0)
        return;
    if (!eventSourceRegistry.contains(event))
        eventSourceRegistry[event] = {};
    eventSourceRegistry[event].emplace_back(sourceType, source);
}

void Events::RegisterToEvent(Types::Sources sourceType, std::string source, std::string mod, int event) {
    RegisterToEvent(sourceType, source, MetaCore::Events::FindEvent(mod, event));
}

void Events::RegisterToQountersEvent(Types::Sources sourceType, std::string source, std::string mod, int event) {
    RegisterToEvent(sourceType, source, MOD_ID, event);
}

void Events::BroadcastQountersEvent(int event) {
    MetaCore::Events::Broadcast(MOD_ID, event);
}

static void OnEvent(int event) {
    if (!eventSourceRegistry.contains(event))
        return;
    for (auto& [sourceType, source] : eventSourceRegistry[event])
        HUD::UpdateSource(sourceType, source);
}

AUTO_FUNCTION {
    MetaCore::Events::AddCallback(OnEvent);
    // seems like this function happens before eventSourceRegistry is initialized
    infoEvent = MetaCore::Events::RegisterEvent(MOD_ID, Events::MapInfo);
}
