Edlib
&middot;
[![Latest Github release](https://img.shields.io/github/release/Martinsos/edlib.svg)](https://github.com/Martinsos/edlib/releases/latest)
[![Build status of the master branch on Linux/OSX](https://img.shields.io/github/actions/workflow/status/Martinsos/edlib/ci.yaml?branch=master&label=Linux%2FOSX%20build)
](https://github.com/Martinsos/edlib/actions?query=branch%3Amaster)
[![Published in Bioinformatics](https://img.shields.io/badge/Published%20in-Bioinformatics-167DA4.svg)](https://doi.org/10.1093/bioinformatics/btw753)
=====

A lightweight and super fast C/C++ library for sequence alignment using [edit distance](https://en.wikipedia.org/wiki/Edit_distance).

Calculating edit distance of two strings is as simple as:
```c
edlibAlign("hello", 5, "world!", 6, edlibDefaultAlignConfig()).editDistance;
```

Edlib is also available for **Python** [![PyPI version](https://img.shields.io/pypi/v/edlib.svg) (Click here for Python README)](https://pypi.python.org/pypi/edlib), with code residing at [bindings/python](bindings/python).

There are third-party bindings to edlib in other languages as well:

* [Edlib.jl](https://github.com/cjdoris/Edlib.jl), a Julia package created and supported by Christopher Rowley ([@cjdoris](https://github.com/cjdoris))
* [edlibR](https://github.com/evanbiederstedt/edlibr), an R package created and supported by Evan Biederstedt ([@evanbiederstedt](https://github.com/evanbiederstedt))
* [ruby-edlib](https://github.com/kojix2/ruby-edlib), a Ruby package created and supported by [@kojix2](https://github.com/kojix2)
* [Text::Levenshtein::Edlib](https://metacpan.org/pod/Text::Levenshtein::Edlib), a Perl wrapper created and supported by [Marius Gavrilescu](https://metacpan.org/author/MGV)

## Features
* Calculates **edit distance (Levenshtein distance)**.
* It can find **optimal alignment path** (instructions how to transform first sequence into the second sequence).
* It can find just the **start and/or end locations of alignment path** - can be useful when speed is more important than having exact alignment path.
* Supports **multiple [alignment methods](#alignment-methods)**: global(**NW**), prefix(**SHW**) and infix(**HW**), each of them useful for different scenarios.
* You can **extend character equality definition**, enabling you to e.g. have wildcard characters, to have case insensitive alignment or to work with degenerate nucleotides.
* It can easily handle small or **very large sequences**, even when finding alignment path, while consuming very little memory.
* **Super fast** thanks to Myers's bit-vector algorithm.


## Contents
- [Features](#features)
- [Building](#building)
- [Using Edlib in your project](#using-edlib-in-your-project)
- [Usage and examples](#usage-and-examples)
- [API documentation](#api-documentation)
- [Alignment methods](#alignment-methods)
- [Aligner](#aligner)
- [Running tests](#running-tests)
- [Benchmarks](#benchmarks)
- [Time and space complexity](#time-and-space-complexity)
- [Test data](#test-data)
- [Development and contributing](#development-and-contributing)
- [Publication](#publication)
- [Acknowledgements](#acknowledgements)


## Using Edlib in your project
You can use Edlib in you project by either directly copying header and source files from [edlib/](edlib/), or by linking Edlib library (see [Building](#building) for instructions how to build Edlib libraries).
In any case, only thing that you have to do in your source files is to include `edlib.h`.

To get you started quickly, let's take a look at a few ways to get simple Hello World project working.

Our Hello World project has just one source file, `helloWorld.cpp` file, and it looks like this:
```cpp
#include <cstdio>
#include "edlib.h"

int main() {
    EdlibAlignResult result = edlibAlign("hello", 5, "world!", 6, edlibDefaultAlignConfig());
    if (result.status == EDLIB_STATUS_OK) {
        printf("edit_distance('hello', 'world!') = %d\n", result.editDistance);
    }
    edlibFreeAlignResult(result);
}
```

Running it should output `edit_distance('hello', 'world!') = 5`.

### Approach #1: Directly copying edlib source and header files.
Here we directly copied [edlib/](edlib/) directory to our project, to get following project structure:
```
edlib/  -> copied from edlib/
  include/
    edlib.h
  src/
    edlib.cpp
helloWorld.cpp -> your program
```

Since `helloWorld` is a c++ program, we can compile it with just one line: `c++ helloWorld.cpp edlib/src/edlib.cpp -o helloWorld -I edlib/include`.

If hello world was a C program, we would compile it like this:
```
    c++ -c edlib/src/edlib.cpp -o edlib.o -I edlib/include
    cc -c helloWorld.c -o helloWorld.o -I edlib/include
    c++ helloWorld.o edlib.o -o helloWorld
```

### Approach #2: Copying edlib header file and static library.
Instead of copying edlib source files, you could copy static library (check [Building](#building) on how to create static library). We also need to copy edlib header files. We get following project structure:
```
edlib/  -> copied from edlib
  include/
    edlib.h
  edlib.a
helloWorld.cpp -> your program
```

Now you can compile it with `c++ helloWorld.cpp -o helloWorld -I edlib/include -L edlib -ledlib`.

### Approach #3: Install edlib library on machine.
Alternatively, you could avoid copying any Edlib files and instead install libraries by running `sudo make install` (check [Building](#building) for exact instructions depending on approach you used for building). Now, all you have to do to compile your project is `c++ helloWorld.cpp -o helloWorld -ledlib`.
If you get error message like `cannot open shared object file: No such file or directory`, make sure that your linker includes path where edlib was installed.

### Approach #4: Use edlib in your project via CMake.
#### Using git submodule
If you are using CMake for compilation, we suggest adding edlib as a git submodule with the command `git submodule add https://github.com/martinsos/edlib vendor/edlib`. Afterwards, modify your top level CMakeLists.txt file accordingly:
```
add_subdirectory(vendor/edlib EXCLUDE_FROM_ALL)
target_link_libraries(your_exe edlib) # or target_link_libraries(your_exe edlib)
```
The `add_subdirectory` command adds a folder to the build tree, meaning it will run CMakeLists.txt from the included folder as well. Flag `EXCLUDE_FROM_ALL` disables building (and instalment) of targets in the added folder which are not needed in your project. In the above example only the (static) library `edlib` will be build, while `edlib-aligner`, `hello_world` and the rest won't. In order to access the `edlib` API, add `#include "edlib.h"` in your source file (CMake will automatically update your include path).


For more example projects take a look at applications in [apps/](apps/).

#### Using VCPKG
Edlib is available on [VCPKG](https://github.com/microsoft/vcpkg) package manager. With VCPKG on your system, Edlib can be downloaded using the VCPKG install command `vcpkg install edlib`. Once the library has been downloaded, add the following instructions to your CMakeLists.txt file:
```
find_package(edlib CONFIG REQUIRED)
target_link_libraries(MyProject PRIVATE edlib::edlib)
```

then you should be able to include the library header in your project (`#include "edlib.h`)


## Building
### Meson
Primary way of building Edlib is via [Meson](https://mesonbuild.com/) build tool.

Requirements: make sure that you have `meson` installed on your system.

Execute
```
make
```
to build **static** library and binaries (apps and tests) and also run tests.  
To build **shared** library and binaries, do `make LIBRARY_TYPE=shared`.

Library and binaries will be created in `meson-build` directory.  
You can choose alternate build directory like this: `make BUILD_DIR=some-other-dir`.

Optionally, you can run
```
sudo make install
```
to install edlib library on your machine (on Linux, this will usually install it to `usr/local/lib` and `usr/local/include`).

Check Makefile if you want to run individual steps on your own (building, tests, ...).

NOTE: If you need more control, use `meson` command directly, `Makefile` is here only to help with common commands.

### CMake
Edlib can alternatively be built with CMake.

Execute following command to build Edlib using CMAKE:
```
cd build && cmake -D CMAKE_BUILD_TYPE=Release .. && make
```
This will create binaries in `bin/` directory and libraries (static and shared) in `lib/` directory.

```
./bin/runTests
```
to run tests.

Optionally, you can run
```
sudo make install
```
to install edlib library on your machine.

### Conda
Edlib can also be installed via Conda: [![Anaconda-Server Badge](https://anaconda.org/bioconda/edlib/badges/installer/conda.svg)](https://conda.anaconda.org/bioconda): `conda install edlib`.


## Usage and examples
Main function in edlib is `edlibAlign`. Given two sequences (and their lengths), it will find edit distance, alignment path or its end and start locations.

```c
char* query = "ACCTCTG";
char* target = "ACTCTGAAA"
EdlibAlignResult result = edlibAlign(query, 7, target, 9, edlibDefaultAlignConfig());
if (result.status == EDLIB_STATUS_OK) {
    printf("%d", result.editDistance);
}
edlibFreeAlignResult(result);
```

NOTE: One character is expected to occupy one char/byte, meaning that characters spanning multiple chars/bytes are not supported. As long as your alphabet size is <= 256 you can manually map it to numbers/chars from 0 to 255 and solve this that way, but if its size is > 256 then you will not be able to use Edlib.

### Configuring edlibAlign()
`edlibAlign` takes configuration object (it is a struct `EdlibAlignConfig`), which allows you to further customize how alignment will be done. You can choose [alignment method](#alignment-methods), tell edlib what to calculate (just edit distance or also path and locations) and set upper limit for edit distance.

For example, if you want to use infix(HW) alignment method, want to find alignment path (and edit distance), are interested in result only if edit distance is not larger than 42 and do not want to extend character equality definition, you would call it like this:
```c
edlibAlign(seq1, seq1Length, seq2, seq2Length,
           edlibNewAlignConfig(42, EDLIB_MODE_HW, EDLIB_TASK_PATH, NULL, 0));
```
Or, if you want to use suffix(SHW) alignment method, want to find only edit distance, do not have any limits on edit distance and want character '?' to match both itself and characters 'X' and 'Y', you would call it like this:
```c
EdlibEqualityPair additionalEqualities[2] = {{'?', 'X'}, {'?', 'Y'}};
edlibAlign(seq1, seq1Length, seq2, seq2Length,
           edlibNewAlignConfig(-1, EDLIB_MODE_SHW, EDLIB_TASK_DISTANCE, additionalEqualities, 2));
```

We used `edlibNewAlignConfig` helper function to easily create config, however we could have also just created an instance of it and set its members accordingly.

### Handling result of edlibAlign()
`edlibAlign` function returns a result object (`EdlibAlignResult`), which will contain results of alignment (corresponding to the task that you passed in config).

```c
EdlibAlignResult result = edlibAlign(seq1, seq1Length, seq2, seq2Length,
                                     edlibNewAlignConfig(-1, EDLIB_MODE_HW, EDLIB_TASK_PATH, NULL, 0));
if (result.status == EDLIB_STATUS_OK) {
    printf("%d\n", result.editDistance);
    printf("%d\n", result.alignmentLength);
    printf("%d\n", result.endLocations[0]);
}
edlibFreeAlignResult(result);
```

It is important to remember to free the result object using `edlibFreeAlignResult` function, since Edlib allocates memory on heap for certain members. If you decide to do the cleaning manually and not use `edlibFreeAlignResult`, do not forget to manually `free()` required members.

### Turning alignment to cigar
Cigar is a standard way to represent alignment path.
Edlib has helper function that transforms alignment path into cigar.
```c
char* cigar = edlibAlignmentToCigar(result.alignment, result.alignmentLength, EDLIB_CIGAR_STANDARD);
printf("%s", cigar);
free(cigar);
```

## API documentation

For complete documentation of Edlib library API, visit [http://martinsos.github.io/edlib](https://martinsos.github.io/edlib) (should be updated to the latest release).

To generate the latest API documentation yourself from the source, you need to have [doxygen](www.doxygen.org) installed.
Position yourself in the root directory and run `doxygen`, this will generate `docs/` directory. Then open `docs/html/index.html` file with you favorite browser.

Alternatively, you can directly check [edlib.h](edlib/include/edlib.h).

## Alignment methods

Edlib supports 3 alignment methods:
* **global (NW)** - This is the standard method, when we say "edit distance" this is the method that is assumed.
  It tells us the smallest number of operations needed to transform first sequence into second sequence.
  *This method is appropriate when you want to find out how similar is first sequence to second sequence.*
* **prefix (SHW)** - Similar to global method, but with a small twist - gap at query end is not penalized. What that means is that deleting elements from the end of second sequence is "free"!
  For example, if we had `AACT` and `AACTGGC`, edit distance would be 0, because removing `GGC` from the end of second sequence is "free" and does not count into total edit distance.
  *This method is appropriate when you want to find out how well first sequence fits at the beginning of second sequence.*
* **infix (HW)**: Similar as prefix method, but with one more twist - gaps at query end **and start** are not penalized. What that means is that deleting elements from the start and end of second sequence is "free"!
  For example, if we had `ACT` and `CGACTGAC`, edit distance would be 0, because removing `CG` from the start and `GAC` from the end of second sequence is "free" and does not count into total edit distance.
  *This method is appropriate when you want to find out how well first sequence fits at any part of second sequence.* For example, if your second sequence was a long text and your first sequence was a sentence from that text, but slightly scrambled, you could use this method to discover how scrambled it is and where it fits in that text.
  *In bioinformatics, this method is appropriate for aligning read to a sequence.*


## Aligner
Edlib comes with a standalone aligner cli app, which can be found at [apps/aligner/](apps/aligner).

![Edlib aligner screenshot](images/edlib-aligner-screenshot.png)

Aligner reads sequences from fasta files, and it can display alignment path in graphical manner or as a cigar.
It also measures calculation time, so it can be useful for testing speed and comparing Edlib with other tools.

Check [Building](#building) to see how to build binaries (including `edlib-aligner`).
Run `./build/bin/edlib-aligner` with no params for help and detailed instructions.

Example of usage:
`./build/bin/edlib-aligner -p apps/aligner/test_data/query.fasta apps/aligner/test_data/target.fasta`

**NOTE**: Aligner currently does not work on Windows, because it uses `getopt` to parse command line arguments, which is not supported on Windows.


## Running tests
Check [Building](#building) to see how to build binaries (including binary `runTests`).
To run tests, just run `./runTests`. This will run random tests for each alignment method, and also some specific unit tests.

### Sanitizers

With Meson, Ninja and Clang installed:

```sh
CC=clang CXX=clang++ make sanitizer
```

This recreates `build/sanitizer`, builds a debugoptimized static library with
AddressSanitizer and UndefinedBehaviorSanitizer, and runs the existing tests.
Undefined behavior is fatal; frame pointers are retained for diagnostics.
Override `SANITIZER_BUILD_DIR` to use another disposable build directory, or
`SANITIZER_TEST_TIMEOUT_MULTIPLIER` (default 10) for slower machines.
On Linux, use `ASAN_OPTIONS=detect_leaks=1` to explicitly enable leak detection.
Apple Clang does not provide the same LeakSanitizer coverage.

CI runs this as a separate, blocking Linux/Clang job, including leak detection
and a sanitizer-instrumented smoke benchmark matrix. Sanitized builds are for
correctness checks, not performance comparisons; do not run them under Valgrind.

## Benchmarks

The self-contained matrix in `benchmarks/run.py` needs Python 3 (standard library
only) and the native `edlib-benchmark` executable. It does not require downloaded
datasets, Python bindings, or other aligners. The executable is an explicit target
in both build systems and is excluded from ordinary builds.

```sh
cmake -S . -B build/benchmarks -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTING=OFF \
  -DEDLIB_BUILD_EXAMPLES=OFF -DEDLIB_BUILD_UTILITIES=OFF
cmake --build build/benchmarks --target edlib-benchmark
python3 benchmarks/run.py --binary build/benchmarks/bin/edlib-benchmark \
  --suite smoke --label "baseline, Release" --output build/benchmarks/baseline.json
```

With Meson, configure a separate release directory and explicitly build the target:
`meson setup build/benchmarks-meson --buildtype=release`, then
`meson compile -C build/benchmarks-meson edlib-benchmark`.
The executable is `build/benchmarks-meson/edlib-benchmark`.

Use `--suite full` for the complete matrix:

| Dimension | Coverage |
| --- | --- |
| Modes | NW, SHW, HW |
| Tasks | Distance, locations, path |
| Equal-length inputs before mutation | 32, 63, 64, 65, 128, 1,024, 10,000, 100,000 characters |
| Short query / long target | 128 / 100,000 and 1,024 / 100,000 |
| Alphabets | DNA (4), protein (20), arbitrary bytes (256, including NUL) |
| Mutation probabilities | 0%, 5%, 30%; mixed substitutions, insertions and deletions |
| Distance limits | Automatic; exact optimum; one below optimum (no solution) |

The full matrix selects 2,430 cases. The smoke matrix selects 216 cases using
32 / 32 and 65 / 4,096 inputs, DNA/bytes, and 0%/30% mutations, retaining every
mode, task and distance-limit policy. A rejection case with optimum zero is
explicitly skipped: a negative `k` would request an unbounded search instead.
Long, dissimilar full-matrix cases can be expensive; `--timeout` limits each
case to 300 seconds by default and failures stop the run.

Inputs use a fixed seed (default 42), with the mutated query-derived segment
at the target's beginning for NW/SHW and centered for HW when there is padding.
Indels change the actual target length, which is recorded alongside the requested
size. Mutation probability is not measured sequence identity or edit distance.
`--list` lists cases without executing them; `--filter q100000-t100000` selects
only those long-input cases.

Each case runs in a fresh process. An untimed unbounded-distance call establishes
the cutoff and expected score; every timed call checks score/status consistency.
This is not an independent correctness oracle. After warmup and batch calibration,
the driver records five samples by default, targeting at least 10 ms per batch.
`--samples` and `--sample-ms` control these settings. Timing includes alignment,
status/score checks and result deallocation, but excludes input generation and
process startup. Reports retain raw nanoseconds per alignment and median/min/max.

Memory is **peak process resident memory**, not live Edlib allocations. It includes
input generation, the reference call, warmup and allocator retention. Linux/macOS
use `getrusage`; Windows uses peak working-set size. Compare memory on the same OS;
small changes can be obscured by runtime overhead and page granularity.

After rebuilding with a candidate change, run the same selection and seed:

```sh
python3 benchmarks/run.py --binary build/benchmarks/bin/edlib-benchmark \
  --suite smoke --label "candidate, Release" --output build/benchmarks/candidate.json \
  --baseline build/benchmarks/baseline.json > build/benchmarks/comparison.csv
# Or compare two existing reports without rerunning:
python3 benchmarks/run.py --compare build/benchmarks/candidate.json \
  --baseline build/benchmarks/baseline.json
```

Comparison rejects mismatched cases, seeds, input fingerprints or reference
distances. Speedup is baseline median / candidate median: above 1 is faster.
Inspect per-case results, not just the geometric mean. Use an idle machine,
identical compiler flags and consistent power settings; repeat measurements before
calling a small difference a regression. JSON records the binary hash, compiler
version, platform and measurement settings; use `--label` to record the revision,
CPU model and build flags, and retain `compile_commands.json` with the results.

The Benchmarks workflow runs the release smoke matrix on PRs/pushes, and supports
manual full-matrix runs. It uploads JSON results and compiler configuration.
Hosted-runner timing noise does not fail CI on speed differences; build, execution
and comparison-test errors do fail it. Run comparison safeguards locally with
`python3 -m unittest discover -s benchmarks -p 'test_*.py'`.


## Time and space complexity
Edlib is based on [Myers's bit-vector algorithm](http://www.gersteinlab.org/courses/452/09-spring/pdf/Myers.pdf) and extends from it.
It calculates a dynamic programming matrix of dimensions `Q x T`, where `Q` is the length of the first sequence (query), and `T` is the length of the second sequence (target). It uses Ukkonen's banded algorithm to reduce the space of search, and there is also parallelization from Myers's algorithm, however time complexity is still quadratic.
Edlib uses Hirschberg's algorithm to find alignment path, therefore space complexity is linear.

Time complexity: `O(T * Q)`.

Space complexity: `O(T + Q)`.

It is worth noting that Edlib works best for large, similar sequences, since such sequences get the highest speedup from banded approach and bit-vector parallelization.


## Test data
In [test_data/](test_data) directory there are different genome sequences, ranging from 10 kbp to 5 Mbp in length. They are ranging in length and similarity, so they can be useful for testing and measuring speed in different scenarios.


## Development and contributing
Feel free to send pull requests and raise issues.

When developing, you may want to use `-D CMAKE_BUILD_TYPE=Debug` flag when calling `cmake` in order to get debugging flags passed to compiler. This should also happen if you just run `cmake ..` with no flags, but I think I have noticed it does not always works as expected (probably has something to do with cmake cache). To check which flags is compiler using, run `make` with `VERBOSE=1`: `make VERBOSE=1`.


## Publication

Martin Šošić, Mile Šikić; Edlib: a C/C ++ library for fast, exact sequence alignment using edit distance. Bioinformatics 2017 btw753. doi: [10.1093/bioinformatics/btw753](https://doi.org/10.1093/bioinformatics/btw753)


## Acknowledgements

Mile Šikić (@msikic) - Mentoring and guidance through whole project.

Ivan Sović (@isovic) - Help with testing and prioritizing features, valuable comments on the manuscript.

## FAQ

### What do terms NW, HW and SHW stand for?
NW stands for Needleman-Wunsch, HW for Hybrid Wunsch, and SHW for Semi Hybrid Wunsch. While NW is a common abbreviation, HW and SHW abbreviations were made up at the very start of this project to describe additional modes of alignment. Later we started using terms "global", "infix" and "prefix" more, as they describe the modes better, but terms NW, HW and SHW are still very present in the project.
