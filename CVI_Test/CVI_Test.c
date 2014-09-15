//==============================================================================
//
// Title:       CVI_Test.c
// Purpose:     A short description of the implementation.
//
// Created on:  6/20/2014 at 10:05:41 PM by Nick Beer.
// Copyright:   National Instruments. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files
#include <toolbox.h>
#include "CVI_Test.h"

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// Static global variables

//==============================================================================
// Static functions
//==============================================================================
// Global variables

//==============================================================================
// Global functions


/// HIFN Creates the test context to be used by tests.
/// HIRET The constructed test context to be used for tests.
TestContext *CreateContext() {
    TestContext *context = calloc(1, sizeof *context);
    context->data = calloc(1, sizeof(TestData));
    context->AssertDoubleEqual = AssertDoubleEqual;
    context->AssertIntEqual = AssertIntEqual;
    context->AssertStringEqual = AssertStringEqual;
    return context;
}

/// HIFN Frees the memory of the test context.
/// HIPAR context/The context to be freed.
void FreeContext(TestContext *context) {
    free(context->data);
    free(context);
}
