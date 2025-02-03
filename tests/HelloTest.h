/*  Trivial test, just to see if everything is set up correctly with the
    google test submodule
*/

#pragma once

#include <gtest/gtest.h>

TEST(HelloTest, BasicAssertions) 
{
  std::string hello = "hello";
  std::string world = "world";
  std::string hw = hello + " " + world;
  EXPECT_STRNE(hw.c_str(), hello.c_str());
  EXPECT_EQ(7 * 6, 42);
}
