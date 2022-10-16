
#include "Chessboard.h"
#include "MainLoop_ReedSwitchTest.h"
#include "Board.h"
#include "LedLoop.h"


bool getJsonBool(ShortString str, DynamicJsonDocument& jsonDoc, bool bGameFull) {
    if(bGameFull) {
        if(jsonDoc["state"].containsKey(str.c_str())) {
            return jsonDoc["state"][str.c_str()].as<bool>();
        } else {
            return false;
        }
    } else {
        if(jsonDoc.containsKey(str.c_str())) {
            return jsonDoc[str.c_str()].as<bool>();
        } else {
            return false;
        }
    }
}


void dealWithDrawOrTakebackOffer(DynamicJsonDocument& jsonDoc, bool bGameFull) {

    bool b;
    if(playerColor == PlayerColor::White) {
        
        b = getJsonBool("wdraw", jsonDoc, bGameFull);
        if(drawOfferFromPlayer && !b) {
            if(gameState != GameState::GameEnded &&
               gameState != GameState::GameEndedWithPlayerRatingsOrQrCode)
            {
                notificationOpponentRefusesDraw = true;
            }
        }
        drawOfferFromPlayer = b;

        notificationOpponentOffersDraw = getJsonBool("bdraw", jsonDoc, bGameFull);

        notificationOpponentOffersMoveTakeback = getJsonBool("btakeback", jsonDoc, bGameFull);

    } else if(playerColor == PlayerColor::Black) {

        b = getJsonBool("bdraw", jsonDoc, bGameFull);
        if(drawOfferFromPlayer && !b) {
            if(gameState != GameState::GameEnded &&
               gameState != GameState::GameEndedWithPlayerRatingsOrQrCode)
            {
                notificationOpponentRefusesDraw = true;
            }
        }
        drawOfferFromPlayer = b;

        notificationOpponentOffersDraw = getJsonBool("wdraw", jsonDoc, bGameFull);

        notificationOpponentOffersMoveTakeback = getJsonBool("wtakeback", jsonDoc, bGameFull);
    }

    if(notificationOpponentRefusesDraw ||
       notificationOpponentOffersDraw ||
       notificationOpponentOffersMoveTakeback)
    {
        bGameStateDirty = true;
    }
}


