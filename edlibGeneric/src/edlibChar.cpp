//
// Created by mobin on 1/13/20.
// The definitions of C-wrapper functions
// In the body of each function, the same generic function
// is called with necessary conversions beforehand
//


#include "edlibGeneric.h"
#include "charHeader.h"

extern "C" {
    EdlibAlignResult edlibAlign(const char *query, int queryLength,
                                const char *target, int targetLength,
                                const EdlibAlignConfig config) {
        // There are two versions of EdlibEqualityPair
        // One is in the namespace of edlibGeneric, which is for
        // generic version of edlib. The other one is for char version.
        // Before calling the generic version of edlibAlign we have to
        // instantiate an array of edlibGeneric::EdlibEqualityPair<char> and
        // fill it with the same pairs as config.additionalEqualities
        edlibGeneric::EdlibEqualityPair<char> additionalEqualities[config.additionalEqualitiesLength];
        for (int i = 0; i < config.additionalEqualitiesLength; i++) {
            additionalEqualities[i].first = config.additionalEqualities[i].first;
            additionalEqualities[i].second = config.additionalEqualities[i].second;
        }

        // instantiate a generic config
        edlibGeneric::EdlibAlignConfig<char> genericConfig = {config.k,
                                                        config.mode,
                                                        config.task,
                                                        additionalEqualities,
                                                        config.additionalEqualitiesLength};
        return edlibGeneric::edlibAlign<char, uint8_t>(
                query, queryLength,
                target, targetLength,
                genericConfig
        );
    }
    EdlibAlignConfig edlibNewAlignConfig(int k, EdlibAlignMode mode, EdlibAlignTask task,
                                             EdlibEqualityPair* additionalEqualities,
                                             int additionalEqualitiesLength){
        // The definition is similar to the generic version of this function
        // The generic version of edlibNewAlignConfig is in include/edlibGeneric.h
        EdlibAlignConfig config;
        config.k = k;
        config.mode = mode;
        config.task = task;
        config.additionalEqualities = additionalEqualities;
        config.additionalEqualitiesLength = additionalEqualitiesLength;
        return config;
    }

    EdlibAlignConfig edlibDefaultAlignConfig(void){
        // The definition is similar to the generic version of this function
        // The generic version of edlibDefaultAlignConfig is in include/edlibGeneric.h
        return edlibNewAlignConfig(-1, EDLIB_MODE_NW, EDLIB_TASK_DISTANCE, NULL, 0);
    }
}
