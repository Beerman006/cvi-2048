#include <toolbox.h>
#include <stdint.h>
#include "tile.h"
#include "change_notification.h"
#include "../../CVI_Core/log.h"

struct Tile {
    uint32_t row;
    uint32_t col;
    uint32_t val;
    ListenerList valueChangedListeners;
};

typedef struct TileChangeData {
    TileChangeHandler handler;
    void *clientData;
} TileChangeData;

static void OnTileChanged(void *target, void *data) {
    Tile *tile = (Tile *)target;
    TileChangeData *d = (TileChangeData *)data;
    d->handler(tile, d->clientData);
}

static void IncrementValue(Tile *tile, Tile *by) {
    tile->val += by->val;
    ChangeHandlerNotifyListeners(tile->valueChangedListeners);
}

Tile *TileCreate(uint32_t row, uint32_t column) {
    Tile *tile = calloc(1, sizeof(Tile));
    // TODO: tile can sometimes start with 2 or 4.
    tile->val = 2;
    tile->row = row;
    tile->col = column;
    return tile;
}

void TileDispose(Tile *tile) {
    free(tile);
}

uint32_t TileGetRow(Tile *tile) {
    return tile->row;
}

uint32_t TileGetColumn(Tile *tile) {
    return tile->col;
}

uint32_t TileGetValue(Tile *tile) {
    return tile->val;
}

void TileAddValueChangeHandler(Tile *tile, TileChangeHandler handler, void *data) {
    LOG_ASSERT_REASON(tile && handler, ArgumentNullReason);
    
    TileChangeData *d = calloc(1, sizeof(TileChangeData));
    d->handler = handler;
    d->clientData = data;
    
    ChangeData changeData = { .target = tile, .data = d, .handler = OnTileChanged };
    ChangeHandlerAdd(&tile->valueChangedListeners, changeData);
}

void TileRemoveValueChangeHandler(Tile *tile, TileChangeHandler handler) {
    LOG_ASSERT_REASON(tile && handler, ArgumentNullReason);
    TileChangeData *d;
    ChangeData data = { .target = tile, .data = 0, .handler = OnTileChanged };
    d = (TileChangeData *)ChangeHandlerRemove(&tile->valueChangedListeners, data);
    free(d);
}

int TileCanMerge(Tile *tile, Tile *toMerge) {
    LOG_ASSERT_REASON(tile && toMerge, ArgumentNullReason);
    
    int hasSameX = tile->col == toMerge->col;
    int touchesX = 
        tile->col == toMerge->col - 1 ||
        tile->col == toMerge->col + 1;
    
    int hasSameY = tile->row == toMerge->row;
    int touchesY = 
        tile->row == toMerge->row - 1 ||
        tile->row == toMerge->row + 1;
    
    return ((touchesX && hasSameY) || (hasSameX && touchesY))
        && tile->val == toMerge->val;
}

void TileMerge(Tile *tile, Tile* toMerge) {
    LOG_ASSERT_REASON(tile && toMerge, ArgumentNullReason);
    LOG_ASSERT_REASON(TileCanMerge(tile, toMerge), InvalidOperationReason);
    IncrementValue(tile, toMerge);
}

Tile *TileCopyTo(Tile *tile, uint32_t row, uint32_t column) {
    Tile *t = TileCreate(row, column);
    t->val = tile->val;
    if (tile->valueChangedListeners) {
        t->valueChangedListeners = tile->valueChangedListeners;//ListCopy(tile->valueChangedListeners);
    }
    return t;
}
