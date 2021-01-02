/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedResultSystem_UnitTest.cpp
///  \brief         Unit test for CalculatedResultSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 20:39:00
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
#include "../CalculatedResultSystem.h"
#include <catch.hpp>

#include "../Condition.h"
#include "../ResultSystem.h"

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

class MyResultSystem : public NS::ResultSystem {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    MyResultSystem(void) :
        NS::ResultSystem(
            NS::Score(),
            NS::Index()
        )
    {}

#define ARGS                                BASES(NS::ResultSystem)

    NON_COPYABLE(MyResultSystem);
    MOVE(MyResultSystem, ARGS);
    COMPARE(MyResultSystem, ARGS);
    SERIALIZATION(MyResultSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::System)));

#undef ARGS

    std::string ToString(void) const override { return "MyResultSystem"; }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyResultSystem);

class MyCalculatedResultSystem : public NS::CalculatedResultSystem {
public:
    // ----------------------------------------------------------------------
    // |  Public Data
    bool const                              GenerateValid;

    // ----------------------------------------------------------------------
    // |  Public Methods
    MyCalculatedResultSystem(
        bool generateValid
    ) :
        NS::CalculatedResultSystem(
            NS::Score(NS::Condition::Result(g_pCondition, true), false),
            NS::Index(0)
        ),
        GenerateValid(std::move(generateValid))
    {}

    ~MyCalculatedResultSystem(void) override = default;

#define ARGS                                MEMBERS(GenerateValid), BASES(NS::CalculatedResultSystem)

    NON_COPYABLE(MyCalculatedResultSystem);
    MOVE(MyCalculatedResultSystem, ARGS);
    COMPARE(MyCalculatedResultSystem, ARGS);
    SERIALIZATION(MyCalculatedResultSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::System)));

#undef ARGS

    std::string ToString(void) const override { return "MyCalculatedResultSystem"; }

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    ResultSystemUniquePtr CommitImpl(NS::Score, NS::Index) override {
        if(GenerateValid)
            return std::make_unique<MyResultSystem>();

        return std::unique_ptr<MyResultSystem>();
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyCalculatedResultSystem);

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
TEST_CASE("Construct") {
    MyCalculatedResultSystem(true);
    CHECK(true);
}

TEST_CASE("Commit") {
    CHECK(MyCalculatedResultSystem(true).Commit().get());
    CHECK_THROWS_MATCHES(
        MyCalculatedResultSystem(false).Commit(),
        std::logic_error,
        Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid result")
    );
}
