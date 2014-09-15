#ifndef __tile_H__
#define __tile_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>
#include "cvidef.h"

typedef struct Tile Tile;
typedef void (*TileChangeHandler)(Tile *, void *data);

Tile *TileCreate(uint32_t row, uint32_t column);
void TileDispose(Tile *tile);

uint32_t TileGetRow(Tile *tile);
uint32_t TileGetColumn(Tile *tile);
uint32_t TileGetValue(Tile *tile);

void TileAddValueChangeHandler(Tile *tile, TileChangeHandler handler, void *data);
void TileRemoveValueChangeHandler(Tile *tile, TileChangeHandler handler);

int TileCanMerge(Tile *target, Tile *toMerge);
void TileMerge(Tile *target, Tile *toMerge);

Tile *TileCopyTo(Tile * tile, uint32_t row, uint32_t column);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __tile_H__ */
