# Myers

C/C++ library for sequence alignment based on Myers's "Fast Bit-Vector Algorithm for Approximate String Matching Based on Dynamic Programming".  
Implementation combines Myers's bit-vector algorithm with Ukonnen's banded algorithm.  
Calculates best score (Levenshtein distance) and first position of best score.  


#### Alignment modes
There are 3 different modes of alignment:
* NW: Global alignment (Needleman-Wunsch).
      Appropriate for aligning two sequences of similar length. 
* SHW: Semi-global alignment. Gap at query end is not penalized.
       Appropriate for aligning prefix(query) to sequence(target).
* HW: Semi-global alignment. Gap at query start and gap at query end are not penalized.
      Appropriate for aligning read(query) to sequence(target)
      (read is sequence obtained by reading part of larger sequence).


#### Usage
Include myers.h in your code and compile it together with myers.cpp.  

```
...
#include "myers.h"
...
```
```
...
int alphabetLength = 9;
int queryLength = 5;
int targetLength = 9;
unsigned char query[5] = {0,1,2,3,4};
unsigned char target[9] = {8,5,0,1,3,4,6,7,5};
int score, pos;
myersCalcEditDistance(query, queryLength, target, targetLength,
                      alphabetLength, -1, MYERS_MODE_HW, &score, &pos);
printf("%d %d", score, pos);
...
```    

For more examples of usage take a look at test.cpp.


#### Running tests
In order to run tests, do following:

    cd src
    make
    ./test
