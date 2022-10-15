
#pragma once

#include "util.h"


bool nextMoveIsWhiteMove(int nbMovesSoFar);
bool nextMoveIsUserMove(int nbMovesSoFar);
bool boardPiecesMatchSwitches(char pieces[8][8]);
void copyBoardPieces(char to[8][8], char from[8][8]);
void setBoardToInitialPositions(char pieces[8][8]);
void makeBoardMoves(char pieces[8][8], Move* moves, int nbMoves, int nbMovesSoFar);
void makeBoardMove(char pieces[8][8], Move& move, int nbMovesSoFar);
bool isOpponentPiece(char pieces[8][8], Vector2i square);
bool isPlayerPiece(char pieces[8][8], Vector2i square);
bool isEmptySquare(char pieces[8][8], Vector2i square);
Move squaresToMove(Vector2i from, Vector2i to, char promotion);
char getPieceChar(char pieces[8][8], Vector2i square);
bool isCastlingMove(char pieces[8][8], Move move);
bool isEnPassantMove(char pieces[8][8], Move move);
int getChangedSquares(Vector2i* modifiedSquares,  char serverBoardToCheck[8][8]);
bool isSimpleCaptureMove(int nbChangedSquares);