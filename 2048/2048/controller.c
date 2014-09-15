#include "toolbox.h"
#include <ansi_c.h>
#include "controller.h"
#include "../../CVI_Core/log.h"

struct Controller {
    GameBoard *gameBoard;
    GameUpdateHandler *updateHandler;
};

static void NotifyTileAddRemove(GameUpdateHandler *handler, Tile *tile, AddRemoveReason reason) {
    if (handler != 0) {
        if (reason == Added && handler->handleTileAdded != 0) {
            handler->handleTileAdded(handler->target, tile);
        } else if (reason == Removed && handler->handleTileRemoved != 0) {
            handler->handleTileRemoved(handler->target, tile);
        }
    }
}

static void NotifyTileValueChange(GameUpdateHandler *handler, Tile *tile) {
    if (handler != 0 && handler->handleTileValueChange != 0) {
        handler->handleTileValueChange(handler->target, tile);
    }
}

static void NotifyBeginUpdate(GameBoard *gameBoard, GameUpdateHandler *handler) {
    if (handler != 0 && handler->beginUpdateGame != 0) {
        handler->beginUpdateGame(handler->target, gameBoard);
    }
}

static void NotifyEndUpdate(GameBoard *gameBoard, GameUpdateHandler *handler) {
    if (handler != 0 && handler->endUpdateGame != 0) {
        handler->endUpdateGame(handler->target, gameBoard);
    }
}

static void HandleTileValueChange(Tile *tile, void *data) {
    Controller *controller = (Controller *)data;
    GameUpdateHandler *handler = controller->updateHandler;
    NotifyTileValueChange(handler, tile);
}

static void HandleTileAddRemove(Tile *tile, AddRemoveReason reason, void *data) {
    Controller *controller = (Controller *)data;
    GameUpdateHandler *handler = controller->updateHandler;

    NotifyTileAddRemove(handler, tile, reason);

    if (reason == Added) {
        TileAddValueChangeHandler(tile, HandleTileValueChange, controller);
    } else if (reason == Removed) {
        TileRemoveValueChangeHandler(tile, HandleTileValueChange);
    }
}

Controller *ControllerCreate(GameBoard *gameBoard) {
    Controller *controller = calloc(1, sizeof(Controller));
    controller->gameBoard = gameBoard;

    GameBoardAddTileAddRemoveHandler(gameBoard, controller, HandleTileAddRemove);

    return controller;
}

void ControllerDispose(Controller *controller) {
    uint32_t rows = GameBoardNumRows(controller->gameBoard);
    uint32_t cols = GameBoardNumCols(controller->gameBoard);
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            Tile *t = GameBoardGetTile(controller->gameBoard, i, j);
            if (t != 0) {
                TileRemoveValueChangeHandler(t, HandleTileValueChange);
            }
        }
    }
    GameBoardRemoveTileAddRemoveHandler(controller->gameBoard, HandleTileAddRemove);

    controller->gameBoard = 0;
    controller->updateHandler = 0;
    free(controller);
}

void ControllerSetGameUpdateHandler(Controller *controller, GameUpdateHandler *handler) {
    controller->updateHandler = handler;
}

static void HandleAddNewTile(void *data) {
    Controller *controller = (Controller *)data;
    GameBoardCell cell;
    int result = GameBoardTryGetOpenCell(controller->gameBoard, &cell);
    LOG_ASSERTMSG(result, "should only ever be here if we can get an open cell");
    GameBoardAddTile(controller->gameBoard, cell.row, cell.col);
}

void ControllerHandleSlide(Controller *controller, SlideDirection direction) {
    NotifyBeginUpdate(controller->gameBoard, controller->updateHandler);
    int didSlide = GameBoardTrySlide(controller->gameBoard, direction);
    GameBoardCell cell;
    int anyOpenCell = GameBoardTryGetOpenCell(controller->gameBoard, &cell);
    if (didSlide && anyOpenCell) {
        PostDelayedCall(HandleAddNewTile, controller, .2);
    } else if (!didSlide && !anyOpenCell) {
        // TODO: the game might be over! Need a way to check if we can slide in any direction.
    }
    NotifyEndUpdate(controller->gameBoard, controller->updateHandler);
}
