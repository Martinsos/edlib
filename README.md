EDLIB
=====

A lightweight and super fast C/C++ library for calculating [edit distance](https://en.wikipedia.org/wiki/Edit_distance) between two sequences(strings).


### Features
* Calculates **edit distance**.
* It can find **optimal alignment path** (instructions how to transform first sequence into the second sequence).
* It can find just the **start and/or end locations of alignment path** - can be useful when speed is more important than having exact alignment path.
* Supports **multiple alignment methods**: global(**NW**), prefix(**SHW**) and infix(**HW**), each of them useful for different scenarios.
* It can easily handle small or very large sequences, even when finding alignment path.


### Usage

1. Copy `edlib.h` and `edlib.cpp` from [src/library/](src/library/) directory to your project.
2. Include `edlib.h` in your source files as needed.
3. Compile your code together with `edlib.cpp`.


#### Example project
To get you started quickly, here is a short example project that works.

```
edlib.h   -> copied from edlib/src/library/
edlib.cpp -> copied from edlib/src/library/
example.c -> your program
```

example.c
```c
#include <stdio.h>
#include <stdlib.h>
#include "edlib.h"

int main(int argc, char * const argv[]) {
    unsigned char query[4] = {0,1,2,1};
    unsigned char target[5] = {0,2,1,1,1};
    int editDistance, numLocations, alignmentLength;
    int* startLocations, * endLocations;
    unsigned char* alignment;

    edlibCalcEditDistance(query, 4, target, 5, 3,
                          -1, EDLIB_MODE_NW, 0, 0,
                          &editDistance, &endLocations, &startLocations, &numLocations,
                          &alignment, &alignmentLength);
    printf("%d\n", editDistance);
    if (endLocations) free(endLocations);
}
```

Compile it with `g++ example.c edlib.cpp -o example` and that is it! Running `./example` should output `2`.


For more examples take a look at applications in [src/apps/](src/apps/).


### Alignment methods

Edlib support 3 alignment methods:
* **global (NW)** - This is the standard method, when we say "edit distance" this is the method that is assumed.
  It tells us the smallest number of operations needed to transform first sequence into second sequence.
  This method is appropriate when you want to find out how similar is first sequence to second sequence.
* **prefix (SHW)** - Similar to global method, but with a small twist - gap at query end is not penalized. What that means is that deleting elements from the end of second sequence is "free"!
  For example, if we had `AACT` and `AACTGGC`, edit distance would be 0, because removing `GGC` from the end of second sequence is "free" and does not count into total edit distance.
  This method is appropriate when you want to find out how well first sequence fits at the beginning of second sequence.
* **infix (HW)**: Similar as prefix method, but with one more twist - gaps at query end **and start** are not penalized. What that means is that deleting elements from the start and end of second sequence is "free"!
  For example, if we had `ACT` and `CGACTGAC`, edit distance would be 0, because removing `CG` from the start and `GAC` from the end of second sequence is "free" and does not count into total edit distance.
  This method is appropriate when you want to find out how well first sequence fits at any part of second sequence. For example, if your second sequence was a long text and your first sequence was a sentence from that text, but slightly scrambled, you could use this method to discover how scrambled it is and where it fits in that text.
  In bioinformatics, this method is appropriate for aligning read to a sequence.



### Methods
#### edlibCalcEditDistance(...)
  TODO
#### edlibAlignmentToCigar(...)
  TODO

    
    
### Aligner
Edlib comes with a standalone aligner, which can be found at [src/apps/aligner/](src/apps/aligner).

Aligner reads sequences from fasta files, and it can display alignment path in graphical manner or as a cigar.
It also measures calculation time, so it can be useful for testing speed and comparing Edlib with other tools.

In order to compile aligner, position yourself in [src/](src/) directory and run `make aligner`.
Run `./aligner` for help and detailed instructions.

Example of usage (assuming you are positioned in [src/](src/) directory):
`./aligner -p apps/aligner/test_data/query.fasta apps/aligner/test_data/target.fasta`


### Running tests
In order to run tests, position your self in [src/](src/) directory, run `make test`, and run `./test`. This will run random tests for each alignment method, and also some specific unit tests.


### Time and space complexity
Edlib is based on [Myers's bit-vector algorithm](http://www.gersteinlab.org/courses/452/09-spring/pdf/Myers.pdf) and extends from it.
It calculates a dynamic programming matrix of dimensions `Q x T`, where `Q` is the length of the first sequence (query), and `T` is the length of the second sequence (target). It uses Ukkonen's banded algorithm to reduce the space of search, and there is also parallelization from Myers's algorithm, however time complexity is still quadratic.
Edlib uses Hirschberg's algorithm to find alignment path, therefore space complexity is linear.

Time complexity: `O(T * Q)`.

Space complexity: `O(T + Q)`.

It is worth noting that Edlib works best for large, similar sequences, since such sequences get the highest speedup from banded approach and bit-vector parallelization.


#### Nodejs
For those who want to use edlib in nodejs there is a nodejs addon, [node-edlib](https://www.npmjs.com/package/node-edlib)!
