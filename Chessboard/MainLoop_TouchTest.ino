
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

        Vector2 p = getCalibratedTouchPos();

        float r = touchCalibrationTargetRadius;
        DisplayDrawLine(p.x, p.y - r, p.x, p.y + r, RED);
        DisplayDrawLine(p.x - r, p.y, p.x + r, p.y, RED);
    }
}
