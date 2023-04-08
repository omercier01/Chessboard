
#include "Chessboard.h"
#include "MainLoop_LedTest.h"
#include "Util.h"
#include "MainLoop_ReedSwitchTest.h"
#include "ReedSwitch.h"
#include "LedLoop.h"
#include "Board.h"
#include "DrawTouch.h"


int ratingDiff(ShortString before, ShortString after) {
    int beforeInt = before.asInt();
    int afterInt = after.asInt();
    return afterInt - beforeInt;
}


void checkPotentialCaptureSquareWithSentMove() {
    if(!userMoveToSend.isValid()) {
        return;
    }

    copyBoardPieces(boardCopy, serverBoardPieces);
    makeBoardMove(boardCopy, userMoveToSend, nbKnownServerMoves);
    int nbChangedSquares = getChangedSquares(modifiedSquares, boardCopy);

    // if an opponent piece has been removed, store it as candidate for capture.
    for(int i = 0; i < nbChangedSquares; i++) {
        Vector2i sqr = modifiedSquares[i];

        if(gameState == GameState::WaitingToReceiveUserMoveConfirmation) {
            // While waiting to send the move, we look for user pieces instead of opponent pieces,
            // so the opponent can prepare a capture move while the move is being sent
            if(isPlayerPiece(boardCopy, sqr) &&
               boardCopy[sqr.x][sqr.y] > 0)
            {
                potentialCapturedSquare = modifiedSquares[i];
            }
        } else {
            // if it's an opponent piece that was removed, store as potential captured piece.
            if(isOpponentPiece(boardCopy, sqr) &&
               boardCopy[sqr.x][sqr.y] > 0)
            {
                potentialCapturedSquare = modifiedSquares[i];
            }
        }
        
    }
}


