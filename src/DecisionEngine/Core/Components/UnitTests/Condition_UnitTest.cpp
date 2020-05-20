/////////////////////////////////////////////////////////////////////////
///
///  \file          Condition_UnitTest.cpp
///  \brief         Unit test for Condition.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-08 21:41:21
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
#include "../Condition.h"
#include <catch.hpp>

#include <CommonHelpers/TestHelpers.h>
#include <BoostHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::Core::Components;

TEST_CASE("Condition - Construct") {
    std::shared_ptr<NS::Condition> const    pCondition(NS::Condition::Create("The Condition", static_cast<unsigned short>(10)));

    CHECK(pCondition->Name == "The Condition");
    CHECK(pCondition->MaxScore == 10);
}

TEST_CASE("Condition - Construct errors") {
    CHECK_THROWS_MATCHES(NS::Condition::Create("", static_cast<unsigned short>(10)), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("name"));
    CHECK_THROWS_MATCHES(NS::Condition::Create("The Condition", static_cast<unsigned short>(0)), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("maxScore"));
}

TEST_CASE("Condition - Move") {
    std::shared_ptr<NS::Condition>          p1(NS::Condition::Create("The Condition", static_cast<unsigned short>(10)));
    std::shared_ptr<NS::Condition>          p2(std::make_shared<NS::Condition>(std::move(*p1)));

    CHECK(p2->Name == "The Condition");
    CHECK(p2->MaxScore == 10);
}

TEST_CASE("Condition - Compare") {
    CHECK(CommonHelpers::TestHelpers::CompareTest(*NS::Condition::Create("0", static_cast<unsigned short>(10)), *NS::Condition::Create("1", static_cast<unsigned short>(10))) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(*NS::Condition::Create("0", static_cast<unsigned short>(10)), *NS::Condition::Create("0", static_cast<unsigned short>(100))) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(*NS::Condition::Create("0", static_cast<unsigned short>(10)), *NS::Condition::Create("0", static_cast<unsigned short>(10)), true) == 0);
}

TEST_CASE("Condition - Serialize") {
    CHECK(BoostHelpers::TestHelpers::SerializePtrTest(NS::Condition::Create("Condition", static_cast<unsigned short>(10))) == 0);
}

TEST_CASE("Result construction - bool ctor") {
    NS::Condition::Result::ConditionPtr const           pCondition(NS::Condition::Create("Condition", static_cast<unsigned short>(1)));

    SECTION("true - no reason") {
        NS::Condition::Result const         r(pCondition, true);

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(r.Ratio == 1.0f);
        CHECK(r.Reason.empty());
    }

    SECTION("true - with reason") {
        NS::Condition::Result const         r(pCondition, true, "The reason");

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(r.Ratio == 1.0f);
        CHECK(r.Reason == "The reason");
    }

    SECTION("false - no reason") {
        NS::Condition::Result const                     r(pCondition, false);

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful == false);
        CHECK(r.Ratio == 0.0f);
        CHECK(r.Reason.empty());
    }

    SECTION("false - with reason") {
        NS::Condition::Result const                     r(pCondition, false, "The reason");

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful == false);
        CHECK(r.Ratio == 0.0f);
        CHECK(r.Reason == "The reason");
    }
}

TEST_CASE("Result construction - ratio ctor") {
    NS::Condition::Result::ConditionPtr const           pCondition(NS::Condition::Create("Condition", static_cast<unsigned short>(1)));

    SECTION("1.0 - no reason") {
        NS::Condition::Result const         r(pCondition, 1.0f);

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(r.Ratio == 1.0f);
        CHECK(r.Reason.empty());
    }

    SECTION("1.0 - with reason") {
        NS::Condition::Result const         r(pCondition, 1.0f, "The reason");

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(r.Ratio == 1.0f);
        CHECK(r.Reason == "The reason");
    }

    SECTION("0.5 - no reason") {
        NS::Condition::Result const         r(pCondition, 0.5f);

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(r.Ratio == 0.5f);
        CHECK(r.Reason.empty());
    }

    SECTION("0.5 - with reason") {
        NS::Condition::Result const         r(pCondition, 0.5f, "The reason");

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(r.Ratio == 0.5f);
        CHECK(r.Reason == "The reason");
    }

    SECTION("0.1 - no reason") {
        NS::Condition::Result const         r(pCondition, 0.1f);

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(Approx(r.Ratio) == 0.1);
        CHECK(r.Reason.empty());
    }

    SECTION("0.1 - with reason") {
        NS::Condition::Result const         r(pCondition, 0.1f, "The reason");

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(Approx(r.Ratio) == 0.1);
        CHECK(r.Reason == "The reason");
    }

    SECTION("0.0 - no reason") {
        NS::Condition::Result const         r(pCondition, 0.0f);

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful == false);
        CHECK(r.Ratio == 0.0f);
        CHECK(r.Reason.empty());
    }

    SECTION("0.0 - with reason") {
        NS::Condition::Result const         r(pCondition, 0.0f, "The reason");

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful == false);
        CHECK(r.Ratio == 0.0f);
        CHECK(r.Reason == "The reason");
    }
}

