/////////////////////////////////////////////////////////////////////////
///
///  \file          Components_UnitTest.cpp
///  \brief         Unit test for Components.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-20 22:24:59
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
#include "../Components.h"
#include <catch.hpp>

TEST_CASE("Standard") {
    CHECK(DecisionEngine::Core::Components::MaxScore >= 1.0f);
    CHECK(Approx(static_cast<float>(static_cast<unsigned long>(DecisionEngine::Core::Components::MaxScore))) == DecisionEngine::Core::Components::MaxScore);
}
