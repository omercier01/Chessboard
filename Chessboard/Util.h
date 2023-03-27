
#pragma once

#include "FixedString.h"


#if defined(BOARD_DEF_RP2040)
    // copied from library XPT2046_Touchscreen/XPT2046_Touchscreen.h
    class TS_Point {
    public:
        TS_Point(void) : x(0), y(0), z(0) {}
        TS_Point(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) {}
        bool operator==(TS_Point p) { return ((p.x == x) && (p.y == y) && (p.z == z)); }
        bool operator!=(TS_Point p) { return ((p.x != x) || (p.y != y) || (p.z != z)); }
        int16_t x, y, z;
    };
#endif

bool TouchscreenIsTouched();
TS_Point TouchscreenGetPoint();

void DisplayFillScreen(uint16_t color);
void DisplayFillRect(int posX, int posY, int widthX, int widthY, uint16_t color);
void DisplayDrawRect(int posX, int posY, int widthX, int widthY, uint16_t color);
void DisplayFillCircle(int32_t x, int32_t y, int32_t r, uint32_t color);
void DisplaySetFont(const GFXfont *font);
void DisplaySetTextSize(uint8_t size);
void DisplaySetTextColor(uint16_t color);
void DisplaySetCursor(int16_t x, int16_t y);
void DisplayPrintString(const char* text);
void DisplayPrintChar(char text);
void DisplayDrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);



void drawRectContour(int posX, int posY, int widthX, int widthY, int contourThickness, uint16_t color);

struct Vector2 {
    Vector2(float x, float y) {
        this->x = x;
        this->y = y;
    }
    Vector2() {
        x = 0;
        y = 0;
    }
    
    float x;
    float y;
};


struct Vector2i {
    Vector2i(int x, int y) {
        this->x = x;
        this->y = y;
    }
    Vector2i() {
        x = 0;
        y = 0;
    }
    
    bool sameAs(Vector2i other) {
        return (x == other.x) && (y == other.y);
    }
    
    bool isValid() {
        return x >= 0 && x <= 7 && y >= 0 && y <= 7;
    }

    int x;
    int y;
};

void displayPrintUsingPreviousParams(
    ShortString text,
    Vector2 pos
);
bool IsDisplayTouchedTimeBuffer(Vector2& touchPos, bool useCalibration);



void displayPrint(
    ShortString text,
    Vector2 pos,
    const GFXfont& font,
    int textSize,
    uint16_t color
);

struct Move {

    // 1 in last char indicates that we want to make a promotion but
    // haven't selected the pieces to promote to yet.
    char chars[5] = {0,0,0,0,0};

    bool hasPromotion() {
        return chars[4] != 0;
    }

    ShortString toString() {
        // null terminated
        char charStr[6] = {chars[0], chars[1], chars[2], chars[3], chars[4], 0};
        return ShortString(charStr);
    }

    bool isValid() {
        bool ret = true;
        for(int i = 0; i < 4; i++) {
            if(chars[i] == 0) {
                ret = false;
                break;
            }
        }
        return ret;
    }


    Vector2i getFrom() {
            Vector2i from(chars[0]-'a', chars[1]-'1');
            return from;
    }

    Vector2i getTo() {
        Vector2i to(chars[2]-'a', chars[3]-'1');
        return to;
    }
};


Move toMove(ShortString str) {
    Move ret;
    
    int length = str.length();
    for(int i = 0; i < length; i++) {
        ret.chars[i] = str.charAt(i);
    }
    
    return ret;
}


// Using dynamic heap String here because string can be very large
int tokenizeMoves(String str, char delimiter, Move* tokens, int maxNbTokens) {
    int iToken = 0;
    while(true) {
        int idx = str.indexOf(delimiter);
        if(idx == -1) {
            if(str.length() != 0) {
                if(iToken < maxNbTokens) {
                    tokens[iToken] = toMove(ShortString(str.c_str()));
                    iToken++;
                } else {
                    return -1;
                }
            }
            break;
        }
        int length = str.length();
        ShortString token;
        token = str.substring(0, idx).c_str();
        str = str.substring(idx+1, length);
        if(iToken < maxNbTokens) {
            tokens[iToken] = toMove(token);
            iToken++;
        } else {
            return -1;
        }
    }

    return iToken;
}


char toUpper(char letter) {
    return letter & ~(0x20);
}


bool isWithinRect(Vector2 touchPos, Vector2 posMin, Vector2 width) {
    return touchPos.x >= posMin.x &&
           touchPos.x <= posMin.x + width.x &&
           touchPos.y >= posMin.y &&
           touchPos.y <= posMin.y + width.y;
}