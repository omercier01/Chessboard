
#include "Chessboard.h"
#include "NetworkLoop.h"
#include "MainLoop.h"
#include "MainLoop_LedTest.h"
#include "LedLoop.h"
#include "Menus.h"

void setup() {
    
    Serial.begin(115200);

    Serial.println("starting...");
    
    pinMode(PIN_MAINMODE_BUTTON, INPUT);

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


    preferences.begin("Chessboard", false);
    touchCalibrationData.x0 = preferences.getFloat("touch_x0", 0.f);
    touchCalibrationData.dxdx = preferences.getFloat("touch_dxdx", 0.f);
    touchCalibrationData.dxdy = preferences.getFloat("touch_dxdy", 1.f);
    touchCalibrationData.y0 = preferences.getFloat("touch_y0", 0.f);
    touchCalibrationData.dydx = preferences.getFloat("touch_dydx", 0.f);
    touchCalibrationData.dydy = preferences.getFloat("touch_dydy", 1.f);
    wifiName = preferences.getString("wifi_name", "").c_str();
    wifiPassword = preferences.getString("wifi_password", "").c_str();
    lichessToken = preferences.getString("lichess_token", "").c_str();
    lichessBoardAccountUsername = preferences.getString("lich_brd_name", "").c_str();
    lichessBoardAccountToken = preferences.getString("lich_brd_tok", "").c_str();
    twoPlayersTimePerSide = preferences.getInt("twop_time", 10);
    twoPlayersIncrement = preferences.getInt("twop_inc", 10);
    preferences.end();


    bus = Arduino_ESP32SPI(PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_SCK, PIN_TFT_MOSI, PIN_TFT_MISO);
    pDisplay = new Arduino_ILI9341(&bus, PIN_TFT_RESET);
    pDisplay->begin(displaySpeed);
    pDisplay->setRotation(2);



    pTouch = new XPT2046_Touchscreen(PIN_TFT_TOUCH_CS);
    pTouch->begin();


    // In the main game mode, the network thread does all the work, and the main thread only does display and input/output. In the debug modes, the main thread does all the work.
    xTaskCreatePinnedToCore( networkLoop, "networkLoop", networkTaskStackSize, NULL, 1, &networkTask, 0);

    // Prioritize LEDs loop on main core. Very visible when the LEDs are not smooth, so
    // we want to keep that thread running smoothly if possible.
    xTaskCreatePinnedToCore( ledLoop, "ledLoop", ledTaskStackSize, NULL, 2, &ledTask, BaseType_t(1));


    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            clientBoardBool[i][j] = 0;
            clientBoardBoolTempWrite[i][j] = 0;
            //clientBoardPieces[i][j] = 0;
            serverBoardPieces[i][j] = 0;
        }
    }

    LightLed(-1,-1);

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
}

// main loop runs on core 1 by default
void loop() {
    mainLoop();
}
