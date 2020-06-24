/////////////////////////////////////////////////////////////////////////
///
///  \file          ConstrainedResource_UnitTest.cpp
///  \brief         Unit test for ConstrainedResource
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-23 09:55:58
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2020
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../ConstrainedResource.h"
#include <catch.hpp>

// Nothing to test; content is to prevent warnings
TEST_CASE("Standard") {
    CHECK(true);
}
