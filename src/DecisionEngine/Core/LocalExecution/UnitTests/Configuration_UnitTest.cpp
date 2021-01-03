/////////////////////////////////////////////////////////////////////////
///
///  \file          Configuration_UnitTest.cpp
///  \brief         Unit test for Configuration
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-06 00:37:25
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2020-21
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../Configuration.h"
#include <catch.hpp>

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::Core::LocalExecution;

// ----------------------------------------------------------------------
class MyConfiguration : public NS::Configuration {
public:
    using NS::Configuration::Configuration;

    NON_COPYABLE(MyConfiguration);

#define ARGS                                BASES(NS::Configuration)

    MOVE(MyConfiguration, ARGS);
    COMPARE(MyConfiguration, ARGS);
    SERIALIZATION(MyConfiguration, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::Configuration)));

#undef ARGS

    size_t GetMaxNumPendingSystems(void) const override {
        return 1;
    }

    size_t GetMaxNumPendingSystems(WorkingSystem const &) const override {
        return 1;
    }

    size_t GetMaxNumChildrenPerGeneration(WorkingSystem const &) const override {
        return 1;
    }

    size_t GetMaxNumIterationsPerRound(WorkingSystem const &) const override {
        return 1;
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyConfiguration);

TEST_CASE("Construct") {
    {
        MyConfiguration const                   config(true, false, 10);

        CHECK(config.ContinueProcessingSystemsWithFailures);
        CHECK(config.IsDeterministic == false);
        CHECK((config.NumConcurrentTasks && *config.NumConcurrentTasks == 10));
    }

    {
        MyConfiguration const                   config(true, false);

        CHECK(config.ContinueProcessingSystemsWithFailures);
        CHECK(config.IsDeterministic == false);
        CHECK(!config.NumConcurrentTasks);
    }
}

TEST_CASE("Construct - Errors") {
    CHECK_THROWS_MATCHES(MyConfiguration(true, true, 0), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("numConcurrentTasks"));
}

TEST_CASE("Compare") {
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MyConfiguration(true, false, 10),
            MyConfiguration(true, false, 10),
            true
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MyConfiguration(false, true, 1),
            MyConfiguration(true, true, 1)
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MyConfiguration(true, false, 1),
            MyConfiguration(true, true, 1)
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MyConfiguration(true, true, 1),
            MyConfiguration(true, true, 2)
        ) == 0
    );
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            MyConfiguration(true, true, 1),
            [](std::string const &value) {
                UNSCOPED_INFO(value);
                CHECK(true);
            }
        ) == 0
    );
}
