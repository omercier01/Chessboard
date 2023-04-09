
#include "Chessboard.h"

void takeBoardBoolMutex() {
#if defined(BOARD_DEF_ESP32)
    while(clientBoardBoolBeingWrittenOrRead) {vTaskDelay(1);}
#elif defined(BOARD_DEF_RP2040)
    clientBoardBoolBeingWrittenOrRead = true;
#endif
}

void releaseBoardBoolMutex() {
    clientBoardBoolBeingWrittenOrRead = false;
}