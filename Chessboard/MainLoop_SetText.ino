
#include "Chessboard.h"

//
// Debug mode to enter the wifi name, passwork, and lichess tokens
//

void mainLoop_SetText() {
    if(bMainModeDirty) {
        keyboardText = "";
        setMenuKeyboard1();
        switch(mainMode) {
            case MainMode::WifiName:
                currentMenu.titles[0].text = "Wifi Name";
                currentMenu.titles[0].centerX();
                break;
            case MainMode::WifiPassword:
                currentMenu.titles[0].text = "Wifi Password";
                currentMenu.titles[0].centerX();
                break;
            case MainMode::LichessToken:
                currentMenu.titles[0].text = "Lichess Token";
                currentMenu.titles[0].centerX();
                break;
            case MainMode::LichessBoardAccountUsername:
                currentMenu.titles[0].text = "Lichess Board Name";
                currentMenu.titles[0].centerX();
                break;
            case MainMode::LichessBoardAccountToken:
                currentMenu.titles[0].text = "Lichess Board Token";
                currentMenu.titles[0].centerX();
                break;
            case MainMode::Count:
            default:
                Serial.println("mainLoop_SetText: unknown main mode");
                break;
        }
        currentMenu.draw();

        bMainModeDirty = false;
    }

    Vector2 touchPos;
    if(IsDisplayTouchedTimeBuffer(touchPos)) {
        currentMenu.onClick(touchPos);
    }
}



