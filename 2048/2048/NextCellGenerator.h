#ifndef __NextCellGenerator_H__
#define __NextCellGenerator_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include "gameboard.h"
#include "cvidef.h"

typedef struct NextCellGenerator NextCellGenerator;

NextCellGenerator *CellGeneratorCreate(GameBoard *gameBoard, SlideDirection direction);
void CellGeneratorDispose(NextCellGenerator *generator);

void CellGeneratorReset(NextCellGenerator *generator);
int CellGeneratorMoveNext(NextCellGenerator *generator);
void CellGeneratorGetCurrent(NextCellGenerator *generator, GameBoardCell *toSlide, GameBoardCell *target);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __NextCellGenerator_H__ */
