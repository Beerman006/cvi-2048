//==============================================================================
//
// Title:       SymbolParser.c
// Purpose:     A short description of the implementation.
//
// Created on:  6/9/2014 at 9:22:03 PM by Nick Beer.
// Copyright:   National Instruments. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files
#include <windows.h>
#include <ansi_c.h>
#include <toolbox.h>
#include "SymbolParser.h"

//==============================================================================
// Constants

//==============================================================================
// Types
struct SymbolParser {
    char * path;
    ListType exports;
    HMODULE module;
    PIMAGE_NT_HEADERS header;
};

//==============================================================================
// Static global variables

//==============================================================================
// Static functions
static void InitParser(SymbolParser *parser) {
    AssertMsg(parser, "The parser was not valid");
    if (parser->header) {
        return;
    }
    
    HMODULE module = LoadLibrary(parser->path);
    AssertMsg(module, "Failed to load library");
    AssertMsg(((PIMAGE_DOS_HEADER)module)->e_magic == IMAGE_DOS_SIGNATURE, "Module was not valid PE file");
    
    PIMAGE_NT_HEADERS header = (BYTE *)module + ((PIMAGE_DOS_HEADER)module)->e_lfanew;
    AssertMsg(header->Signature == IMAGE_NT_SIGNATURE, "Module header was corrupt");
    AssertMsg(header->OptionalHeader.NumberOfRvaAndSizes > 0, "Module header was corrupt");
    
    parser->header = header;
    parser->module = module;
}
//==============================================================================
// Global variables

//==============================================================================
// Global functions

/// HIFN  Creates a Parser able to pull symbol names out of a library.
/// HIPAR libararyPath/The path to the library to be parsed.
/// HIRET The constructed parser object.
SymbolParser* MakeParser(const char *libraryPath) {
    SymbolParser * parser = calloc(1, sizeof(SymbolParser));
    parser->path = libraryPath;
    return parser;
}


/// HIFN Gets all of the exports from the library owned by the parser.
/// HIRET Returns a list containing all of the exports owned by the parser.
/// HIRET This list, and the items in it, should not be disposed.
/// HIPAR parser/The parser from which to get the names of the symbols.
ListType GetExports(SymbolParser *parser) {
    InitParser(parser);
    if (parser->exports) {
        return parser->exports;
    }
    
    PIMAGE_EXPORT_DIRECTORY exports = (BYTE *)parser->module + parser->header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    void *names = (BYTE *)parser->module + exports->AddressOfNames;
    
    ListType exportedNames = ListCreate(sizeof(char *));
    
    for (int i = 0; i < exports->NumberOfNames; i++) {
        char *name = (char *)(BYTE *)parser->module + ((DWORD *)names)[i];
        ListInsertItem(exportedNames, &name, i);
    }
    
    parser->exports = exportedNames;
    return parser->exports;
}


/// HIFN Loads a function with the given name from the loaded module.
/// HIRET A pointer to the loaded function.
/// HIPAR parser/The parser from which to load the function.
/// HIPAR functionName/The name of the function to be loaded.
void *LoadFunction(SymbolParser *parser, const char *functionName) {
    InitParser(parser);
    
    return GetProcAddress(parser->module, functionName);
}

/// HIFN Deletes the given parser.
/// HIPAR parser/The parser to be deleted.
void DeleteParser(const SymbolParser *parser) {
    AssertMsg(parser, "parser was null");
    
    if (parser->module) {
        FreeLibrary(parser->module);
    }
    if (parser->exports) {
        ListDispose(parser->exports);
    }
    free(parser);
}
