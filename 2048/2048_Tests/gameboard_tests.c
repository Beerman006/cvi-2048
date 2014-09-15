#include <toolbox.h>
#include <ansi_c.h>
#include "../../CVI_Core/log.h"
#include "../../CVI_Test/CVI_Test.h"
#include "../../2048/2048/gameboard.h"

#define NUM_ROWS 1
#define NUM_COLS 2

static GameBoard *gameBoard;
static int tileAddCount;
static int tileRemoveCount;
static Tile *tileAdded;
static Tile *tileRemoved;

static void TestHandleAddRemoveTile(Tile *tile, AddRemoveReason reason, void *data) {
    int *countPtr = (int *)data;
    if (reason == Added) {
        tileAdded = tile;
    } else if (reason == Removed) {
        tileRemoved = tile;
    } else {
        LOG_ASSERTMSG(0, "invalid add remove reason");
    }
    (*countPtr)++;
}

/// REGION START Tests
void TESTEXPORT GameBoardGetRows(TestContext *context) {
    ASSERT_INT_EQUAL(NUM_ROWS, GameBoardNumRows(gameBoard), "should have 1 row!");
}

void TESTEXPORT GameBoardGetCols(TestContext *context) {
    ASSERT_INT_EQUAL(NUM_COLS, GameBoardNumCols(gameBoard), "should have 2 cols!");
}

void TESTEXPORT GameBoardNoInitialTiles(TestContext *context) {
    int noFirstTile = !GameBoardGetTile(gameBoard, 0, 0);
    int noSecondTile = !GameBoardGetTile(gameBoard, 0, 1);
    ASSERT_TRUE(noFirstTile && noSecondTile, "should have no tiles!");
}

void TESTEXPORT GameBoardAssertRowGreaterThan0(TestContext *context) {
    EXPECT_ASSERT_REASON(ArgumentOutOfRangeReason);
    GameBoard *gb = GameBoardCreate(0, 1);
}

void TESTEXPORT GameBoardAssertColGreaterThan0(TestContext *context) {
    EXPECT_ASSERT_REASON(ArgumentOutOfRangeReason);
    GameBoard *gb = GameBoardCreate(1, 0);
}

void TESTEXPORT GameBoardCanAddTile1(TestContext *context) {
    ASSERT_TRUE(GameBoardCanAddTile(gameBoard, 0, 0), "can add tile at 0,0");
}

void TESTEXPORT GameBoardCanAddTile2(TestContext *context) {
    ASSERT_TRUE(GameBoardCanAddTile(gameBoard, 0, 1), "can add tile at 0,1");
}

void TESTEXPORT GameBoardCanNotAddTile(TestContext *context) {
    EXPECT_ASSERT_REASON(ArgumentOutOfRangeReason);
    GameBoardCanAddTile(gameBoard, 1, 1);
}

void TESTEXPORT GameBoardCanNotAddTile2(TestContext *context) {
    EXPECT_ASSERT_REASON(ArgumentOutOfRangeReason);
    GameBoardCanAddTile(gameBoard, 0, 2);
}

void TESTEXPORT GameBoardCanNotAddTile3(TestContext *context) {
    EXPECT_ASSERT_REASON(ArgumentNullReason);
    GameBoardCanAddTile(0, 0, 0);
}

void TESTEXPORT GameBoardGetTileFalse(TestContext *context) {
    ASSERT_FALSE(GameBoardGetTile(gameBoard, 0, 0), "Should not have tile at 0,0");
}

void TESTEXPORT GameBoardTestAddTile(TestContext *context) {
    GameBoardAddTile(gameBoard, 0, 0);
    ASSERT_TRUE(GameBoardGetTile(gameBoard, 0, 0), "Should have tile at 0,0");
    ASSERT_FALSE(GameBoardGetTile(gameBoard, 0, 1), "Should not have tile at 0,1");
}

void TESTEXPORT GameBoardAddTileNotifies(TestContext *context) {
    GameBoardAddTileAddRemoveHandler(gameBoard, &tileAddCount, TestHandleAddRemoveTile);
    GameBoardAddTile(gameBoard, 0, 0);

    ASSERT_IS_NULL(tileRemoved, "a tile should not have been removed");
    ASSERT_NOT_NULL(tileAdded, "a tile should have been added");
    ASSERT_INT_EQUAL(1, tileAddCount, "should have gotten a single add");
    ASSERT_INT_EQUAL(0, tileRemoveCount, "should have gotten no remove");
}

