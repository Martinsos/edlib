//
// Created by mobin on 1/13/20.
//

#ifndef EDLIB_EDLIBCHAR_H
#define EDLIB_EDLIBCHAR_H


#include "baseHeader.h"

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
