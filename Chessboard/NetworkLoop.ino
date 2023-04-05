
#include "Chessboard.h"
#include "NetworkLoop_Player.h"
#include "NetworkLoop_Game.h"
#include "Board.h"
#include "LedLoop.h"
#include "ReedSwitch.h"

void StartLichessStream() {

    Serial.println("Connecting...");

    WiFi.begin(strdup(wifiName.c_str()), wifiPassword.c_str());
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }

    Serial.println("Connected.");

    httpClientStream->setInsecure();
    httpClientApi->setInsecure();
}


void getPlayerStream() {
    bool success = false;
    while(!success) {
        httpClientStream->begin("https://lichess.org/api/stream/event");
        httpClientStream->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str());
        int httpCode = httpClientStream->GET();
        Serial.println("player stream httpCode: " + String(httpCode));
        if(httpCode > 0) {
            if(httpCode == HTTP_CODE_OK) {
                // get tcp stream
                wifiClientStream = httpClientStream->getStreamPtr();

                gameState = GameState::NoGame;
                bGameStateDirty = true;

                success = true;
            }
        } else {
            // stream failed
        }

        delay(10000);
    }
}




void sendMove(Move move) {
    int httpCode;

    // Try once with connection reuse, in case the connection is still open from the last move.
    // Try one more time, which will attempt to create a new connection.

    // The connections seem to remain active for about 10 seconds. If successive moves take more than that,
    // a new connection will be made (including the SSL handshake which takes like 2-3 seconds)
    
    bool bOk = false;
    for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
        httpClientApi->begin((LongString("https://lichess.org/api/board/game/")
                              + gameId + "/move/" + move.toString()).c_str());

        if(isTwoPlayersGame && activePlayer == Player::Opponent) {
            httpClientApi->addHeader("Authorization", (LongString("Bearer ")
                                                      + lichessBoardAccountToken).c_str(), true);
        } else {
            httpClientApi->addHeader("Authorization", (LongString("Bearer ")
                                                      + lichessToken).c_str(), true);
        }

        httpCode = httpClientApi->POST(nullptr, 0);
        httpClientApi->setReuse(true);

        Serial.println((ShortString("sendmove httpCode: ") + ShortString(httpCode)).c_str());

        if(httpCode > 0) {
            if(httpCode == HTTP_CODE_OK) {

                // all good.

            } else if(httpCode == HTTP_CODE_BAD_REQUEST) {

                #ifdef VISUAL_STUDIO_CODE
                    DynamicJsonDocument jsonDoc(std::size_t(LONG_STRING_LENGTH));
                #else
                    DynamicJsonDocument jsonDoc(LONG_STRING_LENGTH);
                #endif

                deserializeJson(jsonDoc, httpClientApi->getString());

                LongString error = "sendmove error: ";
                error += (const char*)(jsonDoc["error"]);
                Serial.println(error.c_str());

                gameState = GameState::UserFixingInvalidMove;
                bGameStateDirty = true;
            }
            bOk = true;
            break;
        }
        delay(DELAY_BETWEEN_HTTP_TRIES);
    }
    if(!bOk) {
        Serial.println("sendMove failed");
        gameState = GameState::Error;
        clearErrorMessages();
        errorMessage[0] = "sendMove failed.";
        errorMessage[1] = "";
        errorMessage[2] = "Please reboot.";
        bGameStateDirty = true;
        return;
    }
    
}