void TESTEXPORT GameBoardRemoveTileNotifies(TestContext *context) {
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);

    GameBoardAddTileAddRemoveHandler(gameBoard, &tileRemoveCount, TestHandleAddRemoveTile);

    GameBoardTrySlide(gameBoard, SlideLeft);
    ASSERT_NOT_NULL(tileRemoved, "a tile should have been removed");
    ASSERT_INT_EQUAL(1, tileRemoveCount, "should have gotten a remove");
}

void TESTEXPORT GameBoardGetOpenTile(TestContext *context) {
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardCell cell;
    int openTile = GameBoardTryGetOpenCell(gameBoard, &cell);

    ASSERT_TRUE(openTile, "should have an open tile");
    ASSERT_INT_EQUAL(0, cell.row, "should have open on row 0");
    ASSERT_INT_EQUAL(1, cell.col, "should have open on col 1");
}

void TESTEXPORT GameBoardGetOpenTileGetsAllTiles(TestContext *context) {
    GameBoard *board = GameBoardCreate(2, 2);

    int found00 = 0, found01 = 0, found10 = 0, found11 = 0;
    // 100 should be enough to find all the tiles, if not, our algorithm should be double checked.
    for(int i = 0; i < 100; i++) {
        GameBoardCell cell;
        ASSERT_TRUE(GameBoardTryGetOpenCell(board, &cell), "should have open cell");
        found00 |= cell.row == 0 && cell.col == 0;
        found01 |= cell.row == 0 && cell.col == 1;
        found10 |= cell.row == 1 && cell.col == 0;
        found11 |= cell.row == 1 && cell.col == 1;
        if (found00 && found01 && found10 && found11) {
            GameBoardDispose(board);
            return;
        }
    }

    GameBoardDispose(board);
    ASSERT_TRUE(found00, "did not find 0,0");
    ASSERT_TRUE(found01, "did not find 0,1");
    ASSERT_TRUE(found10, "did not find 1,0");
    ASSERT_TRUE(found11, "did not find 1,1");
}

void TESTEXPORT GameBoardNoOpenTiles(TestContext *context) {
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);

    GameBoardCell cell;
    ASSERT_FALSE(GameBoardTryGetOpenCell(gameBoard, &cell), "should not have any open cells");
    ASSERT_FALSE(GameBoardIsValidCell(gameBoard, cell), "should not have valid cell");
}

void TESTEXPORT GameBoard_SlideTiles_Left1(TestContext *context) {
    gameBoard = GameBoardCreate(1, 2);
    GameBoardAddTile(gameBoard, 0, 1);

    GameBoardTrySlide(gameBoard, SlideLeft);

    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should now be in 0, 0");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "The tile should have value 2");
}

void TESTEXPORT GameBoard_SlideTiles_Left2(TestContext *context) {
    gameBoard = GameBoardCreate(1, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);

    GameBoardTrySlide(gameBoard, SlideLeft);

    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should now be in 0, 0");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "Should have doubled the tiles value");
}

void TESTEXPORT GameBoard_SlideTiles_Left3(TestContext *context) {
    gameBoard = GameBoardCreate(1, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);

    GameBoardTrySlide(gameBoard, SlideLeft);
    GameBoardAddTile(gameBoard, 0, 1);
    GameBoardTrySlide(gameBoard, SlideLeft);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should not have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not have slid");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "Should have doubled the tiles value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "Should have original tile value");
}

void TESTEXPORT GameBoard_SlideTiles_Left4(TestContext * context) {
    gameBoard = GameBoardCreate(2, 3);
    GameBoardAddTile(gameBoard, 0, 2);
    GameBoardAddTile(gameBoard, 1, 2);

    GameBoardTrySlide(gameBoard, SlideLeft);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should slide to front");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should slide to front");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 2), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 2), "tile should not longer be present");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "tile should have original value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Left5(TestContext * context) {
    gameBoard = GameBoardCreate(2, 3);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);
    GameBoardAddTile(gameBoard, 0, 2);
    GameBoardAddTile(gameBoard, 1, 2);

    GameBoardTrySlide(gameBoard, SlideLeft);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should slide to front");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should slide to front");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 2), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 2), "tile should not longer be present");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "tile should have original value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Left6(TestContext * context) {
    gameBoard = GameBoardCreate(2, 3);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);
    GameBoardAddTile(gameBoard, 0, 1);
    GameBoardAddTile(gameBoard, 1, 1);
    GameBoardAddTile(gameBoard, 0, 2);
    GameBoardAddTile(gameBoard, 1, 2);

    GameBoardTrySlide(gameBoard, SlideLeft);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should slide to front");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should slide to front");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should have slid");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 2), "tile should not longer be present");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should have slid");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 2), "tile should not longer be present");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "tile should have doubled value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "tile should have doubled value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "tile should have original value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 1)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Right1(TestContext *context) {
    gameBoard = GameBoardCreate(1, 2);
    GameBoardAddTile(gameBoard, 0, 0);

    GameBoardTrySlide(gameBoard, SlideRight);

    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should now be in 0, 0");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "The tile should have value 2");
}

