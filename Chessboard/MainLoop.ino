
#include "Chessboard.h"

#include "MainLoop_Game.h"
#include "MainLoop_LedTest.h"
#include "mainLoop_ReedSwitchTest.h"
#include "mainLoop_TouchTest.h"
#include "mainLoop_TouchCalibration.h"


void mainLoop() {

    timeMs = millis();
    
    Vector2 touchPos;

    mainModeButtonPressed = digitalRead(PIN_MAINMODE_BUTTON);
    if(
        mainModeButtonPressed && !prevMainModeButtonPressed &&
        abs(int(timeMs - mainModeButtonPressedTimePrev)) > mainModeButtonPressedTimeThreshold) // to avoid the button falsely reporting quick button pressed
    {
        mainModeButtonPressedTimePrev = timeMs;
        mainMode = MainMode((int(mainMode) + 1) % int(MainMode::Count));
        bMainModeDirty = true;
        Serial.print("button\n");

    }
    prevMainModeButtonPressed = mainModeButtonPressed;

    switch(mainMode) {
    case MainMode::Game:
        mainLoop_Game();
        break;
    case MainMode::WifiName:
    case MainMode::WifiPassword:
    case MainMode::LichessToken:
    case MainMode::LichessBoardAccountUsername:
    case MainMode::LichessBoardAccountToken:
        mainLoop_SetText();
        break;
    case MainMode::LedTest:
        mainLoop_LedTest();
        break;
    case MainMode::ReedSwitchTest:
        mainLoop_ReedSwitchTest();
        break;
    case MainMode::TouchTest:
        mainLoop_TouchTest();
        break;
    case MainMode::TouchCalibration:
        mainLoop_TouchCalibration();
        break;
    case MainMode::Count:
    default:
        // nothing
        break;
    }

    vTaskDelay(1); // so other threads can run
    
}