void checkUpdatedSquaresForMoves() {

    // check if an opponent piece has been removed. Need to know that if the user
    // later confirms a capture move. Also check all user pieces that have moved
    // so we can light them up.
    int nbChangedSquares = getChangedSquares(modifiedSquares, serverBoardPieces);

    userMoveTentative = Move();

    // if an opponent piece has been removed, store it as candidate for capture.
    for(int i = 0; i < nbChangedSquares; i++) {
        Vector2i sqr = modifiedSquares[i];

        // if it's an opponent piece that was removed, store as potential captured piece
        if(isOpponentPiece(serverBoardPieces, sqr) &&
           serverBoardPieces[sqr.x][sqr.y] > 0)
        {
            potentialCapturedSquare = modifiedSquares[i];
        }
    }

    bool clearLeds = true;
    if(isSimpleCaptureMove(nbChangedSquares))
    {
        Vector2i playerSquare = modifiedSquares[0];

        char promotion = 0;
        if(toUpper(getPieceChar(serverBoardPieces, playerSquare)) == 'P' &&
           (potentialCapturedSquare.y == 0 || potentialCapturedSquare.y == 7))
        {
            promotion = 1;
        }

        userMoveTentative = squaresToMove(playerSquare, potentialCapturedSquare, promotion);
        Serial.print("isSimpleCaptureMove ");
        Serial.println(userMoveTentative.toString().c_str());

        takeActiveLedsMutex();
        clearActiveLeds();
        addActiveLedSquare(modifiedSquares[0]);
        addActiveLedSquare(potentialCapturedSquare);
        releaseActiveLedsMutex();

        clearLeds = false;
    }
    else if(nbChangedSquares == 2)
    {
        // simple piece move

        Vector2i playerSquare(-1, -1);
        Vector2i emptySquare(-1, -1);
        if(isPlayerPiece(serverBoardPieces, modifiedSquares[0])) {
            playerSquare = modifiedSquares[0];
            emptySquare = modifiedSquares[1];
        } else if(isPlayerPiece(serverBoardPieces, modifiedSquares[1])) {
            playerSquare = modifiedSquares[1];
            emptySquare = modifiedSquares[0];
        }

        // corner case: check if it's a en passant move where we forgot to capture the piece,
        // in which this is not a legal move
        if(abs(playerSquare.x - emptySquare.x) == 1 &&
           abs(playerSquare.y - emptySquare.y) == 1 &&
           toUpper(getPieceChar(serverBoardPieces, playerSquare)) == 'P') {
            return;
        }

        if(playerSquare.isValid() &&
           emptySquare.isValid() )
        {
            char promotion = 0;
            if(toUpper(getPieceChar(serverBoardPieces, playerSquare)) == 'P' &&
            (emptySquare.y == 0 || emptySquare.y == 7))
            {
                promotion = 1; // to be set to a piece char later
            }

            userMoveTentative = squaresToMove(playerSquare, emptySquare, promotion);

            takeActiveLedsMutex();
            clearActiveLeds();
            addActiveLedSquare(modifiedSquares[0]);
            addActiveLedSquare(modifiedSquares[1]);
            releaseActiveLedsMutex();

            clearLeds = false;
        }
    }
    else if(nbChangedSquares == 3)
    {
        // en passant

        // check that one of the pieces is an opponent pawn
        Vector2i opponentPawnSquare(-1, -1);
        for(int i = 0; i < 3; i++) {
            Vector2i sqr = modifiedSquares[i];
            char c = serverBoardPieces[sqr.x][sqr.y];
            if((c == 'p' || c == 'P') && isOpponentPiece(serverBoardPieces, sqr)) {
                opponentPawnSquare = sqr;
                break;
            }
        }

        // check that one of the pieces is a player pawn
        Vector2i playerPawnSquare(-1, -1);
        for(int i = 0; i < 3; i++) {
            Vector2i sqr = modifiedSquares[i];
            char c = serverBoardPieces[sqr.x][sqr.y];
            if((c == 'p' || c == 'P') && isPlayerPiece(serverBoardPieces, sqr)) {
                playerPawnSquare = sqr;
                break;
            }
        }

        // check that one of the squares is empty
        Vector2i emptySquare(-1, -1);
        for(int i = 0; i < 3; i++) {
            Vector2i sqr = modifiedSquares[i];
            char c = serverBoardPieces[sqr.x][sqr.y];
            if(c == 0) {
                emptySquare = sqr;
                break;
            }
        }

        if(opponentPawnSquare.isValid() &&
           playerPawnSquare.isValid() &&
           emptySquare.isValid())
        {
            // valid en passant capture

            userMoveTentative = squaresToMove(playerPawnSquare, emptySquare, 0);

            takeActiveLedsMutex();
            clearActiveLeds();
            addActiveLedSquare(modifiedSquares[0]);
            addActiveLedSquare(modifiedSquares[1]);
            addActiveLedSquare(modifiedSquares[2]);
            releaseActiveLedsMutex();

            clearLeds = false;
        }
    }
    else if(nbChangedSquares == 4)
    {
        // castle

        Vector2i playerKingSquare(-1, -1);
        for(int i = 0; i < 4; i++) {
            Vector2i sqr = modifiedSquares[i];
            char c = serverBoardPieces[sqr.x][sqr.y];
            if((c == 'k' || c == 'K') && isPlayerPiece(serverBoardPieces, sqr)) {
                playerKingSquare = sqr;
                break;
            }
        }

        Vector2i playerRookSquare(-1, -1);
        for(int i = 0; i < 4; i++) {
            Vector2i sqr = modifiedSquares[i];
            char c = serverBoardPieces[sqr.x][sqr.y];
            if((c == 'r' || c == 'R') && isPlayerPiece(serverBoardPieces, sqr)) {
                playerRookSquare = sqr;
                break;
            }
        }

        int nbEmptySquares = 0;
        for(int i = 0; i < 4; i++) {
            Vector2i sqr = modifiedSquares[i];
            char c = serverBoardPieces[sqr.x][sqr.y];
            if(c == 0) {
                nbEmptySquares++;
            }
        }

        int distKingRook = abs(playerKingSquare.x - playerRookSquare.x);

        if(playerKingSquare.isValid() &&
           playerRookSquare.isValid() &&
           nbEmptySquares == 2 &&
           (distKingRook == 3 || distKingRook == 4))
        {
            Vector2i newKingSquare = playerKingSquare;
            if(playerRookSquare.x > playerKingSquare.x) {
                newKingSquare.x += 2;
            } else {
                newKingSquare.x -= 2;
            }

            userMoveTentative = squaresToMove(playerKingSquare, newKingSquare, 0);

            takeActiveLedsMutex();
            clearActiveLeds();
            for(int i = 0; i < 4; i++) {
                addActiveLedSquare(modifiedSquares[i]);
            }
            releaseActiveLedsMutex();

            clearLeds = false;
        }
    }


    if(clearLeds) {
        takeActiveLedsMutex();
        clearActiveLeds();
        releaseActiveLedsMutex();
    }

}


