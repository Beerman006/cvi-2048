#include "toolbox.h"
#include <utility.h>
#include <ansi_c.h>
#include "../CVI_Core/log.h"

//==============================================================================
//
// Title:       TestContext.c
// Purpose:     A short description of the implementation.
//
// Created on:  6/20/2014 at 10:55:58 PM by Nick Beer.
// Copyright:   National Instruments. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files
#include "CVI_Test.h"
#include "TestContext.h"

//==============================================================================
// Constants
#define PRINT_MSG(msg, ...) if (BeingDebuggedByCVI()) DebugPrintf(msg, ##__VA_ARGS__); else printf(msg, ##__VA_ARGS__);
#define LOG_CONTEXT_KEY "CVI Test Context Key"

//==============================================================================
// Types
struct TestData {
    int failed;
    ListType addedTests;
    Log *log;
    AssertReason expectedReason;
};

typedef struct UserTest {
    TestMethod test;
    TestMethod init;
    TestMethod cleanup;
    char *testName;
    char *attributes;
} UserTest;

//==============================================================================
// Static global variables
static jmp_buf environment;
static char failMessage[4096];

//==============================================================================
// Static functions

static void AssertEqual(TestContext *context, int passed, char *expected, char *actual, char *msg) {
    context->data->failed |= !passed;
    
    if (!passed) {
        snprintf(failMessage, sizeof(failMessage), "    expected \"%s\", but got \"%s\".  Message: %s", expected, actual, msg);
        longjmp(environment, 1);
    }
}

static void InvokeIfNotNull(TestContext *context, TestMethod method) {
    if (method) {
        method(context);
    }
}

static void AssertFail(TestContext *context, char *msg) {
    context->data->failed = 1;
    snprintf(failMessage, sizeof(failMessage), "    Failed with message: %s", msg);
    longjmp(environment, 1);
}

static void AssertIsTrue(TestContext *context, int expected, char *msg){
    char *actual = expected ? "True" : "False";
    AssertEqual(context, expected, "True", actual, msg);
}

static void AssertIsFalse(TestContext *context, int expected, char *msg) {
    char *actual = expected ? "True" : "False";
    AssertEqual(context, !expected, "False", actual, msg);
}

static void AssertIsNull(TestContext *context, void *actual, char *msg) {
    char actualString[256];
    if (actual) {
        snprintf(actualString, sizeof(actualString), "%p", actual);
    } else {
        snprintf(actualString, sizeof(actualString), "NULL");
    }
    AssertEqual(context, !actual, "NULL", actualString, msg);
}

static void AssertIsNotNull(TestContext *context, void *actual, char *msg) {
    char actualString[256];
    int isNull = !actual;
    if (isNull) {
        snprintf(actualString, sizeof(actualString), "NULL");
    } else {
        snprintf(actualString, sizeof(actualString), "%p", actual);
    }
    AssertEqual(context, !isNull, "NOT NULL", actualString, msg);
}

static void AssertPtrEqual(TestContext *context, void *expected, void *actual, char *msg) {
    char expectedString[256], actualString[256];
    snprintf(expectedString, sizeof(expectedString), "%p", expected);
    snprintf(actualString, sizeof(actualString), "%p", actual);
    
    AssertEqual(context, expected == actual, expectedString, actualString, msg);
}

static void AssertIntEqual(TestContext *context, int expected, int actual, char *msg) {
    char expectedString[256], actualString[256];
    snprintf(expectedString, sizeof(expectedString), "%d", expected);
    snprintf(actualString, sizeof(actualString), "%d", actual);
    
    AssertEqual(context, expected == actual, expectedString, actualString, msg);
}

static void AssertStringEqual(TestContext *context, char *expected, char *actual, char *msg) {
    char expectedString[256], actualString[256];
    snprintf(expectedString, sizeof(expectedString), "%f", expected);
    snprintf(actualString, sizeof(actualString), "%f", actual);
    
    AssertEqual(context, !strcmp(expected, actual), expected, actual, msg);
}

static void AssertDoubleEqual(TestContext *context, double expected, double actual, char *msg) {
    char expectedString[256], actualString[256];
    snprintf(expectedString, sizeof(expectedString), "%f", expected);
    snprintf(actualString, sizeof(actualString), "%f", actual);
    
    AssertEqual(context, expected == actual, expectedString, actualString, msg);
}

