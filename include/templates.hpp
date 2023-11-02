#pragma once

#include "UnityEngine/Vector2.hpp"

#include "types.hpp"

namespace Qounters {
    extern std::vector<std::pair<std::string, TemplateUIFn>> templates;

    namespace Templates {
        void CloseModal();

        void AddEmpty(int anchor, UnityEngine::Vector2 pos);
        void AddScore(int anchor, UnityEngine::Vector2 pos, bool score, bool percent, bool rank, int decimals, bool rankColors);
        void AddPersonalBest(int anchor, UnityEngine::Vector2 pos, bool absolute, bool hideFirst, int decimals);
        void AddAverageCut(int anchor, UnityEngine::Vector2 pos, bool splitSaber, bool splitCut, int decimals);
        void AddTimeDependence(int anchor, UnityEngine::Vector2 pos, bool splitSaber, int decimals, int decimalOffset);
        void AddNotes(int anchor, UnityEngine::Vector2 pos, int display, int decimals);
        void AddMistakes(int anchor, UnityEngine::Vector2 pos, bool badCuts, bool bombs, bool walls);
        void AddFails(int anchor, UnityEngine::Vector2 pos, bool restarts);
        void AddSongTime(int anchor, UnityEngine::Vector2 pos, int display, bool timeLeft);
        void AddPP(int anchor, UnityEngine::Vector2 pos, bool beatleader, bool scoresaber, bool hideUnranked, int decimals);
        void AddSaberSpeed(int anchor, UnityEngine::Vector2 pos, bool split, bool last5Secs, int decimals);
        void AddSpinometer(int anchor, UnityEngine::Vector2 pos, bool split, bool highest);

        void EmptyUI(UnityEngine::GameObject* parent);
        void ScoreUI(UnityEngine::GameObject* parent);
        void PersonalBestUI(UnityEngine::GameObject* parent);
        void AverageCutUI(UnityEngine::GameObject* parent);
        void TimeDependenceUI(UnityEngine::GameObject* parent);
        void NotesUI(UnityEngine::GameObject* parent);
        void MistakesUI(UnityEngine::GameObject* parent);
        void FailsUI(UnityEngine::GameObject* parent);
        void SongTimeUI(UnityEngine::GameObject* parent);
        void PPUI(UnityEngine::GameObject* parent);
        void SaberSpeedUI(UnityEngine::GameObject* parent);
        void SpinometerUI(UnityEngine::GameObject* parent);
    }
}