char getOpponentPromotionChar(char c) {
    if(playerColor == PlayerColor::White) {
        // opponent is black
        switch(c) {
            case 'q':
                return FONT_PIECE_BLACK_Q_BLACKLAYER;
                break;
            case 'r':
                return FONT_PIECE_BLACK_R_BLACKLAYER;
                break;
            case 'b':
                return FONT_PIECE_BLACK_B_BLACKLAYER;
                break;
            case 'n':
                return FONT_PIECE_BLACK_N_BLACKLAYER;
                break;
            default:
                Serial.println("getCapitalizedOpponentPromotionChar error, c not a promotion piece");
                return 0;
                break;
        }
    } else if(playerColor == PlayerColor::Black){
        // opponent is white
        switch(c) {
            case 'q':
                return FONT_PIECE_WHITE_Q_BLACKLAYER;
                break;
            case 'r':
                return FONT_PIECE_WHITE_R_BLACKLAYER;
                break;
            case 'b':
                return FONT_PIECE_WHITE_B_BLACKLAYER;
                break;
            case 'n':
                return FONT_PIECE_WHITE_N_BLACKLAYER;
                break;
            default:
                Serial.println("getCapitalizedOpponentPromotionChar error, c not a promotion piece");
                return 0;
                break;
        }
    } else {
        Serial.println("getCapitalizedOpponentPromotionChar error, no player color");
        return 0;
    }
}


