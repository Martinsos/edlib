//
// Created by mobin on 1/13/20.
//


#include <stdio.h>
#include <stdlib.h>
#include "edlibChar.h"

int main() {
    EdlibAlignResult result = edlibAlign("hello", 5, "world!", 6, edlibDefaultAlignConfig());
    printf("edit_distance('hello', 'world!') = %d\n", result.editDistance);
    edlibFreeAlignResult(result);
}

