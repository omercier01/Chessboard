
#include "Chessboard.h"
#include "Util.h"
#include "DrawTouch.h"


void displayPrintUsingPreviousParams(
    ShortString text,
    Vector2 pos
) {
    pDisplay->setCursor(pos.x, pos.y);
    pDisplay->print(text.c_str());
}


void displayPrint(
    ShortString text,
    Vector2 pos,
    const GFXfont& font,
    int textSize,
    uint16_t color
) {
    pDisplay->setFont(&font);
    pDisplay->setTextSize(textSize);
    pDisplay->setTextColor(color);
    displayPrintUsingPreviousParams(text, pos);
}


// check if display is touched, and only return true if it has been some time before the last touch event (to avoid double taps)
bool IsDisplayTouchedTimeBuffer(Vector2& touchPos) {
    static bool prevTouched = false;
    
    bool result;
    bool newTouched = pTouch->touched();

    if (newTouched &&
        !prevTouched &&
        abs(int(timeMs - touchCalibTimePrev)) > touchCalibTimeThreshold)
    {
        touchPos = getCalibratedTouchPos();

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
    pDisplay->fillRect(
        posX - contourThickness,
        posY - contourThickness,
        widthX + 2*contourThickness,
        contourThickness,
        color
    );
    pDisplay->fillRect(
        posX - contourThickness,
        posY + widthY,
        widthX + 2*contourThickness,
        contourThickness,
        color
    );
    pDisplay->fillRect(
        posX - contourThickness,
        posY - contourThickness,
        contourThickness,
        widthX + 2*contourThickness,
        color
    );
    pDisplay->fillRect(
        posX + widthX,
        posY - contourThickness,
        contourThickness,
        widthX + 2*contourThickness,
        color
    );
}
