/////////////////////////////////////////////////////////////////////////
///
///  \file          ResultSystem_UnitTest.cpp
///  \brief         Unit test for ResultSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 19:46:13
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
#include "../ResultSystem.h"
#include <catch.hpp>

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::Core::Components;

// ----------------------------------------------------------------------
// |
// |  Internal Types and Methods
// |
// ----------------------------------------------------------------------
class MyResultSystem : public NS::ResultSystem {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    MyResultSystem(NS::Score score, NS::Index index) : NS::ResultSystem(std::move(score), std::move(index)) {}

#define ARGS                                BASES(NS::ResultSystem)

    NON_COPYABLE(MyResultSystem);
    MOVE(MyResultSystem, ARGS);
    COMPARE(MyResultSystem, ARGS);
    SERIALIZATION(MyResultSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::System)));

#undef ARGS

    std::string ToString(void) const override { return "MyResultSystem"; }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyResultSystem);

TEST_CASE("Construct") {
    MyResultSystem(NS::Score(), NS::Index());
    CHECK(true);
}
