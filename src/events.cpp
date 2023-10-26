#include "events.hpp"
#include "config.hpp"
#include "qounters.hpp"

namespace Qounters {
    std::map<int, std::vector<std::pair<Sources, std::string>>> eventSourceRegistry = {
        {(int) Events::ScoreChanged, {
            {Sources::Text, "Score"},
            {Sources::Text, "Rank"},
            {Sources::Text, "PP"},
            {Sources::Shape, "Score"},
            {Sources::Color, "Rank"},
            {Sources::Color, "Personal Best"},
        }},
        {(int) Events::NoteCut, {
            {Sources::Text, "Average Cut"},
            {Sources::Text, "Mistakes"},
            {Sources::Text, "Notes"},
            {Sources::Shape, "Average Cut"},
            {Sources::Shape, "Notes"},
        }},
        {(int) Events::NoteMissed, {
            {Sources::Text, "Mistakes"},
            {Sources::Text, "Notes"},
            {Sources::Shape, "Notes"},
        }},
        {(int) Events::BombCut, {
            {Sources::Text, "Mistakes"},
            {Sources::Text, "Notes"},
            {Sources::Shape, "Notes"},
        }},
        {(int) Events::WallHit, {
            {Sources::Text, "Mistakes"},
        }},
        {(int) Events::ComboChanged, {
            {Sources::Text, "Combo"},
            {Sources::Text, "Multiplier"},
            {Sources::Shape, "Multiplier"},
            {Sources::Color, "Combo"},
            {Sources::Color, "Multiplier"},
        }},
        {(int) Events::HealthChanged, {
            {Sources::Text, "Health"},
            {Sources::Text, "Fails"},
            {Sources::Shape, "Health"},
            {Sources::Color, "Health"},
        }},
        {(int) Events::Update, {
            {Sources::Text, "Time"},
            {Sources::Shape, "Time"},
        }},
        {(int) Events::SlowUpdate, {
            {Sources::Text, "Saber Speed"},
        }},
        {(int) Events::PPInfo, {
            {Sources::Text, "PP"},
        }},
    };

    std::map<std::string, std::map<int, int>> customEvents = {};

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

    void RegisterToEvent(Sources sourceType, std::string source, int event) {
        if (!eventSourceRegistry.contains(event))
            eventSourceRegistry[event] = {};
        eventSourceRegistry[event].emplace_back(sourceType, source);
    }

    void BroadcastEvent(int event) {
        if (!eventSourceRegistry.contains(event))
            return;
        for (auto& [sourceType, source] : eventSourceRegistry[event])
            UpdateSource(sourceType, source);
    }

    void BroadcastEvent(std::string mod, int event) {
        if (!customEvents.contains(mod))
            return;
        if (!customEvents[mod].contains(event))
            return;
        BroadcastEvent(customEvents[mod][event]);
    }
}
