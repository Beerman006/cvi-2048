#include <ansi_c.h>
#include <userint.h>
#include "toolbox.h"
#include "../2048/game.h"
#include "../../CVI_Core/log.h"

#define TILE_PADDING 6
#define TILE_FONT "TileFont"

typedef struct Window {
    Controller *controller;
    GameBoard *gameBoard;
    GameUpdateHandler *updateHandler;
    int boardPanel;
    int tileCanvas;
    ListType tilesToUpdate;
} Window;

static int OnPanelEvent(int panel, int event, void *ptr, int eventData1, int eventData2);

static Rect GetTileRect(int height, int width, uint32_t numRows, uint32_t numCols, uint32_t row, uint32_t col) {
    int colMarginWidth = (numCols + 1) * TILE_PADDING;
    int rowMarginWidth = (numRows + 1) * TILE_PADDING;
    int tileWidth = (int)floor((width - colMarginWidth) / numCols);
    int tileHeight = (int)floor((height - rowMarginWidth) / numRows);
    int leftOverWidth = width - (tileWidth * numCols + colMarginWidth);
    int leftOverHeight = height - (tileHeight * numRows + rowMarginWidth);
    int topRowMarginExtra = (int)ceil(leftOverHeight / 2);
    int leftColMarginExtra = (int)ceil(leftOverWidth / 2);
    int top = tileHeight * row + (row + 1) * TILE_PADDING + topRowMarginExtra;
    int left = tileWidth * col + (col + 1) * TILE_PADDING + leftColMarginExtra;

    return MakeRect(top, left, tileHeight, tileWidth);
}

static int GetColorForTile(Tile *tile) {
    if (!tile) {
        return 0xCDC0B4;
    }
    uint32_t val = TileGetValue(tile);
    switch(val) {
        case 2: return 0xEEE4DA;
        case 4: return 0xEDE0C8;
        case 8: return 0xF2B179;
        case 16: return 0xF59563;
        case 32: return 0xF67C5F;
        case 64: return 0xF65D3B;
        case 128: return 0xEDCE71;
        case 256: return 0xEDCC61;
        case 512: return 0xECC850;
        case 1024: return 0xEDC53F;
        case 2048: return 0xEEC22E;
    }
    LOG_ASSERTMSG(0, "got unexpected value!");
    return 0;
}

static void DrawTile(Window *window, uint32_t row, uint32_t col) {
    int p = window->boardPanel;
    int c = window->tileCanvas;
    CanvasStartBatchDraw(p, c);

    int height, width;
    uint32_t numRows, numCols;
    numRows = GameBoardNumRows(window->gameBoard);
    numCols = GameBoardNumCols(window->gameBoard);
    GetCtrlAttribute(p, c, ATTR_WIDTH, &width);
    GetCtrlAttribute(p, c, ATTR_HEIGHT, &height);
    Rect r = GetTileRect(height, width, numRows, numCols, row, col);
    CanvasClear(p, c, r);

    Tile *tile = GameBoardGetTile(window->gameBoard, row, col);
    int color = GetColorForTile(tile);
    SetCtrlAttribute(p, c, ATTR_PEN_COLOR, color);
    SetCtrlAttribute(p, c, ATTR_PEN_FILL_COLOR, color);
    CanvasDrawRoundedRect(p, c, r, 6, 6, VAL_DRAW_FRAME_AND_INTERIOR);

    if (tile != 0) {
        char numberText[64];
        uint32_t val = TileGetValue(tile);
        snprintf(numberText, sizeof(numberText), "%u", val);
        color = val > 4 ? 0xF6F7FB : 0x756C65;
        SetCtrlAttribute(p, c, ATTR_PEN_COLOR, color);
        //SetCtrlAttribute(p, c, ATTR_PEN_FILL_COLOR, VAL_TRANSPARENT);
        CanvasDrawText(p,c, numberText, TILE_FONT, r, VAL_CENTER);
    }

    CanvasEndBatchDraw(p, c);
}

static void DrawAllTiles(Window *window) {
    int p = window->boardPanel;
    int c = window->tileCanvas;
    GameBoard *gb = window->gameBoard;
    uint32_t rows = GameBoardNumRows(gb);
    uint32_t cols = GameBoardNumCols(gb);

    CanvasStartBatchDraw(p, c);
    CanvasClear(p, c, VAL_ENTIRE_OBJECT);
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            DrawTile(window, i, j);
        }
    }
    int level = CanvasEndBatchDraw(p, c);
    LOG_ASSERTMSG(!level, "unmatched end batch draw!");
}

