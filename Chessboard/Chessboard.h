
#pragma once

// define only one board to use
//#define BOARD_DEF_ESP32
#define BOARD_DEF_RP2040

#if defined(BOARD_DEF_ESP32)
    #define PIN_TFT_SCK    18 // can't be changed
    #define PIN_TFT_MOSI   23 // can't be changed
    #define PIN_TFT_MISO   19 // can't be changed
    #define PIN_TFT_CS     22 
    #define PIN_TFT_DC     21
    #define PIN_TFT_RESET  17

    #define PIN_TFT_TOUCH_CS 16

    #define PIN_MAINMODE_BUTTON 2

    #define PIN_SWITCH_IN_0 36
    #define PIN_SWITCH_IN_1 39
    #define PIN_SWITCH_IN_2 34
    #define PIN_SWITCH_IN_3 35

    #define PIN_SWITCH_QUERY_0 15
    #define PIN_SWITCH_QUERY_1 0
    #define PIN_SWITCH_QUERY_2 4
    #define PIN_SWITCH_QUERY_3 5

    #define PIN_LED_POS_0 32
    #define PIN_LED_POS_1 33
    #define PIN_LED_POS_2 25
    #define PIN_LED_POS_3 26

    #define PIN_LED_NEG_0 27
    #define PIN_LED_NEG_1 14
    #define PIN_LED_NEG_2 12
    #define PIN_LED_NEG_3 13


#elif defined(BOARD_DEF_RP2040)

    // pins for the display and touch are defined in
    // C:\Users\<user>\Documents\Arduino\libraries\TFT_eSPI\User_Setups\Setup60_RP2040_ILI9341.h
    // or wherever your arduino libraries are. Also uncomment the line
    // #include <User_Setups/Setup60_RP2040_ILI9341.h>
    // in C:\Users\<user>\Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h .
    // The pins used are
    // TFT_MISO  0
    // TFT_MOSI  3
    // TFT_SCLK  2
    // TFT_CS   4
    // TFT_DC   5
    // TFT_RST  6
    // TOUCH_CS 7

    #define PIN_MAINMODE_BUTTON 15

    #define PIN_SWITCH_IN_0 20 // 36
    #define PIN_SWITCH_IN_1 28 // 39
    #define PIN_SWITCH_IN_2 22 // 34
    #define PIN_SWITCH_IN_3 21 // 35

    #define PIN_SWITCH_QUERY_0 14 // 15
    #define PIN_SWITCH_QUERY_1 1 // 0
    #define PIN_SWITCH_QUERY_2 9 // 4
    #define PIN_SWITCH_QUERY_3 8 // 5

    #define PIN_LED_POS_0 19 // 32
    #define PIN_LED_POS_1 17 // 33
    #define PIN_LED_POS_2 18 // 25
    #define PIN_LED_POS_3 16 // 26

    #define PIN_LED_NEG_0 10 // 27
    #define PIN_LED_NEG_1 12 // 14
    #define PIN_LED_NEG_2 11 // 12
    #define PIN_LED_NEG_3 13 // 13
#endif


#define DISPLAY_PIXELS_WIDTH 240
#define DISPLAY_PIXELS_HEIGHT 320

#define MAX_NB_MENU_TITLES 7
#define MAX_NB_MENU_BUTTONS (6*5)
#define MAX_NB_MENU_BUTTONS_SECOND 2

#define MENU_BORDER_WIDTH 10

#define UNLIMITED_TIME 2147483647

