
#include "Chessboard.h"
#include "Menus.h"
#include "DrawTouch.h"
#include "LedLoop.h"
#include "Board.h"

void MenuLabel::draw() {
    DisplaySetFont(&font);
    DisplaySetTextSize(textSize);
    DisplaySetTextColor(textColor);
    DisplaySetCursor(pos.x, pos.y);
    DisplayPrintString(text.c_str());
}

void MenuLabel::centerX(int iDivX, int nbDivX) {
    float labelWidth = (DISPLAY_PIXELS_WIDTH-(nbDivX+1)*MENU_BORDER_WIDTH)/nbDivX;
    pos.x = int((iDivX+1) * MENU_BORDER_WIDTH
          + (iDivX+0.5) * labelWidth)
          + textSize*getCenteredLabelPosX(text, font);
}

void MenuLabel::rightAlignX(int iDivX, int nbDivX) {
    float labelWidth = (DISPLAY_PIXELS_WIDTH-(nbDivX+1)*MENU_BORDER_WIDTH)/nbDivX;
    pos.x = int((iDivX+1) * MENU_BORDER_WIDTH
          + (iDivX+1) * labelWidth)
          + textSize*getRightAlignedLabelPosX(text, font);
}

void MenuButton::draw() {
    if(fill) {
        DisplayFillRect(minPos.x, minPos.y, width.x, width.y, color);
    } else {
        DisplayDrawRect(minPos.x, minPos.y, width.x, width.y, color);
    }
    label.draw();
}


void Menu::addTitle(ShortString str, uint16_t color) {
    if(nbTitles == MAX_NB_MENU_TITLES) {
        Serial.println("Can't add more titles to menu.");
        return;
    }

    Vector2 pos(0, (nbTitles + 1)*titleTextSize*titleFont.yAdvance + 1); // x=0 but recentered later
    titles[nbTitles] = MenuLabel(str, pos, titleFont, titleTextSize, color);
    titles[nbTitles].centerX();

    nbTitles++;
}

void Menu::addButton(ShortString str, uint16_t labelColor, uint16_t buttonColor, bool fill, void(*onClick)()) {
    if(nbButtons == MAX_NB_MENU_BUTTONS) {
        Serial.println("Can't add more buttons to menu.");
        return;
    }

    int nbButtonFirst = buttonConfig.x*buttonConfig.y;
    bool inSecondSet = (nbButtons >= nbButtonFirst);

    float borderWidthTitle = nbTitles*titleFont.yAdvance;

    Vector2 width;
    int configYTotal = buttonConfig.y + buttonConfigSecond.y;
    if(!inSecondSet) {
        width.x = (DISPLAY_PIXELS_WIDTH - (buttonConfig.x+1)*MENU_BORDER_WIDTH) / buttonConfig.x;
    } else {
        width.x = (DISPLAY_PIXELS_WIDTH - (buttonConfigSecond.x+1)*MENU_BORDER_WIDTH) / buttonConfigSecond.x;
    }
    width.y = (DISPLAY_PIXELS_HEIGHT - borderWidthTitle - (configYTotal+1)*MENU_BORDER_WIDTH) / configYTotal;
    
    Vector2 minPos;
    int idPosX;
    int idPosY;

    if(!inSecondSet) {
        idPosX = nbButtons % buttonConfig.x;
        idPosY = nbButtons / buttonConfig.x;
        
        minPos.x = MENU_BORDER_WIDTH +
                    idPosX*(width.x+MENU_BORDER_WIDTH);
        minPos.y = borderWidthTitle +
                MENU_BORDER_WIDTH + 
                idPosY*(width.y+MENU_BORDER_WIDTH);
    } else {
        idPosX = (nbButtons - nbButtonFirst) % buttonConfigSecond.x;
        idPosY = buttonConfig.y + (nbButtons - nbButtonFirst) / buttonConfigSecond.x;
        
        minPos.x = MENU_BORDER_WIDTH +
                    idPosX*(width.x+MENU_BORDER_WIDTH);
        minPos.y = borderWidthTitle +
                MENU_BORDER_WIDTH + 
                idPosY*(width.y+MENU_BORDER_WIDTH);
    }

    Vector2 labelPos(0, minPos.y + width.y/2 + buttonTextSize*buttonFontHeight/2 + 1);
    MenuLabel label(str, labelPos, buttonFont, buttonTextSize, labelColor);

    if(!inSecondSet) {
        label.centerX(idPosX, buttonConfig.x);
    } else {
        label.centerX(idPosX, buttonConfigSecond.x);
    }

    buttons[nbButtons] = MenuButton(label, minPos, width, buttonColor, fill, onClick);
    nbButtons++;
}


void Menu::drawTitles() {
    // clear titles background
    int titleMaxHeight;
    if(nbButtons > 0) {
        titleMaxHeight = buttons[0].minPos.y - 1;
    } else {
        titleMaxHeight = DISPLAY_PIXELS_HEIGHT / 4; // rough estimate, just so we don't clear the whole screen.
    }
    DisplayFillRect(0, 0, DISPLAY_PIXELS_WIDTH, titleMaxHeight, backgroundColor);

    for(int i = 0; i < nbTitles; i++) {
        titles[i].draw();
    }
}

void Menu::drawButtons() {
    for(int i = 0; i < nbButtons; i++) {
        buttons[i].draw();
    }
}


