
#include "Chessboard.h"

void takeBoardBoolMutex() {
    //while(clientBoardBoolBeingWrittenOrRead) {vTaskDelay(1);}
    clientBoardBoolBeingWrittenOrRead = true;
}

void releaseBoardBoolMutex() {
    clientBoardBoolBeingWrittenOrRead = false;
}