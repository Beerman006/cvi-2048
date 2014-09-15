#include <windows.h>
#include <ansi_c.h>
#include <cvirte.h>
#include <toolbox.h>
#include "SymbolParser.h"
#include "CVI_Test.h"

#define EXPAND_GET_EXPORTS_PREFIX(prefix) #prefix
#define GET_EXPORTS_PREFIX(prefix) EXPAND_GET_EXPORTS_PREFIX(prefix)

typedef void (*ContextHandler)(TestContext *);

typedef struct LoadTestInfo {
    SymbolParser *parser;
    ContextHandler executeOnContext;
} LoadTestInfo;

extern TestContext *CreateContext();
extern void FreeContext(TestContext *context);

static TestContext *context;

static int IsGetTestMethodName(const char *name) {
    AssertMsg(name, "name should not be null");
    
    return strstr(name, GET_EXPORTS_PREFIX(MODULE_TEST_NAME_PREFIX)) != 0;
}

static int LoadUserTests(int index, void *ptrToItem, void *callbackData) {
    char *name = *(char **)ptrToItem;
    if (!IsGetTestMethodName(name)) {
        return 0;
    }
    LoadTestInfo *info = (LoadTestInfo *)callbackData;
    ContextHandler getTestMethods = (ContextHandler)LoadFunction(info->parser, name);
    getTestMethods(context);
    info->executeOnContext(context);
    return 0;
}

int main (int argc, char *argv[]) {
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;    /* out of memory */
    
    if (argc != 2) {
        fprintf(stderr, "you must pass the path to a module to test");
        return -1;
    }
    
    char *testModule = argv[1];
    if (!FileExists(testModule, 0)) {
        fprintf(stderr, "you must pass the path to a module to test");
    }
    
    ListType names;
    SymbolParser *parser = MakeParser(testModule);
    names = GetExports(parser);
    
    context = CreateContext();
    LoadTestInfo info = { .parser = parser, .executeOnContext = context->ExecuteTests };
    ListApplyToEach(names, 1, LoadUserTests, &info);
    
    DeleteParser(parser);
    FreeContext(context);
    return 0;
}