void Menu::draw() {
    DisplayFillScreen(backgroundColor);
    drawTitles();
    drawButtons();
    if(extraDrawFunct) {
        extraDrawFunct(*this);
    }
}


void Menu::onClick(Vector2 pos) {
    bool clicked = false;
    for(int iButton = 0; iButton < nbButtons; iButton++) {
        MenuButton& button = buttons[iButton];

        if(button.onClick && isWithinRect(pos, button.minPos, button.width)) {
            clickedButtonLabelText = button.label.text;
            button.onClick();
            clicked = true;
            break;
        }
    }
    if(!clicked && onBackgroundClick) {
        onBackgroundClick();
    }
}


void Menu::centerXTitles() {
    for(int i = 0; i < nbTitles; i++) {
        titles[i].centerX();
    }
}





void onResign() {
    sendResign = true;
    showGameOptionsMenu = false;
    bGameStateDirty = true;
}

void onOfferDraw() {
    sendAcceptDraw = true;
    showGameOptionsMenu = false;
    bGameStateDirty = true;
}

void onClaimVictory() {
    sendClaimVictory = true;
    showGameOptionsMenu = false;
    bGameStateDirty = true;
}

void reedSwitchTestExtraDraw(Menu& /*menu*/) {
    drawBoardBool(clientBoardBool);
}

void menuDrawBoardChar(Menu& /*menu*/) {
    drawBoardChar(serverBoardPieces);
}


void prepareToSendPromotion() {
    if(userMoveTentative.chars[4] != 0) {
        userMoveToSend = userMoveTentative;
        userMoveTentative = Move();

        gameState = GameState::WaitingToReceiveUserMoveConfirmation;
        bGameStateDirty = true;

        potentialCapturedSquare = Vector2i(-1, -1);
        
        takeActiveLedsMutex();
        clearActiveLeds();
        releaseActiveLedsMutex();
    }
}

void promoteToQ() {
    userMoveTentative.chars[4] = 'q';
    prepareToSendPromotion();
}
void promoteToR() {
    userMoveTentative.chars[4] = 'r';
    prepareToSendPromotion();
}
void promoteToN() {
    userMoveTentative.chars[4] = 'n';
    prepareToSendPromotion();
}
void promoteToB() {
    userMoveTentative.chars[4] = 'b';
    prepareToSendPromotion();
}

void userMakingMoveMenuClick() {
    if(userMoveTentative.isValid()) {
        if(userMoveTentative.chars[4] == 1) {
            gameState = GameState::UserMakingPromotion;
            bGameStateDirty = true;
        } else {

            userMoveToSend = userMoveTentative;
            userMoveTentative = Move();
            gameState = GameState::WaitingToReceiveUserMoveConfirmation;
            bGameStateDirty = true;

            potentialCapturedSquare = Vector2i(-1, -1);
            
            takeActiveLedsMutex();
            clearActiveLeds();
            releaseActiveLedsMutex();
        }
    } else {
        int nbChangedSquares = getChangedSquares(modifiedSquares, serverBoardPieces);

        if(nbChangedSquares == 0) {
            showGameOptionsMenu = true;
            bGameStateDirty = true;
        } else {
            gameState = GameState::UserFixingInvalidMove;
            bGameStateDirty = true;
        }
    }
}

void addMenuTimer(Menu& menu) {
    menu.buttonFont = FreeSansBold18pt7b;
    menu.buttonTextSize = 2;
    menu.buttonFontHeight = getFontHeightChar(menu.buttonFont, '0');
    menu.buttonConfig = Vector2i(1,2);
    if(playerColor == PlayerColor::White) {
        menu.addButton("00:00", WHITE, BLACK, true, nullptr);
        menu.addButton("00:00", BLACK, BLACK, false, nullptr);
    } else if(playerColor == PlayerColor::Black) {
        menu.addButton("00:00", BLACK, BLACK, false, nullptr);
        menu.addButton("00:00", WHITE, BLACK, true, nullptr);
    } else {
        Serial.println("addMenuTimer(): playerColor == None !");
    }
}


void redrawTitlesAndButtons(Menu& menu) {
    for(int i = 0; i < menu.nbTitles; i++) {
        menu.titles[i].draw();
    }
    for(int i = 0; i < menu.nbButtons; i++) {
        menu.buttons[i].draw();
    }
    if(menu.extraDrawFunct) {
        menu.extraDrawFunct(menu);
    }
}


ShortString prettifyTime(unsigned long timeMs) {
    if(timeMs == UNLIMITED_TIME) {
        return ShortString("UNL.");
    }

    unsigned int s = (timeMs/1000) % 60;
    unsigned int m = timeMs/1000/60;
    char buffer[12];
    sprintf(buffer, "%02u:%02u", m, s);
    return ShortString(buffer);
}


