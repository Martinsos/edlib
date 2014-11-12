# EDLIB

EDLIB is C/C++ library for sequence alignment using edit distance based on Myers's ["Fast Bit-Vector Algorithm for Approximate String Matching Based on Dynamic Programming"](http://www.gersteinlab.org/courses/452/09-spring/pdf/Myers.pdf).  
Implementation combines Myers's bit-vector algorithm with banded approach.  
Calculates best score (Levenshtein distance) and first position of best score.
It can also return alignment path.


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
int score, numPositions, alignmentLength;
int* positions;
unsigned char* alignment;

myersCalcEditDistance(query, queryLength, target, targetLength,
                      alphabetLength, -1, MYERS_MODE_HW, &score,
                      &positions, &numPositions,
                      true, &alignment, &alignmentLength);

printf("Score %d\n", score);
if (positions) {
  printf("First position: %d\n", positions[0]);
}
if (alignment) {
  for (int i = 0; i < alignmentLength; i++)
    printf("%d", alignment[i]);
}

if (positions) free(positions);
if (alignment) free(alignment);
...
```    

For more examples of usage take a look at test.cpp or aligner.cpp.

#### Running tests
In order to run tests, do following:

    cd src
    make
    ./test
    
    
## Aligner

In order to compile and use simple aligner that uses EDLIB run makefile in src:

    cd src
    make aligner
    
Type `./aligner` for help.

Example of usage:
    ./aligner -p ../test_data/query.fasta ../test_data/target.fasta

