/////////////////////////////////////////////////////////////////////////
///
///  \file          Request_UnitTest.cpp
///  \brief         Unit test for Request.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-23 10:03:38
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
#include "../Request.h"
#include <catch.hpp>

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::ConstrainedResource;

// ----------------------------------------------------------------------
// |
// |  Internal Types and Methods
// |
// ----------------------------------------------------------------------
class MyCondition : public NS::Condition {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    CREATE(MyCondition);

    template <typename PrivateConstructorTagT>
    MyCondition(PrivateConstructorTagT tag) :
        NS::Condition(tag, "MyCondition", 100)
    {}

    ~MyCondition(void) override = default;

#define ARGS                                BASES(NS::Condition)

    NON_COPYABLE(MyCondition);
    MOVE(MyCondition, ARGS);
    COMPARE(MyCondition, ARGS);
    SERIALIZATION(MyCondition, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_POLYMORPHIC(DecisionEngine::Core::Components::Condition)));
    SERIALIZATION_POLYMORPHIC_ADDITIONAL_VOID_CASTS(MyCondition, NS::Condition);

#undef ARGS

    Result Apply(Request const &, Resource const &) const override {
        return Result(SharedFromThis(), true);
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyCondition);

#if (defined __clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

static std::shared_ptr<MyCondition>         sg_pCondition(MyCondition::Create());

#if (defined __clang__)
#   pragma clang diagnostic pop
#endif

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
TEST_CASE("Construct - Simple") {
    NS::Request const                       request("MyRequest");

    CHECK(request.ToString() == "MyRequest");
    CHECK(!request.OptionalApplicabilityConditions);
    CHECK(!request.OptionalRequirementConditions);
    CHECK(!request.OptionalPreferenceConditions);
}

TEST_CASE("Construct - Applicability") {
    NS::Request const                       request("MyRequest2", std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }));

    CHECK(request.ToString() == "MyRequest2");
    REQUIRE(request.OptionalApplicabilityConditions);
    REQUIRE(request.OptionalApplicabilityConditions->size() == 1);
    REQUIRE((*request.OptionalApplicabilityConditions)[0].get() == sg_pCondition.get());
    CHECK(!request.OptionalRequirementConditions);
    CHECK(!request.OptionalPreferenceConditions);
}

TEST_CASE("Construct - Requirements") {
    NS::Request const                       request("MyRequest3", NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }));

    CHECK(request.ToString() == "MyRequest3");
    CHECK(!request.OptionalApplicabilityConditions);
    REQUIRE(request.OptionalRequirementConditions);
    REQUIRE(request.OptionalRequirementConditions->size() == 1);
    REQUIRE((*request.OptionalRequirementConditions)[0].get() == sg_pCondition.get());
    CHECK(!request.OptionalPreferenceConditions);
}

TEST_CASE("Construct - Preferences") {
    NS::Request const                       request("MyRequest4", NS::ConditionPtrsPtr(), NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }));

    CHECK(request.ToString() == "MyRequest4");
    CHECK(!request.OptionalApplicabilityConditions);
    CHECK(!request.OptionalRequirementConditions);
    REQUIRE(request.OptionalPreferenceConditions);
    REQUIRE(request.OptionalPreferenceConditions->size() == 1);
    REQUIRE((*request.OptionalPreferenceConditions)[0].get() == sg_pCondition.get());
}

TEST_CASE("Construct - Requirements and Preferences") {
    NS::Request const                       request(
        "MyRequest5",
        NS::ConditionPtrsPtr(),
        std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition, sg_pCondition }),
        std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition })
    );

    CHECK(request.ToString() == "MyRequest5");
    CHECK(!request.OptionalApplicabilityConditions);

    REQUIRE(request.OptionalRequirementConditions);
    REQUIRE(request.OptionalRequirementConditions->size() == 2);
    REQUIRE((*request.OptionalRequirementConditions)[0].get() == sg_pCondition.get());
    REQUIRE((*request.OptionalRequirementConditions)[1].get() == sg_pCondition.get());

    REQUIRE(request.OptionalPreferenceConditions);
    REQUIRE(request.OptionalPreferenceConditions->size() == 1);
    REQUIRE((*request.OptionalPreferenceConditions)[0].get() == sg_pCondition.get());
}

TEST_CASE("Construct - Errors") {
    // name
    CHECK_THROWS_MATCHES(NS::Request(""), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("name"));

    // optionalApplicabilityConditions
    CHECK_THROWS_MATCHES(
        NS::Request("Name", std::make_shared<NS::ConditionPtrs>()),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalApplicabilityConditions")
    );

    CHECK_THROWS_MATCHES(
        NS::Request("Name", std::make_shared<NS::ConditionPtrs>(std::vector<NS::ConditionPtr>(1))),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalApplicabilityConditions")
    );

    // optionalRequirementConditions
    CHECK_THROWS_MATCHES(
        NS::Request("Name", NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>()),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalRequirementConditions")
    );

    CHECK_THROWS_MATCHES(
        NS::Request("Name", NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>(std::vector<NS::ConditionPtr>(1))),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalRequirementConditions")
    );

    // optionalPreferenceConditions
    CHECK_THROWS_MATCHES(
        NS::Request("Name", NS::ConditionPtrsPtr(), NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>()),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalPreferenceConditions")
    );

    CHECK_THROWS_MATCHES(
        NS::Request("Name", NS::ConditionPtrsPtr(), NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>(std::vector<NS::ConditionPtr>(1))),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalPreferenceConditions")
    );
}

TEST_CASE("Compare") {
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Request("a"), NS::Request("a"), true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Request("a"), NS::Request("z")) == 0);

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Request("a"),
            NS::Request("a", std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }))
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Request("a"),
            NS::Request("a", NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }))
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Request("a"),
            NS::Request("a", NS::ConditionPtrsPtr(), NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }))
        ) == 0
    );
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::Request(
                "Name",
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition, sg_pCondition }),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition, sg_pCondition, sg_pCondition })
            ),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