#define FONT_PIECE_WHITE_K_BLACKLAYER (40+0)
#define FONT_PIECE_WHITE_K_WHITELAYER (40+1)
#define FONT_PIECE_WHITE_Q_BLACKLAYER (40+2)
#define FONT_PIECE_WHITE_Q_WHITELAYER (40+3)
#define FONT_PIECE_WHITE_R_BLACKLAYER (40+4)
#define FONT_PIECE_WHITE_R_WHITELAYER (40+5)
#define FONT_PIECE_WHITE_B_BLACKLAYER (40+6)
#define FONT_PIECE_WHITE_B_WHITELAYER (40+7)
#define FONT_PIECE_WHITE_N_BLACKLAYER (40+8)
#define FONT_PIECE_WHITE_N_WHITELAYER (40+9)
#define FONT_PIECE_WHITE_P_BLACKLAYER (40+10)
#define FONT_PIECE_WHITE_P_WHITELAYER (40+11)
#define FONT_PIECE_BLACK_K_BLACKLAYER (40+12)
#define FONT_PIECE_BLACK_K_WHITELAYER (40+13)
#define FONT_PIECE_BLACK_Q_BLACKLAYER (40+14)
#define FONT_PIECE_BLACK_Q_WHITELAYER (40+15)
#define FONT_PIECE_BLACK_R_BLACKLAYER (40+16)
#define FONT_PIECE_BLACK_R_WHITELAYER (40+17)
#define FONT_PIECE_BLACK_B_BLACKLAYER (40+18)
#define FONT_PIECE_BLACK_B_WHITELAYER (40+19)
#define FONT_PIECE_BLACK_N_BLACKLAYER (40+20)
#define FONT_PIECE_BLACK_N_WHITELAYER (40+21)
#define FONT_PIECE_BLACK_P_BLACKLAYER (40+22)
#define FONT_PIECE_BLACK_P_WHITELAYER (40+23)

// making this too large creates random problems (e.g. HTTP requests failing because not enough memory)
// ideally would be 6000 to cover possible max moves, but 1400 is plenty
#define MAX_NB_MOVES 1400
//#define MAX_NB_MOVES 100

// sizes for custom fixed-length strings (to avoid allocating too much Strings on the heap)
#define SHORT_STRING_LENGTH 32
#define LONG_STRING_LENGTH 256

#define HTTP_REQUEST_NB_TRIES 4

// milliseconds
#define DELAY_BETWEEN_HTTP_TRIES 200

// json size to account for mazimum number of moves.
// moves without promotion + promotion + other stuff
const int jsonDocumentAllocSize = 5*MAX_NB_MOVES + 1000;

// stack size for different threads
const int networkTaskStackSize = 30000;
const int ledTaskStackSize = 1000;


#if defined(BOARD_DEF_ESP32)
    #include "XPT2046_Touchscreen.h"
    #include <Arduino_GFX_Library.h>

    #include "Fonts/FreeSansBold18pt7b.h"
    #include "Fonts/FreeSansBold24pt7b.h"
    #include "Fonts/FreeSans12pt7b.h"
#elif defined(BOARD_DEF_RP2040)
    #include <SPI.h>
    #include <TFT_eSPI.h>

    #define WHITE TFT_WHITE
    #define BLACK TFT_BLACK
    #define RED TFT_RED
    #define DARKGREEN TFT_DARKGREEN
    #define GREEN TFT_GREEN
#endif


#include <WiFi.h>
#include <HTTPClient.h>
#include <qrcode_renamed.h>


#include "Fonts/ChessFont25.h"

#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 1
#include <ArduinoJson.h>
#include <StreamUtils.h>

#define BUFFERED_FILE_SIZE 64

// tweaks so VSCode stops complaining
#if VISUAL_STUDIO_CODE
    #define configSUPPORT_DYNAMIC_ALLOCATION 1
    #include "freertos/task.h"

    #define CONFIG_IDF_TARGET_ESP32 1
    #include "Arduino_ESP32SPI.h"
#endif

#include "Util.h"
#include "Menus.h"
#include "FixedString.h"



const unsigned long mainModeButtonPressedTimeThreshold = 200; // increase if false presses detected
const float touchCalibrationTargetRadius = 10;
const int nbTouchCalibTargets = 3;
const Vector2 calibTargetPts[nbTouchCalibTargets] = {{50,50}, {200,50}, {200,280}};
const unsigned long touchCalibTimeThreshold = 100;

enum class MainMode {
    Game,
    WifiName,
    WifiPassword,
    LichessToken,
    LichessBoardAccountUsername,
    LichessBoardAccountToken,
    LedTest,
    ReedSwitchTest,
    TouchTest,
    TouchCalibration,
    Count
};
MainMode mainMode = MainMode::Game;
bool bMainModeDirty = true;
bool mainModeButtonPressed = false;
bool prevMainModeButtonPressed = false;
unsigned long mainModeButtonPressedTimePrev = 0;

