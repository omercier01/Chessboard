
#include "Chessboard.h"

#include "Util.h"
#include "MainLoop_LedTest.h"
#include "DrawTouch.h"

//
// Debug mode to test the magnetic switches
//

void digitalWritePinSwitchQuery(int val0, int val1, int val2, int val3) {
    digitalWrite(PIN_SWITCH_QUERY_0, val0);
    digitalWrite(PIN_SWITCH_QUERY_1, val1);
    digitalWrite(PIN_SWITCH_QUERY_2, val2);
    digitalWrite(PIN_SWITCH_QUERY_3, val3);
}


// (0,0) is the A1-A4 group, (7,1) is the H5-H8 group.
void checkSwitchQuad(bool outSquares[4], int x, int y) {

    if(y == 0) {
        // w.r.t. schematics for 74LS138
        // A is PIN_SWITCH_QUERY_1
        // B is PIN_SWITCH_QUERY_2
        // C is PIN_SWITCH_QUERY_3
        switch(x) {
            case 0:
                // 74LS138 output 0
                digitalWritePinSwitchQuery(0,0,0,0);
                break;
            case 1:
                // 74LS138 output 1
                digitalWritePinSwitchQuery(0,1,0,0);
                break;
            case 2:
                // 74LS138 output 2
                digitalWritePinSwitchQuery(0,0,1,0);
                break;
            case 3:
                // 74LS138 output 3
                digitalWritePinSwitchQuery(0,1,1,0);
                break;
            case 4:
                // 74LS138 output 4
                digitalWritePinSwitchQuery(0,0,0,1);
                break;
            case 5:
                // 74LS138 output 5
                digitalWritePinSwitchQuery(0,1,0,1);
                break;
            case 6:
                // 74LS138 output 6
                digitalWritePinSwitchQuery(0,0,1,1);
                break;
            case 7:
                // 74LS138 output 7
                digitalWritePinSwitchQuery(0,1,1,1);
                break;
        }

        // reads 0 if switch is closed
        delay(switchDelayAfterQueryMs);
        outSquares[0] = !digitalRead(PIN_SWITCH_IN_0);
        outSquares[1] = !digitalRead(PIN_SWITCH_IN_1);
        outSquares[2] = !digitalRead(PIN_SWITCH_IN_2);
        outSquares[3] = !digitalRead(PIN_SWITCH_IN_3);
    } else {
        // w.r.t. schematics for 74LS138
        // A is PIN_SWITCH_QUERY_3
        // B is PIN_SWITCH_QUERY_2
        // C is PIN_SWITCH_QUERY_1
        switch(x) {
            case 0:
                // 74LS138 output 0
                digitalWritePinSwitchQuery(1,0,0,0);
                break;
            case 1:
                // 74LS138 output 1
                digitalWritePinSwitchQuery(1,0,0,1);
                break;
            case 2:
                // 74LS138 output 2
                digitalWritePinSwitchQuery(1,0,1,0);
                break;
            case 3:
                // 74LS138 output 3
                digitalWritePinSwitchQuery(1,0,1,1);
                break;
            case 4:
                // 74LS138 output 4
                digitalWritePinSwitchQuery(1,1,0,0);
                break;
            case 5:
                // 74LS138 output 5
                digitalWritePinSwitchQuery(1,1,0,1);
                break;
            case 6:
                // 74LS138 output 6
                digitalWritePinSwitchQuery(1,1,1,0);
                break;
            case 7:
                // 74LS138 output 7
                digitalWritePinSwitchQuery(1,1,1,1);
                break;
        }

        // reads 0 if switch is closed
        delay(switchDelayAfterQueryMs);
        outSquares[0] = !digitalRead(PIN_SWITCH_IN_3);
        outSquares[1] = !digitalRead(PIN_SWITCH_IN_2);
        outSquares[2] = !digitalRead(PIN_SWITCH_IN_1);
        outSquares[3] = !digitalRead(PIN_SWITCH_IN_0);
    }
}


void checkAllSwitches(bool outSquares[8][8]) {
    for(int x = 0; x < 8; x++) {
        for(int y = 0; y < 2; y++) {
            bool squareQuad[4];
            checkSwitchQuad(squareQuad, x, y);

            int yOffset = (y == 0 ? 0 : 4);
            for(int i = 0; i < 4; i++) {
                outSquares[x][i+yOffset] = squareQuad[i];
            }
        }
    }
}


void mainLoop_ReedSwitchTest() {
    // init
    if(bMainModeDirty) {
        LightLed(-1, -1);
        setMenuReedSwitchTest();
        currentMenu.draw();
        
        bMainModeDirty = false;
    }

    bool newSquares[8][8];
    checkAllSwitches(newSquares);

    bool squaresUpdated = false;
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(clientBoardBool[i][j] != newSquares[i][j]) {
                squaresUpdated = true;
            }
            clientBoardBool[i][j] = newSquares[i][j];
        }
    }

    if(squaresUpdated && currentMenu.extraDrawFunct) { 
        currentMenu.extraDrawFunct(currentMenu);
    }
    
}