void updateDrawTimer(Menu& menu) {
    for(int i = 0; i < menu.nbButtons; i++) {
        MenuButton& button = menu.buttons[i];
        ShortString newLabelStr = prettifyTime(i ==0 ? opponentTimeLeft : playerTimeLeft);
        if(button.label.text != newLabelStr) {
            ShortString oldLabelStr = button.label.text;
            button.label.text = newLabelStr;
            button.label.centerX();

            // clear button background
            int color = (button.fill ? button.color : menu.backgroundColor);
            int xEnd = button.minPos.x + button.width.x - timerButtonClearFillRectMarginX;
            
            // find first character that changes, accumulating the x pos along the way
            int xStart = button.label.pos.x;
            if(oldLabelStr.length() != newLabelStr.length()) {
                xStart = button.minPos.x + timerButtonClearFillRectMarginX;
            } else {
                for(unsigned int ic = 0; ic < newLabelStr.length(); ic++) {
                    if(newLabelStr.charAt(ic) != oldLabelStr.charAt(ic)) {
                        break;
                    }
                    int glyphId = newLabelStr.charAt(ic) - menu.buttonFont.first;
                    xStart += button.label.font.glyph[glyphId].xAdvance * button.label.textSize;
                }
            }

            DisplayFillRect(xStart,
                               button.minPos.y + timerButtonClearFillRectMarginY,
                               xEnd - xStart, 
                               button.width.y - 2*timerButtonClearFillRectMarginY,
                               color);

            button.label.draw();
        }
    }
}


void goBackMainToNoGameMenu() {

    drawOfferFromPlayer = false;
    drawOfferFromOpponent = false;
    takebackOfferFromPlayer = false;
    takebackOfferFromOpponent = false;
    qrCodeUrl = "";
    opponentIsGoneDrawDirty = false;

    notificationOpponentRefusesDraw = false;
    notificationOpponentOffersDraw = false;
    notificationOpponentOffersMoveTakeback = false;
    notificationOpponentPromotes = false;
    notificationVictoryClaimRefused = false;

    gameOngoing = false;

    takeActiveLedsMutex();
    clearActiveLeds();
    releaseActiveLedsMutex();

    gameState = GameState::NoGame;
    bGameStateDirty = true;
    streamsDirty = true; // this will cause it to fetch the player stream again
}


void acceptDraw() {
    sendAcceptDraw = true;
    notificationOpponentOffersDraw = false;
    bGameStateDirty = true;
}

void refuseDraw() {
    sendRefuseDraw = true;
    notificationOpponentOffersDraw = false;
    bGameStateDirty = true;
}

void acknowledgeDrawRefusal() {
    // Our draw offer was refused by the opponent
    notificationOpponentRefusesDraw = false;
    bGameStateDirty = true;
}

void acknowledgeVictoryClaimRefusal() {
    notificationVictoryClaimRefused = false;
    bGameStateDirty = true;
}

void acknowledgeOpponentPromotion() {
    notificationOpponentPromotes = false;
    bGameStateDirty = true;
}

void onOptionsMenuBack() {
    showGameOptionsMenu = false;
    bGameStateDirty = true;
}



void startGameSeek(bool twoPlayers, bool vsCpu, bool rated, int time, int increment) {
    gameSeekInfo = GameSeekInfo(true, twoPlayers, vsCpu, rated, time, increment);
}


void finalizeGameStartMenu() {
    currentMenu.buttons[0].label.text = gameSeekOrFoundButtonText;
    currentMenu.buttons[0].label.centerX();
    currentMenu.draw();
}

void onStartGame10_0() {
    isTwoPlayersGame = false;
    setMenuGameSeekSent();
    gameSeekOrFoundButtonText = "10+0";
    finalizeGameStartMenu();
    startGameSeek(false, false, true, 10, 0);
}

void onStartGame10_5() {
    isTwoPlayersGame = false;
    setMenuGameSeekSent();
    gameSeekOrFoundButtonText = "10+5";
    finalizeGameStartMenu();
    startGameSeek(false, false, true, 10, 5);
}

void onStartGame15_10() {
    isTwoPlayersGame = false;
    setMenuGameSeekSent();
    gameSeekOrFoundButtonText = "15+10";
    finalizeGameStartMenu();
    startGameSeek(false, false, true, 15, 10);
}

void onStartGame30_0() {
    isTwoPlayersGame = false;
    setMenuGameSeekSent();
    gameSeekOrFoundButtonText = "30+0";
    finalizeGameStartMenu();
    startGameSeek(false, false, true, 30, 0);
}

void onStartGame30_20() {
    isTwoPlayersGame = false;
    setMenuGameSeekSent();
    gameSeekOrFoundButtonText = "30+20";
    finalizeGameStartMenu();
    startGameSeek(false, false, true, 30, 20);
}

void onStartGameCPU_1() {
    isTwoPlayersGame = false;
    setMenuGameSeekSent();
    gameSeekOrFoundButtonText = "CPU 1";
    finalizeGameStartMenu();
    startGameSeek(false, true, false, 0, 0);
}

void onStartGameTwoPayers() {
    isTwoPlayersGame = true;
    setMenuGameSeekSent();
    gameSeekOrFoundButtonText = "2 OTB";
    finalizeGameStartMenu();
    startGameSeek(true, false, false, twoPlayersTimePerSide, twoPlayersIncrement);
}


void onSetKeyboard1() {
    setMenuKeyboard1();
    currentMenu.draw();
}
void onSetKeyboard2() {
    setMenuKeyboard2();
    currentMenu.draw();
}
void onSetKeyboard3() {
    setMenuKeyboard3();
    currentMenu.draw();
}
void onSetKeyboard4() {
    setMenuKeyboard4();
    currentMenu.draw();
}


