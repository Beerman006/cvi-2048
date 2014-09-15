//==============================================================================
//
// Title:       TestContext.h
// Purpose:     A short description of the interface.
//
// Created on:  6/20/2014 at 10:55:58 PM by Nick Beer.
// Copyright:   National Instruments. All Rights Reserved.
//
//==============================================================================

#ifndef __TestContext_H__
#define __TestContext_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files
#include "cvidef.h"
#include "../CVI_Core/log.h"

//==============================================================================
// Constants

//==============================================================================
// Types
typedef struct TestData TestData;
typedef struct TestContext TestContext;

typedef void (*TestMethod)(TestContext *);

struct TestContext {
    TestData *data;
    char *testName;
    void (*ExpectAssertReason)(TestContext *context, AssertReason reason);
    void (*AssertFail)(TestContext *context, char *msg);
    void (*AssertIsNull)(TestContext *context, void *actual, char *msg);
    void (*AssertIsNotNull)(TestContext *context, void *actual, char *msg);
    void (*AssertIsTrue)(TestContext *context, int actual, char *msg);
    void (*AssertIsFalse)(TestContext *context, int actual, char *msg);
    void (*AssertPtrEqual)(TestContext *context, void *expected, void *actual, char *msg);
    void (*AssertIntEqual)(TestContext *context, int expected, int actual, char *msg);
    void (*AssertStringEqual)(TestContext *context, char *expected, char *actual, char *msg);
    void (*AssertDoubleEqual)(TestContext *context, double expected, double actual, char *msg);
    void (*AddTest)(TestContext *context, char *testName, TestMethod test, TestMethod init, TestMethod cleanup, char *attributes);
    void (*ExecuteTests)(TestContext *context);      
};

//==============================================================================
// External variables

//==============================================================================
// Global functions

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __TestContext_H__ */
