
#include "Chessboard.h"
#include "LedLoop.h"

//
// Debug mode to test the LEDs
//

void mainLoop_LedTest() {
    // init
    if(bMainModeDirty) {
        ledTestId = 0;

        DisplayFillScreen(WHITE);
        displayPrint("Testing LEDs", Vector2(0,20), FreeSans12pt7b, 1, BLACK);
        
        bMainModeDirty = false;
    }

    // how many LEDs to light at once
    const int ledPacketSize = 8;

    // need to slow down, otherwise if we switch too quickly there is cross talk between different states and
    // LED will light up in weird places. Either use a duplicate call per LED with timePerLed > 1,
    // or just use a delay at the end of this function.
    const int timePerLed = 1; 
    
    // how long before we change the LED group (make it a multiple of ledPacketSize gives more uniform lighting)
    const int ledTimePerPacket = ledPacketSize * timePerLed * 50;

    int minLedId = (ledTestId/timePerLed)/ledTimePerPacket;
    int ledId = minLedId + (ledTestId % ledPacketSize);
    
    int x = ledId % 9;
    int y = (ledId / 9) % 9;
    LightLed(x, y);

    const int maxLedId =  9*9*ledTimePerPacket*timePerLed;
    ledTestId = (ledTestId+1) % maxLedId;

    delay(1);
}