void streamGame_gameFull(DynamicJsonDocument& jsonDoc) {
    gameStatusServerStr = (const char*)jsonDoc["state"]["status"];
    if(gameStatusServerStr == "started") {

        // get player and game info
        if(playerColor == PlayerColor::White) {
            if(jsonDoc["white"].containsKey("rating")) {
                playerStartRatingStr = ShortString(jsonDoc["white"]["rating"].as<int>());
                if(jsonDoc["white"].containsKey("provisional") &&
                   jsonDoc["white"]["provisional"].as<bool>())
                {
                    playerStartRatingStr += '?';
                }
                playerName = jsonDoc["white"]["id"].as<const char*>();
            } else {
                playerStartRatingStr = "NA";
                playerName = "";
            }
            if(jsonDoc["black"].containsKey("rating")) {
                opponentStartRatingStr = ShortString(jsonDoc["black"]["rating"].as<int>());
                if(jsonDoc["black"].containsKey("provisional") &&
                   jsonDoc["black"]["provisional"].as<bool>())
                {
                    opponentStartRatingStr += '?';
                }
                opponentName = jsonDoc["black"]["id"].as<const char*>();
            } else {
                opponentStartRatingStr = "NA";
                opponentName = "";
            }
        } else if(playerColor == PlayerColor::Black) {
            if(jsonDoc["white"].containsKey("rating")) {
                opponentStartRatingStr = ShortString(jsonDoc["white"]["rating"].as<int>());
                if(jsonDoc["white"].containsKey("provisional") &&
                   jsonDoc["white"]["provisional"].as<bool>())
                {
                    opponentStartRatingStr += '?';
                }
                opponentName = jsonDoc["white"]["id"].as<const char*>();
            } else {
                opponentStartRatingStr = "NA";
                opponentName = "";
            }
            if(jsonDoc["black"].containsKey("rating")) {
                playerStartRatingStr = ShortString(jsonDoc["black"]["rating"].as<int>());
                if(jsonDoc["black"].containsKey("provisional") &&
                   jsonDoc["black"]["provisional"].as<bool>())
                {
                    playerStartRatingStr += '?';
                }
                playerName = jsonDoc["black"]["id"].as<const char*>();
            } else {
                playerStartRatingStr = "NA";
                playerName = "";
            }
        } else {
            Serial.println("ERROR streamGame_gameFull: no player color");
        }
        gameSpeedStr = jsonDoc["speed"].as<const char*>();
        

        // Using dynamic heap String here because string can be very large
        String newServerMovesStr = jsonDoc["state"]["moves"].as<String>();
        nbKnownServerMoves = tokenizeMoves(newServerMovesStr, ' ', knownServerMoves, MAX_NB_MOVES);
        prevNbKnownServerMoves = nbKnownServerMoves;

        if(nbKnownServerMoves == -1) {
            Serial.println("Max nb of moves exceeded, can't continue.");
            gameState = GameState::Error;
            clearErrorMessages();
            errorMessage[0] = "Max nb moves";
            errorMessage[1] = "exceeded";
            errorMessage[2] = "";
            errorMessage[3] = "Continue on Lichess.";
            bGameStateDirty = true;
            return;
        }
        
        setBoardToInitialPositions(serverBoardPieces);
        makeBoardMoves(serverBoardPieces, knownServerMoves, nbKnownServerMoves, 0);


        // if board matches switches, assume all is good and set client board
        // from server board
        bool problemWithBoardSetting = !boardPiecesMatchSwitches(serverBoardPieces);
        if(!problemWithBoardSetting) {
            if(nextMoveIsUserMove(nbKnownServerMoves)) {
                gameState = GameState::UserMakingUserMove;
                bGameStateDirty = true;
                activePlayer = Player::Player;
            } else if(isTwoPlayersGame) {
                gameState = GameState::UserMakingUserMove;
                bGameStateDirty = true;
                activePlayer = Player::Opponent;
            } else {
                gameState = GameState::WaitingToReceiveOpponentMove;
                bGameStateDirty = true;
            }
        } else {
            // check if we're just missing the last move our opponent just made
            if(nbKnownServerMoves > 0 && nextMoveIsUserMove(nbKnownServerMoves)) {
                setBoardToInitialPositions(serverBoardPieces);
                makeBoardMoves(serverBoardPieces, knownServerMoves, nbKnownServerMoves-1, 0);
                if(boardPiecesMatchSwitches(serverBoardPieces)) {
                    // Assume the client state is in the previous move, and
                    // we need to make the last opponent move on the board

                    opponentMove = knownServerMoves[nbKnownServerMoves-1];
                    if(opponentMove.hasPromotion()) {
                        notificationOpponentPromotes = true;   
                    }
                    
                    takeActiveLedsMutex();
                    clearActiveLeds();
                    addActiveLedMove(serverBoardPieces, opponentMove);
                    releaseActiveLedsMutex();

                    makeBoardMove(serverBoardPieces, knownServerMoves[nbKnownServerMoves-1], nbKnownServerMoves-1);
                    gameState = GameState::UserMakingOpponentMove;
                    bGameStateDirty = true;

                    problemWithBoardSetting = false;
                }
            }
        }

        dealWithDrawOrTakebackOffer(jsonDoc, true);

        if(problemWithBoardSetting) {
            setBoardToInitialPositions(serverBoardPieces);
            makeBoardMoves(serverBoardPieces, knownServerMoves, nbKnownServerMoves, 0);

            Serial.println("Problem with board setting");
            Serial.println("nbKnownServerMoves: " + String(nbKnownServerMoves));

            gameState = GameState::UserFixingBadBoard;
            bGameStateDirty = true;
        }
        
        if(playerColor == PlayerColor::White) {
            playerTimeLeftReceived = jsonDoc["state"]["wtime"].as<long>();
            opponentTimeLeftReceived = jsonDoc["state"]["btime"].as<long>();
        } else {
            playerTimeLeftReceived = jsonDoc["state"]["btime"].as<long>();
            opponentTimeLeftReceived = jsonDoc["state"]["wtime"].as<long>();
        }
        playerTimeLeft = playerTimeLeftReceived;
        opponentTimeLeft = opponentTimeLeftReceived;
        timeLastTimerReceived = millis();
        menuTimerDirty = true;
    } else if(gameStatusServerStr == "aborted") {

        if(playerColor == PlayerColor::White) {
            if(jsonDoc["white"].containsKey("rating")) {
                playerStartRatingStr = ShortString(jsonDoc["white"]["rating"].as<int>());
                if(jsonDoc["white"].containsKey("provisional") &&
                   jsonDoc["white"]["provisional"].as<bool>())
                {
                    playerStartRatingStr += '?';
                }
                playerName = jsonDoc["white"]["id"].as<const char*>();
            } else {
                playerStartRatingStr = "NA";
                playerName = "";
            }
            if(jsonDoc["black"].containsKey("rating")) {
                opponentStartRatingStr = ShortString(jsonDoc["black"]["rating"].as<int>());
                if(jsonDoc["black"].containsKey("provisional") &&
                   jsonDoc["black"]["provisional"].as<bool>())
                {
                    opponentStartRatingStr += '?';
                }
                opponentName = jsonDoc["black"]["id"].as<const char*>();
            } else {
                opponentStartRatingStr = "NA";
                opponentName = "";
            }
        } else if(playerColor == PlayerColor::Black) {
            if(jsonDoc["white"].containsKey("rating")) {
                opponentStartRatingStr = ShortString(jsonDoc["white"]["rating"].as<int>());
                if(jsonDoc["white"].containsKey("provisional") &&
                   jsonDoc["white"]["provisional"].as<bool>())
                {
                    opponentStartRatingStr += '?';
                }
                opponentName = jsonDoc["white"]["id"].as<const char*>();
            } else {
                opponentStartRatingStr = "NA";
                opponentName = "";
            }
            if(jsonDoc["black"].containsKey("rating")) {
                playerStartRatingStr = ShortString(jsonDoc["black"]["rating"].as<int>());
                if(jsonDoc["black"].containsKey("provisional") &&
                   jsonDoc["black"]["provisional"].as<bool>())
                {
                    playerStartRatingStr += '?';
                }
                playerName = jsonDoc["black"]["id"].as<const char*>();
            } else {
                playerStartRatingStr = "NA";
                playerName = "";
            }
        } else {
            Serial.println("ERROR streamGame_gameFull: no player color");
        }
        gameSpeedStr = jsonDoc["speed"].as<const char*>();

        gameEndedMenuTitle0 = "Game Aborted";
        gameEndedMenuTitle1 = "No Winner";
        endGame();
    } else {
        Serial.println((LongString("error streamGame_gameFull status: ") + gameStatusServerStr).c_str());
        gameState = GameState::Error;
        clearErrorMessages();
        errorMessage[0] = "Error while";
        errorMessage[1] = "starting";
        errorMessage[2] = "game.";
        errorMessage[3] = "";
        errorMessage[4] = "Please reboot.";
        bGameStateDirty = true;
    }
}


