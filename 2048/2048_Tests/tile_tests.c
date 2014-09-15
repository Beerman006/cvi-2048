#include <ansi_c.h>
#include "../../CVI_Test/CVI_Test.h"
#include "../../2048/2048/tile.h"

static Tile *tile1;
static Tile *tile2;
static Tile *notified;
static int notifyCount;

static void TestTileDispose(Tile *tile);
static void TestMergeNotifies(Tile *tile, void *data);

void TESTEXPORT TestTileGetRow(TestContext *context) {
    uint32_t row = 3;
    Tile *t = TileCreate(row, 0);

    ASSERT_INT_EQUAL(row, TileGetRow(t), "wrong row given for tile!");
    TileDispose(t);
}

void TESTEXPORT TestTileGetColumn(TestContext *context) {
    uint32_t col = 6;
    Tile *t = TileCreate(0, col);

    ASSERT_INT_EQUAL(col, TileGetColumn(t), "wrong column given for tile!");
    TileDispose(t);
}

void TESTEXPORT TestTileCanMerge(TestContext *context) {
    ASSERT_TRUE(TileCanMerge(tile1, tile2), "could not merge!");
}

void TESTEXPORT TileCanNotMergeDifferentValues(TestContext *context) {
    TileMerge(tile1, tile2);
    Tile *tile3 = TileCreate(0, 1);

    ASSERT_FALSE(TileCanMerge(tile1, tile3), "tiles had different values");

    TileDispose(tile3);
}

void TESTEXPORT TileCanNotMergeRowsTooFar(TestContext *context) {
    Tile *t1 = TileCreate(0, 0);
    Tile *t2 = TileCreate(2, 0);

    ASSERT_FALSE(TileCanMerge(t1, t2), "tiles too far away");

    TileDispose(t1);
    TileDispose(t2);
}

void TESTEXPORT TileCanNotMergeColsTooFar(TestContext *context) {
    Tile *t1 = TileCreate(0, 0);
    Tile *t2 = TileCreate(0, 2);

    ASSERT_FALSE(TileCanMerge(t1, t2), "tiles too far away");

    TileDispose(t1);
    TileDispose(t2);
}

void TESTEXPORT TileCanNotMergeNeitherRowColSame(TestContext *context) {
    Tile *t1 = TileCreate(0, 0);
    Tile *t2 = TileCreate(1, 1);

    ASSERT_FALSE(TileCanMerge(t1, t2), "tiles too far away");

    TileDispose(t1);
    TileDispose(t2);
}

void TESTEXPORT TestTileMerge(TestContext *context) {
    uint32_t original = TileGetValue(tile1);
    TileMerge(tile1, tile2);
    ASSERT_INT_EQUAL(original * 2, TileGetValue(tile1), "should have doubled tile value!");
}

void TESTEXPORT TileMergeNotifies(TestContext *context) {
    TileAddValueChangeHandler(tile1, TestMergeNotifies, 0);
    TileMerge(tile1, tile2);

    ASSERT_PTR_EQUAL(tile1, notified, "tile1 should have notified!");
    ASSERT_INT_EQUAL(1, notifyCount, "tile1 should have notified once!");
    TileRemoveValueChangeHandler(tile1, TestMergeNotifies);
}

void TESTEXPORT TileMergeNotifiesTwice(TestContext *context) {
    TileAddValueChangeHandler(tile1, TestMergeNotifies, 0);
    TileAddValueChangeHandler(tile1, TestMergeNotifies, 0);
    TileMerge(tile1, tile2);

    ASSERT_PTR_EQUAL(tile1, notified, "tile1 should have notified!");
    ASSERT_INT_EQUAL(2, notifyCount, "tile1 should have notified once!");
    TileRemoveValueChangeHandler(tile1, TestMergeNotifies);
    TileRemoveValueChangeHandler(tile1, TestMergeNotifies);
}

void TESTEXPORT TileMergeNotifiesOnce(TestContext *context) {
    TileAddValueChangeHandler(tile1, TestMergeNotifies, 0);
    TileAddValueChangeHandler(tile1, TestMergeNotifies, 0);
    TileRemoveValueChangeHandler(tile1, TestMergeNotifies);
    TileMerge(tile1, tile2);

    ASSERT_PTR_EQUAL(tile1, notified, "tile1 should have notified!");
    ASSERT_INT_EQUAL(1, notifyCount, "tile1 should have notified once!");
    TileRemoveValueChangeHandler(tile1, TestMergeNotifies);
}

void TESTEXPORT TileMergeRemoveDoesNotNotify(TestContext *context) {
    TileAddValueChangeHandler(tile1, TestMergeNotifies, 0);
    TileRemoveValueChangeHandler(tile1, TestMergeNotifies);
    TileMerge(tile1, tile2);

    ASSERT_IS_NULL(notified, "tile1 should not have notified!");
}

static void DefaultInitTileTest(TestContext *context) {
    tile1 = TileCreate(0, 0);
    tile2 = TileCreate(0, 1);
    notified = 0;
    notifyCount = 0;
}

static void DefaultCleanupTileTest(TestContext *context) {
    TileDispose(tile1);
    TileDispose(tile2);
    tile1 = 0;
    tile2 = 0;
}

static void TestMergeNotifies(Tile *tile, void *data) {
    notified = tile;
    notifyCount++;
}

static void TestTileDispose(Tile *tile) {
    if (tile == tile1) {
        tile1 = 0;
    } else if (tile == tile2) {
        tile2 = 0;
    }
    TileDispose(tile);
}

BEGIN_MODULE_TEST(tile)
    ADD_TEST(TestTileGetRow, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TestTileGetColumn, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TestTileCanMerge, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TestTileMerge, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TileMergeNotifies, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TileMergeNotifiesTwice, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TileMergeNotifiesOnce, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TileMergeRemoveDoesNotNotify, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TileCanNotMergeDifferentValues, DefaultInitTileTest, DefaultCleanupTileTest)
    ADD_TEST(TileCanNotMergeRowsTooFar, 0, 0)
    ADD_TEST(TileCanNotMergeColsTooFar, 0, 0)
    ADD_TEST(TileCanNotMergeRowsTooFar, 0, 0)
    ADD_TEST(TileCanNotMergeNeitherRowColSame, 0, 0)
END_MODULE_TEST