static void HandleBeginUpdateGame(void *target, GameBoard *gameBoard) {
    Window *window = (Window *)target;
    LOG_ASSERTMSG(!window->tilesToUpdate, "When we handle a begin update, our list should be NULL");
    window->tilesToUpdate = ListCreate(sizeof(GameBoardCell));
    CanvasStartBatchDraw(window->boardPanel, window->tileCanvas);
}

static int UpdateChangedTiles(int index, void *ptr, void *data) {
    GameBoardCell *cell = (GameBoardCell *)ptr;
    Window *window = (Window *)data;
    DrawTile(window, cell->row, cell->col);
    return 0;
}

static void HandleEndUpdateGame(void *target, GameBoard *gameBoard) {
    Window *window = (Window *)target;

    ListApplyToEach(window->tilesToUpdate, 1, UpdateChangedTiles, window);

    int level = CanvasEndBatchDraw(window->boardPanel, window->tileCanvas);
    LOG_ASSERTMSG(!level, "unmatched end batch draw!");
    ListDispose(window->tilesToUpdate);
    window->tilesToUpdate = 0;
}

static void HandleTileChange(void *target, Tile *tile) {
    Window *window = (Window *)target;
    GameBoardCell cell = GameBoardMakeCell(TileGetRow(tile), TileGetColumn(tile));
    if (window->tilesToUpdate != 0) {
        ListInsertItem(window->tilesToUpdate, &cell, END_OF_LIST);
    } else {
        DrawTile(window, cell.row, cell.col);
    }
}

static GameUpdateHandler *MakeUpdateHandler(Window *window) {
    GameUpdateHandler *handler = calloc(1, sizeof(GameUpdateHandler));

    handler->target = window;
    handler->endUpdateGame = HandleEndUpdateGame;
    handler->beginUpdateGame = HandleBeginUpdateGame;
    handler->handleTileAdded = HandleTileChange;
    handler->handleTileRemoved = HandleTileChange;
    handler->handleTileValueChange = HandleTileChange;

    return handler;
}

static int ToCanvasIndex(GameBoard *gameBoard, uint32_t row, uint32_t col) {
    LOG_ASSERT_REASON(gameBoard, ArgumentNullReason);
    uint32_t numRows = GameBoardNumRows(gameBoard);
    uint32_t numCols = GameBoardNumCols(gameBoard);
    LOG_ASSERT_REASON(row < numRows, ArgumentOutOfRangeReason);
    LOG_ASSERT_REASON(col < numCols, ArgumentOutOfRangeReason);

    uint32_t idx = col + row * numCols;
    return idx;
}

static Rect HandlePanelSizing(Window *window, int isUpDown, int eventData2) {
    Rect r;
    GetPanelEventRect(eventData2, &r);

    int p = window->boardPanel;
    int c = window->tileCanvas;
    int titleBarThickness, frameWidth, frameHeight, height, width;
    GetPanelAttribute(p, ATTR_TITLEBAR_ACTUAL_THICKNESS, &titleBarThickness);
    GetPanelAttribute(p, ATTR_FRAME_ACTUAL_HEIGHT, &frameHeight);
    GetPanelAttribute(p, ATTR_FRAME_ACTUAL_WIDTH, &frameWidth);
    height = r.height - frameHeight * 2 - titleBarThickness;
    width = r.width - frameWidth * 2;

    height = height < 200 ? 200 : height;
    width = width < 200 ? 200 : width;
    if (isUpDown) { width = height; }
    else { height = width; }
    SetCtrlAttribute(p, c, ATTR_HEIGHT, height);
    SetCtrlAttribute(p, c, ATTR_WIDTH, width);
    int size = 64;
    if (width >= 650) { size = 64; }
    else if (width < 300) { size = 18; }
    else if (width < 410) { size = 24; }
    else if (width < 550) { size = 36; }
    else { size = 48; }
    CreateMetaFont(TILE_FONT, "Segoe UI", size, 1, 0, 0, 0);

    height += titleBarThickness + frameHeight * 2;
    width += frameWidth * 2;
    r.width = width;
    r.height = height;

    DrawAllTiles(window);

    return r;
}

