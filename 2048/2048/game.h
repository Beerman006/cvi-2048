#ifndef __game_H__
#define __game_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include "cvidef.h"
#include "gameboard.h"
#include "controller.h"

typedef struct Game2048 Game2048;

typedef struct UserInterface UserInterface;
typedef struct UserInterface {
    void *data;
    void (*dispose)(UserInterface *);
    void (*run)(UserInterface *);
} UserInterface;

typedef UserInterface *(*CreateUserInterfaceHandler)(Game2048 *);

Game2048 *Game2048Create(uint32_t numRows, uint32_t numCols, CreateUserInterfaceHandler createUI);
void Game2048Dispose(Game2048 *game);

GameBoard *Game2048GameBoard(Game2048 *game);
Controller *Game2048Controller(Game2048 *game);
UserInterface *Game2048UserInterface(Game2048 *game);

void Game2048Run(Game2048 *game);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __game_H__ */
