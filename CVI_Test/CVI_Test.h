//==============================================================================
//
// Title:       CVI_Test.h
// Purpose:     Helpers and information available for running tests.
//
// Created on:  6/9/2014 at 10:56:32 PM by Nick Beer.
// Copyright:   National Instruments. All Rights Reserved.
//
//==============================================================================

#ifndef __CVI_Test_H__
#define __CVI_Test_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"
#include "TestContext.h"

//==============================================================================
// Constants
#define MODULE_TEST_NAME_PREFIX GET_TEST_EXPORTS_FOR_
        
//==============================================================================
// Macros
#define TESTEXPORT DLLEXPORT __cdecl
        
#define MAKE_TEST_NAME_INNER(prefix, name) void TESTEXPORT prefix ## name(TestContext *context)
#define MAKE_TEST_NAME(prefix, name) MAKE_TEST_NAME_INNER(prefix, name)
        
#define BEGIN_MODULE_TEST(name) MAKE_TEST_NAME(MODULE_TEST_NAME_PREFIX, name) {
#define ADD_TEST(name, init, cleanup, ...) context->AddTest(context, #name, name, init, cleanup, #__VA_ARGS__);
#define END_MODULE_TEST }
        
#define EXPECT_ASSERT_REASON(reason) context->ExpectAssertReason(context, reason);
#define EXPECT_ASSERT EXPECT_ASSERT_REASON(AnyReason)

#define ASSERT_FAIL(msg) context->AssertFail(context, msg);
#define ASSERT_TRUE(actual, msg) context->AssertIsTrue(context, !!(actual), msg)
#define ASSERT_FALSE(actual, msg) context->AssertIsFalse(context, !!(actual), msg)
#define ASSERT_IS_NULL(actual, msg) context->AssertIsNull(context, actual, msg);
#define ASSERT_NOT_NULL(actual, msg) context->AssertIsNotNull(context, actual, msg);
#define ASSERT_PTR_EQUAL(expected, actual, msg) context->AssertPtrEqual(context, expected, actual, msg)
#define ASSERT_INT_EQUAL(expected, actual, msg) context->AssertIntEqual(context, expected, actual, msg)
#define ASSERT_DOUBLE_EQUAL(expected, actual, msg) context->AssertDoubleEqual(context, expected, actual, msg)
#define ASSERT_STRING_EQUAL(expected, actual, msg) context->AssertStringEqual(context, expected, actual, msg)
        
//==============================================================================
// Types

//==============================================================================
// External variables

//==============================================================================
// Global functions

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __CVI_Test_H__ */
