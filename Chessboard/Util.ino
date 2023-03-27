
#include "Chessboard.h"
#include "Util.h"
#include "DrawTouch.h"

#if defined(BOARD_DEF_ESP32)
    bool TouchscreenIsTouched() {
        return pTouch->touched();
    }

    TS_Point TouchscreenGetPoint() {
        pTouch->getPoint();
    }

    void DisplayFillScreen(uint16_t color) {
        pDisplay->fillScreen(color);
    }

    void DisplayFillRect(int posX, int posY, int widthX, int widthY, uint16_t color) {
        pDisplay->fillRect(posX, posY, widthX, widthY, color);
    }

    void DisplayDrawRect(int posX, int posY, int widthX, int widthY, uint16_t color) {
        pDisplay->drawRect(posX, posY, widthX, widthY, color);
    }

    void DisplayFillCircle(int32_t x, int32_t y, int32_t r, uint32_t color) {
        pDisplay->fillCircle(x, y, r, color);
    }

    void DisplaySetFont(const GFXfont *font) {
        pDisplay->setFont(&font);
    }

    void DisplaySetTextSize(uint8_t size) {
        pDisplay->setTextSize(textSize);
    }

    void DisplaySetTextColor(uint16_t color) {
        pDisplay->setTextColor(color);
    }

    void DisplaySetCursor(int16_t x, int16_t y) {
        pDisplay->setCursor(pos.x, pos.y);
    }

    void DisplayPrintString(const char* text) {
        pDisplay->print(text);
    }

    void DisplayPrintChar(char c) {
        pDisplay->print(c);
    }

    void DisplayDrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
        pDisplay->drawLine(x0, y0, x1, y1, color);
    }

#elif defined(BOARD_DEF_RP2040)

    bool TouchscreenIsTouched() {
        uint16_t x = 0, y = 0;
        return tft.getTouch(&x, &y);
    }
    TS_Point TouchscreenGetPoint() {
        uint16_t x = 0, y = 0;
        tft.getTouch(&x, &y);
        return TS_Point(x,y,0);
    }

    void DisplayFillScreen(uint16_t color) {
        tft.fillScreen(color);
    }

    void DisplayFillRect(int posX, int posY, int widthX, int widthY, uint16_t color) {
        tft.fillRect(posX, posY, widthX, widthY, color);
    }

    void DisplayDrawRect(int posX, int posY, int widthX, int widthY, uint16_t color) {
        tft.drawRect(posX, posY, widthX, widthY, color);
    }

    void DisplayFillCircle(int32_t x, int32_t y, int32_t r, uint32_t color) {
        tft.fillCircle(x, y, r, color);
    }

    void DisplaySetFont(const GFXfont *font) {
        tft.setFreeFont(font);
    }

    void DisplaySetTextSize(uint8_t size) {
        tft.setTextSize(size);
    }

    void DisplaySetTextColor(uint16_t color) {
        tft.setTextColor(color);
    }

    void DisplaySetCursor(int16_t x, int16_t y) {
        tft.setCursor(x, y);
    }

    void DisplayPrintString(const char* text) {
        tft.print(text);
    }

    void DisplayPrintChar(char c) {
        tft.print(c);
    }

    void DisplayDrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
        tft.drawLine(x0, y0, x1, y1, color);
    }

#endif


void displayPrintUsingPreviousParams(
    ShortString text,
    Vector2 pos
) {
    DisplaySetCursor(pos.x, pos.y);
    DisplayPrintString(text.c_str());
}


void displayPrint(
    ShortString text,
    Vector2 pos,
    const GFXfont& font,
    int textSize,
    uint16_t color
) {
    DisplaySetFont(&font);
    DisplaySetTextSize(textSize);
    DisplaySetTextColor(color);
    displayPrintUsingPreviousParams(text, pos);
}


// check if display is touched, and only return true if it has been some time before the last touch event (to avoid double taps)
bool IsDisplayTouchedTimeBuffer(Vector2& touchPos, bool useCalibration) {
    static bool prevTouched = false;
    
    bool result;
    bool newTouched = TouchscreenIsTouched();

    if (newTouched &&
        !prevTouched &&
        abs(int(timeMs - touchCalibTimePrev)) > touchCalibTimeThreshold)
    {
        if(useCalibration) {
            touchPos = getCalibratedTouchPos();
        } else {
            TS_Point pt = TouchscreenGetPoint();
            touchPos = Vector2(pt.x, pt.y);
        }

        touchCalibTimePrev = timeMs;

        result = true;
    } else {
        result = false;
    }
    prevTouched = newTouched;
    return result;
}


// draws a thick rectangle around the given pos/width rectangle.
void drawRectContour(int posX, int posY, int widthX, int widthY, int contourThickness, uint16_t color) {
    DisplayFillRect(
        posX - contourThickness,
        posY - contourThickness,
        widthX + 2*contourThickness,
        contourThickness,
        color
    );
    DisplayFillRect(
        posX - contourThickness,
        posY + widthY,
        widthX + 2*contourThickness,
        contourThickness,
        color
    );
    DisplayFillRect(
        posX - contourThickness,
        posY - contourThickness,
        contourThickness,
        widthX + 2*contourThickness,
        color
    );
    DisplayFillRect(
        posX + widthX,
        posY - contourThickness,
        contourThickness,
        widthX + 2*contourThickness,
        color
    );
}
