//
// Created by mobin on 2/27/20.
// This header contains the structs and functions for
// preparing the configuration of alignment
// Its content is similar to charHeader.h with the
// difference of replacing char by AlphabetType since
// this header is written for generic edlib
//

#ifndef EDLIBGENERIC_PYTHON_GENERICHEADER_H
#define EDLIBGENERIC_PYTHON_GENERICHEADER_H

#include "baseHeader.h"

// using the namespace of edlibGeneric to
// distinguish the functions and classes with same names in c-wrapper
namespace edlibGeneric {
/**
 * @brief Defines two given symbols as equal.
 */
    template<class AlphabetType>
    struct EdlibEqualityPair {
        AlphabetType first;
        AlphabetType second;
    };

/**
 * @brief Configuration object for edlibAlign() function.
 */
    template<class AlphabetType>
    struct EdlibAlignConfig {
        /**
         * Set k to non-negative value to tell edlib that edit distance is not larger than k.
         * Smaller k can significantly improve speed of computation.
         * If edit distance is larger than k, edlib will set edit distance to -1.
         * Set k to negative value and edlib will internally auto-adjust k until score is found.
         */
        int k;

        /**
         * Alignment method.
         * EDLIB_MODE_NW: global (Needleman-Wunsch)
         * EDLIB_MODE_SHW: prefix. Gap after query is not penalized.
         * EDLIB_MODE_HW: infix. Gaps before and after query are not penalized.
         */
        EdlibAlignMode mode;

        /**
         * Alignment task - tells Edlib what to calculate. Less to calculate, faster it is.
         * EDLIB_TASK_DISTANCE - find edit distance and end locations of optimal alignment paths in target.
         * EDLIB_TASK_LOC - find edit distance and start and end locations of optimal alignment paths in target.
         * EDLIB_TASK_PATH - find edit distance, alignment path (and start and end locations of it in target).
         */
        EdlibAlignTask task;

        /**
         * List of pairs of characters, where each pair defines two characters as equal.
         * This way you can extend edlib's definition of equality (which is that each character is equal only
         * to itself).
         * This can be useful if you have some wildcard characters that should match multiple other characters,
         * or e.g. if you want edlib to be case insensitive.
         * Can be set to NULL if there are none.
         */
        EdlibEqualityPair<AlphabetType> *additionalEqualities;

        /**
         * Number of additional equalities, which is non-negative number.
         * 0 if there are none.
         */
        int additionalEqualitiesLength;
    };

/**
 * Helper method for easy construction of configuration object.
 * @return Configuration object filled with given parameters.
 */
    template<class AlphabetType>
    EdlibAlignConfig<AlphabetType> edlibNewAlignConfig(int k, EdlibAlignMode mode, EdlibAlignTask task,
                                                       EdlibEqualityPair<AlphabetType> *additionalEqualities,
                                                       int additionalEqualitiesLength);

/**
 * @return Default configuration object, with following defaults:
 *         k = -1, mode = EDLIB_MODE_NW, task = EDLIB_TASK_DISTANCE, no additional equalities.
 */
    template<class AlphabetType>
    EdlibAlignConfig<AlphabetType> edlibDefaultAlignConfig(void);
}
#endif //EDLIBGENERIC_PYTHON_GENERICHEADER_H
