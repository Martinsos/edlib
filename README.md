EDLIB
=====

[![Join the chat at https://gitter.im/Martinsos/edlib](https://badges.gitter.im/Martinsos/edlib.svg)](https://gitter.im/Martinsos/edlib?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

A lightweight and super fast C/C++ library for sequence alignment using [edit distance](https://en.wikipedia.org/wiki/Edit_distance).

Calculating edit distance of two strings is as simple as:
```c
edlibAlign("hello", 5, "world!", 6, edlibDefaultAlignConfig()).editDistance;
```

[![Build Status](https://travis-ci.org/Martinsos/edlib.svg?branch=master)](https://travis-ci.org/Martinsos/edlib)


---


### Features
* Calculates **edit distance**.
* It can find **optimal alignment path** (instructions how to transform first sequence into the second sequence).
* It can find just the **start and/or end locations of alignment path** - can be useful when speed is more important than having exact alignment path.
* Supports **multiple alignment methods**: global(**NW**), prefix(**SHW**) and infix(**HW**), each of them useful for different scenarios.
* It can easily handle small or **very large** sequences, even when finding alignment path.
* **Super fast** thanks to Myers's bit-vector algorithm.


---


### Usage

1. Copy `edlib.h` and `edlib.cpp` from [src/library/](src/library/) directory to your project.
2. Include `edlib.h` in your source files as needed.
3. Compile your code together with `edlib.cpp`.

#### Hello World
To get you started quickly, here is a short Hello World project that works.

```
edlib.h   -> copied from edlib/src/library/
edlib.cpp -> copied from edlib/src/library/
helloWorld.c -> your program
```

helloWorld.c
```c
#include <stdio.h>
#include "edlib.h"

int main() {
    EdlibAlignResult result = edlibAlign("hello", 5, "world!", 6, edlibDefaultAlignConfig());
    printf("edit_distance('hello', 'world!') = %d\n", result.editDistance);
    edlibFreeAlignResult(result);
}
```

Compile it with `g++ helloWorld.c edlib.cpp -o helloWorld` and that is it!
Running `./helloWorld` should output `edit_distance('hello', 'world!') = 5`.

For more example projects take a look at applications in [src/apps/](src/apps/).

#### Examples
Main function in edlib is `edlibAlign`. Given two sequences (and their lengths), it will find edit distance, alignment path or its end and start locations.

##### EdlibAlignConfig
`edlibAlign` takes config object (it is a struct), which allows you to further customize how alignment will be done. You can choose alignment method, tell edlib what to calculate (just edit distance or also path and locations) and set upper limit for edit distance.

For example, if you want to use infix(HW) alignment method, want to find alignment path (and edit distance), and are interested in result only if edit distance is not larger than 42, you would call it like this:
```c
edlibAlign(seq1, seq1Length, seq2, seq2Length,
           edlibNewAlignConfig(42, EDLIB_MODE_HW, EDLIB_TASK_PATH));
```
Or, if you want to use suffix(SHW) alignment method, want to find only edit distance, and do not have any limits on edit distance, you would call it like this:
```c
edlibAlign(seq1, seq1Length, seq2, seq2Length,
           edlibNewAlignConfig(-1, EDLIB_MODE_SHW, EDLIB_TASK_DISTANCE));
```

We used `edlibNewAlignConfig` helper function to easily create config, however we could have also just created an instance of it and set its members accordingly.

##### EdlibAlignResult
`edlibAlign` function returns a result object, which will contain results of alignment (corresponding to the task that you passed in config).

```c
EdlibAlignResult result = edlibAlign(seq1, seq1Length, seq2, seq2Length,
                                     edlibNewAlignConfig(-1, EDLIB_MODE_HW, EDLIB_TASK_PATH));
printf("%d\n", result.editDistance);
printf("%d\n", result.alignmentLength);
printf("%d\n", result.endLocations[0]);
edlibFreeAlignResult(result);
```

It is important to remember to free the result object using `edlibFreeAlignResult` function, since Edlib allocates memory on heap for certain members. If you decide to do the cleaning manually and not use `edlibFreeAlignResult`, do not forget to manually `free()` required members.

##### Turning alignment to cigar
Cigar is a standard way to represent alignment path.
Edlib has helper function that transforms alignment path into cigar.
```c
char* cigar;
edlibAlignmentToCigar(result.alignment, result.alignmentLength, EDLIB_CIGAR_STANDARD, &cigar);
printf("%s", cigar);
```


---


### Alignment methods

Edlib supports 3 alignment methods:
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


---


### API

Check [edlib.h](/src/library/edlib.h) for list of well commented public structures and functions.


---


### Aligner
Edlib comes with a standalone aligner, which can be found at [src/apps/aligner/](src/apps/aligner).

Aligner reads sequences from fasta files, and it can display alignment path in graphical manner or as a cigar.
It also measures calculation time, so it can be useful for testing speed and comparing Edlib with other tools.

In order to compile aligner, position yourself in [src/](src/) directory and run `make aligner`.
Run `./aligner` for help and detailed instructions.

Example of usage (assuming you are positioned in [src/](src/) directory):
`./aligner -p apps/aligner/test_data/query.fasta apps/aligner/test_data/target.fasta`


---


### Running tests
In order to run tests, position your self in [src/](src/) directory, run `make test`, and run `./test`. This will run random tests for each alignment method, and also some specific unit tests.


---


### Time and space complexity
Edlib is based on [Myers's bit-vector algorithm](http://www.gersteinlab.org/courses/452/09-spring/pdf/Myers.pdf) and extends from it.
It calculates a dynamic programming matrix of dimensions `Q x T`, where `Q` is the length of the first sequence (query), and `T` is the length of the second sequence (target). It uses Ukkonen's banded algorithm to reduce the space of search, and there is also parallelization from Myers's algorithm, however time complexity is still quadratic.
Edlib uses Hirschberg's algorithm to find alignment path, therefore space complexity is linear.

Time complexity: `O(T * Q)`.

Space complexity: `O(T + Q)`.

It is worth noting that Edlib works best for large, similar sequences, since such sequences get the highest speedup from banded approach and bit-vector parallelization.


---


### Test data
In [test_data/](test_data) directory there are different genome sequences, ranging from 10 kbp to 5 Mbp in length. They are ranging in length and similarity, so they can be useful for testing and measuring speed in different scenarios.


---


#### Nodejs
For those who want to use edlib in nodejs there is a nodejs addon, [node-edlib](https://www.npmjs.com/package/node-edlib)!
