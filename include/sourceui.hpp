#pragma once

#include "main.hpp"

#include "UnityEngine/GameObject.hpp"

#include "types.hpp"

namespace Qounters {
    namespace TextSource {
        void StaticUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void ScoreUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void RankUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void PersonalBestUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void ComboUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void MultiplierUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void HealthUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void TimeUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void AverageCutUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void TimeDependenceUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void FailsUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void MistakesUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void NotesUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void PPUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void SaberSpeedUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void SpinometerUI(UnityEngine::GameObject* parent, UnparsedJSON options);

        void CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options);
    }
    namespace ShapeSource {
        void StaticUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void ScoreUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void MultiplierUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void HealthUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void TimeUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void AverageCutUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void NotesUI(UnityEngine::GameObject* parent, UnparsedJSON options);

        void CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options);
    }
    namespace ColorSource {
        void StaticUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void PlayerUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void RankUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void PersonalBestUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void ComboUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void MultiplierUI(UnityEngine::GameObject* parent, UnparsedJSON options);
        void HealthUI(UnityEngine::GameObject* parent, UnparsedJSON options);

        void CreateUI(UnityEngine::GameObject* parent, std::string source, UnparsedJSON options);
    }
}
