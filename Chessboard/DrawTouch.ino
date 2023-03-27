

#include "Chessboard.h"
#include "DrawTouch.h"

float getPromotionTopBorder() {
    return 22;
}

Vector2 getPromotionZoneMin(int iX, int iY) {
    Vector2 ret;
    float topBorder = getPromotionTopBorder();
    Vector2 width = getPromotionZomeWidth();
    ret.x = MENU_BORDER_WIDTH + iX*(width.x + MENU_BORDER_WIDTH);
    ret.y = topBorder + MENU_BORDER_WIDTH + iY*(width.y + MENU_BORDER_WIDTH);
    return ret;
}

Vector2 getPromotionZomeWidth() {
    Vector2 ret;
    float topBorder = getPromotionTopBorder();
    ret.x = 0.5*(DISPLAY_PIXELS_WIDTH - 3*MENU_BORDER_WIDTH);
    ret.y = 0.5*(DISPLAY_PIXELS_HEIGHT - topBorder - 3*MENU_BORDER_WIDTH);
    return ret;
}

Vector2 getCalibratedTouchPos() {
    TS_Point touchP = TouchscreenGetPoint();
    
    Vector2 p;
    TouchCalibrationData& tc = touchCalibrationData;
    p.x = tc.x0 + touchP.x * tc.dxdx + touchP.y * tc.dxdy;
    p.y = tc.y0 + touchP.x * tc.dydx + touchP.y * tc.dydy;
    return p;
}


void drawChessChar(Vector2i cursor, char blackLayer, char whiteLayer) {
    DisplaySetCursor(cursor.x, cursor.y);
    DisplaySetTextColor(BLACK);
    DisplayPrintChar(blackLayer);
    DisplaySetCursor(cursor.x, cursor.y);
    DisplaySetTextColor(WHITE);
    DisplayPrintChar(whiteLayer);
}


void drawBoardBoolOrChar(bool squares[8][8], char pieces[8][8]) {
    
    const Vector2 topLeftPos = Vector2(DISPLAY_PIXELS_WIDTH/2 - 4*drawBoardSquareSize, 
                                       drawBoardSquarePosY);

    if(pieces) {
        DisplaySetFont(drawBoardFont);
    }

    // draw contour
    drawRectContour(topLeftPos.x, topLeftPos.y,
                    8*drawBoardSquareSize, 8*drawBoardSquareSize,
                    drawBoardContourThickness, BLACK);

    // draw grid
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            DisplayFillRect(
                topLeftPos.x + i*drawBoardSquareSize,
                topLeftPos.y + j*drawBoardSquareSize,
                drawBoardSquareSize,
                drawBoardSquareSize,
                ((i+j)%2 ? 0xBC4C : // 10111 100010 01100
                           0xF6D7)  // 11110 110110 10111
            );

            if(squares) {
                // draw marker if piece on square
                if(squares[i][8-j-1]) {
                    DisplayFillCircle(
                        topLeftPos.x + i*drawBoardSquareSize + drawBoardSquareSize/2,
                        topLeftPos.y + j*drawBoardSquareSize + drawBoardSquareSize/2,
                        (drawBoardSquareSize - 2*drawBoardBoolMarkerBorder)/2,
                        RED
                    );
                }
            } else if (pieces) {
                // get piece index with correct board orientation depending on player color
                int pieceIndexX;
                int pieceIndexY;
                if(playerColor == PlayerColor::White) {
                    pieceIndexX = i;
                    pieceIndexY = 8-j-1;
                } else {
                    pieceIndexX = 8-i-1;
                    pieceIndexY = j;
                }

                if(pieces[pieceIndexX][pieceIndexY] > 0) {
                    Vector2i cursor;
                    cursor.x =   topLeftPos.x
                               + (i+0.5)*drawBoardSquareSize
                               - drawBoardFont->yAdvance/2;
                    cursor.y =   topLeftPos.y
                               + (j+0.5)*drawBoardSquareSize
                               + drawBoardFont->yAdvance/2 + 1;

                    switch(pieces[pieceIndexX][pieceIndexY]) {
                    case 'K':
                        drawChessChar(cursor, FONT_PIECE_WHITE_K_BLACKLAYER,
                                              FONT_PIECE_WHITE_K_WHITELAYER);
                        break;
                    case 'Q':
                        drawChessChar(cursor, FONT_PIECE_WHITE_Q_BLACKLAYER,
                                              FONT_PIECE_WHITE_Q_WHITELAYER);
                        break;
                    case 'R':
                        drawChessChar(cursor, FONT_PIECE_WHITE_R_BLACKLAYER,
                                              FONT_PIECE_WHITE_R_WHITELAYER);
                        break;
                    case 'B':
                        drawChessChar(cursor, FONT_PIECE_WHITE_B_BLACKLAYER,
                                              FONT_PIECE_WHITE_B_WHITELAYER);
                        break;
                    case 'N':
                        drawChessChar(cursor, FONT_PIECE_WHITE_N_BLACKLAYER,
                                              FONT_PIECE_WHITE_N_WHITELAYER);
                        break;
                    case 'P':
                        drawChessChar(cursor, FONT_PIECE_WHITE_P_BLACKLAYER,
                                              FONT_PIECE_WHITE_P_WHITELAYER);
                        break;
                    case 'k':
                        drawChessChar(cursor, FONT_PIECE_BLACK_K_BLACKLAYER,
                                              FONT_PIECE_BLACK_K_WHITELAYER);
                        break;
                    case 'q':
                        drawChessChar(cursor, FONT_PIECE_BLACK_Q_BLACKLAYER,
                                              FONT_PIECE_BLACK_Q_WHITELAYER);
                        break;
                    case 'r':
                        drawChessChar(cursor, FONT_PIECE_BLACK_R_BLACKLAYER,
                                              FONT_PIECE_BLACK_R_WHITELAYER);
                        break;
                    case 'b':
                        drawChessChar(cursor, FONT_PIECE_BLACK_B_BLACKLAYER,
                                              FONT_PIECE_BLACK_B_WHITELAYER);
                        break;
                    case 'n':
                        drawChessChar(cursor, FONT_PIECE_BLACK_N_BLACKLAYER,
                                              FONT_PIECE_BLACK_N_WHITELAYER);
                        break;
                    case 'p':
                        drawChessChar(cursor, FONT_PIECE_BLACK_P_BLACKLAYER,
                                              FONT_PIECE_BLACK_P_WHITELAYER);
                        break;
                    }
                }
            }
        }
    }
}


void drawBoardBool(bool squares[8][8]) {
    (void)squares;
    drawBoardBoolOrChar(clientBoardBool, nullptr);
}


void drawBoardChar(char pieces[8][8]) {
    drawBoardBoolOrChar(nullptr, pieces);
}