
#include "Chessboard.h"
#include "NetworkLoop.h"
#include "MainLoop.h"
#include "MainLoop_LedTest.h"
#include "LedLoop.h"
#include "Menus.h"
#include "PersistentParam.h"

TFT_eSPI tft;

struct repeating_timer timer;

void setup() {
    
    Serial.begin(115200);

    Serial.println("starting...");
    
#if defined(BOARD_DEF_ESP32)
    pinMode(PIN_MAINMODE_BUTTON, INPUT);
#elif defined(BOARD_DEF_RP2040)
    pinMode(PIN_MAINMODE_BUTTON, INPUT_PULLDOWN);
#endif

    pinMode(PIN_SWITCH_IN_0, INPUT);
    pinMode(PIN_SWITCH_IN_1, INPUT);
    pinMode(PIN_SWITCH_IN_2, INPUT);
    pinMode(PIN_SWITCH_IN_3, INPUT);
    
    pinMode(PIN_SWITCH_QUERY_0, OUTPUT);
    pinMode(PIN_SWITCH_QUERY_1, OUTPUT);
    pinMode(PIN_SWITCH_QUERY_2, OUTPUT);
    pinMode(PIN_SWITCH_QUERY_3, OUTPUT);
    
    pinMode(PIN_LED_POS_0, OUTPUT);
    pinMode(PIN_LED_POS_1, OUTPUT);
    pinMode(PIN_LED_POS_2, OUTPUT);
    pinMode(PIN_LED_POS_3, OUTPUT);
    
    pinMode(PIN_LED_NEG_0, OUTPUT);
    pinMode(PIN_LED_NEG_1, OUTPUT);
    pinMode(PIN_LED_NEG_2, OUTPUT);
    pinMode(PIN_LED_NEG_3, OUTPUT);


    PersistentParamBegin();
    touchCalibrationData.x0 = PersistentParamLoadFloat(PersistentParamType::TouchX0);
    touchCalibrationData.dxdx = PersistentParamLoadFloat(PersistentParamType::TouchDxDx);
    touchCalibrationData.dxdy = PersistentParamLoadFloat(PersistentParamType::TouchDxDy);
    touchCalibrationData.y0 = PersistentParamLoadFloat(PersistentParamType::TouchY0);
    touchCalibrationData.dydx = PersistentParamLoadFloat(PersistentParamType::TouchDyDx);
    touchCalibrationData.dydy = PersistentParamLoadFloat(PersistentParamType::TouchDyDy);
    wifiName = PersistentParamLoadString(PersistentParamType::WifiName).c_str();
    wifiPassword = PersistentParamLoadString(PersistentParamType::WifiPassword).c_str();
    lichessToken = PersistentParamLoadString(PersistentParamType::LichessToken).c_str();
    lichessBoardAccountUsername = PersistentParamLoadString(PersistentParamType::LichessBoardAccountUsername).c_str();
    lichessBoardAccountToken = PersistentParamLoadString(PersistentParamType::LichessBoardAccountToken).c_str();
    twoPlayersTimePerSide = PersistentParamLoadInt(PersistentParamType::TwoPlayersTimePerSide);
    twoPlayersIncrement = PersistentParamLoadInt(PersistentParamType::TwoPlayersIncrement);
    PersistentParamEnd();

    twoPlayersTimePerSide = clamp(twoPlayersTimePerSide,
                                  twoPlayersTimePerSideMin,
                                  twoPlayersTimePerSideMax);

    twoPlayersIncrement = clamp(twoPlayersIncrement,
                                twoPlayersIncrementMin,
                                twoPlayersIncrementMax);


#if defined(BOARD_DEF_ESP32)
    bus = Arduino_ESP32SPI(PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_SCK, PIN_TFT_MOSI, PIN_TFT_MISO);
    pDisplay = new Arduino_ILI9341(&bus, PIN_TFT_RESET);

    pDisplay->begin(displaySpeed);
    pDisplay->setRotation(2);

    pTouch = new XPT2046_Touchscreen(PIN_TFT_TOUCH_CS);
    pTouch->begin();
#elif defined(BOARD_DEF_RP2040)
    tft.init();
    tft.setRotation(2);
#endif

LightLed(-1,-1);
    


#if defined(BOARD_DEF_ESP32)
    // In the main game mode, the network thread does all the work, and the main thread only does display and input/output. In the debug modes, the main thread does all the work.
    xTaskCreatePinnedToCore( networkLoop, "networkLoop", networkTaskStackSize, NULL, 1, &networkTask, 0);

    // Prioritize LEDs loop on main core. Very visible when the LEDs are not smooth, so
    // we want to keep that thread running smoothly if possible.
    xTaskCreatePinnedToCore( ledLoop, "ledLoop", ledTaskStackSize, NULL, 2, &ledTask, BaseType_t(1));
#elif defined(BOARD_DEF_RP2040)
    multicore_launch_core1(networkLoop);
#endif

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            clientBoardBool[i][j] = 0;
            clientBoardBoolTempWrite[i][j] = 0;
            serverBoardPieces[i][j] = 0;
        }
    }

    // document to receive json info from lichess
    jsonDocStream = new DynamicJsonDocument(jsonDocumentAllocSize);

    // not sure how much these are necessary
    httpClientStream->setReuse(false);
    httpClientStream->useHTTP10(true);
    httpClientApi->useHTTP10(true);

    knownServerMoves = new Move[MAX_NB_MOVES+2];

    timeLastDummyMovePost = millis();
    timeLastTimerDrawUpdate = millis();
    timeLastTimerReceived = millis();

    Serial.println("initialized.");
    
    add_repeating_timer_us(1000, ledLoop, NULL, &timer);
}



// main loop runs on core 1 by default
void loop() {
    mainLoop();
}