void TESTEXPORT GameBoard_SlideTiles_Right2(TestContext *context) {
    gameBoard = GameBoardCreate(1, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);

    GameBoardTrySlide(gameBoard, SlideRight);

    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should now be in 0, 0");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "Should have doubled the tiles value");
}

void TESTEXPORT GameBoard_SlideTiles_Right3(TestContext *context) {
    gameBoard = GameBoardCreate(1, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);

    GameBoardTrySlide(gameBoard, SlideRight);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardTrySlide(gameBoard, SlideRight);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should not have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not have slid");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "Should have doubled the tiles value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "Should have original tile value");
}

void TESTEXPORT GameBoard_SlideTiles_Right4(TestContext * context) {
    gameBoard = GameBoardCreate(2, 3);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);

    GameBoardTrySlide(gameBoard, SlideRight);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 2), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 2), "tile should slide to end");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not longer be present");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 2)), "tile should have original value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 2)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Right5(TestContext * context) {
    gameBoard = GameBoardCreate(2, 3);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);
    GameBoardAddTile(gameBoard, 0, 2);
    GameBoardAddTile(gameBoard, 1, 2);

    GameBoardTrySlide(gameBoard, SlideRight);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 2), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 2), "tile should slide to end");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not longer be present");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 2)), "tile should have original value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 1, 2)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Right6(TestContext * context) {
    gameBoard = GameBoardCreate(2, 3);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);
    GameBoardAddTile(gameBoard, 0, 1);
    GameBoardAddTile(gameBoard, 1, 1);
    GameBoardAddTile(gameBoard, 0, 2);
    GameBoardAddTile(gameBoard, 1, 2);

    GameBoardTrySlide(gameBoard, SlideRight);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 2), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 2), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should have slid");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not longer be present");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should have slid");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not longer be present");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 2)), "tile should have doubled value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 1, 2)), "tile should have doubled value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "tile should have original value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 1)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Up1(TestContext *context) {
    gameBoard = GameBoardCreate(2, 1);
    GameBoardAddTile(gameBoard, 1, 0);

    GameBoardTrySlide(gameBoard, SlideUp);

    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should now be in 0, 0");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "The tile should have value 2");
}

void TESTEXPORT GameBoard_SlideTiles_Up2(TestContext *context) {
    gameBoard = GameBoardCreate(2, 1);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);

    GameBoardTrySlide(gameBoard, SlideUp);

    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should now be in 0, 0");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "Should have doubled the tiles value");
}

void TESTEXPORT GameBoard_SlideTiles_Up3(TestContext *context) {
    gameBoard = GameBoardCreate(2, 1);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);

    GameBoardTrySlide(gameBoard, SlideUp);
    GameBoardAddTile(gameBoard, 1, 0);
    GameBoardTrySlide(gameBoard, SlideUp);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not have slid");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "Should have doubled the tiles value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "Should have original tile value");
}

void TESTEXPORT GameBoard_SlideTiles_Up4(TestContext * context) {
    gameBoard = GameBoardCreate(3, 2);
    GameBoardAddTile(gameBoard, 2, 0);
    GameBoardAddTile(gameBoard, 2, 1);

    GameBoardTrySlide(gameBoard, SlideUp);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should slide to end");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 2, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 2, 0), "tile should not longer be present");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "tile should have original value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Up5(TestContext * context) {
    gameBoard = GameBoardCreate(3, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);
    GameBoardAddTile(gameBoard, 2, 0);
    GameBoardAddTile(gameBoard, 2, 1);

    GameBoardTrySlide(gameBoard, SlideUp);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should slide to end");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 2, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 2, 0), "tile should not longer be present");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "tile should have original value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Up6(TestContext * context) {
    gameBoard = GameBoardCreate(3, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);
    GameBoardAddTile(gameBoard, 2, 0);
    GameBoardAddTile(gameBoard, 0, 1);
    GameBoardAddTile(gameBoard, 1, 1);
    GameBoardAddTile(gameBoard, 2, 1);

    GameBoardTrySlide(gameBoard, SlideUp);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should have slid");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 2, 0), "tile should no longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 2, 1), "tile should no longer be present");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "tile should have doubled value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 0, 1)), "tile should have doubled value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 1)), "tile should have original value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Down1(TestContext *context) {
    gameBoard = GameBoardCreate(2, 1);
    GameBoardAddTile(gameBoard, 0, 0);

    GameBoardTrySlide(gameBoard, SlideDown);

    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should now be in 0, 0");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "The tile should have value 2");
}