void streamGame_gameState(DynamicJsonDocument& jsonDoc) {

    // get new move (if any) and light LEds
    if(gameState == GameState::WaitingToReceiveOpponentMove) {
        Serial.println("gameState == GameState::WaitingToReceiveOpponentMove");

        // Using dynamic heap String here because string can be very large
        String newServerMovesStr = jsonDoc["moves"].as<String>();
        nbKnownServerMoves = tokenizeMoves(newServerMovesStr, ' ', knownServerMoves, MAX_NB_MOVES);

        if(nbKnownServerMoves == -1) {
            Serial.println("Max nb of moves exceeded, can't continue.");
            gameState = GameState::Error;
            clearErrorMessages();
            errorMessage[0] = "Max nb moves";
            errorMessage[1] = "exceeded";
            errorMessage[2] = "";
            errorMessage[3] = "Continue on Lichess.";
            bGameStateDirty = true;
            return;
        }

        Serial.println((LongString("prevNbKnownServerMoves: ") + LongString(prevNbKnownServerMoves)).c_str());
        Serial.println((LongString("nbKnownServerMoves: ") + LongString(nbKnownServerMoves)).c_str());
        if(nbKnownServerMoves != prevNbKnownServerMoves) {
            opponentMove = knownServerMoves[nbKnownServerMoves-1];
            if(opponentMove.hasPromotion()) {
                notificationOpponentPromotes = true;   
            }

            takeActiveLedsMutex();
            clearActiveLeds();
            if(opponentMove.isValid()) {
                addActiveLedMove(serverBoardPieces, opponentMove);
            }
            releaseActiveLedsMutex();

            if(opponentMove.isValid()) {
                makeBoardMove(serverBoardPieces, opponentMove, prevNbKnownServerMoves);
            }

            gameState = GameState::UserMakingOpponentMove;
            bGameStateDirty = true;

            prevNbKnownServerMoves = nbKnownServerMoves;
        }
        
    } else if(gameState == GameState::WaitingToReceiveUserMoveConfirmation) {

        Serial.println("gameState == GameState::WaitingToReceiveUserMoveConfirmation");

        // Using dynamic heap String here because string can be very large
        String newServerMovesStr = jsonDoc["moves"].as<String>();
        nbKnownServerMoves = tokenizeMoves(newServerMovesStr, ' ', knownServerMoves, MAX_NB_MOVES);
        
        if(nbKnownServerMoves == -1) {
            Serial.println("Max nb of moves exceeded, can't continue.");
            gameState = GameState::Error;
            clearErrorMessages();
            errorMessage[0] = "Max nb moves";
            errorMessage[1] = "exceeded";
            errorMessage[2] = "";
            errorMessage[3] = "Continue on Lichess.";
            bGameStateDirty = true;
            return;
        }

        if(nbKnownServerMoves != prevNbKnownServerMoves) {
        
            userMove = knownServerMoves[nbKnownServerMoves-1];

            makeBoardMove(serverBoardPieces, userMove, prevNbKnownServerMoves);

            if(isTwoPlayersGame) {
                gameState = GameState::UserMakingUserMove;
                if(nextMoveIsUserMove(nbKnownServerMoves)) {
                    activePlayer = Player::Player;
                } else {
                    activePlayer = Player::Opponent;
                }
            } else {
                gameState = GameState::WaitingToReceiveOpponentMove;
            }
            bGameStateDirty = true;

            prevNbKnownServerMoves = nbKnownServerMoves;
        }
    }


    gameStatusServerStr = (const char*)(jsonDoc["status"]);
    Serial.println((ShortString("gameStatusServerStr: ") + gameStatusServerStr).c_str());
    if(gameStatusServerStr == "created" ||
       gameStatusServerStr == "started")
    {
        // nothing
    } else if(gameStatusServerStr == "resign") {
        gameEndedMenuTitle0 = "Game Resigned";
        gameEndedMenuTitle1 = (const char*)jsonDoc["winner"];
        gameEndedMenuTitle1.firstCharToUpperCase();
        gameEndedMenuTitle1 += " Wins";
        endGame();
    } else if(gameStatusServerStr == "aborted") {
        gameEndedMenuTitle0 = "Game Aborted";
        gameEndedMenuTitle1 = "No Winner";
        endGame();
    } else if(gameStatusServerStr == "mate") {
        gameEndedMenuTitle0 = "Checkmate";
        gameEndedMenuTitle1 = (const char*)jsonDoc["winner"];
        gameEndedMenuTitle1.firstCharToUpperCase();
        gameEndedMenuTitle1 += " Wins";
        endGame();
    } else if(gameStatusServerStr == "stalemate") {
        gameEndedMenuTitle0 = "Stalemate";
        gameEndedMenuTitle1 = "";
        endGame();
    } else if(gameStatusServerStr == "draw") {
        gameEndedMenuTitle0 = "Game Ended";
        gameEndedMenuTitle1 = "Draw";
        endGame();
    } else if(gameStatusServerStr == "outoftime") {
        gameEndedMenuTitle0 = "Out of Time";
        if(jsonDocStream->containsKey("winner")) {
            gameEndedMenuTitle1 = (const char*)jsonDoc["winner"];
            gameEndedMenuTitle1.firstCharToUpperCase();
            gameEndedMenuTitle1 += " Wins";
        } else {
            gameEndedMenuTitle1 = "Draw";
        }
        endGame();
    } else if(gameStatusServerStr == "timeout") {
        gameEndedMenuTitle0 = "Player Left";
         if(jsonDocStream->containsKey("winner")) {
            gameEndedMenuTitle1 = (const char*)jsonDoc["winner"];
            gameEndedMenuTitle1.firstCharToUpperCase();
            gameEndedMenuTitle1 += " Wins";
         } else {
            gameEndedMenuTitle1 = "Draw";
         }
        endGame();
    } else {
        gameEndedMenuTitle0 = "Game Ended";
        gameEndedMenuTitle1 = "Reason Unclear";
        endGame();
    }

    dealWithDrawOrTakebackOffer(jsonDoc, false);

    if(playerColor == PlayerColor::White) {
        playerTimeLeftReceived = jsonDoc["wtime"].as<long>();
        opponentTimeLeftReceived = jsonDoc["btime"].as<long>();
    } else {
        playerTimeLeftReceived = jsonDoc["btime"].as<long>();
        opponentTimeLeftReceived = jsonDoc["wtime"].as<long>();
    }
    playerTimeLeft = playerTimeLeftReceived;
    opponentTimeLeft = opponentTimeLeftReceived;
    timeLastTimerReceived = millis();
    menuTimerDirty = true;
}


