Edlib
=====

A lightweight and super fast C/C++ library for sequence alignment using [edit distance](https://en.wikipedia.org/wiki/Edit_distance).

Calculating edit distance of two strings is as simple as:
```c
edlibAlign("hello", 5, "world!", 6, edlibDefaultAlignConfig()).editDistance;
```

Supported on Linux / OSX [![Build Status](https://travis-ci.org/Martinsos/edlib.svg?branch=master)](https://travis-ci.org/Martinsos/edlib) and Windows [![Build status](https://ci.appveyor.com/api/projects/status/7owowdwja516ydu3/branch/master?svg=true)](https://ci.appveyor.com/project/Martinsos/edlib/branch/master).

[![Join the chat at https://gitter.im/Martinsos/edlib](https://badges.gitter.im/Martinsos/edlib.svg)](https://gitter.im/Martinsos/edlib?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


---


### Features
* Calculates **edit distance**.
* It can find **optimal alignment path** (instructions how to transform first sequence into the second sequence).
* It can find just the **start and/or end locations of alignment path** - can be useful when speed is more important than having exact alignment path.
* Supports **multiple [alignment methods](#align-methods)**: global(**NW**), prefix(**SHW**) and infix(**HW**), each of them useful for different scenarios.
* It can easily handle small or **very large** sequences, even when finding alignment path.
* **Super fast** thanks to Myers's bit-vector algorithm.


---


### <a name="building">Building</a>
Edlib uses CMAKE to build libraries (static and shared) and binaries (apps and tests).
Execute following commands to build Edlib using CMAKE:

1. `cd build`
2. `cmake -D CMAKE_BUILD_TYPE=Release ..`
3. `make`

This will create binaries in `bin/` directory and libraries (static and shared) in `lib/` directory.
You can run `./bin/runTests` to confirm that it works!

Optionally, you can run `sudo make install` to install edlib library on your machine (on Linux, this will usually install it to `usr/local/lib` and `usr/local/include`).


---


### Using Edlib in your project
You can use Edlib in you project by either directly copying header and source files from [edlib/](edlib/), or by linking Edlib library (see [Building](#building) for instructions how to build Edlib libraries).
In any case, only thing that you have to do in your source files is to include `edlib.h`.

#### Hello World
To get you started quickly, let's take a look at a few ways to get simple Hello World project working.

Our Hello World project has just one source file, helloWorld.c file, and it looks like this:
```c
#include <stdio.h>
#include "edlib.h"

int main() {
    EdlibAlignResult result = edlibAlign("hello", 5, "world!", 6, edlibDefaultAlignConfig());
    printf("edit_distance('hello', 'world!') = %d\n", result.editDistance);
    edlibFreeAlignResult(result);
}
```

Running it should output `edit_distance('hello', 'world!') = 5`.

##### Directly copying edlib source and header files.
Here we directly copied [edlib/](edlib/) directory to our project, to get following project structure:
```
edlib/  -> copied from edlib/
  include/
    edlib.h
  src/
    edlib.cpp
helloWorld.c -> your program
```

Compile it with `g++ helloWorld.c edlib/src/edlib.cpp -o helloWorld -I edlib/include` and that is it!

##### Copying edlib header file and static library.
Instead of copying edlib source files, you could copy static library (check [Building](#building) on how to create static library). We also need to copy edlib header files. We get following project structure:
```
edlib/  -> copied from edlib
  include/
    edlib.h
  edlib.a
helloWorld.c -> your program
```

Now you can compile it with `g++ helloWorld.c -o helloWorld -I edlib/include -L edlib -ledlib_static`.

##### Install edlib library on machine.
Alternatively, you could avoid copying any Edlib files and instead install libraries by running `sudo make install` (check [Building](#building)). Now, all you have to do to compile your project is `g++ helloWorld.c -o helloWorld -ledlib`.
If you get error message like `cannot open shared object file: No such file or directory`, make sure that your linker includes path where edlib was installed.


For more example projects take a look at applications in [apps/](apps/).


---


### Usage
Main function in edlib is `edlibAlign`. Given two sequences (and their lengths), it will find edit distance, alignment path or its end and start locations.

```c
char* query = "ACCTCTG";
char* target = "ACTCTGAAA"
EdlibAlignResult result = edlibAlign(query, 7, target, 9, edlibDefaultAlignConfig());
printf("%d", result.editDistance);
edlibFreeAlignResult(result);
```

##### EdlibAlignConfig
`edlibAlign` takes config object (it is a struct), which allows you to further customize how alignment will be done. You can choose [alignment method](#align-methods), tell edlib what to calculate (just edit distance or also path and locations) and set upper limit for edit distance.

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
char* cigar = edlibAlignmentToCigar(result.alignment, result.alignmentLength, EDLIB_CIGAR_STANDARD);
printf("%s", cigar);
free(cigar);
```


---


### API

Visit [http://martinsos.github.io/edlib](https://martinsos.github.io/edlib) for online documentation with documented API (should be for the latest release).

To generate latest API documentation, you need to have [doxygen](www.doxygen.org) installed.
Position yourself in root directory and run `doxygen`, this will generate `docs/` directory. Then open `docs/html/index.html` file with you favorite browser.


---


### <a name="align-methods">Alignment methods</a>

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


### Aligner
Edlib comes with a standalone aligner, which can be found at [apps/aligner/](apps/aligner).

Aligner reads sequences from fasta files, and it can display alignment path in graphical manner or as a cigar.
It also measures calculation time, so it can be useful for testing speed and comparing Edlib with other tools.

Check [Building](#building) to see how to build binaries (including `aligner`).
Run `./aligner` for help and detailed instructions.

Example of usage:
`./aligner -p apps/aligner/test_data/query.fasta apps/aligner/test_data/target.fasta`

NOTE: Aligner currently does not work on Windows, because it uses `getopt` to parse command line arguments, which is not supported on Windows.

---


### Running tests
Check [Building](#building) to see how to build binaries (including binary `runTests`).
To run tests, just run `./runTests`. This will run random tests for each alignment method, and also some specific unit tests.


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


### Nodejs
For those who want to use edlib in nodejs there is a nodejs addon, [node-edlib](https://www.npmjs.com/package/node-edlib)!


---


### Development
Feel free to send pull requests and raise issues.

When developing, you may want to use `-D CMAKE_BUILD_TYPE=Debug` flag when calling `cmake` in order to get debugging flags passed to compiler. This should also happen if you just run `cmake ..` with no flags, but I think I have noticed it does not always works as expected (probably has something to do with cmake cache). To check which flags is compiler using, run `make` with `VERBOSE=1`: `make VERBOSE=1`.
