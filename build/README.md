This directory is here for your convenience while building edlib.

To build edlib library, apps and tests, do following:
- position yourself in this directory
- `cmake -D CMAKE_BUILD_TYPE=Release ..` if you want optimized code, or just `cmake ..` if you want to do debugging.
- `make`

This will create apps and tests in `bin/` directory, and static and shared libraries in `lib/` directory.