#if defined(BOARD_DEF_ESP32)
    void networkLoop(void* /*parameter*/) {
#elif defined(BOARD_DEF_RP2040)
    void networkLoop() {
#endif

    StartLichessStream();

    while(true) {

        if(gameState == GameState::Error) {
            return;
        }

        wl_status_t wifiStatus = wl_status_t(WiFi.status());
        if(wifiStatus != wl_status_t::WL_CONNECTED) {
            streamsDirty = true;

            bGameStateDirty = true;
            gameState = GameState::Connecting;

            continue;
        }

        if(wifiStatus == wl_status_t::WL_CONNECTED && streamsDirty) {
            getPlayerStream();
            streamsDirty = false;
        }

        if(!gameOngoing) {
            checkStreamPlayer();
        } else {
            checkStreamGame();
        }

        if(gameState == GameState::WaitingToReceiveUserMoveConfirmation && userMoveToSend.isValid()) {

            Serial.println((ShortString("Sending move: ") + userMoveToSend.toString()).c_str());

            sendMove(userMoveToSend);
            userMoveToSend = Move();
        } else if(gameState == GameState::UserMakingOpponentMove) {
            if(boardPiecesMatchSwitches(serverBoardPieces)) {

                takeActiveLedsMutex();
                clearActiveLeds();
                releaseActiveLedsMutex();

                gameState = GameState::UserMakingUserMove;
                bGameStateDirty = true;
            }
        }

        // Send dummy move to keep the connection alive
        if(gameId != "" &&
           long(millis()) - timeLastDummyMovePost > timeRefreshDummyMovePost)
        {
            // just to make sure we're not running out of memory, useful for debugging
            Serial.println((ShortString("getFreeHeap: ") + ShortString(int(rp2040.getFreeHeap()))).c_str());

            Serial.println("Sending dummy move");

            bool bOk = false;
            for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                httpClientApi->begin((LongString("https://lichess.org/api/board/game/") + gameId + "/move/0").c_str());
                httpClientApi->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str(), true);
                int httpCode = httpClientApi->POST(nullptr, 0);

                httpClientApi->setReuse(true);

                Serial.println("dummymove httpCode: " + String(httpCode));

                if(httpCode > 0) {
                    bOk = true;
                    break;
                }
                delay(DELAY_BETWEEN_HTTP_TRIES);
            }
            if(!bOk) {
                Serial.println("Send dummy move failed");
                gameState = GameState::Error;
                clearErrorMessages();
                errorMessage[0] = "Send dummy move";
                errorMessage[1] = "failed.";
                errorMessage[2] = "";
                errorMessage[3] = "Please reboot.";
                bGameStateDirty = true;
                return;
            }

            timeLastDummyMovePost = millis();
        }

        // These 3 commands assume success. Not a big deal if they fail, we'll
        // just keep playing and the draw or takeback will get cancelled after the next moves.
        if(sendTakebackRefusal) {
            bool bOk = false;
            for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                httpClientApi->begin((LongString("https://lichess.org/api/board/game/") + gameId + "/takeback/no").c_str());
                httpClientApi->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str(), true);
                int httpCode = httpClientApi->POST(nullptr, 0);
                Serial.println("sendTakebackRefusal httpCode: " + String(httpCode));
                httpClientApi->setReuse(true);

                if(httpCode > 0) {
                    bOk = true;
                    break;
                }
                delay(DELAY_BETWEEN_HTTP_TRIES);
            }
            if(!bOk) {
                Serial.println("sendTakebackRefusal failed");
                gameState = GameState::Error;
                clearErrorMessages();
                errorMessage[0] = "sendTakebackRefusal";
                errorMessage[1] = "failed.";
                errorMessage[2] = "";
                errorMessage[3] = "Please reboot.";
                bGameStateDirty = true;
                return;
            }

            sendTakebackRefusal = false;
            notificationOpponentOffersMoveTakeback = false;
            bGameStateDirty = true;
        }

        if(sendRefuseDraw) {
            bool bOk = false;
            for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                httpClientApi->begin((LongString("https://lichess.org/api/board/game/") + gameId + "/draw/no").c_str());
                httpClientApi->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str(), true);
                int httpCode = httpClientApi->POST(nullptr, 0);
                Serial.println("sendRefuseDraw httpCode: " + String(httpCode));
                httpClientApi->setReuse(true);

                if(httpCode > 0) {
                    bOk = true;
                    break;
                }
                delay(DELAY_BETWEEN_HTTP_TRIES);
            }
            if(!bOk) {
                Serial.println("sendRefuseDraw failed");
                gameState = GameState::Error;
                clearErrorMessages();
                errorMessage[0] = "sendRefuseDraw failed.";
                errorMessage[1] = "";
                errorMessage[2] = "Please reboot.";
                bGameStateDirty = true;
                return;
            }

            sendRefuseDraw = false;
        }

        if(sendAcceptDraw) {
            bool bOk = false;
            for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                httpClientApi->begin((LongString("https://lichess.org/api/board/game/") + gameId + "/draw/yes").c_str());
                
                httpClientApi->addHeader("Authorization", (LongString("Bearer ")
                                                        + lichessToken).c_str(), true);
                
                int httpCode = httpClientApi->POST(nullptr, 0);
                Serial.println("sendAcceptDraw httpCode: " + String(httpCode));
                httpClientApi->setReuse(true);

                if(httpCode > 0) {
                    bOk = true;
                    break;
                }
                delay(DELAY_BETWEEN_HTTP_TRIES);
            }
            if(!bOk) {
                Serial.println("sendAcceptDraw failed");
                gameState = GameState::Error;
                clearErrorMessages();
                errorMessage[0] = "sendAcceptDraw failed.";
                errorMessage[1] = "";
                errorMessage[2] = "Please reboot.";
                bGameStateDirty = true;
                return;
            }

            if(isTwoPlayersGame) {
                // Have the opponent accept the draw right away
                bOk = false;
                for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                    httpClientApi->begin((LongString("https://lichess.org/api/board/game/") + gameId + "/draw/yes").c_str());
                    
                    httpClientApi->addHeader("Authorization", (LongString("Bearer ")
                                                            + lichessBoardAccountToken).c_str(), true);
                    
                    int httpCode = httpClientApi->POST(nullptr, 0);
                    Serial.println("sendAcceptDraw httpCode: " + String(httpCode));
                    httpClientApi->setReuse(true);

                    if(httpCode > 0) {
                        bOk = true;
                        break;
                    }
                    delay(DELAY_BETWEEN_HTTP_TRIES);
                }
                if(!bOk) {
                    Serial.println("sendAcceptDraw failed");
                    gameState = GameState::Error;
                    clearErrorMessages();
                    errorMessage[0] = "sendAcceptDraw failed.";
                    errorMessage[1] = "";
                    errorMessage[2] = "Please reboot.";
                    bGameStateDirty = true;
                    return;
                }
            }

            sendAcceptDraw = false;
        }

        if(sendResign) {
            bool bOk = false;
            for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                httpClientApi->begin((LongString("https://lichess.org/api/board/game/") + gameId + "/resign").c_str());
                
                if(isTwoPlayersGame && activePlayer == Player::Opponent) {
                    httpClientApi->addHeader("Authorization", (LongString("Bearer ")
                                                            + lichessBoardAccountToken).c_str(), true);
                } else {
                    httpClientApi->addHeader("Authorization", (LongString("Bearer ")
                                                            + lichessToken).c_str(), true);
                }
                
                int httpCode = httpClientApi->POST(nullptr, 0);
                Serial.println("sendResign httpCode: " + String(httpCode));
                httpClientApi->setReuse(true);

                if(httpCode > 0) {
                    bOk = true;
                    break;
                }
                delay(DELAY_BETWEEN_HTTP_TRIES);
            }
            if(!bOk) {
                Serial.println("sendResign failed");
                gameState = GameState::Error;
                clearErrorMessages();
                errorMessage[0] = "sendResign failed.";
                errorMessage[1] = "";
                errorMessage[2] = "Please reboot.";
                bGameStateDirty = true;
                return;
            }

            sendResign = false;
        }

        if(sendClaimVictory) {
            bool bOk = false;
            for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                httpClientApi->begin((LongString("https://lichess.org/api/board/game/") + gameId + "/claim-victory").c_str());
                httpClientApi->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str(), true);
                int httpCode = httpClientApi->POST(nullptr, 0);
                Serial.println("sendClaimVictory httpCode: " + String(httpCode));

                if(httpCode == 400) {
                    notificationVictoryClaimRefused = true;
                    bGameStateDirty = true;
                }
                httpClientApi->setReuse(true);

                if(httpCode > 0) {
                    bOk = true;
                    break;
                }
                delay(DELAY_BETWEEN_HTTP_TRIES);
            }
            if(!bOk) {
                Serial.println("sendClaimVictory failed");
                gameState = GameState::Error;
                clearErrorMessages();
                errorMessage[0] = "sendClaimVictory failed.";
                errorMessage[1] = "";
                errorMessage[2] = "Please reboot.";
                bGameStateDirty = true;
                return;
            }

            sendClaimVictory = false;
        }

        if(gameSeekInfo.active) {
            Serial.println("Sending game seek");
            GameSeekInfo& g = gameSeekInfo;
            int httpCode;
            bool bOk = false;
            if(g.twoPlayers) {
                for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                    Serial.println("Send Game Challenge: attempt");
                    httpClientApi->begin((LongString("https://lichess.org/api/challenge/") + lichessBoardAccountUsername).c_str());
                    httpClientApi->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str(), true);
                    httpClientApi->addHeader("Content-Type", "application/json", false);
                    String params;
                    if(g.time == 0) {
                        params = String("{\
                        \"rated\":false,\
                        \"color\":\"white\",\
                        \"variant\":\"standard\",\
                        \"acceptByToken\":\"") + String(lichessBoardAccountToken.c_str()) + String("\"\
                        }");
                    } else {
                        params = String("{\
                        \"rated\":false,\
                        \"clock.limit\":" + String(60*g.time) + ",\
                        \"clock.increment\":" + String(g.increment) + ",\
                        \"color\":\"white\",\
                        \"variant\":\"standard\",\
                        \"acceptByToken\":\"") + String(lichessBoardAccountToken.c_str()) + String("\"\
                        }");
                    }
                    Serial.println(params.c_str());
                    httpCode = httpClientApi->POST(params.c_str());

                    Serial.println((ShortString("gameSeekInfo httpCode: ") + ShortString(httpCode)).c_str());
                    httpClientApi->setReuse(true);
                    if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
                        Serial.println("Sending game seek: success");
                        bOk = true;
                        break;
                    }
                    delay(DELAY_BETWEEN_HTTP_TRIES);
                }
            } else {
                if(g.vsCpu) {
                    for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                        Serial.println("Sending game seek: attempt");
                        httpClientApi->begin("https://lichess.org/api/challenge/ai");
                        httpClientApi->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str(), true);
                        httpClientApi->addHeader("Content-Type", "application/json", false);
                        httpCode = httpClientApi->POST("{\"level\":1}");
                        Serial.println((ShortString("gameSeekInfo httpCode: ") + ShortString(httpCode)).c_str());
                        if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
                            Serial.println("Sending game seek: success");
                            bOk = true;
                            break;
                        }
                        delay(DELAY_BETWEEN_HTTP_TRIES);
                    }

                } else {
                    for(int i = 0; i < HTTP_REQUEST_NB_TRIES; i++) {
                        httpClientApi->setReuse(false);
                        httpClientApi->begin("https://lichess.org/api/board/seek");
                        httpClientApi->addHeader("Authorization", (LongString("Bearer ") + lichessToken).c_str(), true);
                        httpClientApi->addHeader("Content-Type", "application/json", false);
                        LongString content = ShortString("{\"rated\":") + ShortString(g.rated ? "true" : "false") + ",";
                                content += ShortString("\"time\":") + ShortString(g.time) + ",";
                                content += ShortString("\"increment\":") + ShortString(g.increment) + "}";
                        httpCode = httpClientApi->POST(content.c_str());
                        Serial.println((LongString("gameSeekInfo content: ") + content).c_str());
                        Serial.println((ShortString("gameSeekInfo httpCode: ") + ShortString(httpCode)).c_str());
                        if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
                            Serial.println("Sending game seek: success");
                            bOk = true;
                            break;
                        }
                        delay(DELAY_BETWEEN_HTTP_TRIES);
                    }

                }
            }

            if(httpCode == HTTP_CODE_BAD_REQUEST) {
                DynamicJsonDocument jsonDoc(LONG_STRING_LENGTH);

                deserializeJson(jsonDoc, httpClientApi->getString());
                Serial.print("(const char*)jsonDoc[\"error\"] = ");
                Serial.println((const char*)jsonDoc["error"]);
            }

            gameSeekInfo.active = false;

            if(!bOk) {
                Serial.println("sendGameSeek failed");
                gameState = GameState::Error;
                clearErrorMessages();
                errorMessage[0] = "sendGameSeek failed.";
                errorMessage[1] = "";
                errorMessage[2] = "Please reboot.";
                bGameStateDirty = true;
                return;
            }
        }

        // some delay here otherwise some thread watchdog complains that the core is idle.
        // could be much smaller, like 10ms
        delay(200); 
    }
}
