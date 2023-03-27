
#pragma once

#include "FixedString.h"

enum class PersistentParamType {
    TouchX0,
    TouchDxDx,
    TouchDxDy,
    TouchY0,
    TouchDyDx,
    TouchDyDy,
    WifiName,
    WifiPassword,
    LichessToken,
    LichessBoardAccountUsername,
    LichessBoardAccountToken,
    TwoPlayersTimePerSide,
    TwoPlayersIncrement
};

void PersistentParamBegin();

void PersistentParamSaveFloat(PersistentParamType type, float value);
float PersistentParamLoadFloat(PersistentParamType type);

void PersistentParamSaveString(PersistentParamType type, LongString value);
LongString PersistentParamLoadString(PersistentParamType type);

void PersistentParamSaveInt(PersistentParamType type, int value);
int PersistentParamLoadInt(PersistentParamType type);

void PersistentParamEnd();
