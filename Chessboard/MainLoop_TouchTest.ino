
#include "Chessboard.h"
#include "DrawTouch.h"

//
// Debug mode to test the touchscreen calibration
//

void mainLoop_TouchTest() {
    if(bMainModeDirty) {
        DisplayFillScreen(WHITE);
        displayPrint("Touch Test", Vector2(0,20), FreeSans12pt7b, 1, BLACK);
        bMainModeDirty = false;
    }

    if (TouchscreenIsTouched()) {

        Serial.println("TOUCHED");

        TS_Point touchP = TouchscreenGetPoint();
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
        
        Serial.println("11111");
        float r = touchCalibrationTargetRadius;
        Serial.println("22222");
        DisplayDrawLine(p.x, p.y - r, p.x, p.y + r, RED);
        Serial.println("33333");
        DisplayDrawLine(p.x - r, p.y, p.x + r, p.y, RED);
        Serial.println("44444");
    } else {
        //Serial.println("NOT TOUCHED");
    }
}