static void HandleTestLogAssert(Log *log, int passed, char *fileName, int lineNumber, AssertReason reason, char *msg) {
    if (passed) {
        return;
    }
    
    TestContext *context = (TestContext *)LogGetPrivateData(log, LOG_CONTEXT_KEY);
    if (context->data->expectedReason != AnyReason && context->data->expectedReason != reason) {
        context->data->failed = 1;
        if (msg) {
            snprintf(failMessage, sizeof(failMessage), "    Assertion failure! %s:%d  Message: %s", fileName, lineNumber, msg);
        } else {
            snprintf(failMessage, sizeof(failMessage), "    Assertion failure! %s:%d", fileName, lineNumber);
        }
    }
    
    longjmp(environment, 1);
}

static void ExpectAssertReason(TestContext *context, AssertReason reason) {
    LOG_ASSERT_REASON(context && context->data, ArgumentNullReason);
    context->data->expectedReason = reason;
}

static void AddTest(TestContext *context, char *testName, TestMethod test, TestMethod init, TestMethod cleanup, char *attributes) {
    if (!context->data->addedTests) {
        context->data->addedTests = ListCreate(sizeof(UserTest*));
    }
    
    UserTest *userTest = calloc(1, sizeof(UserTest));
    userTest->test = test;
    userTest->init = init;
    userTest->cleanup = cleanup;
    userTest->testName = testName;
    userTest->attributes = attributes;
    
    ListInsertItem(context->data->addedTests, &userTest, END_OF_LIST);
}

static void ExecuteTests(TestContext *context) {
    if (!context || !context->data || !context->data->addedTests) {
        return;
    }
    
    while(ListNumItems(context->data->addedTests)) {
        UserTest *test;
        ListRemoveItem(context->data->addedTests, &test, FRONT_OF_LIST);
        
        context->testName = test->testName;
        if (!setjmp(environment)) {
            InvokeIfNotNull(context, test->init);
            if (!setjmp(environment)) {
                InvokeIfNotNull(context, test->test);
                if (context->data->expectedReason != NoReason) {
                    // if we've gotten here (there was no longjmp), and the expected reason
                    // is set, we need to fail.  An assertion was expected, but none was received.
                    context->data->failed = 1;
                    snprintf(failMessage, sizeof(failMessage), "    Expected log assertion, but got none!");
                }
            }
        }
        if (!setjmp(environment)) {
            InvokeIfNotNull(context, test->cleanup);
        }
        
        if (context->data->failed) {
            PRINT_MSG("(X) Test: %s failed!", context->testName);
            PRINT_MSG(failMessage);
        } else {
            PRINT_MSG("(+) Test: %s passed!", context->testName);
        }
        PRINT_MSG("\n");
        
        failMessage[0] = 0;
        context->testName = 0;
        context->data->failed = 0;
        context->data->expectedReason = NoReason;
        free(test);
    }
}

//==============================================================================
// Global variables

//==============================================================================
// Global functions

/// HIFN Creates the test context to be used by tests.
/// HIRET The constructed test context to be used for tests.
TestContext *CreateContext() {
    TestContext *context = calloc(1, sizeof *context);
    context->data = calloc(1, sizeof(TestData));
    context->AddTest = AddTest;
    context->ExecuteTests = ExecuteTests;
    context->AssertFail = AssertFail;
    context->AssertIsNull = AssertIsNull;
    context->AssertIsNotNull = AssertIsNotNull;
    context->AssertIsTrue = AssertIsTrue;
    context->AssertIsFalse = AssertIsFalse;
    context->AssertPtrEqual = AssertPtrEqual;
    context->AssertIntEqual = AssertIntEqual;
    context->AssertStringEqual = AssertStringEqual;
    context->AssertDoubleEqual = AssertDoubleEqual;
    context->ExpectAssertReason = ExpectAssertReason;
    
    Log *log = LogCreate();
    LogSetAssertHandler(log, HandleTestLogAssert);
    LogSetPrivateData(log, LOG_CONTEXT_KEY, context);
    LogSetGlobal(log);
    context->data->log = log;
    
    return context;
}

/// HIFN Frees the memory of the test context.
/// HIPAR context/The context to be freed.
void FreeContext(TestContext *context) {
    LogDispose(context->data->log);
    free(context->data);
    free(context);
}
