# Agent Instructions for `tests`

Host-side unit tests live here and use GoogleTest.

- Build all tests with `make`.
- Execute the binaries such as `./falling_bricks_test` and `./strip_state_test` after building.
- New tests should be named `<feature>_test.cpp` and linked through the `Makefile` using g++ with the `-std=c++17` flag.
