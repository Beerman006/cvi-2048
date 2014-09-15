#include <toolbox.h>
#include <ansi_c.h>
#include "../../CVI_Core/log.h"
#include "../../CVI_Test/CVI_Test.h"
#include "../../2048/2048/NextCellGenerator.h"

#define NUM_ROWS 3
#define NUM_COLS 3

static GameBoard *gameBoard;
static NextCellGenerator *generator;

static void RunGeneratorTest(TestContext *context, GameBoardCell cells[6][2], SlideDirection direction) {
    generator = CellGeneratorCreate(gameBoard, direction);
    uint32_t idx = 0;
    while(CellGeneratorMoveNext(generator)) {
        ASSERT_TRUE(idx < 6, "should only slide 6 tiles!");
        GameBoardCell target, toSlide;
        CellGeneratorGetCurrent(generator, &toSlide, &target);
        ASSERT_INT_EQUAL(cells[idx][0].row, toSlide.row, "wrong row for slide");
        ASSERT_INT_EQUAL(cells[idx][0].col, toSlide.col, "wrong col for slide");
        ASSERT_INT_EQUAL(cells[idx][1].row, target.row, "wrong row for target");
        ASSERT_INT_EQUAL(cells[idx][1].col, target.col, "wrong col for target");
        idx++;
    }
    ASSERT_TRUE(idx == 6, "should have slid 6 tiles!");
}

/// REGION START Tests
void TESTEXPORT NextCellGenerator_SlideUp(TestContext *context) {
    GameBoardCell cells[6][2] = {
        { { .row = 1, .col = 0 }, { .row = 0, .col = 0 } },
        { { .row = 1, .col = 1 }, { .row = 0, .col = 1 } },
        { { .row = 1, .col = 2 }, { .row = 0, .col = 2 } },
        { { .row = 2, .col = 0 }, { .row = 1, .col = 0 } },
        { { .row = 2, .col = 1 }, { .row = 1, .col = 1 } },
        { { .row = 2, .col = 2 }, { .row = 1, .col = 2 } }
    };
    RunGeneratorTest(context, cells, SlideUp);
}

void TESTEXPORT NextCellGenerator_SlideDown(TestContext *context) {
    GameBoardCell cells[6][2] = {
        { { .row = 1, .col = 0 }, { .row = 2, .col = 0 } },
        { { .row = 1, .col = 1 }, { .row = 2, .col = 1 } },
        { { .row = 1, .col = 2 }, { .row = 2, .col = 2 } },
        { { .row = 0, .col = 0 }, { .row = 1, .col = 0 } },
        { { .row = 0, .col = 1 }, { .row = 1, .col = 1 } },
        { { .row = 0, .col = 2 }, { .row = 1, .col = 2 } }
    };
    RunGeneratorTest(context, cells, SlideDown);
}

void TESTEXPORT NextCellGenerator_SlideLeft(TestContext *context) {
    GameBoardCell cells[6][2] = {
        { { .row = 0, .col = 1 }, { .row = 0, .col = 0 } },
        { { .row = 1, .col = 1 }, { .row = 1, .col = 0 } },
        { { .row = 2, .col = 1 }, { .row = 2, .col = 0 } },
        { { .row = 0, .col = 2 }, { .row = 0, .col = 1 } },
        { { .row = 1, .col = 2 }, { .row = 1, .col = 1 } },
        { { .row = 2, .col = 2 }, { .row = 2, .col = 1 } }
    };
    RunGeneratorTest(context, cells, SlideLeft);
}

void TESTEXPORT NextCellGenerator_SlideRight(TestContext *context) {
    GameBoardCell cells[6][2] = {
        { { .row = 0, .col = 1 }, { .row = 0, .col = 2 } },
        { { .row = 1, .col = 1 }, { .row = 1, .col = 2 } },
        { { .row = 2, .col = 1 }, { .row = 2, .col = 2 } },
        { { .row = 0, .col = 0 }, { .row = 0, .col = 1 } },
        { { .row = 1, .col = 0 }, { .row = 1, .col = 1 } },
        { { .row = 2, .col = 0 }, { .row = 2, .col = 1 } }
    };
    RunGeneratorTest(context, cells, SlideRight);
}
/// REGION END

static void DefaultInitNextCellGenerator(TestContext *context) {
    gameBoard = GameBoardCreate(NUM_ROWS, NUM_COLS);
}

static void DefaultCleanupNextCellGenerator(TestContext *context) {
    free(gameBoard);
    free(generator);
    gameBoard = 0;
    generator = 0;
}

BEGIN_MODULE_TEST(nextcellgenerator)
    ADD_TEST(NextCellGenerator_SlideUp, DefaultInitNextCellGenerator, DefaultCleanupNextCellGenerator)
    ADD_TEST(NextCellGenerator_SlideDown, DefaultInitNextCellGenerator, DefaultCleanupNextCellGenerator)
    ADD_TEST(NextCellGenerator_SlideLeft, DefaultInitNextCellGenerator, DefaultCleanupNextCellGenerator)
    ADD_TEST(NextCellGenerator_SlideRight, DefaultInitNextCellGenerator, DefaultCleanupNextCellGenerator)
END_MODULE_TEST
