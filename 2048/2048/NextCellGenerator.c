#include <ansi_c.h>
#include "NextCellGenerator.h"
#include "../../CVI_Core/log.h"

typedef int (*MoveNextHandler)(NextCellGenerator *);
typedef GameBoardCell (*GetTargetHandler)(NextCellGenerator *);

struct NextCellGenerator {
    GameBoard *gb;
    uint32_t minRows;
    uint32_t minCols;
    GameBoardCell current;
    MoveNextHandler moveNext;
    GetTargetHandler getNextTarget;
};

static GameBoardCell HandleGetTargetUp(NextCellGenerator *gen) {
    return GameBoardMakeCell(gen->current.row - 1, gen->current.col);
}

static GameBoardCell HandleGetTargetDown(NextCellGenerator *gen) {
    return GameBoardMakeCell(gen->current.row + 1, gen->current.col);
}

static GameBoardCell HandleGetTargetLeft(NextCellGenerator *gen) {
    return GameBoardMakeCell(gen->current.row, gen->current.col - 1);
}

static GameBoardCell HandleGetTargetRight(NextCellGenerator *gen) {
    return GameBoardMakeCell(gen->current.row, gen->current.col + 1);
}

static int HandleMoveNextUp(NextCellGenerator *gen) {
    GameBoardCell cell = gen->current;
    uint32_t rows = GameBoardNumRows(gen->gb);
    uint32_t cols = GameBoardNumCols(gen->gb);
    if (!GameBoardIsValidCell(gen->gb, cell)) {
        gen->current = GameBoardMakeCell(1, 0);
        return 1;
    }
    if (cell.row == rows - 1 && cell.col == cols - 1) {
        return 0;
    }
    if (cell.col < cols - 1) {
        gen->current = GameBoardMakeCell(cell.row, cell.col + 1);
        return 1;
    }
    gen->current = GameBoardMakeCell(cell.row + 1, 0);
    return 1;
}

static int HandleMoveNextDown(NextCellGenerator *gen) {
    GameBoardCell cell = gen->current;
    uint32_t rows = GameBoardNumRows(gen->gb);
    uint32_t cols = GameBoardNumCols(gen->gb);
    if (!GameBoardIsValidCell(gen->gb, cell)) {
        gen->current = GameBoardMakeCell(rows - 2, 0);
        return 1;
    }
    if (cell.row == 0 && cell.col == cols - 1) {
        return 0;
    }
    if (cell.col < cols - 1) {
        gen->current = GameBoardMakeCell(cell.row, cell.col + 1);
        return 1;
    }
    gen->current = GameBoardMakeCell(cell.row - 1, 0);
    return 1;
}

static int HandleMoveNextLeft(NextCellGenerator *gen) {
    GameBoardCell cell = gen->current;
    uint32_t rows = GameBoardNumRows(gen->gb);
    uint32_t cols = GameBoardNumCols(gen->gb);
    if (!GameBoardIsValidCell(gen->gb, cell)) {
        gen->current = GameBoardMakeCell(0, 1);
        return 1;
    }
    if (cell.row == rows - 1 && cell.col == cols - 1) {
        return 0;
    }
    if (cell.row < rows - 1) {
        gen->current = GameBoardMakeCell(cell.row + 1, cell.col);
        return 1;
    }
    gen->current = GameBoardMakeCell(0, cell.col + 1);
    return 1;
}

static int HandleMoveNextRight(NextCellGenerator *gen) {
    GameBoardCell cell = gen->current;
    uint32_t rows = GameBoardNumRows(gen->gb);
    uint32_t cols = GameBoardNumCols(gen->gb);
    if (!GameBoardIsValidCell(gen->gb, cell)) {
        gen->current = GameBoardMakeCell(0, cols - 2);
        return 1;
    }
    if (cell.row == rows - 1 && cell.col == 0) {
        return 0;
    }
    if (cell.row < rows - 1) {
        gen->current = GameBoardMakeCell(cell.row + 1, cell.col);
        return 1;
    }
    gen->current = GameBoardMakeCell(0, cell.col - 1);
    return 1;
}

NextCellGenerator *CellGeneratorCreate(GameBoard *gameBoard, SlideDirection direction) {
    LOG_ASSERTMSG_REASON(gameBoard, "gameboard", ArgumentNullReason);
    NextCellGenerator *gen = calloc(1, sizeof(NextCellGenerator));
    gen->gb = gameBoard;
    gen->current = GameBoardMakeCell(-1, -1);
    switch(direction) {
        case SlideUp:
            gen->minCols = 1;
            gen->minRows = 2;
            gen->moveNext = HandleMoveNextUp;
            gen->getNextTarget = HandleGetTargetUp;
            break;
        case SlideDown:
            gen->minCols = 1;
            gen->minRows = 2;
            gen->moveNext = HandleMoveNextDown;
            gen->getNextTarget = HandleGetTargetDown;
            break;
        case SlideLeft:
            gen->minCols = 2;
            gen->minRows = 1;
            gen->moveNext = HandleMoveNextLeft;
            gen->getNextTarget = HandleGetTargetLeft;
            break;
        case SlideRight:
            gen->minCols = 2;
            gen->minRows = 1;
            gen->moveNext = HandleMoveNextRight;
            gen->getNextTarget = HandleGetTargetRight;
            break;
        default:
            LOG_ASSERTMSG_REASON(0, "invalid slide direction!", ArgumentOutOfRangeReason);
            break;
    }
    return gen;
}

void CellGeneratorDispose(NextCellGenerator *generator) {
    generator->gb = 0;
    free(generator);
}

void CellGeneratorReset(NextCellGenerator *generator) {
    generator->current = GameBoardMakeCell(-1, -1);
}

int CellGeneratorMoveNext(NextCellGenerator *generator) {
    LOG_ASSERT_REASON(generator && generator->gb && generator->moveNext, ArgumentNullReason);

    uint32_t rows = GameBoardNumRows(generator->gb);
    uint32_t cols = GameBoardNumCols(generator->gb);
    if (cols < generator->minCols || rows < generator->minRows) {
        return 0;
    }
    return generator->moveNext(generator);
}

void CellGeneratorGetCurrent(NextCellGenerator *generator, GameBoardCell *toSlide, GameBoardCell *target) {
    LOG_ASSERT_REASON(generator && generator->gb && generator->getNextTarget, ArgumentNullReason);
    *toSlide = generator->current;
    *target = generator->getNextTarget(generator);
}