void mainLoop_Game() {

    int diff;

    if(bMainModeDirty) {
        bGameStateDirty = true;
        bMainModeDirty = false;
    }

    if(bGameStateDirty) {

        if(gameState == GameState::Error) {
            setMenuError();
            currentMenu.draw();
            bGameStateDirty = false;
            return;
        }
        
        if(notificationOpponentOffersMoveTakeback) {
            // auto refuse move takeback offers from opponent
            sendTakebackRefusal = true;
        }

        if(showGameOptionsMenu) {
            setMenuGameOptions();
            currentMenu.draw();
        } else if(notificationOpponentPromotes) {
            setMenuAcknowledgeOpponentPromotion();
            currentMenu.buttons[0].label.text = getOpponentPromotionChar(opponentMove.chars[4]);
            currentMenu.draw();
        } else if(notificationOpponentRefusesDraw) {
            setMenuAcknowledgeDrawRefusal();
            currentMenu.draw();
        } else if(notificationOpponentOffersDraw) {
            setMenuAcceptOrRefuseDraw();
            currentMenu.draw();
        } else if(notificationVictoryClaimRefused) {
            setMenuVictoryClaimRefused();
            currentMenu.draw();
        } else {
            switch(gameState) {
            case GameState::Connecting:
                setMenuConnecting();
                currentMenu.draw();
                activePlayer = Player::None;
                break;
            case GameState::NoGame:
                // in case we sent the game seek before we get the player stream
                if(currentMenuType != MenuType::GameSeekSent) {
                    setMenuNoGame();
                }
                currentMenu.draw();
                activePlayer = Player::None;
                break;
            case GameState::GameFound:
            {
                // assuming we were on MenuType::GameSeekSent menu before
                setMenuGameFound();
                ShortString playerColorStrUpper = playerColorStr;
                playerColorStrUpper.firstCharToUpperCase();
                currentMenu.titles[1].text = ShortString("Playing ") + playerColorStrUpper;
                currentMenu.titles[1].centerX();
                currentMenu.buttons[0].label.text = gameSeekOrFoundButtonText;
                currentMenu.buttons[0].label.centerX();
                currentMenu.draw();
                break;
            }
            case GameState::WaitingToReceiveOpponentMove:
            {
                bool alreadyCorrectMenuType = (currentMenuType == MenuType::MoveAndTimer);
                if(!alreadyCorrectMenuType) {
                    setMenuMoveAndTimer();
                }
                currentMenu.titles[0].text = "Opponent thinking";
                currentMenu.titles[1].text = "";
                currentMenu.centerXTitles();
                currentMenu.onBackgroundClick = userMakingMoveMenuClick;
                if(alreadyCorrectMenuType) {
                    currentMenu.drawTitles();
                } else {
                    currentMenu.draw();
                }
                activePlayer = Player::Opponent;
                break;
            }
            case GameState::UserMakingOpponentMove:
            {
                bool alreadyCorrectMenuType = (currentMenuType == MenuType::MoveAndTimer);
                if(!alreadyCorrectMenuType) {
                    setMenuMoveAndTimer();
                }
                currentMenu.titles[0].text = "Opponent played";
                currentMenu.titles[1].text = opponentMove.toString();
                currentMenu.centerXTitles();
                currentMenu.onBackgroundClick = nullptr;
                if(alreadyCorrectMenuType) {
                    currentMenu.drawTitles();
                } else {
                    currentMenu.draw();
                }
                activePlayer = Player::Player;
                break;
            }
            case GameState::UserMakingUserMove:
            {
                bool alreadyCorrectMenuType = (currentMenuType == MenuType::MoveAndTimer);
                if(!alreadyCorrectMenuType) {
                    setMenuMoveAndTimer();
                }
                if(isTwoPlayersGame) {
                    if(activePlayer == Player::Player) {
                        currentMenu.titles[0].text = "White to play";
                    } else {
                        currentMenu.titles[0].text = "Black to play";
                    }
                } else {
                    currentMenu.titles[0].text = "Make your move";
                }
                currentMenu.titles[1].text = "";
                currentMenu.centerXTitles();
                currentMenu.onBackgroundClick = userMakingMoveMenuClick;
                if(alreadyCorrectMenuType) {
                    currentMenu.drawTitles();
                } else {
                    currentMenu.draw();
                }
                //activePlayer = Player::Player;
                break;
            }
            case GameState::WaitingToReceiveUserMoveConfirmation:
            {
                bool alreadyCorrectMenuType = (currentMenuType == MenuType::MoveAndTimer);
                if(!alreadyCorrectMenuType) {
                    setMenuMoveAndTimer();
                }
                currentMenu.titles[0].text = "Sending move";
                currentMenu.titles[1].text = userMoveToSend.toString();
                currentMenu.centerXTitles();
                currentMenu.onBackgroundClick = nullptr;
                if(alreadyCorrectMenuType) {
                    currentMenu.drawTitles();
                } else {
                    currentMenu.draw();
                }
                //activePlayer = Player::Player;
                break;
            }
            case GameState::UserMakingPromotion:
                setMenuUserPromotion();
                currentMenu.draw();
                //activePlayer = Player::Player;
                break;
            case GameState::UserFixingBadBoard:
                setMenuFixBadBoard();
                currentMenu.titles[0].text = "Fix board position";
                currentMenu.centerXTitles();
                currentMenu.draw();
                break;
            case GameState::UserFixingInvalidMove:
                setMenuFixBadBoard();
                currentMenu.titles[0].text = "Invalid move, reset";
                currentMenu.centerXTitles();
                currentMenu.draw();
                break;
            case GameState::GameEnded:
                if(!isTwoPlayersGame) {
                    setMenuGameEndedRatings();
                    currentMenu.titles[0].text = gameEndedMenuTitle0;
                    currentMenu.titles[0].centerX();
                    currentMenu.titles[1].text = gameEndedMenuTitle1;
                    currentMenu.titles[1].centerX();
                    currentMenu.draw();
                } else {
                    setMenuGameEndedQrCode();
                    currentMenu.titles[0].text = gameEndedMenuTitle0;
                    currentMenu.titles[0].centerX();
                    currentMenu.titles[1].text = gameEndedMenuTitle1;
                    currentMenu.titles[1].centerX();
                    currentMenu.draw();
                }
                break;
            case GameState::GameEndedWithPlayerRatingsOrQrCode:
                if(!isTwoPlayersGame) {
                    setMenuGameEndedRatings();
                    currentMenu.titles[0].text = gameEndedMenuTitle0;
                    currentMenu.titles[0].centerX();

                    currentMenu.titles[1].text = gameEndedMenuTitle1;
                    currentMenu.titles[1].centerX();

                    currentMenu.titles[3].text = (opponentName == "" ? "CPU" : opponentName);
                    currentMenu.titles[3].centerX();

                    currentMenu.titles[4].text = opponentStartRatingStr + "  >  " + opponentEndRatingStr;
                    diff = ratingDiff(opponentStartRatingStr, opponentEndRatingStr);
                    currentMenu.titles[4].textColor = (diff > 0 ? DARKGREEN : (diff < 0 ? RED : BLACK));
                    currentMenu.titles[4].centerX();

                    currentMenu.titles[5].text = (playerName == "" ? "UNKNOWN" : playerName);
                    currentMenu.titles[5].centerX();

                    currentMenu.titles[6].text = playerStartRatingStr + "  >  " + playerEndRatingStr;
                    diff = ratingDiff(playerStartRatingStr, playerEndRatingStr);
                    currentMenu.titles[6].textColor = (diff > 0 ? DARKGREEN : (diff < 0 ? RED : BLACK));
                    currentMenu.titles[6].centerX();

                    currentMenu.draw();
                } else {
                    setMenuGameEndedQrCode();
                }
                break;
            case GameState::Error:
            default:
                // nothing
                break;
            }
        }
        bGameStateDirty = false;
    }

    // update opponent gone timer
    if(opponentIsGone) {    
        opponentGoneTimeLeftMs = opponentGoneTimeLeftWhenReceivedMs -
                                  (millis() - opponentGoneTimeReceivedMs);
        opponentGoneTimeLeftMs = max((long)0, (long)opponentGoneTimeLeftMs);

        // fix missing opponentGone=false events
        if((millis() - opponentGoneTimeReceivedMs) > forgetOpponentGoneTimerAfterNbSecs*1000) {
            opponentIsGone = false;
        }
        
    }

    // update draw timer
    if(millis() - timeLastTimerDrawUpdate > timerRefreshDrawFrequency ||
    menuTimerDirty)
    {
        if(currentMenu.updateTimer) {
            currentMenu.updateTimer(currentMenu);
        }
        if(menuTimerDirty) {
            menuTimerDirty = false;
        }

        if(gameState == GameState::WaitingToReceiveOpponentMove &&
           opponentIsGoneDrawDirty)
        {
            if(opponentIsGone &&
               !notificationOpponentRefusesDraw &&
               !notificationOpponentOffersDraw &&
               !notificationOpponentOffersMoveTakeback &&
               !notificationOpponentPromotes &&
               !notificationVictoryClaimRefused
            ) {
                ShortString newLabelStr = prettifyTime(opponentGoneTimeLeftMs);
                currentMenu.titles[1].text = ShortString("Oppon. left: ")  + newLabelStr;
                currentMenu.titles[1].centerX();
            } else if(!notificationOpponentRefusesDraw &&
                      !notificationOpponentOffersDraw &&
                      !notificationOpponentOffersMoveTakeback &&
                      !notificationOpponentPromotes &&
                      !notificationVictoryClaimRefused
            ){
                currentMenu.titles[1].text = ShortString("");
                opponentIsGoneDrawDirty = false;
            }
            currentMenu.drawTitles();
        }

        timeLastTimerDrawUpdate = millis();
    }

    if(playerTimeLeftReceived != UNLIMITED_TIME) {
        // update timer
        if(nbKnownServerMoves >= 2) {
            if(activePlayer == Player::Player) {
                playerTimeLeft = playerTimeLeftReceived - (millis() - timeLastTimerReceived);
                playerTimeLeft = (playerTimeLeft >= 0 ? playerTimeLeft : 0);
            } else if(activePlayer == Player::Opponent) {
                opponentTimeLeft = opponentTimeLeftReceived - (millis() - timeLastTimerReceived);
                opponentTimeLeft = (opponentTimeLeft >= 0 ? opponentTimeLeft : 0);
            }
        }
    }

    if(isTwoPlayersGame && gameState == GameState::WaitingToReceiveUserMoveConfirmation) {
        // check for potential move captures if the opponent is making a
        // capture while we're sending the previous move
        checkPotentialCaptureSquareWithSentMove();
    }

    // Check switches for move inputs
    if(gameState == GameState::UserMakingOpponentMove) {
        if(boardPiecesMatchSwitches(serverBoardPieces)) {
            gameState = GameState::UserMakingUserMove;
            bGameStateDirty = true;
        }
    } else if(gameState == GameState::UserMakingUserMove) {
        checkUpdatedSquaresForMoves();
    } else if(gameState == GameState::UserFixingBadBoard ||
              gameState == GameState::UserFixingInvalidMove)
    {
        int nbChangedSquares = getChangedSquares(modifiedSquares, serverBoardPieces);

        if(nbChangedSquares == 0) {

            takeActiveLedsMutex();
            clearActiveLeds();
            releaseActiveLedsMutex();

            if(nextMoveIsUserMove(nbKnownServerMoves)) {
                gameState = GameState::UserMakingUserMove;
                activePlayer = Player::Player;
            } else if(isTwoPlayersGame) {
                gameState = GameState::UserMakingUserMove;
                activePlayer = Player::Opponent;
            } else {
                gameState = GameState::WaitingToReceiveOpponentMove;
            }
            potentialCapturedSquare = Vector2i(-1, -1);
            bGameStateDirty = true;
        } else {
            takeActiveLedsMutex();
            clearActiveLeds();
            for(int iSquare = 0; iSquare < nbChangedSquares; iSquare++) {
                addActiveLedSquare(modifiedSquares[iSquare]);
            }
            releaseActiveLedsMutex();
        }
    } else if(gameState == GameState::GameEnded ||
              gameState == GameState::GameEndedWithPlayerRatingsOrQrCode)
    {
        // case where we get checkmated and we're making the final opponent checkmate move
        if(boardPiecesMatchSwitches(serverBoardPieces)) {
            takeActiveLedsMutex();
            clearActiveLeds();
            releaseActiveLedsMutex();
        }
    }



    // update switches (one quad group per frame)
    static int switchGroupTick = 0;
    int switchGroupX = (switchGroupTick/1) % 8;
    int switchGroupY = (switchGroupTick/8) % 2;
    static bool squareQuad[4];
    checkSwitchQuad(squareQuad, switchGroupX, switchGroupY);

    int yOffset = (switchGroupY == 0 ? 0 : 4);
    for(int i = 0; i < 4; i++) {
        clientBoardBoolTempWrite[switchGroupX][i+yOffset] = squareQuad[i];
    }
    switchGroupTick++;
    if(switchGroupTick % 16 == 0) {
        // update readable data
        takeBoardBoolMutex();
        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 8; j++) {
                clientBoardBool[i][j] = clientBoardBoolTempWrite[i][j];
            }
        }
        releaseBoardBoolMutex();
    }
    
    Vector2 touchPos;
    if(IsDisplayTouchedTimeBuffer(touchPos, true)) {
        currentMenu.onClick(touchPos);
    }

}



