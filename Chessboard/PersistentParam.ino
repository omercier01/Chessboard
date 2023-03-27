
#include "Chessboard.h"
#include "PersistentParam.h"

#if defined(BOARD_DEF_ESP32)

    #include <Preferences.h>

    Preferences preferences;

    ShortString TypeToString(Type type) {
        switch(type) {
        case TouchX0:
            return ShortString("touch_x0");
            break;
        case TouchDxDx:
            return ShortString("touch_dxdx");
            break;
        case TouchDxDy:
            return ShortString("touch_dxdy");
            break;
        case TouchY0:
            return ShortString("touch_y0");
            break;
        case TouchDyDx:
            return ShortString("touch_dydx");
            break;
        case TouchDyDy:
            return ShortString("touch_dydy");
            break;
        case WifiName:
            return ShortString("wifi_name");
            break;
        case WifiPassword:
            return ShortString("wifi_password");
            break;
        case LichessToken:
            return ShortString("lichess_token");
            break;
        case LichessBoardAccountUsername:
            return ShortString("lich_brd_name");
            break;
        case LichessBoardAccountToken:
            return ShortString("lich_brd_tok");
            break;
        case TwoPlayersTimePerSide:
            return ShortString("twop_time");
            break;
        case TwoPlayersIncremen:
            return ShortString("twop_inc");
            break;

        default:
            Serial.println("TypeToString: unknown type.");
            break;
        }
    }

    void PersistentParamBegin() {
        preferences.begin("Chessboard", false);
    }

    void PersistentParamSaveFloat(PersistentParamType type, float value) {
        preferences.putFloat(TypeToString(type), value);
    }

    float PersistentParamLoadFloat(PersistentParamType type) {
        return preferences.getFloat(TypeToString(type), 0.f);
    }

    void PersistentParamSaveString(PersistentParamType type, LongString value) {
        preferences.putString(TypeToString(type), value);
    }

    LongString PersistentParamLoadString(PersistentParamType type) {
        return preferences.getString(TypeToString(type), "");
    }

    void PersistentParamSaveInt(PersistentParamType type, int value) {
        preferences.putInt(TypeToString(type), value);
    }

    int PersistentParamLoadInt(PersistentParamType type) {
        return preferences.getInt(TypeToString(type), 0);
    }

    void PersistentParamEnd() {
        preferences.end();
    }


