
#include "Chessboard.h"
#include "DrawTouch.h"

//
// Debug mode to test the touchscreen calibration
//

void mainLoop_TouchTest() {
    if(bMainModeDirty) {
        pDisplay->fillScreen(WHITE);
        displayPrint("Touch Test", Vector2(0,20), FreeSans12pt7b, 1, BLACK);
        bMainModeDirty = false;
    }

    if (pTouch->touched()) {

        TS_Point touchP = pTouch->getPoint();
        Vector2 p = getCalibratedTouchPos();

        Serial.print("( ");
        Serial.print(touchP.x);
        Serial.print(" ),( ");
        Serial.print(touchP.y);
        Serial.print(" )  ->  ( ");
        Serial.print(p.x);
        Serial.print(" ),( ");
        Serial.print(p.y);
        Serial.print(" )\n");
        
        float r = touchCalibrationTargetRadius;
        pDisplay->drawLine(p.x, p.y - r, p.x, p.y + r, RED);
        pDisplay->drawLine(p.x - r, p.y, p.x + r, p.y, RED);
    }
}