void setMenuGameOptions() {
    currentMenuType = MenuType::GameOptions;
    currentMenu = Menu();
    currentMenu.addTitle("Game Options", BLACK);
    currentMenu.buttonConfig = Vector2i(1,4);
    if(!isTwoPlayersGame) {
        currentMenu.addButton("Resign", BLACK, BLACK, false, onResign);
        currentMenu.addButton("Offer Draw", BLACK, BLACK, false, onOfferDraw);
        currentMenu.addButton("Claim Victory", BLACK, BLACK, false, onClaimVictory);
        currentMenu.addButton("Back", WHITE, BLACK, true, onOptionsMenuBack);
    } else {
        currentMenu.addButton("Resign", BLACK, BLACK, false, onResign);
        currentMenu.addButton("Draw", BLACK, BLACK, false, onOfferDraw);
        currentMenu.addButton("", WHITE, WHITE, false, nullptr);
        currentMenu.addButton("Back", WHITE, BLACK, true, onOptionsMenuBack);
    }
}

void setMenuReedSwitchTest() {
    currentMenuType = MenuType::ReedSwitchTest;
    currentMenu = Menu();
    currentMenu.addTitle("Testing reed switches", BLACK);
    currentMenu.extraDrawFunct = reedSwitchTestExtraDraw;
}

void setMenuFixBadBoard() {
    currentMenuType = MenuType::FixBadBoard;
    currentMenu = Menu();
    currentMenu.addTitle("Fix board position", BLACK);
    currentMenu.extraDrawFunct = menuDrawBoardChar;
}

