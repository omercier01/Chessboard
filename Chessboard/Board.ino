

#include "Chessboard.h"
#include "ReedSwitch.h"


bool nextMoveIsWhiteMove(int nbMovesSoFar) {
    return (nbMovesSoFar % 2 == 0);
}

bool nextMoveIsUserMove(int nbMovesSoFar) {
    return ((playerColor ==  PlayerColor::White) && nextMoveIsWhiteMove(nbMovesSoFar)) ||
           ((playerColor ==  PlayerColor::Black) && !nextMoveIsWhiteMove(nbMovesSoFar));
}

void setBoardToInitialPositions(char pieces[8][8]) {
    for(int i = 0; i < 8; i++) {
        for(int j = 2; j < 6; j++) {
            pieces[i][j] = 0;
        }
    }
    pieces[0][0] = 'R';
    pieces[1][0] = 'N';
    pieces[2][0] = 'B';
    pieces[3][0] = 'Q';
    pieces[4][0] = 'K';
    pieces[5][0] = 'B';
    pieces[6][0] = 'N';
    pieces[7][0] = 'R';
    pieces[0][7] = 'r';
    pieces[1][7] = 'n';
    pieces[2][7] = 'b';
    pieces[3][7] = 'q';
    pieces[4][7] = 'k';
    pieces[5][7] = 'b';
    pieces[6][7] = 'n';
    pieces[7][7] = 'r';
    for(int i = 0; i < 8; i++) {
        pieces[i][1] = 'P';
        pieces[i][6] = 'p';
    }
}


bool boardPiecesMatchSwitches(char pieces[8][8]) {
    takeBoardBoolMutex();
    bool ret = true;
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if((pieces[i][j] > 0) != clientBoardBool[i][j]) {
                ret = false;
                break;
            }
        }
        if(!ret) {
            break;
        }
    }
    releaseBoardBoolMutex();
    return ret;
}

void copyBoardPieces(char to[8][8], char from[8][8]) {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            to[i][j] = from[i][j];
        }
    }
}


// Assumes the move is legal
void makeBoardMove(char pieces[8][8], Move& move, int nbMovesSoFar) {
    Serial.println((ShortString("Making move ") + move.toString()).c_str());

    // check for castling
    Vector2i from = move.getFrom();
    Vector2i to = move.getTo();
    
    if(isCastlingMove(pieces, move))
    {
        // castling
        int row = to.y;
        if(to.x == 6) {
            // short castle
            pieces[6][row] = pieces[4][row];
            pieces[5][row] = pieces[7][row];
            pieces[4][row] = 0;
            pieces[7][row] = 0;
        } else {
            // long castle
            pieces[2][row] = pieces[4][row];
            pieces[3][row] = pieces[0][row];
            pieces[4][row] = 0;
            pieces[0][row] = 0;
        }
    } else if(isEnPassantMove(pieces, move)) {
        pieces[to.x][to.y] = pieces[from.x][from.y];
        pieces[from.x][from.y] = 0;
        pieces[to.x][from.y] = 0;
    } else {
        if(move.hasPromotion()) {
            Serial.print("makeBoardMove hasPromotion ");
            Serial.print(String(nbMovesSoFar));
            Serial.print(" ");
            Serial.println(move.chars);
            pieces[to.x][to.y] = nextMoveIsWhiteMove(nbMovesSoFar) ? toUpper(move.chars[4]) : move.chars[4];
            pieces[from.x][from.y] = 0;
        } else {
            pieces[to.x][to.y] = pieces[from.x][from.y];
            pieces[from.x][from.y] = 0;
        }
    }
}


// Assumes all moves are legal
void makeBoardMoves(char pieces[8][8], Move* moves, int nbMoves, int nbMovesSoFar) {
    for(int i = 0; i < nbMoves; i++) {
        makeBoardMove(pieces, moves[i], nbMovesSoFar + i);
    }
}


