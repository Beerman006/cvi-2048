#include <ansi_c.h>
#include <toolbox.h>
#include "gameboard.h"
#include "tile.h"
#include "change_notification.h"
#include "NextCellGenerator.h"
#include "../../CVI_Core/log.h"

static Tile *changeTile;

struct GameBoard {
    uint32_t numRows;
    uint32_t numCols;
    Tile **tiles;
    NextCellGenerator *slideHandlers[4];
    ListenerList addRemoveListeners;
};

typedef struct ClientAddRemoveTileData {
    void *data;
    AddRemoveTileHandler handler;
} ClientAddRemoveTileData;

static uint32_t MakeBoardIndex(GameBoard *gameBoard, uint32_t row, uint32_t col) {
    LOG_ASSERT_REASON(gameBoard, ArgumentNullReason);
    LOG_ASSERT_REASON(row < gameBoard->numRows, ArgumentOutOfRangeReason);
    LOG_ASSERT_REASON(col < gameBoard->numCols, ArgumentOutOfRangeReason);

    uint32_t idx = col + row * gameBoard->numCols;
    return idx;
}

static void SetTile(GameBoard *gameBoard, uint32_t row, uint32_t col, Tile *tile) {
    uint32_t idx = MakeBoardIndex(gameBoard, row, col);
    gameBoard->tiles[idx] = tile;
}

static void OnAddRemoveTile(void *target, void *data) {
    GameBoard *gb = (GameBoard *)target;
    ClientAddRemoveTileData *d = (ClientAddRemoveTileData *)data;
    AddRemoveTileHandler handler = d->handler;

    LOG_ASSERT_REASON(changeTile, InvalidOperationReason);
    uint32_t row = TileGetRow(changeTile);
    uint32_t col = TileGetColumn(changeTile);
    LOG_ASSERT_REASON(row < gb->numRows, InvalidOperationReason);
    LOG_ASSERT_REASON(col < gb->numCols, InvalidOperationReason);
    uint32_t idx = MakeBoardIndex(gb, row, col);
    int removed = !gb->tiles[idx];
    AddRemoveReason reason = removed ? Removed : Added;

    handler(changeTile, reason, d->data);
}

GameBoard *GameBoardCreate(uint32_t numRows, uint32_t numCols) {
    LOG_ASSERT_REASON(numRows && numCols, ArgumentOutOfRangeReason);

    srand(time(0));
    GameBoard *gb = calloc(1, sizeof(*gb));
    gb->tiles = calloc(numRows * numCols, sizeof(Tile*));
    gb->numRows = numRows;
    gb->numCols = numCols;
    gb->slideHandlers[SlideUp] = CellGeneratorCreate(gb, SlideUp);
    gb->slideHandlers[SlideDown] = CellGeneratorCreate(gb, SlideDown);
    gb->slideHandlers[SlideLeft] = CellGeneratorCreate(gb, SlideLeft);
    gb->slideHandlers[SlideRight] = CellGeneratorCreate(gb, SlideRight);
    return gb;
}

void GameBoardDispose(GameBoard *gameBoard) {
    LOG_ASSERT_REASON(gameBoard && gameBoard->tiles, ArgumentNullReason);

    CellGeneratorDispose(gameBoard->slideHandlers[SlideUp]);
    CellGeneratorDispose(gameBoard->slideHandlers[SlideDown]);
    CellGeneratorDispose(gameBoard->slideHandlers[SlideLeft]);
    CellGeneratorDispose(gameBoard->slideHandlers[SlideRight]);
    gameBoard->slideHandlers[SlideUp] = 0;
    gameBoard->slideHandlers[SlideDown] = 0;
    gameBoard->slideHandlers[SlideLeft] = 0;
    gameBoard->slideHandlers[SlideRight] = 0;

    for (int i = 0; i < gameBoard->numRows; i++) {
        for (int j = 0; j < gameBoard->numCols; j++) {
            Tile *t = GameBoardGetTile(gameBoard, i, j);
            if (t != 0) {
                TileDispose(t);
            }
        }
    }

    free(gameBoard->tiles);
    gameBoard->tiles = 0;
    free(gameBoard);
}

uint32_t GameBoardNumRows(GameBoard *gameBoard) {
    LOG_ASSERT_REASON(gameBoard, ArgumentNullReason);
    return gameBoard->numRows;
}

uint32_t GameBoardNumCols(GameBoard  *gameBoard) {
    LOG_ASSERT_REASON(gameBoard, ArgumentNullReason);
    return gameBoard->numCols;
}

void GameBoardAddTileAddRemoveHandler(GameBoard *gameBoard, void *data, AddRemoveTileHandler handler) {
    LOG_ASSERT_REASON(gameBoard && handler, ArgumentNullReason);

    ClientAddRemoveTileData *d = calloc(1, sizeof(ClientAddRemoveTileData));
    d->data = data;
    d->handler = handler;

    ChangeData changeData = { .target = gameBoard, .data = d, .handler = OnAddRemoveTile };
    ChangeHandlerAdd(&gameBoard->addRemoveListeners, changeData);
}

