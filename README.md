# Myers

C/C++ library for sequence alignment based on Myers's "Fast Bit-Vector Algorithm for Approximate String Matching Based on Dynamic Programming".  
Implementation combines Myers's bit-vector algorithm with Ukonnen's banded algorithm.  
Calculates best score (edit distance) and first position of best score.  


#### Alignment modes
There are 3 different modes of alignment:
* NW: Global alignment (Needleman-Wunsch).
* SHW: Semi-global alignment. Gap at query end is not penalized.
* HW: Semi-global alignment. Gap at query start and gap at query end are not penalized.


#### Usage
Include myers.h in your code and compile it together with myers.cpp.  
For examples of usage take a look at test.cpp.


#### Running tests
In order to run tests, do following:

    cd src
    make
    ./test