bool isWhitePiece(char pieces[8][8], Vector2i square) {
    return pieces[square.x][square.y] >= 'A' && pieces[square.x][square.y] <= 'Z';
}


bool isBlackPiece(char pieces[8][8], Vector2i square) {
    return pieces[square.x][square.y] >= 'a' && pieces[square.x][square.y] <= 'z';
}


bool isOpponentPiece(char pieces[8][8], Vector2i square) {
    if(isTwoPlayersGame) {
        if((playerColor == PlayerColor::White && activePlayer == Player::Player) ||
           (playerColor == PlayerColor::Black && activePlayer == Player::Opponent)) {
            return isBlackPiece(pieces, square);
        } else if((playerColor == PlayerColor::Black && activePlayer == Player::Player) ||
                  (playerColor == PlayerColor::White && activePlayer == Player::Opponent)) {
            return isWhitePiece(pieces, square);
        } else {
            return false;
        }
    } else {
        if(playerColor == PlayerColor::White) {
            return isBlackPiece(pieces, square);
        } else if(playerColor == PlayerColor::Black) {
            return isWhitePiece(pieces, square);
        } else {
            return false;
        }
    }
}


bool isPlayerPiece(char pieces[8][8], Vector2i square) {
    if(isTwoPlayersGame) {
        if((playerColor == PlayerColor::White && activePlayer == Player::Player) ||
           (playerColor == PlayerColor::Black && activePlayer == Player::Opponent)) {
            return isWhitePiece(pieces, square);
        } else if((playerColor == PlayerColor::Black && activePlayer == Player::Player) ||
                  (playerColor == PlayerColor::White && activePlayer == Player::Opponent)) {
            return isBlackPiece(pieces, square);
        } else {
            return false;
        }
    } else {
        if(playerColor == PlayerColor::White) {
            return isWhitePiece(pieces, square);
        } else if(playerColor == PlayerColor::Black) {
            return isBlackPiece(pieces, square);
        } else {
            return false;
        }
    }
}

bool isEmptySquare(char pieces[8][8], Vector2i square) {
    return pieces[square.x][square.y] == 0;
}


// Converts to squares and a promotion to a UCI move
Move squaresToMove(Vector2i from, Vector2i to, char promotion) {
    Move ret;
    ret.chars[0] = 'a' + from.x;
    ret.chars[1] = '1' + from.y;
    ret.chars[2] = 'a' + to.x;
    ret.chars[3] = '1' + to.y;
    ret.chars[4] = promotion;
    return ret;
}


char getPieceChar(char pieces[8][8], Vector2i square) {
    return pieces[square.x][square.y];
}


bool isCastlingMove(char pieces[8][8], Move move) {
    Vector2i from = move.getFrom();
    Vector2i to = move.getTo();

    return toUpper(getPieceChar(pieces, from)) == 'K' &&
           abs(from.x - to.x) > 1;
}


bool isEnPassantMove(char pieces[8][8], Move move) {
    Vector2i from = move.getFrom();
    Vector2i to = move.getTo();

    return toUpper(getPieceChar(pieces, from)) == 'P' &&
           isEmptySquare(pieces, to) &&
           from.x != to.x;
}



// assumes modifiedSquares is big enough
int getChangedSquares(Vector2i* modifiedSquares, char serverBoardToCheck[8][8]) {

    takeBoardBoolMutex();
    int nbSquaresChanged = 0;
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(clientBoardBool[i][j] != (serverBoardToCheck[i][j] > 0)) {
                modifiedSquares[nbSquaresChanged] = Vector2i(i,j);
                nbSquaresChanged++;
            }
        }
    }
    releaseBoardBoolMutex();
    return nbSquaresChanged;
}


bool isSimpleCaptureMove(int nbChangedSquares) {
    return nbChangedSquares == 1 &&
           isPlayerPiece(serverBoardPieces, modifiedSquares[0]) &&
           potentialCapturedSquare.isValid();
}