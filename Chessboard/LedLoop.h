
#pragma once

#include "Chessboard.h"

void clearActiveLeds();
void addActiveLed(Vector2i newLed);
void addActiveLedSquare(Vector2i square);
void addActiveLedMove(char pieces[8][8], Move move);
void takeActiveLedsMutex();
void releaseActiveLedsMutex();
void LightLed(int x, int y);

#if defined(BOARD_DEF_ESP32)
    void ledLoop(void* parameter);
#elif defined(BOARD_DEF_RP2040)
    bool ledLoop(repeating_timer *t);
#endif

void lightNextActiveLed();