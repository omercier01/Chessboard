
#pragma once

#include "Chessboard.h"

float getPromotionTopBorder();
Vector2 getPromotionZoneMin(int iX, int iY);
Vector2 getPromotionZomeWidth();
void drawTimer();
Vector2 getCalibratedTouchPos();
void drawBoardBool(bool pieces[8][8]);
void displayBoardBoolWithTitle();
void drawBoardChar(char pieces[8][8]);

