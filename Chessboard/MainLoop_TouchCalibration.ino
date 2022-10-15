
#include "Chessboard.h"

//
// debug mode to calibrate the touchscreen
//

void mainLoop_TouchCalibration() {
    if(bMainModeDirty) {
        touchCalibrationTargetId = 0;
        bTouchCalibrationTargetIdDirty = true;
        bMainModeDirty = false;
    }

    if(bTouchCalibrationTargetIdDirty) {
        pDisplay->fillScreen(WHITE);
        displayPrint("Touch Calibration", Vector2(0,20), FreeSans12pt7b, 1, BLACK);

        Vector2 p = calibTargetPts[touchCalibrationTargetId];
        float r = touchCalibrationTargetRadius;
        pDisplay->drawLine(p.x, p.y - r, p.x, p.y + r, RED);
        pDisplay->drawLine(p.x - r, p.y, p.x + r, p.y, RED);

        bTouchCalibrationTargetIdDirty = false;
    }

    Vector2 touchPos;
    if(IsDisplayTouchedTimeBuffer(touchPos)) {
        calibUserPts[touchCalibrationTargetId].x = touchPos.x;
        calibUserPts[touchCalibrationTargetId].y = touchPos.y;

        touchCalibrationTargetId++;
        bTouchCalibrationTargetIdDirty = true;
    }

    if(touchCalibrationTargetId == nbTouchCalibTargets) {
        // Compute calibration
        Vector2* p = calibUserPts;
        Vector2 const* t = calibTargetPts;
        float invDet = 1.f/( p[0].x*p[1].y - p[0].x*p[2].y - p[1].x*p[0].y + p[1].x*p[2].y + p[2].x*p[0].y - p[2].x*p[1].y );
        float invMat[3][3] = {{p[1].x*p[2].y - p[2].x*p[1].y, p[2].x*p[0].y - p[0].x*p[2].y, p[0].x*p[1].y - p[1].x*p[0].y},
                              {p[1].y - p[2].y,               p[2].y - p[0].y,               p[0].y - p[1].y},
                              {p[2].x - p[1].x,               p[0].x - p[2].x,               p[1].x - p[0].x}}; // [row][col]

        TouchCalibrationData& tc = touchCalibrationData;
        tc.x0 =   invDet * (invMat[0][0] * t[0].x + invMat[0][1] * t[1].x + invMat[0][2] * t[2].x);
        tc.dxdx = invDet * (invMat[1][0] * t[0].x + invMat[1][1] * t[1].x + invMat[1][2] * t[2].x);
        tc.dxdy = invDet * (invMat[2][0] * t[0].x + invMat[2][1] * t[1].x + invMat[2][2] * t[2].x);
        tc.y0 =   invDet * (invMat[0][0] * t[0].y + invMat[0][1] * t[1].y + invMat[0][2] * t[2].y);
        tc.dydx = invDet * (invMat[1][0] * t[0].y + invMat[1][1] * t[1].y + invMat[1][2] * t[2].y);
        tc.dydy = invDet * (invMat[2][0] * t[0].y + invMat[2][1] * t[1].y + invMat[2][2] * t[2].y);
        
        preferences.begin("Chessboard", false);
        preferences.putFloat("touch_x0", tc.x0);
        preferences.putFloat("touch_dxdx", tc.dxdx);
        preferences.putFloat("touch_dxdy", tc.dxdy);
        preferences.putFloat("touch_y0", tc.y0);
        preferences.putFloat("touch_dydx", tc.dydx);
        preferences.putFloat("touch_dydy", tc.dydy);
        preferences.end();
        
        mainMode = MainMode::TouchTest;
        bMainModeDirty = true;
    }
}
