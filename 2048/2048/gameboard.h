//==============================================================================
//
// Title:       gameboard.h
// Purpose:     A short description of the interface.
//
// Created on:  8/30/2014 at 10:41:49 AM by Nick Beer.
// Copyright:   National Instruments. All Rights Reserved.
//
//==============================================================================

#ifndef __gameboard_H__
#define __gameboard_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include "cvidef.h"
#include "tile.h"

typedef enum AddRemoveReason {
    Added,
    Removed
} AddRemoveReason;

typedef enum SlideDirection {
    SlideUp = 0,
    SlideDown,
    SlideLeft,
    SlideRight
} SlideDirection;

typedef void (*AddRemoveTileHandler)(Tile *tile, AddRemoveReason reason, void *data);

typedef struct GameBoardCell {
    int row;
    int col;
} GameBoardCell;

typedef struct GameBoard GameBoard;

GameBoard *GameBoardCreate(uint32_t numRows, uint32_t numCols);
void GameBoardDispose(GameBoard *gameBoard);

uint32_t GameBoardNumRows(GameBoard *gameBoard);
uint32_t GameBoardNumCols(GameBoard *gameBoard);

void GameBoardAddTileAddRemoveHandler(GameBoard *gameBoard, void *data, AddRemoveTileHandler handler);
void GameBoardRemoveTileAddRemoveHandler(GameBoard *gameBoard, AddRemoveTileHandler handler);

GameBoardCell GameBoardMakeCell(int row, int col);
int GameBoardTryGetOpenCell(GameBoard *gameBoard, GameBoardCell *cell);
int GameBoardIsValidCell(GameBoard *gameBoard, GameBoardCell cell);

int GameBoardCanAddTile(GameBoard *gameBoard, uint32_t row, uint32_t col);
void GameBoardAddTile(GameBoard *gameBoard, uint32_t row, uint32_t col);
Tile *GameBoardGetTile(GameBoard *gameBoard, uint32_t row, uint32_t column);

int GameBoardTrySlide(GameBoard *gameBoard, SlideDirection direction);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __gameboard_H__ */
