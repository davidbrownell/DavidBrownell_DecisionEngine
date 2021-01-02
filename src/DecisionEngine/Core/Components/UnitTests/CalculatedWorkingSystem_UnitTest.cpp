/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedWorkingSystem_UnitTest.cpp
///  \brief         Unit test for CalculatedWorkingSystem.h
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
#include "../CalculatedWorkingSystem.h"
#include <catch.hpp>

#include "../Condition.h"
#include "../WorkingSystem.h"

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

class MyWorkingSystem : public NS::WorkingSystem {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    MyWorkingSystem(void) :
        NS::WorkingSystem(
            NS::Score(),
            NS::Index()
        )
    {}

#define ARGS                                BASES(NS::WorkingSystem)

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
        return SystemPtrs();
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyWorkingSystem);

class MyCalculatedWorkingSystem : public NS::CalculatedWorkingSystem {
public:
    // ----------------------------------------------------------------------
    // |  Public Data
    bool const                              GenerateValid;

    // ----------------------------------------------------------------------
    // |  Public Methods
    MyCalculatedWorkingSystem(
        bool generateValid
    ) :
        NS::CalculatedWorkingSystem(
            NS::Score(NS::Condition::Result(g_pCondition, true), false),
            NS::Index(0)
        ),
        GenerateValid(std::move(generateValid))
    {}

    ~MyCalculatedWorkingSystem(void) override = default;

#define ARGS                                MEMBERS(GenerateValid), BASES(NS::CalculatedWorkingSystem)

    NON_COPYABLE(MyCalculatedWorkingSystem);
    MOVE(MyCalculatedWorkingSystem, ARGS);
    COMPARE(MyCalculatedWorkingSystem, ARGS);
    SERIALIZATION(MyCalculatedWorkingSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::System)));

#undef ARGS

    std::string ToString(void) const override { return "MyCalculatedWorkingSystem"; }

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    WorkingSystemPtr CommitImpl(NS::Score, NS::Index) override {
        if(GenerateValid)
            return std::make_shared<MyWorkingSystem>();

        return std::shared_ptr<MyWorkingSystem>();
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyCalculatedWorkingSystem);

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
TEST_CASE("Construct") {
    MyCalculatedWorkingSystem(true);
    CHECK(true);
}

TEST_CASE("Commit") {
    CHECK(MyCalculatedWorkingSystem(true).Commit().get());
    CHECK_THROWS_MATCHES(
        MyCalculatedWorkingSystem(false).Commit(),
        std::logic_error,
        Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid result")
    );
}
