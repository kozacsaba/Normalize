/*  This will be the main test runner. All unit tests should be included in 
    this file. Google Test automatically registers them and thus they should be
    run automatically if included.
    For more information on google test visit:

    https://google.github.io/googletest/primer.html
    -for a beginners' guide on google unit tests

    https://google.github.io/googletest/quickstart-cmake.html
    -for setting up gtest with cmake
*/

#include <gtest/gtest.h>

// include headers containing the unit tests here
#include "HelloTest.h"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
