//
// Created by mobin on 1/13/20.
//

#include "edlibGeneric.h"

extern "C" {
    EdlibAlignResult edlibAlign(const char *query, int queryLength,
                                const char *target, int targetLength,
                                const EdlibAlignConfig config) {
        return edlibAlign<char, uint8_t>(
                query,queryLength,
                target, targetLength,
                config
        );
    }
}
