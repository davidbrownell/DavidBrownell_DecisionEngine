/////////////////////////////////////////////////////////////////////////
///
///  \file          System_UnitTest.cpp
///  \brief         Unit test for System.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-23 22:06:03
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
#include "../System.h"
#include <catch.hpp>

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::Core::Components;

// ----------------------------------------------------------------------
// |
// |  Internal Types and Methods
// |
// ----------------------------------------------------------------------

#if (defined __clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

NS::Condition::Result::ConditionPtr const   g_pCondition(NS::Condition::Create("Global Condition", static_cast<unsigned short>(100)));

#if (defined __clang__)
#   pragma clang diagnostic pop
#endif

class MySystem : public NS::System {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    template <typename... ArgTs>
    MySystem(ArgTs &&... args) :
        NS::System(std::forward<ArgTs>(args)...)
    {}

    ~MySystem(void) override = default;

    NON_COPYABLE(MySystem);
    MOVE(MySystem, BASES(NS::System));
    COMPARE(MySystem, BASES(NS::System));
    SERIALIZATION(MySystem, BASES(NS::System), FLAGS(SERIALIZATION_POLYMORPHIC(NS::System)));

    std::string ToString(void) const override { return "MySystem"; }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MySystem);

// ----------------------------------------------------------------------
// |
// |  System
// |
// ----------------------------------------------------------------------
TEST_CASE("Construct") {
    SECTION("Test1") {
        MySystem const                      system(
            MySystem::TypeValue::Working,
            MySystem::CompletionValue::Calculated,
            NS::Score(NS::Condition::Result(g_pCondition, true), false),
            NS::Index(0)
        );

        CHECK(system.Type == MySystem::TypeValue::Working);
        CHECK(system.Completion == MySystem::CompletionValue::Calculated);
        CHECK(system.GetScore() == NS::Score(NS::Condition::Result(g_pCondition, true), false));
        CHECK(system.GetIndex() == NS::Index(0));
    }

    SECTION("Test2") {
        MySystem const                      system(
            MySystem::TypeValue::Result,
            MySystem::CompletionValue::Concrete,
            NS::Score(),
            NS::Index()
        );

        CHECK(system.Type == MySystem::TypeValue::Result);
        CHECK(system.Completion == MySystem::CompletionValue::Concrete);
        CHECK(system.GetScore() == NS::Score());
        CHECK(system.GetIndex() == NS::Index());
    }
}

TEST_CASE("Construct - Errors") {
    CHECK_THROWS_MATCHES(
        MySystem(
            MySystem::TypeValue::Working,
            MySystem::CompletionValue::Concrete,
            NS::Score(NS::Condition::Result(g_pCondition, true), false),
            NS::Index()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("score")
    );

    CHECK_THROWS_MATCHES(
        MySystem(
            MySystem::TypeValue::Working,
            MySystem::CompletionValue::Calculated,
            NS::Score(NS::Condition::Result(g_pCondition, true), false).Commit(),
            NS::Index()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("score")
    );

    CHECK_THROWS_MATCHES(
        MySystem(
            MySystem::TypeValue::Working,
            MySystem::CompletionValue::Concrete,
            NS::Score(NS::Condition::Result(g_pCondition, true), false).Commit(),
            NS::Index(0)
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("index")
    );

    CHECK_THROWS_MATCHES(
        MySystem(
            MySystem::TypeValue::Working,
            MySystem::CompletionValue::Calculated,
            NS::Score(NS::Condition::Result(g_pCondition, true), false),
            NS::Index()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("index")
    );
}

TEST_CASE("UpdateScore") {
    SECTION("Calculated") {
        MySystem                            system(
            MySystem::TypeValue::Working,
            MySystem::CompletionValue::Calculated,
            NS::Score(NS::Condition::Result(g_pCondition, false), false),
            NS::Index(1)
        );

        CHECK(system.GetScore() == NS::Score(NS::Condition::Result(g_pCondition, false), false));

        system.UpdateScore(NS::Score(NS::Condition::Result(g_pCondition, true), false));
        CHECK(system.GetScore() == NS::Score(NS::Condition::Result(g_pCondition, true), false));

        CHECK_THROWS_MATCHES(
            system.UpdateScore(NS::Score(NS::Condition::Result(g_pCondition, true), false).Commit()),
            std::invalid_argument,
            Catch::Matchers::Exception::ExceptionMessageMatcher("score")
        );
    }

    SECTION("Concrete") {
        MySystem                            system(
            MySystem::TypeValue::Working,
            MySystem::CompletionValue::Concrete,
            NS::Score(NS::Condition::Result(g_pCondition, false), false).Commit(),
            NS::Index(1).Commit()
        );

        CHECK(system.GetScore() == NS::Score(NS::Condition::Result(g_pCondition, false), false).Commit());

        system.UpdateScore(NS::Score(NS::Condition::Result(g_pCondition, true), false).Commit());
        CHECK(system.GetScore() == NS::Score(NS::Condition::Result(g_pCondition, true), false).Commit());

        CHECK_THROWS_MATCHES(
            system.UpdateScore(NS::Score(NS::Condition::Result(g_pCondition, true), false)),
            std::invalid_argument,
            Catch::Matchers::Exception::ExceptionMessageMatcher("score")
        );
    }
}

TEST_CASE("UpdateScore - Error") {
    CHECK_THROWS_MATCHES(
        MySystem(
            MySystem::TypeValue::Working,
            MySystem::CompletionValue::Concrete,
            NS::Score(),
            NS::Index()
        ).UpdateScore(
            NS::Score(NS::Condition::Result(g_pCondition, true), false)
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("score")
    );
}

TEST_CASE("GetScore") {
    MySystem const                          system(
        MySystem::TypeValue::Working,
        MySystem::CompletionValue::Calculated,
        NS::Score(NS::Condition::Result(g_pCondition, true), false),
        NS::Index(20)
    );

    CHECK(system.GetScore() == NS::Score(NS::Condition::Result(g_pCondition, true), false));
}

TEST_CASE("GetIndex") {
    MySystem const                          system(
        MySystem::TypeValue::Working,
        MySystem::CompletionValue::Calculated,
        NS::Score(NS::Condition::Result(g_pCondition, true), false),
        NS::Index(20)
    );

    CHECK(system.GetIndex() == NS::Index(20));
}

TEST_CASE("Compare") {
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(20)
            ),
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(20)
            ),
            true
        ) == 0
    );

    // Type
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(20)
            ),
            MySystem(
                MySystem::TypeValue::Result,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(20)
            )
        ) == 0
    );

    // Score
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, false), false),
                NS::Index(20)
            ),
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(20)
            )
        ) == 0
    );

    // Index
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(1)
            ),
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(0)
            )
        ) == 0
    );

    // Completion
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(1)
            ),
            MySystem(
                MySystem::TypeValue::Result,
                MySystem::CompletionValue::Concrete,
                NS::Score(NS::Condition::Result(g_pCondition, true), false).Commit(),
                NS::Index(1).Commit()
            )
        ) == 0
    );
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            MySystem(
                MySystem::TypeValue::Working,
                MySystem::CompletionValue::Calculated,
                NS::Score(NS::Condition::Result(g_pCondition, true), false),
                NS::Index(20)
            ),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