int touchCalibrationTargetId = 0;
bool bTouchCalibrationTargetIdDirty = true;
unsigned long touchCalibTimePrev = 0;
struct TouchCalibrationData {
    float x0 = 0;
    float dxdx = 1;
    float dxdy = 0;
    float y0 = 0;
    float dydx = 0;
    float dydy = 1;
};
TouchCalibrationData touchCalibrationData;
Vector2 calibUserPts[3];

// threads
TaskHandle_t networkTask;
TaskHandle_t ledTask;


// LED Test
int ledTestId = 0;

// board square status
// uppercase for white, lowercase for black
// [0][0] is A1, [7][0] is H1
volatile bool clientBoardBoolBeingWrittenOrRead = false;
bool clientBoardBool[8][8]; // reed switch state, true if a piece is present on square
bool clientBoardBoolTempWrite[8][8]; // used to read the values before transferring to readable memory
char serverBoardPieces[8][8]; // pieces on the server

// to store return values for modified squares
Vector2i modifiedSquares[8*8];

// to add virtual moves for verifications
char boardCopy[8][8];

Vector2i potentialCapturedSquare(-1,-1);

Move* knownServerMoves; // allocated on heap


int nbKnownServerMoves = 0;
int prevNbKnownServerMoves = 0;

int switchDelayAfterQueryMs = 1; // to avoid crosstalk when querying switches
//int switchDelayAfterQueryMs = 10; // to avoid crosstalk when querying switches


enum class GameState {
    Connecting,
    NoGame,
    GameFound,
    WaitingToReceiveOpponentMove,
    UserMakingOpponentMove,
    UserMakingUserMove,
    UserMakingPromotion,
    WaitingToReceiveUserMoveConfirmation,
    UserFixingBadBoard,
    UserFixingInvalidMove,
    GameEnded,
    GameEndedWithPlayerRatingsOrQrCode,
    Error,
    Count
};
volatile GameState gameState = GameState::Connecting; // volatile because network thread can change it
volatile bool bGameStateDirty = true;

ShortString gameId = "";
ShortString gameStatusServerStr = ""; // game status on server

DynamicJsonDocument* jsonDocStream = nullptr;

std::shared_ptr<HTTPClient> httpClientStream = std::make_shared<HTTPClient>();
WiFiClient * wifiClientStream = 0;
std::shared_ptr<HTTPClient> httpClientApi = std::make_shared<HTTPClient>();

#if defined(BOARD_DEF_ESP32)
    XPT2046_Touchscreen * pTouch = 0;
    Arduino_ILI9341 * pDisplay = 0; // 240×320 pixels
    Arduino_ESP32SPI bus;
#elif defined(BOARD_DEF_RP2040)
    extern TFT_eSPI tft;
#endif


unsigned long timeMs = millis();


volatile bool activeLedsBeingWrittenOrRead = false;
const int maxNbActiveLeds = 9*9;
Vector2i activeLeds[maxNbActiveLeds];
volatile int nbActiveLeds = 0;

enum class PlayerColor {
    None,
    White,
    Black
};
PlayerColor playerColor = PlayerColor::None;
ShortString playerColorStr;

// init to all 0
Move userMoveToSend;
Move userMoveTentative;
Move opponentMove;
Move userMove;


// looks like I need to slow this down if the display wires are longer
//const int32_t displaySpeed = 32 * 1000 * 1000;
const int32_t displaySpeed = 32 * 1000 * 1000;

Menu currentMenu;
enum class MenuType {
    None,
    GameOptions,
    ReedSwitchTest,
    FixBadBoard,
    Connecting,
    NoGame,
    UserPromotion,
    MoveAndTimer,
    GameSeekSent,
    GameFound,
    GameEndedRatings,
    GameEndedQrCode,
    AcceptOrRefuseDraw,
    AcknowledgeDrawRefusal,
    AcknowledgeOpponentPromotion,
    VictoryClaimRefused,
    Keyboard1,
    Keyboard2,
    Keyboard3,
    Keyboard4,
    Error
};
MenuType currentMenuType = MenuType::None;

