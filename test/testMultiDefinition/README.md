In this test, we want to ensure that when edlib is included in multiple complication units, no "multiple definitions" error will occur.
This error is something we are concerned with because due to using templates we have function definitions in header, and they cause this kind of error in this kind of situation. Solution is to make functions "inlined", and this test ensures that.

The source file, staticTestLib.cpp, is created to find all multiple function definitions originating from edlib library.
Such multiple definitions may happen during linking a static library to another program, where both the library and user program include edlib
A static library is created out of staticTestLib.cpp in which edlib is included.
A null function in this library is going to be linked to the user program, testMultiDefinition.cpp.
Since the user program includes edlib in itself, this linking cause the linker to catch all the multiple function definitions originating from edlib.

The instructions to check multiple definitions is as below:

1. `g++ -c staticTestLib.cpp -I ../../edlib/src -I ../../edlib/include -o staticTestLib.o`
2. `g++ -c testMultiDefinition.cpp -I ../../edlib/src -I ../../edlib/include -o testMultiDefinition.o`
3. `ld testMultiDefinition.o staticTestLib.o`

// TODO: should it be in cmake or not, and how to run it automatically / as part of CI.