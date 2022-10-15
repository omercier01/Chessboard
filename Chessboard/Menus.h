
#pragma once

#include "Chessboard.h"
#include "Util.h"
#include "FixedString.h"

// Colors are R5G6B5

float getLabelWidth(ShortString str, GFXfont font) {
    int totalPixelsX = 0;
    const char* chars = str.c_str();
    int i = 0;
    while(chars[i] != 0) {
        int glyphId = chars[i] - font.first;
        totalPixelsX += font.glyph[glyphId].xAdvance;
        i++;
    }
    return totalPixelsX;
}

// returns the required cursor C position so the text is centered at 0.
float getCenteredLabelPosX(ShortString str, GFXfont font) {
    return -getLabelWidth(str, font)/2;
}

float getRightAlignedLabelPosX(ShortString str, GFXfont font) {
    return -getLabelWidth(str, font);
}


// gets the size of a character for that font.
float getFontHeightChar(GFXfont font, char c) {
    return font.glyph[c - font.first].height;
}


struct MenuLabel {
    MenuLabel() : MenuLabel("", Vector2(0,0), FreeSans12pt7b, 0, 0) {};

    MenuLabel(ShortString text,
              Vector2 pos,
              GFXfont font,
              int textSize,
              int color) :
              text(text),
              pos(pos),
              font(font),
              textSize(textSize),
              textColor(color) {}

    void draw();
    void centerX(int iDivX = 0, int nbDivX = 1);
    void rightAlignX(int iDivX = 0, int nbDivX = 1);

    ShortString text;
    Vector2 pos;
    GFXfont font;
    int textSize;
    uint16_t textColor;
};


struct MenuButton {
    MenuButton() : label(),
                   minPos(0, 0),
                   width(0, 0),
                   onClick(nullptr) {}
    MenuButton(MenuLabel label,
               Vector2 minPos,
               Vector2 width,
               uint16_t color,
               bool fill,
               void (*onClick)()) :
               label(label),
               minPos(minPos),
               width(width),
               color(color),
               fill(fill),
               onClick(onClick) {}

    void draw();

    MenuLabel label;
    Vector2 minPos;
    Vector2 width;
    uint16_t color; // either background or border color, depending on fill
    bool fill = false;
    bool activefill = false;
    void (*onClick)();
};


void onClickKey();

struct Menu {

    void addTitle(ShortString str, uint16_t color);
    void addButton(ShortString str, uint16_t labelColor, uint16_t buttonColor, bool fill, void(*onClick)());
    void draw();
    void onClick(Vector2 pos);
    void centerXTitles();

    void addKey(char c) {
        addButton(ShortString(c), BLACK, BLACK, false, onClickKey);
    }


    Vector2i buttonConfig = Vector2i(1, MAX_NB_MENU_BUTTONS);
    Vector2i buttonConfigSecond = Vector2i(0, 0);

    // uses the menuTopLabelFont font, centered text
    // multiple titles will be shown on consecutive lnes at the top.
    MenuLabel titles[MAX_NB_MENU_TITLES]; 
    int nbTitles = 0;

    MenuButton buttons[MAX_NB_MENU_BUTTONS]; // custom font, centered text
    int nbButtons = 0;

    void (*onBackgroundClick)() = nullptr; // background click, if no buttons report an action on click.
    uint16_t backgroundColor = WHITE;

    GFXfont titleFont = FreeSans12pt7b;
    int titleFontHeight = getFontHeightChar(titleFont, '0');
    int titleTextSize = 1;
    GFXfont buttonFont = FreeSans12pt7b;
    int buttonFontHeight = getFontHeightChar(buttonFont, '0');
    int buttonTextSize = 1;

    void drawTitles();
    void drawButtons();

    void (*extraDrawFunct)(Menu&) = nullptr; // for additional draw stuff, e.g. draw a chess board
    void (*updateTimer)(Menu&) = nullptr;
};

void userMakingMoveMenuClick();
void setMenuGameOptions();
void setMenuReedSwitchTest();
void setMenuFixBadBoard();
void setMenuConnecting();
void setMenuNoGame();
void setMenuTwoPlayers();
void setMenuMoveAndTimer();
void setMenuUserPromotion();
void setMenuGameEndedRatings();
void setMenuGameEndedQrCode();
void setMenuAcceptOrRefuseDraw();
void setMenuAcknowledgeDrawRefusal();
void setMenuAcknowledgeOpponentPromotion();
void setMenuVictoryClaimRefused();
void setMenuGameSeekSent();
void setMenuGameFound();
void setMenuError();
void setMenuKeyboard1();
void setMenuKeyboard2();
void setMenuKeyboard3();
void setMenuKeyboard4();
void clearErrorMessages();