ShortString clickedButtonLabelText = "";

ShortString gameEndedMenuTitle0;
ShortString gameEndedMenuTitle1;

LongString keyboardText = "";

const int drawBoardContourThickness = 2;
const int drawBoardSquareSize = (DISPLAY_PIXELS_WIDTH - 2 * MENU_BORDER_WIDTH)/8;
const int drawBoardSquarePosY = 50;
const int drawBoardBoolMarkerBorder = 3;
const GFXfont* drawBoardFont = &ChessFont25;
    
// whether or not there is a game in progress already.
// We only play the first game we receive.
bool gameOngoing = false; 


bool sendTakebackRefusal = false;
bool sendRefuseDraw = false;
bool sendAcceptDraw = false;
bool sendResign = false;
bool sendClaimVictory = false;


long timeLastDummyMovePost;
long timeRefreshDummyMovePost = 5000; // mililseconds


volatile long opponentTimeLeft;
volatile long playerTimeLeft;
volatile long opponentTimeLeftReceived;
volatile long playerTimeLeftReceived;


bool menuTimerDirty = false;


const int timerButtonClearFillRectMarginX = 2;
const int timerButtonClearFillRectMarginY = 30;

const int timerRefreshDrawFrequency = 500; // milliseconds
unsigned long timeLastTimerDrawUpdate;
unsigned long timeLastTimerReceived;

bool opponentIsPlaying;

enum class Player {
    Player,
    Opponent,
    None
};
Player activePlayer = Player::None;


// keep the previous state of the offers from the state json
bool drawOfferFromPlayer = false;
bool drawOfferFromOpponent = false;
bool takebackOfferFromPlayer = false;
bool takebackOfferFromOpponent = false;

// notifications to show to the user so they can acknowledge or accept/refuse
bool notificationOpponentRefusesDraw = false;
bool notificationOpponentOffersDraw = false;
bool notificationOpponentOffersMoveTakeback = false;
bool notificationOpponentPromotes = false;
bool notificationVictoryClaimRefused = false;

bool showGameOptionsMenu = false;


struct GameSeekInfo {

    GameSeekInfo() {}
    GameSeekInfo(bool active,
                 bool twoPlayers,
                 bool vsCpu,
                 bool rated,
                 int time,
                 int increment) :
                 active(active),
                 twoPlayers(twoPlayers),
                 vsCpu(vsCpu),
                 rated(rated),
                 time(time),
                 increment(increment) {}

    bool active = false;
    bool twoPlayers;
    bool vsCpu;
    bool rated;
    int time;
    int increment;
};
GameSeekInfo gameSeekInfo;


LongString wifiName = "";
LongString wifiPassword = "";
LongString lichessToken = "";
LongString lichessBoardAccountUsername = "";
LongString lichessBoardAccountToken = "";

bool streamsDirty = true;

ShortString playerStartRatingStr = "";
ShortString playerEndRatingStr = "";
LongString playerName = "";
ShortString opponentStartRatingStr = "";
ShortString opponentEndRatingStr = "";
LongString opponentName = "";
ShortString gameSpeedStr = "";
ShortString gameSeekOrFoundButtonText = "";


ShortString errorMessage[MAX_NB_MENU_TITLES-2];

bool isTwoPlayersGame = false;
int twoPlayersTimePerSide = 10; // unlimited time if 0
int twoPlayersIncrement = 10;
const int twoPlayersTimeDeltaSmall = 1;
const int twoPlayersTimeDeltaLarge = 10;
const int twoPlayersTimePerSideMax = 180;
const int twoPlayersIncrementMax = 180;
const int twoPlayersTimePerSideMin = 1;
const int twoPlayersIncrementMin = 0;


QRCode qrCode;
const int qrCodeVersion = 5;
int qrCodeErrorCorrection = ECC_HIGH;
LongString qrCodeUrl = "";
std::vector<uint8_t> qrCodeBytes;
int qrCodeDrawScale = 3;


bool opponentIsGone = false;
volatile long opponentGoneTimeLeftMs;
volatile long opponentGoneTimeLeftWhenReceivedMs;
long opponentGoneTimeReceivedMs;
bool opponentIsGoneDrawDirty = false;