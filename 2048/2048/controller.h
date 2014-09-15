#ifndef __controller_H__
#define __controller_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include "cvidef.h"
#include "gameboard.h"

typedef void (*UpdateGameHandler)(void *target, GameBoard *gameBoard);
typedef void (*TileUpdateHandler)(void *target, Tile *);

typedef struct Controller Controller;

typedef struct GameUpdateHandler {
    void *target;
    UpdateGameHandler beginUpdateGame;
    UpdateGameHandler endUpdateGame;
    TileUpdateHandler handleTileAdded;
    TileUpdateHandler handleTileRemoved;
    TileUpdateHandler handleTileValueChange;
} GameUpdateHandler;

Controller *ControllerCreate(GameBoard *gameBoard);
void ControllerDispose(Controller *controller);
        
void ControllerSetGameUpdateHandler(Controller *controller, GameUpdateHandler *handler);
void ControllerHandleSlide(Controller *controller, SlideDirection direction);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __controller_H__ */