void TESTEXPORT GameBoard_SlideTiles_Down2(TestContext *context) {
    gameBoard = GameBoardCreate(2, 1);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);

    GameBoardTrySlide(gameBoard, SlideDown);

    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should now be in 0, 0");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "Should have doubled the tiles value");
}

void TESTEXPORT GameBoard_SlideTiles_Down3(TestContext *context) {
    gameBoard = GameBoardCreate(2, 1);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);

    GameBoardTrySlide(gameBoard, SlideDown);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardTrySlide(gameBoard, SlideDown);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not have slid");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "Should have doubled the tiles value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 0, 0)), "Should have original tile value");
}

void TESTEXPORT GameBoard_SlideTiles_Down4(TestContext * context) {
    gameBoard = GameBoardCreate(3, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);

    GameBoardTrySlide(gameBoard, SlideDown);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 2, 0), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 2, 1), "tile should slide to end");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not longer be present");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 2, 0)), "tile should have original value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 2, 1)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Down5(TestContext * context) {
    gameBoard = GameBoardCreate(3, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 0, 1);
    GameBoardAddTile(gameBoard, 2, 0);
    GameBoardAddTile(gameBoard, 2, 1);

    GameBoardTrySlide(gameBoard, SlideDown);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 2, 0), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 2, 1), "tile should slide to end");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should not longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should not longer be present");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 2, 0)), "tile should have original value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 2, 1)), "tile should have original value");
}

void TESTEXPORT GameBoard_SlideTiles_Down6(TestContext * context) {
    gameBoard = GameBoardCreate(3, 2);
    GameBoardAddTile(gameBoard, 0, 0);
    GameBoardAddTile(gameBoard, 1, 0);
    GameBoardAddTile(gameBoard, 2, 0);
    GameBoardAddTile(gameBoard, 0, 1);
    GameBoardAddTile(gameBoard, 1, 1);
    GameBoardAddTile(gameBoard, 2, 1);

    GameBoardTrySlide(gameBoard, SlideDown);

    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 2, 0), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 2, 1), "tile should slide to end");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 0), "tile should have slid");
    ASSERT_NOT_NULL(GameBoardGetTile(gameBoard, 1, 1), "tile should have slid");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 0), "tile should no longer be present");
    ASSERT_IS_NULL(GameBoardGetTile(gameBoard, 0, 1), "tile should no longer be present");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 2, 0)), "tile should have doubled value");
    ASSERT_INT_EQUAL(4, TileGetValue(GameBoardGetTile(gameBoard, 2, 1)), "tile should have doubled value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 1)), "tile should have original value");
    ASSERT_INT_EQUAL(2, TileGetValue(GameBoardGetTile(gameBoard, 1, 0)), "tile should have original value");
}
/// REGION END

static void DefaultInitGameBoard(TestContext *context) {
    gameBoard = GameBoardCreate(NUM_ROWS, NUM_COLS);
}

static void DefaultCleanupGameBoard(TestContext *context) {
    free(gameBoard);
    gameBoard = 0;
    tileAdded = 0;
    tileRemoved = 0;
    tileAddCount = 0;
    tileRemoveCount = 0;
}

BEGIN_MODULE_TEST(gameboard)
    ADD_TEST(GameBoardGetRows, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardGetCols, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardNoInitialTiles, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardCanAddTile1, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardCanAddTile2, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardAddTileNotifies, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardRemoveTileNotifies, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardCanNotAddTile, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardCanNotAddTile2, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardCanNotAddTile3, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardTestAddTile, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardGetTileFalse, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardGetOpenTile, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardNoOpenTiles, DefaultInitGameBoard, DefaultCleanupGameBoard)
    ADD_TEST(GameBoardGetOpenTileGetsAllTiles, 0, 0)
    ADD_TEST(GameBoardAssertRowGreaterThan0, 0, 0)
    ADD_TEST(GameBoardAssertColGreaterThan0, 0, 0)
    ADD_TEST(GameBoard_SlideTiles_Left1, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Left2, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Left3, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Left4, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Left5, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Left6, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Right1, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Right2, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Right3, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Right4, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Right5, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Right6, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Up1, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Up2, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Up3, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Up4, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Up5, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Up6, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Down1, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Down2, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Down3, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Down4, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Down5, 0, DefaultCleanupGameBoard)
    ADD_TEST(GameBoard_SlideTiles_Down6, 0, DefaultCleanupGameBoard)
END_MODULE_TEST