static void HandleKeyPress(Window *window, int key) {
    switch(key) {
        case VAL_UP_ARROW_VKEY:
            ControllerHandleSlide(window->controller, SlideUp);
            break;
        case VAL_DOWN_ARROW_VKEY:
            ControllerHandleSlide(window->controller, SlideDown);
            break;
        case VAL_LEFT_ARROW_VKEY:
            ControllerHandleSlide(window->controller, SlideLeft);
            break;
        case VAL_RIGHT_ARROW_VKEY:
            ControllerHandleSlide(window->controller, SlideRight);
            break;
    }
}

static int OnPanelEvent(int panel, int event, void *ptr, int eventData1, int eventData2) {
    Window *window = (Window *)ptr;
    switch(event) {
        case EVENT_CLOSE:
            QuitUserInterface(0);
            break;
        case EVENT_PANEL_SIZING:
            int isUpDown = eventData1 == PANEL_SIZING_TOP || eventData1 == PANEL_SIZING_BOTTOM;
            Rect restrictedSize = HandlePanelSizing(window, isUpDown, eventData2);
            SetPanelEventRect(eventData2, restrictedSize);
            break;
        case EVENT_KEYPRESS:
            HandleKeyPress(window, eventData1);
            break;
    }

    return 0;
}

static Window *CreateWindow(Game2048 *game) {
    Window *w = calloc(1, sizeof(*w));

    w->gameBoard = Game2048GameBoard(game);
    w->controller = Game2048Controller(game);
    w->updateHandler = MakeUpdateHandler(w);
    w->boardPanel = NewPanel(0, "2048", VAL_AUTO_CENTER, VAL_AUTO_CENTER, 500, 500);
    w->tileCanvas = NewCtrl(w->boardPanel, CTRL_CANVAS, 0, 0, 0);

    ControllerSetGameUpdateHandler(w->controller, w->updateHandler);

    SetPanelAttribute(w->boardPanel, ATTR_BACKCOLOR, 0xBBADA0);
    SetPanelAttribute(w->boardPanel, ATTR_CONFORM_TO_SYSTEM_THEME, 1);
    SetPanelAttribute(w->boardPanel, ATTR_CALLBACK_FUNCTION_POINTER, OnPanelEvent);
    SetPanelAttribute(w->boardPanel, ATTR_CALLBACK_DATA, w);

    int height, width;
    GetPanelAttribute(w->boardPanel, ATTR_WIDTH, &width);
    GetPanelAttribute(w->boardPanel, ATTR_HEIGHT, &height);
    SetCtrlAttribute(w->boardPanel, w->tileCanvas, ATTR_WIDTH, width);
    SetCtrlAttribute(w->boardPanel, w->tileCanvas, ATTR_HEIGHT, height);
    SetCtrlAttribute(w->boardPanel, w->tileCanvas, ATTR_PICT_BGCOLOR, 0xBBADA0);
    SetCtrlAttribute(w->boardPanel, w->tileCanvas, ATTR_DRAW_POLICY, VAL_MARK_FOR_UPDATE);
    SetCtrlAttribute(w->boardPanel, w->tileCanvas, ATTR_ENABLE_ANTI_ALIASING, 1);

    CreateMetaFont(TILE_FONT, "Segoe UI", 36, 1, 0, 0, 0);

    GameBoardCell cell;
    GameBoardTryGetOpenCell(w->gameBoard, &cell);

    GameBoardAddTile(w->gameBoard, cell.row, cell.col);
    DrawAllTiles(w);

    return w;
}

static void RunWindow(UserInterface *ui) {
    LOG_ASSERT_REASON(ui, ArgumentNullReason);
    Window *w = (Window *)ui->data;
    DisplayPanel(w->boardPanel);
    RunUserInterface();
}

static void DisposeWindow(UserInterface *ui) {
    LOG_ASSERT_REASON(ui, ArgumentNullReason);
    Window *w = (Window *)ui->data;
    ListDispose(w->tilesToUpdate);
    DiscardPanel(w->boardPanel);
    free(w->updateHandler);
    w->updateHandler = 0;
    free(w);
    free(ui);
}

UserInterface *WindowCreateUserInterface(Game2048 *game) {
    UserInterface *ui = calloc(1, sizeof(UserInterface));
    ui->data = CreateWindow(game);
    ui->run = RunWindow;
    ui->dispose = DisposeWindow;
    return ui;
}
