

#include "Chessboard.h"
#include "LedLoop.h"


void endGame() {

    notificationOpponentRefusesDraw = false;
    notificationOpponentOffersDraw = false;
    notificationOpponentOffersMoveTakeback = false;
    notificationOpponentPromotes = false;
    notificationVictoryClaimRefused = false;

    // Don't completely kill the game yet, because we still want to see
    // what move happens when we get checkmated.

    if(gameState != GameState::NoGame) {
        gameState = GameState::GameEnded;
        bGameStateDirty = true;
    }

    if(gameState == GameState::GameEnded) {
        if(!isTwoPlayersGame) {
            // get opponent and player rating after game
            if(opponentName != "") {
                for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                    httpClientApi->begin((LongString("https://lichess.org/api/user/") + opponentName).c_str());
                    int httpCode = httpClientApi->GET();
                    Serial.println((LongString("endGame(): opponent user info httpCode: ") + LongString(httpCode)).c_str());
                    String httpResponse = httpClientApi->getString();
                    Serial.println(httpResponse);
                    httpClientApi->setReuse(true);
                    if(httpCode == HTTP_CODE_OK) {
                        deserializeJson(*jsonDocStream, httpResponse);
                        opponentEndRatingStr = ShortString((*jsonDocStream)["perfs"][gameSpeedStr.c_str()]["rating"].as<int>());
                        if((*jsonDocStream)["perfs"][gameSpeedStr.c_str()].containsKey("prov") &&
                        (*jsonDocStream)["perfs"][gameSpeedStr.c_str()]["prov"].as<bool>())
                        {
                            opponentEndRatingStr += '?';
                        }
                        break;
                    }
                    delay(DELAY_BETWEEN_HTTP_TRIES);
                }
            } else {
                opponentEndRatingStr = "NA";
            }

            if(playerName != "") {
                for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                    httpClientApi->begin((LongString("https://lichess.org/api/user/") + playerName).c_str());
                    int httpCode = httpClientApi->GET();
                    Serial.println((LongString("endGame(): player user info httpCode: ") + LongString(httpCode)).c_str());
                    String httpResponse = httpClientApi->getString();
                    Serial.println(httpResponse);
                    httpClientApi->setReuse(true);
                    if(httpCode == HTTP_CODE_OK) {
                        deserializeJson(*jsonDocStream, httpResponse);
                        playerEndRatingStr = ShortString((*jsonDocStream)["perfs"][gameSpeedStr.c_str()]["rating"].as<int>());
                        if((*jsonDocStream)["perfs"][gameSpeedStr.c_str()].containsKey("prov") &&
                        (*jsonDocStream)["perfs"][gameSpeedStr.c_str()]["prov"].as<bool>())
                        {
                            playerEndRatingStr += '?';
                        }
                        break;
                    }
                    delay(DELAY_BETWEEN_HTTP_TRIES);
                }
            } else {
                playerEndRatingStr = "NA";
            }

            if(gameState == GameState::GameEnded) {
                gameState = GameState::GameEndedWithPlayerRatingsOrQrCode;
                bGameStateDirty = true;
            }
        } else {
            // isTwoPlayerGame

            // generate QR code
            qrCodeUrl = LongString("lichess.org/") + gameId;
            qrCodeBytes.resize(qrcode_getBufferSize(qrCodeVersion));
            qrcode_initText(&qrCode, qrCodeBytes.data(), qrCodeVersion, qrCodeErrorCorrection, qrCodeUrl.c_str());

            if(gameState == GameState::GameEnded) {
                currentMenu.titles[3].text = "";
                gameState = GameState::GameEndedWithPlayerRatingsOrQrCode;
                bGameStateDirty = true;
            }
        }
    }

    gameId = "";
}


void checkStreamPlayer() {

    // get new player events
    if(!jsonDocStream) {
        return;
    }


    deserializeJson(*jsonDocStream, *wifiClientStream);

    ShortString type("null");
    if(jsonDocStream->containsKey("type")) {
        type = (*jsonDocStream)["type"].as<const char*>();
    }

    if(type == "null") {
        return;
    }

    Serial.println((ShortString("type: ") + type).c_str());

    String jsonText;
    serializeJson(*jsonDocStream, jsonText);
    if(type != "null") {
        Serial.println("jsonText: " + jsonText);
    }
    
    if(type == "gameStart") {
        Serial.println("gameStart event");

        // check if two player game against board account
        ShortString opponentName = (const char*)(*jsonDocStream)["game"]["opponent"]["username"];
        isTwoPlayersGame = (opponentName == lichessBoardAccountUsername);

        // get player color
        playerColorStr = (const char*)(*jsonDocStream)["game"]["color"];
        if(playerColorStr == "white") {
            Serial.println("playerColor == PlayerColor::White");
            playerColor = PlayerColor::White;
        } else if(playerColorStr == "black") {
            Serial.println("playerColor == PlayerColor::Black");
            playerColor = PlayerColor::Black;
        } else {
            Serial.println("playerColor == PlayerColor::None");
            playerColor = PlayerColor::None;
        }

        // Show the player's color so we can rotate the board while getting the game stream.
        // Change the game state and actually change the menu on the main thread because
        // we can't draw on the network thread
        gameState = GameState::GameFound;
        gameSeekOrFoundButtonText = "";
        bGameStateDirty = true;

        // Get game id and read the stream for that game
        gameId = (const char*)(*jsonDocStream)["game"]["gameId"];
        
        bool success = false;
        for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
            httpClientStream->begin((LongString("https://lichess.org/api/board/game/stream/") + gameId).c_str());
            httpClientStream->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str());
            
            int httpCode = httpClientStream->GET();
            Serial.println((ShortString("game stream httpCode: ") + ShortString(httpCode)).c_str());
            Serial.println((ShortString("gameId: ") + ShortString(gameId)).c_str());
            if(httpCode == HTTP_CODE_OK) {
                // get tcp stream
                wifiClientStream = httpClientStream->getStreamPtr();
                if(wifiClientStream != NULL) {
                    success = true;
                    break;
                }
            }
            delay(DELAY_BETWEEN_HTTP_TRIES);
        }

        if(success) {
            gameOngoing = true;
        } else {
            Serial.println("checkStreamPlayer gameStart event: error while getting game stream");
            gameState = GameState::Error;
            clearErrorMessages();
            errorMessage[0] = "Error while";
            errorMessage[1] = "getting";
            errorMessage[2] = "game stream.";
            errorMessage[3] = "";
            errorMessage[4] = "Please reboot.";
            bGameStateDirty = true;
        }
    }
}
