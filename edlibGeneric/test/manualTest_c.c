//
// Created by mobin on 1/13/20.
//  Testing the C-Wrapper of generic edlib
//


#include <stdio.h>
#include <stdlib.h>
#include "edlibChar.h"

int main() {
    EdlibEqualityPair additionalEqualities[2] = {{'e', 'l'}, {'b', 'e'}};
    EdlibAlignResult result = edlibAlign("hello", 5, "hibbo", 5,
                                         edlibNewAlignConfig(-1, EDLIB_MODE_NW, EDLIB_TASK_DISTANCE, additionalEqualities, 2));
    printf("edit_distance('hello', 'hibbo') = %d\n", result.editDistance);
    edlibFreeAlignResult(result);
}