void streamGame_opponentGone(DynamicJsonDocument& jsonDoc) {

    opponentIsGone = jsonDoc["gone"].as<bool>();
    opponentIsGoneDrawDirty = true;

    if(opponentIsGone) {
        if(jsonDoc.containsKey("claimWinInSeconds")) {
            opponentGoneTimeLeftMs = jsonDoc["claimWinInSeconds"].as<int>() * 1000;
        } else {
            opponentGoneTimeLeftMs = 0;
        }
        opponentGoneTimeLeftWhenReceivedMs = opponentGoneTimeLeftMs;
        opponentGoneTimeReceivedMs = millis();
    }
}


void checkStreamGame() {

    if(wifiClientStream && wifiClientStream->available()) {

        deserializeJson(*jsonDocStream, *wifiClientStream);

        if(jsonDocStream && !jsonDocStream->isNull() && jsonDocStream->containsKey("type")) {
            
            ShortString type("null");
            if(jsonDocStream->containsKey("type")) {
                type = (*jsonDocStream)["type"].as<const char*>();
            }
            Serial.println((ShortString("type: ") + type).c_str());

            String jsonText;
            serializeJson(*jsonDocStream, jsonText);
            Serial.println("RECEIVED GAME STREAM EVENT");
            if(type != "null") {
                Serial.println("jsonText: " + jsonText);
            }

            if(type == "gameFull") {
                streamGame_gameFull(*jsonDocStream);
            } else if(type == "gameState") {
                streamGame_gameState(*jsonDocStream);
            } else if(type == "opponentGone") {
                streamGame_opponentGone(*jsonDocStream);
            } else {
                Serial.println("checkStreamGame: unknown type or no type.");
            }
        }
        jsonDocStream->clear();
    }
}
