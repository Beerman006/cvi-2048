//==============================================================================
//
// Title:       SymbolParser.h
// Purpose:     Parses the symbols out of a library.
//
// Created on:  6/9/2014 at 9:22:03 PM by Nick Beer.
// Copyright:   National Instruments. All Rights Reserved.
//
//==============================================================================

#ifndef __SymbolParser_H__
#define __SymbolParser_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files
#include "cvidef.h"
#include <toolbox.h>

//==============================================================================
// Constants

//==============================================================================
// Types
typedef struct SymbolParser SymbolParser;

//==============================================================================
// Global functions

SymbolParser* MakeParser(const char *libraryPath);
void DeleteParser(const SymbolParser *parser);

ListType GetExports(SymbolParser *parser);
void *LoadFunction(SymbolParser *parser, const char *functionName);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __SymbolParser_H__ */
