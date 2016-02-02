# EDLIB

EDLIB is C/C++ library for sequence alignment using edit distance based on Myers's ["Fast Bit-Vector Algorithm for Approximate String Matching Based on Dynamic Programming"](http://www.gersteinlab.org/courses/452/09-spring/pdf/Myers.pdf).  
Implementation combines Myers's bit-vector algorithm with banded approach.  
Calculates best score (Levenshtein distance) and all positions of best score.
It can also return alignment path, or only starting locations of alignment.

#### Nodejs
For those who want to use edlib in nodejs there is a nodejs addon, [node-edlib](https://www.npmjs.com/package/node-edlib)!

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
Include edlib.h in your code and compile it together with myers.cpp.  

```
...
#include "edlib.h"
...
```
```
...
int alphabetLength = 9;
int queryLength = 5;
int targetLength = 9;
unsigned char query[5] = {0,1,2,3,4};
unsigned char target[9] = {8,5,0,1,3,4,6,7,5};
int score, numLocations, alignmentLength;
int* startLocations;
int* endLocations;
unsigned char* alignment;

edlibCalcEditDistance(query, queryLength, target, targetLength,
                      alphabetLength, -1, MYERS_MODE_HW, true, true,
                      &score, &endLocations, &startLocations, &numLocations,
                      &alignment, &alignmentLength);

printf("Score %d\n", score);
for (int i = 0; i < numLocations; i++) {
  printf("(%d, %d)\n", startLocations[i], endLocations[i]);
}

if (alignment) {
  for (int i = 0; i < alignmentLength; i++) {
    printf("%d", alignment[i]);
  }
  printf("\n");

  char* cigar;
  edlibAlignmentToCigar(alignment, alignmentLength, EDLIB_CIGAR_EXTENDED, &cigar);
  printf("%s\n", cigar);
  free(cigar);
}

if (endLocations) free(endLocations);
if (startLocations) free(startLocations);
if (alignment) free(alignment);
...
```

Main function is `edlibCalcEditDistance`.
There is also `edlibAlignmentToCigar` function, which you can use to convert alignment to cigar format.

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
