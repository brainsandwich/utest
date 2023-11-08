# µtest 1.0.0

A micro c++ unit test framework with bare-minimum features

## Features

- easy to setup
- only 6 test operators : 
    - `test_eq`-> equal
    - `test_ne`-> not equal
    - `test_ge`-> greater or equal
    - `test_gt`-> greater than
    - `test_le`-> less or equal
    - `test_lt`-> less than
- sections for better organization
- custom type print (see `example.cc`)
- test summary with verbosity control

## Usage

```cmake
# Here, utest is used as a submodule
add_subdirectory(ext/utest)

# Create new executable
add_executable(example_test "example.cc")

# Link with utest_main which defines an entry point for you
target_link_libraries(example_test PRIVATE utest::main)
```

```cpp
#include <utest.h>

// Create a new test fixture
test_define(example, basic)
{
    // Section for specific cases
    test_section("integers")
    {
        int value = 23;

        // All the operators you need !
        test_eq(value, 29);
        test_ne(value, 29);
        test_ge(value, 29);
        test_gt(value, 29);
        test_le(value, 29);
        test_lt(value, 29);
    }
}
```

```shell

# This will only print tests, the number
# of cases that passed and if it was a success
./example_test --verbosity quiet

# This will print details of cases that
# failed (and express the values that didn't match)
./example_test --verbosity failures

# Same as before, but prints a one-liner for
# cases that passed
./example_test --verbosity passed

# Prints case comparison details for every
# test case
./example_test --verbosity everything

```