#elif defined(BOARD_DEF_RP2040)

    #include <EEPROM.h>

    struct PersistentParams {
        float touchX0;
        float touchDxDx;
        float touchDxDy;
        float touchY0;
        float touchDyDx;
        float touchDyDy;
        LongString wifiName;
        LongString wifiPassword;
        LongString lichessToken;
        LongString lichessBoardAccountUsername;
        LongString lichessBoardAccountToken;
        int twoPlayersTimePerSide;
        int twoPlayersIncrement;

        void CopyFromPtr(PersistentParams* ptr) {
            touchX0 = ptr->touchX0;
            touchDxDx = ptr->touchDxDx;
            touchDxDy = ptr->touchDxDy;
            touchY0 = ptr->touchY0;
            touchDyDx = ptr->touchDyDx;
            touchDyDy = ptr->touchDyDy;
            wifiName = ptr->wifiName;
            wifiPassword = ptr->wifiPassword;
            lichessToken = ptr->lichessToken;
            lichessBoardAccountUsername = ptr->lichessBoardAccountUsername;
            lichessBoardAccountToken = ptr->lichessBoardAccountToken;
            twoPlayersTimePerSide = ptr->twoPlayersTimePerSide;
            twoPlayersIncrement = ptr->twoPlayersIncrement;
        }

        void CopyToPtr(PersistentParams* ptr) {
            ptr->touchX0 = touchX0;
            ptr->touchDxDx = touchDxDx;
            ptr->touchDxDy = touchDxDy;
            ptr->touchY0 = touchY0;
            ptr->touchDyDx = touchDyDx;
            ptr->touchDyDy = touchDyDy;
            ptr->wifiName = wifiName;
            ptr->wifiPassword = wifiPassword;
            ptr->lichessToken = lichessToken;
            ptr->lichessBoardAccountUsername = lichessBoardAccountUsername;
            ptr->lichessBoardAccountToken = lichessBoardAccountToken;
            ptr->twoPlayersTimePerSide = twoPlayersTimePerSide;
            ptr->twoPlayersIncrement = twoPlayersIncrement;
        }
    };
    PersistentParams persistentParams;

    void PersistentParamBegin() {
        // enough for 5 long string + the rest
        EEPROM.begin((5+1)*256);
        PersistentParams* ptr = (PersistentParams*) EEPROM.getDataPtr();
        persistentParams.CopyFromPtr(ptr);
    }

    void PersistentParamSaveFloat(PersistentParamType type, float value) {
        switch(type) {
        case PersistentParamType::TouchX0:
            persistentParams.touchX0 = value;
            break;
        case PersistentParamType::TouchDxDx:
            persistentParams.touchDxDx = value;
            break;
        case PersistentParamType::TouchDxDy:
            persistentParams.touchDxDy = value;
            break;
        case PersistentParamType::TouchY0:
            persistentParams.touchY0 = value;
            break;
        case PersistentParamType::TouchDyDx:
            persistentParams.touchDyDx = value;
            break;
        case PersistentParamType::TouchDyDy:
            persistentParams.touchDyDy = value;
            break;
        default:
            Serial.println("SaveFloat: type error");
        }
    }

    float PersistentParamLoadFloat(PersistentParamType type) {
        switch(type) {
        case PersistentParamType::TouchX0:
            return persistentParams.touchX0;
            break;
        case PersistentParamType::TouchDxDx:
            return persistentParams.touchDxDx;
            break;
        case PersistentParamType::TouchDxDy:
            return persistentParams.touchDxDy;
            break;
        case PersistentParamType::TouchY0:
            return persistentParams.touchY0;
            break;
        case PersistentParamType::TouchDyDx:
            return persistentParams.touchDyDx;
            break;
        case PersistentParamType::TouchDyDy:
            return persistentParams.touchDyDy;
            break;
        default:
            Serial.println("LoadFloat: type error");
            return 0.f;
        }
    }

    void PersistentParamSaveString(PersistentParamType type, LongString value) {
        switch(type) {
        case PersistentParamType::WifiName:
            persistentParams.wifiName = value;
            break;
        case PersistentParamType::WifiPassword:
            persistentParams.wifiPassword = value;
            break;
        case PersistentParamType::LichessToken:
            persistentParams.lichessToken = value;
            break;
        case PersistentParamType::LichessBoardAccountUsername:
            persistentParams.lichessBoardAccountUsername = value;
            break;
        case PersistentParamType::LichessBoardAccountToken:
            persistentParams.lichessBoardAccountToken = value;
            break;
        default:
            Serial.println("SaveString: type error");
        }
    }

    LongString PersistentParamLoadString(PersistentParamType type) {
        switch(type) {
        case PersistentParamType::WifiName:
            return persistentParams.wifiName;
            break;
        case PersistentParamType::WifiPassword:
            return persistentParams.wifiPassword;
            break;
        case PersistentParamType::LichessToken:
            return persistentParams.lichessToken;
            break;
        case PersistentParamType::LichessBoardAccountUsername:
            return persistentParams.lichessBoardAccountUsername;
            break;
        case PersistentParamType::LichessBoardAccountToken:
            return persistentParams.lichessBoardAccountToken;
            break;
        default:
            Serial.println("LoadString: type error");
            return "";
        }
    }


    void PersistentParamSaveInt(PersistentParamType type, int value) {
        switch(type) {
        case PersistentParamType::TwoPlayersTimePerSide:
            persistentParams.twoPlayersTimePerSide = value;
            break;
        case PersistentParamType::TwoPlayersIncrement:
            persistentParams.twoPlayersIncrement = value;
            break;
        default:
            Serial.println("SaveInt: type error");
        }
    }

    int PersistentParamLoadInt(PersistentParamType type) {
        switch(type) {
        case PersistentParamType::TwoPlayersTimePerSide:
            return persistentParams.twoPlayersTimePerSide;
            break;
        case PersistentParamType::TwoPlayersIncrement:
            return persistentParams.twoPlayersIncrement;
            break;
        default:
            Serial.println("LoadInt: type error");
            return 0;
        }
    }

    void PersistentParamEnd() {
        PersistentParams* ptr = (PersistentParams*) EEPROM.getDataPtr();
        persistentParams.CopyToPtr(ptr);
        //EEPROM.commit();
    }

    
#endif