void GameBoardRemoveTileAddRemoveHandler(GameBoard *gameBoard, AddRemoveTileHandler handler) {
    LOG_ASSERT_REASON(gameBoard && handler, ArgumentNullReason);

    ClientAddRemoveTileData *data;
    ChangeData changeData = { .target = gameBoard, .data = 0, .handler = OnAddRemoveTile };
    data = (ClientAddRemoveTileData *)ChangeHandlerRemove(&gameBoard->addRemoveListeners, changeData);
    free(data);
}

Tile *GameBoardGetTile(GameBoard *gameBoard, uint32_t row, uint32_t column) {
    uint32_t idx = MakeBoardIndex(gameBoard, row, column);
    return gameBoard->tiles[idx];
}

GameBoardCell GameBoardMakeCell(int row, int col) {
    GameBoardCell c = { .col = col, .row = row };
    return c;
}

int GameBoardIsValidCell(GameBoard *gameBoard, GameBoardCell cell) {
    LOG_ASSERT_REASON(gameBoard, ArgumentNullReason);
    return (cell.col >= 0 && cell.col < gameBoard->numCols)
        && (cell.row >= 0 && cell.row < gameBoard->numRows);
}

int GameBoardTryGetOpenCell(GameBoard *gameBoard, GameBoardCell *cell) {
    LOG_ASSERT_REASON(gameBoard && cell, ArgumentNullReason);

    int openTiles = 0;
    uint32_t rows = gameBoard->numRows;
    uint32_t cols = gameBoard->numCols;
    GameBoardCell cells[rows * cols];
    memset(cells, 0, sizeof(GameBoardCell) * rows * cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (!GameBoardGetTile(gameBoard, i, j)) {
                cells[openTiles] = GameBoardMakeCell(i, j);
                openTiles++;
            }
        }
    }

    *cell = openTiles ? cells[rand() % openTiles] : GameBoardMakeCell(-1, -1);
    return GameBoardIsValidCell(gameBoard, *cell);
}

int GameBoardCanAddTile(GameBoard *gameBoard, uint32_t row, uint32_t col) {
    uint32_t idx = MakeBoardIndex(gameBoard, row, col);
    int noTileAtCell = !gameBoard->tiles[idx];
    return noTileAtCell;
}

static void RemoveTile(GameBoard *gameBoard, Tile *tile) {
    uint32_t row = TileGetRow(tile);
    uint32_t col = TileGetColumn(tile);
    uint32_t idx = MakeBoardIndex(gameBoard, row, col);
    gameBoard->tiles[idx] = 0;

    changeTile = tile;
    ChangeHandlerNotifyListeners(gameBoard->addRemoveListeners);
    changeTile = 0;
    TileDispose(tile);
}

static void AddTileCore(GameBoard *gameBoard, Tile *tile) {
    uint32_t row = TileGetRow(tile);
    uint32_t col = TileGetColumn(tile);
    uint32_t idx = MakeBoardIndex(gameBoard, row, col);
    gameBoard->tiles[idx] = tile;

    changeTile = tile;
    ChangeHandlerNotifyListeners(gameBoard->addRemoveListeners);
    changeTile = 0;
}

void GameBoardAddTile(GameBoard *gameBoard, uint32_t row, uint32_t col) {
    LOG_ASSERT_REASON(GameBoardCanAddTile(gameBoard, row, col), InvalidOperationReason);
    Tile *t = TileCreate(row, col);
    AddTileCore(gameBoard, t);
}

int GameBoardTrySlide(GameBoard *gameBoard, SlideDirection direction) {
    LOG_ASSERT_REASON(gameBoard && gameBoard->slideHandlers[direction], ArgumentNullReason);

    int slidOnce = 0, didSlide = 0;
    ListType mergedTiles = ListCreate(sizeof(Tile *));
    NextCellGenerator *generator = gameBoard->slideHandlers[direction];

    do {
        slidOnce = 0;
        CellGeneratorReset(generator);
        while(CellGeneratorMoveNext(generator)) {
            GameBoardCell toSlide, target;
            CellGeneratorGetCurrent(generator, &toSlide, &target);

            Tile *targetTile = GameBoardGetTile(gameBoard, target.row, target.col);
            Tile *slideTile = GameBoardGetTile(gameBoard, toSlide.row, toSlide.col);
            if (!targetTile && !slideTile) {
                continue;
            }

            if (!targetTile) {
                AddTileCore(gameBoard, TileCopyTo(slideTile, target.row, target.col));
                RemoveTile(gameBoard, slideTile);
                didSlide = slidOnce |= 1;
                continue;
            }

            int canMerge =
                slideTile != 0 &&
                TileCanMerge(targetTile, slideTile) &&
                !ListFindItem(mergedTiles, &slideTile, FRONT_OF_LIST, 0) &&
                !ListFindItem(mergedTiles, &targetTile, FRONT_OF_LIST, 0);
            if (canMerge) {
                TileMerge(targetTile, slideTile);
                RemoveTile(gameBoard, slideTile);
                ListInsertItem(mergedTiles, &targetTile, END_OF_LIST);
                didSlide = slidOnce |= 1;
            }
        }
    } while(slidOnce);
    ListDispose(mergedTiles);
    return didSlide;
}
