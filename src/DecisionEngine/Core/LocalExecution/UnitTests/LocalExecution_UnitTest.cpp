/////////////////////////////////////////////////////////////////////////
///
///  \file          LocalExecution_UnitTest.cpp
///  \brief         Unit test for LocalExecution
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-19 08:27:09
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2020-22
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../LocalExecution.h"
#include <catch.hpp>

// Nothing to test; content is to prevent warnings
TEST_CASE("Standard") {
    CHECK(true);
}
