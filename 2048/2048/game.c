#include <ansi_c.h>
#include "game.h"
#include "../../CVI_Core/log.h"

struct Game2048{
    uint32_t numRows;
    uint32_t numCols;
    CreateUserInterfaceHandler createUI;
    UserInterface *ui;
    GameBoard *gameBoard;
    Controller *controller;
};

static GameBoard *GetOrCreateGameBoard(Game2048 *game) {
    if (!game->gameBoard) {
        game->gameBoard = GameBoardCreate(game->numRows, game->numCols);
    }
    return game->gameBoard;
}

static Controller *GetOrCreateController(Game2048 *game) {
    if (!game->controller) {
        game->controller = ControllerCreate(GetOrCreateGameBoard(game));
    }
    return game->controller;
}

static UserInterface *GetOrCreateUserInterface(Game2048 *game) {
    if (!game->ui) {
        game->ui = game->createUI(game);
    }
    return game->ui;
}

Game2048 *Game2048Create(uint32_t numRows, uint32_t numCols, CreateUserInterfaceHandler createUI) {
    Game2048 *game = calloc(1, sizeof(Game2048));
    game->numRows = numRows;
    game->numCols = numCols;
    game->createUI = createUI;
    return game;
}

void Game2048Dispose(Game2048 *game) {
    if (game->controller != 0) {
        ControllerDispose(game->controller);
    }
    if (game->gameBoard != 0) {
        GameBoardDispose(game->gameBoard);
    }
    if (game->ui != 0) {
        game->ui->dispose(game->ui);
    }
    game->gameBoard = 0;
    game->controller = 0;
    free(game);

    LogDispose(LogGetGlobal());
}

void Game2048Run(Game2048 *game) {
    UserInterface *ui = GetOrCreateUserInterface(game);
    ui->run(ui);
}

GameBoard *Game2048GameBoard(Game2048 *game) {
    return GetOrCreateGameBoard(game);
}

Controller *Game2048Controller(Game2048 *game) {
    return GetOrCreateController(game);
}

UserInterface *Game2048UserInterface(Game2048 *game) {
    return GetOrCreateUserInterface(game);
}
