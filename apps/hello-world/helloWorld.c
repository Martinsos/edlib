#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "edlib.h"

int main() {
  char elements[4] = "ACTG";

  int s1Size = 3;
  char s1[s1Size];

  int s2Size = 5;
  char s2[s2Size];

  for (int i = 0; i < s1Size; i++) {
    s1[i] = elements[rand() % 4];
  }

  for (int i = 0; i < s2Size; i++) {
    s2[i] = elements[rand() % 4];
  }

  for (int j = 0; j < 1000; j++) {
    for (int i = 0; i < 100; i++) {
      clock_t start = clock();

      EdlibAlignResult result = edlibAlign(s1, s1Size, s2, s2Size,
                                           edlibNewAlignConfig(-1, EDLIB_MODE_NW, EDLIB_TASK_PATH, NULL, 0));
      edlibFreeAlignResult(result);

      double elapsed = (double) (clock() - start) / CLOCKS_PER_SEC;
      printf("%f\n", elapsed);
    }
  }
}
