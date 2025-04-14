#pragma once

namespace Qounters::Playtest {
    void Update();
    void SetEnabled(bool enabled);
    void ResetGameControllers();

    void SpawnNote(bool left, bool chain);
    void SpawnWall();
    void SpawnBomb();

    void ResetNotes();
    void ResetAll();

    void SetPersonalBest(float value);
    void SetSongTime(float value);
    void SetPositiveModifiers(float value);
    void SetNegativeModifiers(float value);
    void SetRankedBL(bool value);
    void SetStarsBL(float value);
    void SetRankedSS(bool value);
    void SetStarsSS(float value);

    float GetOverridePBRatio();
}