TEST_CASE("Result construction - full ctor") {
    NS::Condition::Result::ConditionPtr const           pCondition(NS::Condition::Create("Condition", static_cast<unsigned short>(1)));

    SECTION("Successful with 0.0 ratio - no reason") {
        NS::Condition::Result const         r(pCondition, true, 0.0f);

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(r.Ratio == 0.0f);
        CHECK(r.Reason.empty());
    }

    SECTION("Successful with 0.0 ratio - with reason") {
        NS::Condition::Result const         r(pCondition, true, 0.0f, "The reason");

        CHECK(r.Condition.get() == pCondition.get());
        CHECK(r.IsSuccessful);
        CHECK(r.Ratio == 0.0f);
        CHECK(r.Reason == "The reason");
    }
}

TEST_CASE("Result construction - errors") {
    CHECK_THROWS_MATCHES(NS::Condition::Result(NS::Condition::Result::ConditionPtr(), true, 1.0f), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("pCondition"));

    NS::Condition::Result::ConditionPtr const           pCondition(NS::Condition::Create("Condition", static_cast<unsigned short>(1)));

    CHECK_THROWS_MATCHES(NS::Condition::Result(pCondition, false, -0.1f), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("ratio"));
    CHECK_THROWS_MATCHES(NS::Condition::Result(pCondition, false, 1.1f), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("ratio"));
    CHECK_THROWS_MATCHES(NS::Condition::Result(pCondition, true, 1.0f, std::string()), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("reason"));
}

TEST_CASE("Result - move") {
    NS::Condition::Result::ConditionPtr const           pCondition(NS::Condition::Create("Condition", static_cast<unsigned short>(1)));
    NS::Condition::Result                               r1(pCondition, true, 1.0f, "reason");
    NS::Condition::Result const                         r2(std::move(r1));

    CHECK(r2.Condition.get() == pCondition.get());
    CHECK(r2.IsSuccessful);
    CHECK(r2.Ratio == 1.0f);
    CHECK(r2.Reason == "reason");
}

TEST_CASE("Result - compare") {
    NS::Condition::Result::ConditionPtr const           pCondition1(NS::Condition::Create("1", static_cast<unsigned short>(1)));
    NS::Condition::Result::ConditionPtr const           pCondition2(NS::Condition::Create("2", static_cast<unsigned short>(1)));

    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Condition::Result(pCondition1, true), NS::Condition::Result(pCondition2, true)) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Condition::Result(pCondition1, false), NS::Condition::Result(pCondition1, true)) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Condition::Result(pCondition1, true, 0.1f), NS::Condition::Result(pCondition1, true, 0.2f)) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Condition::Result(pCondition1, true, 1.0f), NS::Condition::Result(pCondition1, true, 1.0f, "reason")) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Condition::Result(pCondition1, true, 1.0f, "0"), NS::Condition::Result(pCondition1, true, 1.0f, "1")) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Condition::Result(pCondition1, true, 1.0f, "0"), NS::Condition::Result(pCondition1, true, 1.0f, "0"), true) == 0);
}

TEST_CASE("Result - serialization") {
    NS::Condition::Result::ConditionPtr const           pCondition(NS::Condition::Create("Condition", static_cast<unsigned short>(1)));

    CHECK(BoostHelpers::TestHelpers::SerializeTest(NS::Condition::Result(pCondition, true, 1.0f, "reason")) == 0);
}
