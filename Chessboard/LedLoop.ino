
#include "Chessboard.h"
#include "Board.h"


void digitalWritePinLedPos(int val0, int val1, int val2, int val3) {
    digitalWrite(PIN_LED_POS_0, val0);
    digitalWrite(PIN_LED_POS_1, val1);
    digitalWrite(PIN_LED_POS_2, val2);
    digitalWrite(PIN_LED_POS_3, val3);
}


void digitalWritePinLedNeg(int val0, int val1, int val2, int val3) {
    digitalWrite(PIN_LED_NEG_0, val0);
    digitalWrite(PIN_LED_NEG_1, val1);
    digitalWrite(PIN_LED_NEG_2, val2);
    digitalWrite(PIN_LED_NEG_3, val3);
}


// (0,0) is the A1 corner, (8,0) is the H1 corner
void LightLed(int x, int y) {

    if(x < 0 || y < 0 || x > 8 || y > 8) {
        // all off
        digitalWritePinLedPos(0,0,1,0);
        return;
    }

    // w.r.t. schematics for 74LS138
    // A is PIN_LED_POS_3
    // B is PIN_LED_POS_0
    // C is PIN_LED_POS_1
    switch(x) {
        case 0:
            // special case
            digitalWritePinLedPos(0,1,1,0);
            break;
        case 1:
            // 74LS138 output 2
            digitalWritePinLedPos(1,0,0,0);
            break;
        case 2:
            // 74LS138 output 1
            digitalWritePinLedPos(0,0,0,1);
            break;
        case 3:
            // 74LS138 output 0
            digitalWritePinLedPos(0,0,0,0);
            break;
        case 4:
            // 74LS138 output 7
            digitalWritePinLedPos(1,1,0,1);
            break;
        case 5:
            // 74LS138 output 6
            digitalWritePinLedPos(1,1,0,0);
            break;
        case 6:
            // 74LS138 output 5
            digitalWritePinLedPos(0,1,0,1);
            break;
        case 7:
            // 74LS138 output 4
            digitalWritePinLedPos(0,1,0,0);
            break;
        case 8:
            // 74LS138 output 3
            digitalWritePinLedPos(1,0,0,1);
            break;
    }

    // w.r.t. schematics for 74LS138
    // A is PIN_LED_NEG_3
    // B is PIN_LED_NEG_2
    // C is PIN_LED_NEG_1
    switch(y) {
        case 0:
            // 74LS138 output 6
            digitalWritePinLedNeg(1,1,1,0);
            break;
        case 1:
            // 74LS138 output 5
            digitalWritePinLedNeg(1,1,0,1);
            break;
        case 2:
            // 74LS138 output 4
            digitalWritePinLedNeg(1,1,0,0);
            break;
        case 3:
            // 74LS138 output 3
            digitalWritePinLedNeg(1,0,1,1);
            break;
        case 4:
            // special case
            digitalWritePinLedNeg(0,0,0,0);
            break;
        case 5:
            // 74LS138 output 7
            digitalWritePinLedNeg(1,1,1,1);
            break;
        case 6:
            // 74LS138 output 0
            digitalWritePinLedNeg(1,0,0,0);
            break;
        case 7:
            // 74LS138 output 1
            digitalWritePinLedNeg(1,0,0,1);
            break;
        case 8:
            // 74LS138 output 2
            digitalWritePinLedNeg(1,0,1,0);
            break;
    }
}


void clearActiveLeds() {
    nbActiveLeds = 0;
}


void addActiveLed(Vector2i newLed) {
    if(nbActiveLeds >= maxNbActiveLeds) {
        Serial.println("Can't add more active leds.");
        return;
    }
    if(newLed.x < 0 || newLed.x > 8 || newLed.y < 0 || newLed.y > 8) {
        Serial.println("Can't add invalid active leds.");
        return;
    }
    
    // check for duplicates
    for(int i = 0; i < nbActiveLeds; i++) {
        if(activeLeds[i].sameAs(newLed)) {
            return; // not an error to try to add duplicate
        }
    }
    
    // All good, add the led
    activeLeds[nbActiveLeds] = newLed;
    nbActiveLeds++;
}


void addActiveLedSquare(Vector2i square) {
    for(int iCorner = 0; iCorner < 2; iCorner++) {
        for(int jCorner = 0; jCorner < 2; jCorner++) {
            addActiveLed(Vector2i(square.x+iCorner, square.y+jCorner));
        }
    }
}


void addActiveLedMove(char pieces[8][8], Move move) {
    Serial.println((ShortString("Adding LED move ") + move.toString()).c_str());

    Vector2i from = move.getFrom();
    Vector2i to = move.getTo();

    // check for castling move
    if(isCastlingMove(pieces, move)) {
        Serial.println("IS CASTLING MOVE");
        int row = to.y;
        if(to.x == 6) {
            // short castle
            addActiveLedSquare(Vector2i(6, row));
            addActiveLedSquare(Vector2i(5, row));
            addActiveLedSquare(Vector2i(4, row));
            addActiveLedSquare(Vector2i(7, row));
        } else {
            // long castle
            addActiveLedSquare(Vector2i(2, row));
            addActiveLedSquare(Vector2i(3, row));
            addActiveLedSquare(Vector2i(4, row));
            addActiveLedSquare(Vector2i(0, row));
        }
    } else if(isEnPassantMove(pieces, move)) {
        Serial.println("IS EN PASSANT MOVE");
        addActiveLedSquare(from);
        addActiveLedSquare(to);
        addActiveLedSquare(Vector2i(to.x, from.y));

    } else {
        addActiveLedSquare(from);
        addActiveLedSquare(to);
    }
}


void takeActiveLedsMutex() {
#if defined(BOARD_DEF_ESP32)
    while(activeLedsBeingWrittenOrRead) {vTaskDelay(1);}
#endif
    activeLedsBeingWrittenOrRead = true;    
}


void releaseActiveLedsMutex() {
    activeLedsBeingWrittenOrRead = false;
}

#if defined(BOARD_DEF_ESP32)
    void ledLoop(void* parameter) {
        while(true) {
            // update LEDs
            takeActiveLedsMutex();
            lightNextActiveLed();
            releaseActiveLedsMutex();  
            vTaskDelay(1);  
        }
    }
#elif defined(BOARD_DEF_RP2040)
    bool ledLoop(repeating_timer *t) {
        if(runLedLoop) {
            lightNextActiveLed();
        }
        return true;
    }
#endif

void lightNextActiveLed() {
    static unsigned int tick = 0;
    if(nbActiveLeds > 0) {
        Vector2i led = activeLeds[tick % nbActiveLeds];
        LightLed(led.x, led.y);
        tick++;
    } else {
        LightLed(-1, -1);
    }
}