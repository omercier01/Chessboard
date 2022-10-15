
#pragma once

#include "Chessboard.h"

void clearActiveLeds();
void addActiveLed(Vector2i newLed);
void addActiveLedSquare(Vector2i square);
void addActiveLedMove(char pieces[8][8], Move move);
void takeActiveLedsMutex();
void releaseActiveLedsMutex();
void LightLed(int x, int y);

void ledLoop(void* parameter);
void lightNextActiveLed();