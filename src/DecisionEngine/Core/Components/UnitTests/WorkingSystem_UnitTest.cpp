/////////////////////////////////////////////////////////////////////////
///
///  \file          WorkingSystem_UnitTest.cpp
///  \brief         Unit test for WorkingSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 18:54:12
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
#include "../WorkingSystem.h"
#include <catch.hpp>

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::Core::Components;

// ----------------------------------------------------------------------
// |
// |  Internal Types and Methods
// |
// ----------------------------------------------------------------------
class MyWorkingSystem : public NS::WorkingSystem {
public:
    // ----------------------------------------------------------------------
    // |  Public Data
    size_t const                            NumResults;

    // ----------------------------------------------------------------------
    // |  Public Methods
    MyWorkingSystem(size_t numResults) : NumResults(std::move(numResults)) {}

#define ARGS                                MEMBERS(NumResults), BASES(NS::WorkingSystem)

    NON_COPYABLE(MyWorkingSystem);
    MOVE(MyWorkingSystem, ARGS);
    COMPARE(MyWorkingSystem, ARGS);
    SERIALIZATION(MyWorkingSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::System)));

#undef ARGS

    std::string ToString(void) const override { return "MyWorkingSystem"; }

    bool IsComplete(void) const override { return false; }

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    SystemPtrs GenerateChildrenImpl(size_t) override {
        SystemPtrs                          results;

        while(results.size() < NumResults) {
            results.emplace_back(std::make_shared<MyWorkingSystem>(0));
        }

        return results;
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyWorkingSystem);

TEST_CASE("Construct") {
    MyWorkingSystem(10);
    CHECK(true);
}

TEST_CASE("GenerateChildren - valid") {
    CHECK(MyWorkingSystem(2).GenerateChildren(10).size() == 2);
}

TEST_CASE("GenerateChildren - errors") {
    // Invalid argument
    CHECK_THROWS_MATCHES(
        MyWorkingSystem(2).GenerateChildren(0),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("maxNumChildren")
    );

    // Empty results
    CHECK_THROWS_MATCHES(
        MyWorkingSystem(0).GenerateChildren(1),
        std::logic_error,
        Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid results")
    );

    // Too many
    CHECK_THROWS_MATCHES(
        MyWorkingSystem(2).GenerateChildren(1),
        std::logic_error,
        Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid results")
    );
}
