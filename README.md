# JSON parser with handlers

A C++23 JSON parser with callback-based handlers. `Searchhandler` extracts a
scalar value at a dot-separated object-key path without building a DOM or
allocating heap memory in the handler. The command-line demo allocates an input
buffer to read the file; file I/O is not a zero-allocation operation.

## Build and test

Use a C++23 compiler and standard library with `std::expected` support, and
CMake 3.15 or newer.

```sh
cmake -S . -B build
cmake --build build
(cd build && ctest --output-on-failure)
python3 tests/cli_tests.py build/MyProject
python3 tests/departure_board_tests.py build/departure_board
```

The Python smoke tests are optional and use only the standard library. For a
multi-configuration generator, pass the executable's configuration directory,
for example `build/Debug/MyProject`.

Without CMake, build and run the same C++ tests with GCC:

```sh
mkdir -p build
g++ -std=c++23 -Wall -Wextra -Wpedantic -Iinclude -Iapi/include \
    main.cpp src/*.cpp api/src/*.cpp -o build/MyProject
g++ -std=c++23 -Wall -Wextra -Wpedantic -Iinclude -Iapi/include \
    tests/searchhandler_tests.cpp src/*.cpp api/src/*.cpp \
    -o build/searchhandler_tests
g++ -std=c++23 -Wall -Wextra -Wpedantic -Iinclude -Iapi/include \
    examples/departure_board.cpp src/*.cpp -o build/departure_board
./build/searchhandler_tests
python3 tests/cli_tests.py build/MyProject
python3 tests/departure_board_tests.py build/departure_board
```

## Render a departure board

`examples/enschede-departures.json` is a **synthetic example**, not a verified
train timetable. Its services, times and status must not be used for travel
planning. Run the example from the repository root:

```sh
./build/departure_board
# Or pass a different fixture using the same schema:
./build/departure_board examples/enschede-departures.json
```

It reads the JSON through a custom handler and renders the whole board:

```text
Enschede | 2026-12-15 | morning
SYNTHETIC EXAMPLE: times are unverified; not for travel planning.

Departure  Destination         Service  Arrival
07:40      Hengelo             -        -
07:56      Dortmund Hbf        RB51     10:06
```

The example handler uses fixed storage for up to 32 departures. Each departure
is an object with `destination` and `departure` string fields; `service` and
`arrival` are optional. The board needs `station`, `date` and `period` strings.
It displays values as stored, without decoding escapes or interpreting dates.
This is a small schema-specific rendering example, not a live transport client.
The table columns use minimum widths, so unusually long values may overflow.

## Search individual fields

Search it from the repository root:

```sh
./build/MyProject examples/enschede-departures.json board.departures.dortmund.destination
# Dortmund Hbf

./build/MyProject examples/enschede-departures.json board.departures.dortmund.departure
# 07:56

./build/MyProject examples/enschede-departures.json board.departures.dortmund.platform
# null
```

The `MyProject` search CLI prints one scalar per invocation; `departure_board`
prints the whole board. The search CLI now requires both a file and a path instead of using a hard-coded `data.json` and
`store.location.zip`. Exit status is `0` for a match, `1` for a file/parse error
or missing scalar, and `2` for invalid arguments/path. Errors go to stderr.

The Dortmund entry follows another nested object deliberately: it demonstrates
that skipping an unrelated departure must not lose the matching parent's path.

## Searchhandler semantics and lifetime

- Paths contain 1 to 64 nonempty object keys separated by dots. Empty keys,
  literal dots in keys, JSON Pointer syntax, and array indices are unsupported.
- Arrays and their contents are skipped. A path naming an object or array does
  not produce a scalar result.
- Values can be strings, numbers, booleans or null. The first matching scalar is
  returned, including when an object contains duplicate keys.
- Strings are returned without quotes but with JSON escapes unchanged. Numbers
  retain their original spelling. Keys are also compared in their raw form.
- The handler borrows its path. String and number results borrow the JSON input.
  Keep both buffers alive and unchanged while their views are in use.
- Create a fresh handler and parser for each document. Check the parser result
  before consuming a match: a prefix may match before a later parse error.
- The handler accepts up to 64 keys, but the parser limits container nesting
  to 63, so object-only paths through this parser can reach at most 63 keys.
  These tests exercise search traversal and the CLI; they are not a
  JSON-conformance test suite.

```cpp
std::string_view path = "board.departures.dortmund.destination";
Searchhandler handler{path};
Parser parser{input.data(), input.size(), handler};
auto parsed = parser.parse();
if (parsed && handler.found()) {
    std::cout << handler.result() << '\n';
}
```