void setMenuConnecting() {
    currentMenuType = MenuType::Connecting;
    currentMenu = Menu();
    currentMenu.addTitle("Connecting to Wifi", BLACK);
    ShortString shortWifiName(wifiName, ShortString::Alignment::Center);
    currentMenu.addTitle(shortWifiName, BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.addTitle("Press Side Button", BLACK);
    currentMenu.addTitle("for Setup", BLACK);
}

void setMenuNoGame() {
    currentMenuType = MenuType::NoGame;
    currentMenu = Menu();
    currentMenu.addTitle("Looking for Game", BLACK);
    currentMenu.buttonConfig = Vector2i(2,3);
    currentMenu.buttonConfigSecond = Vector2i(1,1);

    // first button group
    currentMenu.addButton("10+0", BLACK, BLACK, false, onStartGame10_0);
    currentMenu.addButton("10+5", BLACK, BLACK, false, onStartGame10_5);
    currentMenu.addButton("15+10", BLACK, BLACK, false, onStartGame15_10);
    currentMenu.addButton("30+0", BLACK, BLACK, false, onStartGame30_0);
    currentMenu.addButton("30+20", BLACK, BLACK, false, onStartGame30_20);
    currentMenu.addButton("CPU L1", BLACK, BLACK, false, onStartGameCPU_1);

    // second button group
    currentMenu.addButton("2 PLAYERS OTB", WHITE, BLACK, true, setMenuTwoPlayers);

    currentMenu.draw();
}

int clamp(int x, int a, int b) {
    return max(min(x, b), a);
}

ShortString getTimePerSideString() {
    if(twoPlayersTimePerSide == 0) {
        return ShortString("Time per side: UNL.");
    }
    return ShortString("Time per side: ") +
           ShortString(twoPlayersTimePerSide) +
           ShortString(" min");
}

ShortString getIncrementString() {
    if(twoPlayersTimePerSide == 0) {
        return ShortString("");
    }
    return ShortString("Increment: ") +
           ShortString(twoPlayersIncrement) +
           ShortString(" sec");
}

void SaveTimePerSide() {
    PersistentParamBegin();
    PersistentParamSaveInt(PersistentParamType::TwoPlayersTimePerSide, twoPlayersTimePerSide);
    PersistentParamEnd();
    currentMenu.titles[1].text = getTimePerSideString();
    currentMenu.titles[1].centerX();
    currentMenu.titles[2].text = getIncrementString();
    currentMenu.titles[2].centerX();
    currentMenu.draw();
}

void clampAndSaveTimePerSide() {
    twoPlayersTimePerSide = clamp(twoPlayersTimePerSide,
                                  twoPlayersTimePerSideMin,
                                  twoPlayersTimePerSideMax);
    SaveTimePerSide();
}

void clampAndSaveIncrement() {
    twoPlayersIncrement = clamp(twoPlayersIncrement,
                                twoPlayersIncrementMin,
                                twoPlayersIncrementMax);
    PersistentParamBegin();
    PersistentParamSaveInt(PersistentParamType::TwoPlayersIncrement, twoPlayersIncrement);
    PersistentParamEnd();
    currentMenu.titles[1].text = getTimePerSideString();
    currentMenu.titles[1].centerX();
    currentMenu.titles[2].text = getIncrementString();
    currentMenu.titles[2].centerX();
    currentMenu.draw();
}

void twoPlayersTimeMinusSmall() {
    if(twoPlayersTimePerSide == 0) {
        twoPlayersTimePerSide = twoPlayersTimePerSideMax;
        SaveTimePerSide();
    } else if(twoPlayersTimePerSide == twoPlayersTimePerSideMin) {
        twoPlayersTimePerSide = 0;
        SaveTimePerSide();
    } else {
        twoPlayersTimePerSide -= twoPlayersTimeDeltaSmall;
        clampAndSaveTimePerSide();
    }
}

void twoPlayersTimePlusSmall() {
    if(twoPlayersTimePerSide == 0) {
        twoPlayersTimePerSide = twoPlayersTimePerSideMin;
        SaveTimePerSide();
    } else if(twoPlayersTimePerSide == twoPlayersTimePerSideMax) {
        twoPlayersTimePerSide = 0;
        SaveTimePerSide();
    } else {
        twoPlayersTimePerSide += twoPlayersTimeDeltaSmall;
        clampAndSaveTimePerSide();
    }
}

void twoPlayersTimeMinusLarge() {
    if(twoPlayersTimePerSide == 0) {
        twoPlayersTimePerSide = twoPlayersTimePerSideMax;
        SaveTimePerSide();
    } else if(twoPlayersTimePerSide == twoPlayersTimePerSideMin) {
        twoPlayersTimePerSide = 0;
        SaveTimePerSide();
    } else {
        twoPlayersTimePerSide -= twoPlayersTimeDeltaLarge;
        clampAndSaveTimePerSide();
    }
}

void twoPlayersTimePlusLarge() {
    if(twoPlayersTimePerSide == 0) {
        twoPlayersTimePerSide = twoPlayersTimePerSideMin;
        SaveTimePerSide();
    } else if(twoPlayersTimePerSide == twoPlayersTimePerSideMax) {
        twoPlayersTimePerSide = 0;
        SaveTimePerSide();
    } else {
        twoPlayersTimePerSide += twoPlayersTimeDeltaLarge;
        clampAndSaveTimePerSide();
    }
}

void twoPlayersIncrementMinusSmall() {
    twoPlayersIncrement -= twoPlayersTimeDeltaSmall;
    clampAndSaveIncrement();
}

void twoPlayersIncrementPlusSmall() {
    twoPlayersIncrement += twoPlayersTimeDeltaSmall;
    clampAndSaveIncrement();
}

void twoPlayersIncrementMinusLarge() {
    twoPlayersIncrement -= twoPlayersTimeDeltaLarge;
    clampAndSaveIncrement();
}

void twoPlayersIncrementPlusLarge() {
    twoPlayersIncrement += twoPlayersTimeDeltaLarge;
    clampAndSaveIncrement();
}

void setMenuTwoPlayers() {
    currentMenuType = MenuType::NoGame;
    currentMenu = Menu();
    currentMenu.addTitle("Two Players OTB", BLACK);
    currentMenu.addTitle(getTimePerSideString().c_str(), BLACK);
    currentMenu.addTitle(getIncrementString().c_str(), BLACK);
    currentMenu.buttonConfig = Vector2i(4,3);
    currentMenu.buttonConfigSecond = Vector2i(1,2);

    // first group
    currentMenu.addButton("<<", BLACK, BLACK, false, twoPlayersTimeMinusLarge);
    currentMenu.addButton("<", BLACK, BLACK, false, twoPlayersTimeMinusSmall);
    currentMenu.addButton(">", BLACK, BLACK, false, twoPlayersTimePlusSmall);
    currentMenu.addButton(">>", BLACK, BLACK, false, twoPlayersTimePlusLarge);
    currentMenu.addButton("<<", BLACK, BLACK, false, twoPlayersIncrementMinusLarge);
    currentMenu.addButton("<", BLACK, BLACK, false, twoPlayersIncrementMinusSmall);
    currentMenu.addButton(">", BLACK, BLACK, false, twoPlayersIncrementPlusSmall);
    currentMenu.addButton(">>", BLACK, BLACK, false, twoPlayersIncrementPlusLarge);
    currentMenu.addButton("", BLACK, WHITE, false, nullptr);
    currentMenu.addButton("", BLACK, WHITE, false, nullptr);
    currentMenu.addButton("", BLACK, WHITE, false, nullptr);
    currentMenu.addButton("", BLACK, WHITE, false, nullptr);
    
    // second group
    currentMenu.addButton("START", BLACK, BLACK, false, onStartGameTwoPayers);
    currentMenu.addButton("BACK", WHITE, BLACK, true, setMenuNoGame);

    currentMenu.draw();
}

void setMenuMoveAndTimer() {
    currentMenuType = MenuType::MoveAndTimer;
    currentMenu = Menu();
    currentMenu.addTitle("", BLACK);
    currentMenu.addTitle("", BLACK);
    addMenuTimer(currentMenu);
    currentMenu.updateTimer = updateDrawTimer;
}

void setMenuUserPromotion() {
    currentMenuType = MenuType::UserPromotion;
    currentMenu = Menu();
    currentMenu.addTitle("Promote to", BLACK);
    currentMenu.buttonFont = ChessFont25;
    currentMenu.buttonTextSize = 4;
    currentMenu.buttonFontHeight = ChessFont25.yAdvance;
    currentMenu.buttonConfig = Vector2i(2,2);
    if(playerColor == PlayerColor::White) {
        currentMenu.addButton(ShortString(char(FONT_PIECE_WHITE_Q_BLACKLAYER)),
                                    BLACK, BLACK, false, promoteToQ);
        currentMenu.addButton(ShortString(char(FONT_PIECE_WHITE_N_BLACKLAYER)),
                                    BLACK, BLACK, false, promoteToN);
        currentMenu.addButton(ShortString(char(FONT_PIECE_WHITE_R_BLACKLAYER)),
                                    BLACK, BLACK, false, promoteToR);
        currentMenu.addButton(ShortString(char(FONT_PIECE_WHITE_B_BLACKLAYER)),
                                    BLACK, BLACK, false, promoteToB);
    } else if(playerColor == PlayerColor::Black) {
        currentMenu.addButton(ShortString(char(FONT_PIECE_BLACK_Q_BLACKLAYER)),
                                    BLACK, BLACK, false, promoteToQ);
        currentMenu.addButton(ShortString(char(FONT_PIECE_BLACK_N_BLACKLAYER)),
                                    BLACK, BLACK, false, promoteToN);
        currentMenu.addButton(ShortString(char(FONT_PIECE_BLACK_R_BLACKLAYER)),
                                    BLACK, BLACK, false, promoteToR);
        currentMenu.addButton(ShortString(char(FONT_PIECE_BLACK_B_BLACKLAYER)),
                                    BLACK, BLACK, false, promoteToB);
    } else {
        Serial.println("userPromotionMenu: playerColor == None !");
    }
}

void setMenuGameEndedRatings() {
    notificationOpponentRefusesDraw = false;
    notificationOpponentOffersDraw = false;
    notificationOpponentOffersMoveTakeback = false;
    notificationOpponentPromotes = false;
    notificationVictoryClaimRefused = false;

    currentMenuType = MenuType::GameEndedRatings;
    currentMenu = Menu();
    currentMenu.addTitle("Game Ended", BLACK);
    currentMenu.addTitle("Reason", BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.addTitle("Fetching ratings...", BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.buttonConfig = Vector2i(1,1);
    currentMenu.buttonFont = FreeSansBold18pt7b;
    currentMenu.buttonTextSize = 2;
    currentMenu.buttonFontHeight = getFontHeightChar(currentMenu.buttonFont, '0');
    currentMenu.addButton("OK", WHITE, BLACK, true, goBackMainToNoGameMenu);
    currentMenu.draw();
}


void drawQrCode(Menu& /*menu*/) {
    if(qrCodeUrl != "") {

        Vector2i qrCodeTopLeft;
        qrCodeTopLeft.x = DISPLAY_PIXELS_WIDTH/2 - (qrCode.size*qrCodeDrawScale/2);
        qrCodeTopLeft.y = 80;

        for(int y = 0; y < qrCode.size; y++) {
            for(int x = 0; x < qrCode.size; x++) {
                DisplayFillRect(
                    qrCodeTopLeft.x + x*qrCodeDrawScale,
                    qrCodeTopLeft.y + y*qrCodeDrawScale,
                    qrCodeDrawScale,
                    qrCodeDrawScale,
                    (qrcode_getModule(&qrCode, x, y) ? BLACK : WHITE)
                );
            }
        }
    }
}

void setMenuGameEndedQrCode() {
    currentMenuType = MenuType::GameEndedQrCode;
    currentMenu = Menu();
    currentMenu.addTitle("Game Ended", BLACK);
    currentMenu.addTitle("Reason", BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.addTitle("Getting QR Code...", BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.addTitle("", BLACK);
    currentMenu.buttonConfig = Vector2i(1,1);
    currentMenu.buttonFont = FreeSansBold18pt7b;
    currentMenu.buttonTextSize = 2;
    currentMenu.buttonFontHeight = getFontHeightChar(currentMenu.buttonFont, '0');
    currentMenu.addButton("OK", WHITE, BLACK, true, goBackMainToNoGameMenu);
    currentMenu.extraDrawFunct = drawQrCode;
}


void setMenuAcceptOrRefuseDraw() {
    currentMenuType = MenuType::AcceptOrRefuseDraw;
    currentMenu = Menu();
    currentMenu.addTitle("Opponent Offers Draw", BLACK);
    currentMenu.buttonConfig = Vector2i(1,2);
    currentMenu.buttonFont = FreeSansBold18pt7b;
    currentMenu.buttonTextSize = 1;
    currentMenu.buttonFontHeight = getFontHeightChar(currentMenu.buttonFont, '0');
    currentMenu.addButton("ACCEPT", WHITE, BLACK, true, acceptDraw);
    currentMenu.addButton("REFUSE", WHITE, BLACK, true, refuseDraw);
}

void setMenuVictoryClaimRefused() {
    currentMenuType = MenuType::VictoryClaimRefused;
    currentMenu = Menu();
    currentMenu.addTitle("Victory Claim", BLACK);
    currentMenu.addTitle("Refused", BLACK);
    currentMenu.buttonConfig = Vector2i(1,1);
    currentMenu.buttonFont = FreeSansBold18pt7b;
    currentMenu.buttonTextSize = 2;
    currentMenu.buttonFontHeight = getFontHeightChar(currentMenu.buttonFont, '0');
    currentMenu.addButton("OK", WHITE, BLACK, true, acknowledgeVictoryClaimRefusal);
}

void setMenuAcknowledgeDrawRefusal() {
    currentMenuType = MenuType::AcknowledgeDrawRefusal;
    currentMenu = Menu();
    currentMenu.addTitle("Draw Offer Refused", BLACK);
    currentMenu.buttonConfig = Vector2i(1,1);
    currentMenu.buttonFont = FreeSansBold18pt7b;
    currentMenu.buttonTextSize = 2;
    currentMenu.buttonFontHeight = getFontHeightChar(currentMenu.buttonFont, '0');
    currentMenu.addButton("OK", WHITE, BLACK, true, nullptr);
    currentMenu.onBackgroundClick = acknowledgeDrawRefusal;
}

void setMenuAcknowledgeOpponentPromotion() {
    currentMenuType = MenuType::AcknowledgeOpponentPromotion;
    currentMenu = Menu();
    currentMenu.addTitle("Opponent Promotes to", BLACK);
    currentMenu.buttonConfig = Vector2i(1,2);
    currentMenu.buttonFont = ChessFont25;
    currentMenu.buttonTextSize = 4;
    currentMenu.buttonFontHeight = ChessFont25.yAdvance;
    currentMenu.addButton(ShortString(char(FONT_PIECE_WHITE_Q_BLACKLAYER)), BLACK, BLACK, false, nullptr);
    currentMenu.buttonFont = FreeSansBold18pt7b;
    currentMenu.buttonTextSize = 2;
    currentMenu.buttonFontHeight = getFontHeightChar(currentMenu.buttonFont, '0');
    currentMenu.addButton("OK", WHITE, BLACK, true, nullptr);
    currentMenu.onBackgroundClick = acknowledgeOpponentPromotion;
}

void setMenuGameSeekSent() {
    currentMenuType = MenuType::GameSeekSent;
    currentMenu = Menu();
    if(isTwoPlayersGame) {
        currentMenu.addTitle("Starting Game", BLACK);
    } else {
        currentMenu.addTitle("Game Seek Sent", BLACK);
    }
    currentMenu.addTitle("", BLACK);
    currentMenu.buttonConfig = Vector2i(1,1);
    currentMenu.buttonFont = FreeSansBold18pt7b;
    currentMenu.buttonTextSize = 2;
    currentMenu.buttonFontHeight = getFontHeightChar(currentMenu.buttonFont, '0');
    currentMenu.addButton("GAME TYPE", BLACK, WHITE, false, nullptr);
}

void setMenuGameFound() {
    currentMenuType = MenuType::GameFound;
    currentMenu = Menu();
    currentMenu.addTitle("Game Found", BLACK);
    currentMenu.addTitle("Playing COLOR", BLACK);
    currentMenu.buttonConfig = Vector2i(1,1);
    currentMenu.buttonFont = FreeSansBold18pt7b;
    currentMenu.buttonTextSize = 2;
    currentMenu.buttonFontHeight = getFontHeightChar(currentMenu.buttonFont, '0');
    currentMenu.addButton("GAME TYPE", BLACK, WHITE, false, nullptr);
}

void setMenuError() {
    currentMenuType = MenuType::Error;
    currentMenu = Menu();
    currentMenu.addTitle("ERROR", BLACK);
    currentMenu.addTitle("", BLACK);
    for(int i = 0; i < MAX_NB_MENU_TITLES-2; i++) {
        currentMenu.addTitle(errorMessage[i], BLACK);
    }
}

void setMenuKeyboard1() {
    ShortString prevKeyboardTitle = currentMenu.titles[0].text;
    currentMenuType = MenuType::Keyboard1;
    currentMenu = Menu();
    currentMenu.addTitle(prevKeyboardTitle, BLACK);
    currentMenu.addTitle(keyboardText, BLACK);
    currentMenu.titles[1].rightAlignX();
    currentMenu.buttonConfig = Vector2i(6, 5);
    currentMenu.addKey('A');
    currentMenu.addKey('B');
    currentMenu.addKey('C');
    currentMenu.addKey('D');
    currentMenu.addKey('E');
    currentMenu.addKey('F');
    currentMenu.addKey('G');
    currentMenu.addKey('H');
    currentMenu.addKey('I');
    currentMenu.addKey('J');
    currentMenu.addKey('K');
    currentMenu.addKey('L');
    currentMenu.addKey('M');
    currentMenu.addKey('N');
    currentMenu.addKey('O');
    currentMenu.addKey('P');
    currentMenu.addKey('Q');
    currentMenu.addKey('R');
    currentMenu.addKey('S');
    currentMenu.addKey('T');
    currentMenu.addKey('U');
    currentMenu.addKey('V');
    currentMenu.addKey('W');
    currentMenu.addKey('X');
    currentMenu.addKey('Y');
    currentMenu.addKey('Z');
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("a", WHITE, BLACK, true, onSetKeyboard2);
    currentMenu.addButton("", RED, RED, true, onErase);
    currentMenu.addButton("", GREEN, GREEN, true, onSubmit);
}

void setMenuKeyboard2() {
    ShortString prevKeyboardTitle = currentMenu.titles[0].text;
    currentMenuType = MenuType::Keyboard2;
    currentMenu = Menu();
    currentMenu.addTitle(prevKeyboardTitle, BLACK);
    currentMenu.addTitle(keyboardText, BLACK);
    currentMenu.titles[1].rightAlignX();
    currentMenu.buttonConfig = Vector2i(6, 5);
    currentMenu.addKey('a');
    currentMenu.addKey('b');
    currentMenu.addKey('c');
    currentMenu.addKey('d');
    currentMenu.addKey('e');
    currentMenu.addKey('f');
    currentMenu.addKey('g');
    currentMenu.addKey('h');
    currentMenu.addKey('i');
    currentMenu.addKey('j');
    currentMenu.addKey('k');
    currentMenu.addKey('l');
    currentMenu.addKey('m');
    currentMenu.addKey('n');
    currentMenu.addKey('o');
    currentMenu.addKey('p');
    currentMenu.addKey('q');
    currentMenu.addKey('r');
    currentMenu.addKey('s');
    currentMenu.addKey('t');
    currentMenu.addKey('u');
    currentMenu.addKey('v');
    currentMenu.addKey('w');
    currentMenu.addKey('x');
    currentMenu.addKey('y');
    currentMenu.addKey('z');
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("9", WHITE, BLACK, true, onSetKeyboard3);
    currentMenu.addButton("", RED, RED, true, onErase);
    currentMenu.addButton("", GREEN, GREEN, true, onSubmit);
}

void setMenuKeyboard3() {
    ShortString prevKeyboardTitle = currentMenu.titles[0].text;
    currentMenuType = MenuType::Keyboard3;
    currentMenu = Menu();
    currentMenu.addTitle(prevKeyboardTitle, BLACK);
    currentMenu.addTitle(keyboardText, BLACK);
    currentMenu.titles[1].rightAlignX();
    currentMenu.buttonConfig = Vector2i(6, 5);
    currentMenu.addKey('0');
    currentMenu.addKey('1');
    currentMenu.addKey('2');
    currentMenu.addKey('3');
    currentMenu.addKey('4');
    currentMenu.addKey('5');
    currentMenu.addKey('6');
    currentMenu.addKey('7');
    currentMenu.addKey('8');
    currentMenu.addKey('9');
    currentMenu.addKey('!');
    currentMenu.addKey('@');
    currentMenu.addKey('#');
    currentMenu.addKey('$');
    currentMenu.addKey('%');
    currentMenu.addKey('^');
    currentMenu.addKey('&');
    currentMenu.addKey('*');
    currentMenu.addKey('(');
    currentMenu.addKey(')');
    currentMenu.addKey('{');
    currentMenu.addKey('}');
    currentMenu.addKey('|');
    currentMenu.addKey('[');
    currentMenu.addKey(']');
    currentMenu.addKey('\\');
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("?", WHITE, BLACK, true, onSetKeyboard4);
    currentMenu.addButton("", RED, RED, true, onErase);
    currentMenu.addButton("", GREEN, GREEN, true, onSubmit);
}

void setMenuKeyboard4() {
    ShortString prevKeyboardTitle = currentMenu.titles[0].text;
    currentMenuType = MenuType::Keyboard4;
    currentMenu = Menu();
    currentMenu.addTitle(prevKeyboardTitle, BLACK);
    currentMenu.addTitle(keyboardText, BLACK);
    currentMenu.titles[1].rightAlignX();
    currentMenu.buttonConfig = Vector2i(6, 5);
    currentMenu.addKey('<');
    currentMenu.addKey('>');
    currentMenu.addKey('?');
    currentMenu.addKey(',');
    currentMenu.addKey('.');
    currentMenu.addKey('/');
    currentMenu.addKey('-');
    currentMenu.addKey('=');
    currentMenu.addKey('_');
    currentMenu.addKey('+');
    currentMenu.addKey(' ');
    currentMenu.addKey('\'');
    currentMenu.addKey('"');
    currentMenu.addKey(';');
    currentMenu.addKey(':');
    currentMenu.addKey('`');
    currentMenu.addKey('~');
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("", WHITE, WHITE, true, nullptr);
    currentMenu.addButton("A", WHITE, BLACK, true, onSetKeyboard1);
    currentMenu.addButton("", RED, RED, true, onErase);
    currentMenu.addButton("", GREEN, GREEN, true, onSubmit);
}


void onClickKey() {
    if(clickedButtonLabelText.length() == 0) {
        Serial.println("onClickKey(): clickedButtonLabelText is empty string");
        return;
    }

    if(clickedButtonLabelText.length() > 0) {
        char c = clickedButtonLabelText.charAt(0);

        keyboardText += c;
        currentMenu.titles[1].text = keyboardText;
        currentMenu.titles[1].rightAlignX();
        currentMenu.drawTitles();
    }
}

void onErase() {
    keyboardText.removeLastChar();
    currentMenu.titles[1].text = ShortString(keyboardText, ShortString::Alignment::Right);
    currentMenu.titles[1].rightAlignX();
    currentMenu.drawTitles();
}


void onSubmit() {
    switch(mainMode) {
        case MainMode::WifiName:
            PersistentParamBegin();
            PersistentParamSaveString(PersistentParamType::WifiName, keyboardText.c_str());
            PersistentParamEnd();
            break;
        case MainMode::WifiPassword:
            PersistentParamBegin();
            PersistentParamSaveString(PersistentParamType::WifiPassword, keyboardText.c_str());
            PersistentParamEnd();
            break;
        case MainMode::LichessToken:
            PersistentParamBegin();
            PersistentParamSaveString(PersistentParamType::LichessToken, keyboardText.c_str());
            PersistentParamEnd();
            break;
        case MainMode::LichessBoardAccountUsername:
            PersistentParamBegin();
            PersistentParamSaveString(PersistentParamType::LichessBoardAccountUsername, keyboardText.c_str());
            PersistentParamEnd();
            break;
        case MainMode::LichessBoardAccountToken:
            PersistentParamBegin();
            PersistentParamSaveString(PersistentParamType::LichessBoardAccountToken, keyboardText.c_str());
            PersistentParamEnd();
            break;
        case MainMode::Count:
        default:
            Serial.println("mainLoop_SetText: unknown main mode");
            break;
    }
    keyboardText = "";
    currentMenu.titles[1].text = "";
    currentMenu.titles[1].rightAlignX();
    currentMenu.drawTitles();
}


void clearErrorMessages() {
    for(int i = 0; i < MAX_NB_MENU_TITLES-2; i++) {
        errorMessage[i] = "";
    }
}