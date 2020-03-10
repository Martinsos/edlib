//
// Created by mobin on 1/13/20.
// The declaration of the C-wrapper of edlibAlign
// The declaration of some other functions are in charHeader.h
// The definitions are in src/edlibChar.cpp
//

#ifndef EDLIB_EDLIBCHAR_H
#define EDLIB_EDLIBCHAR_H


#include "baseHeader.h"
#include "charHeader.h"

#ifdef __cplusplus
extern "C" {
#endif

    EdlibAlignResult edlibAlign(const char *query, int queryLength,
                                const char *target, int targetLength,
                                const EdlibAlignConfig config);
#ifdef __cplusplus
}
#endif

#endif //EDLIB_EDLIBCHAR_H
