/////////////////////////////////////////////////////////////////////////
///
///  \file          Score_UnitTest.cpp
///  \brief         Unit test for Score.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-19 21:17:34
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
#include "../Score.h"
#include <catch.hpp>

#include "../Components.h"
#include "../Condition.h"

#include <CommonHelpers/Stl.h>
#include <CommonHelpers/TestHelpers.h>

#include <BoostHelpers/TestHelpers.h>

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

// ----------------------------------------------------------------------
// |
// |  Score::Result
// |
// ----------------------------------------------------------------------
TEST_CASE("Score::Result - Construct - No results") {
    NS::Score::Result const             result(
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == NS::MaxScore);
    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());
    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Applicable - (Success)") {
    NS::Score::Result const                 result(
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true)
        ),
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == NS::MaxScore);
    REQUIRE(result.ApplicabilityResults.size() == 1);
    CHECK(result.ApplicabilityResults[0].Condition == g_pCondition);
    CHECK(result.RequirementResults.empty());
    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Applicable - (Failure)") {
    NS::Score::Result const                 result(
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, false)
        ),
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable == false);
    CHECK(result.IsSuccessful == false);
    CHECK(Approx(result.Score) == 0.0f);
    REQUIRE(result.ApplicabilityResults.size() == 1);
    CHECK(result.ApplicabilityResults[0].Condition == g_pCondition);
    CHECK(result.RequirementResults.empty());
    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Applicable - (Success & Failure)") {
    NS::Score::Result const                 result(
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true),
            NS::Condition::Result(g_pCondition, false)
        ),
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable == false);
    CHECK(result.IsSuccessful == false);
    CHECK(Approx(result.Score) == 0.0f);

    REQUIRE(result.ApplicabilityResults.size() == 2);
    CHECK(result.ApplicabilityResults[0].Condition == g_pCondition);
    CHECK(result.ApplicabilityResults[0].IsSuccessful);
    CHECK(result.ApplicabilityResults[1].Condition == g_pCondition);
    CHECK(result.ApplicabilityResults[1].IsSuccessful == false);

    CHECK(result.RequirementResults.empty());
    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Applicable - (Failure & Success)") {
    NS::Score::Result const                 result(
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, false),
            NS::Condition::Result(g_pCondition, true)
        ),
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable == false);
    CHECK(result.IsSuccessful == false);
    CHECK(Approx(result.Score) == 0.0f);

    REQUIRE(result.ApplicabilityResults.size() == 2);
    CHECK(result.ApplicabilityResults[0].Condition == g_pCondition);
    CHECK(result.ApplicabilityResults[0].IsSuccessful == false);
    CHECK(result.ApplicabilityResults[1].Condition == g_pCondition);
    CHECK(result.ApplicabilityResults[1].IsSuccessful);

    CHECK(result.RequirementResults.empty());
    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Applicable - (Success & Success & Failure)") {
    NS::Score::Result const                 result(
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true),
            NS::Condition::Result(g_pCondition, true),
            NS::Condition::Result(g_pCondition, false)
        ),
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable == false);
    CHECK(result.IsSuccessful == false);
    CHECK(Approx(result.Score) == 0.0f);

    REQUIRE(result.ApplicabilityResults.size() == 3);
    CHECK(result.ApplicabilityResults[0].Condition == g_pCondition);
    CHECK(result.ApplicabilityResults[0].IsSuccessful);
    CHECK(result.ApplicabilityResults[1].Condition == g_pCondition);
    CHECK(result.ApplicabilityResults[1].IsSuccessful);
    CHECK(result.ApplicabilityResults[2].Condition == g_pCondition);
    CHECK(result.ApplicabilityResults[2].IsSuccessful == false);

    CHECK(result.RequirementResults.empty());
    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Requirement - (Success)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true)
        ),
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == NS::MaxScore);
    CHECK(result.ApplicabilityResults.empty());
    REQUIRE(result.RequirementResults.size() == 1);
    CHECK(result.RequirementResults[0].Condition == g_pCondition);
    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Requirement - (Success & Success)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true),
            NS::Condition::Result(g_pCondition, 0.5f)
        ),
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == 75001.0f);
    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 2);
    CHECK(result.RequirementResults[0].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 1.0f);
    CHECK(result.RequirementResults[1].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[1].Ratio) == 0.5f);

    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Requirement - (Failure)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, false)
        ),
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful == false);

    // Note that the score isn't 0 because we didn't see any preference
    // failures.
    CHECK(Approx(result.Score) == 1.0f);
    CHECK(result.ApplicabilityResults.empty());
    REQUIRE(result.RequirementResults.size() == 1);
    CHECK(result.RequirementResults[0].Condition == g_pCondition);
    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Requirement - (Success & Failure)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true),
            NS::Condition::Result(g_pCondition, false, 0.5f)
        ),
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful == false);
    CHECK(Approx(result.Score) == 75001.0f);
    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 2);
    CHECK(result.RequirementResults[0].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 1.0f);
    CHECK(result.RequirementResults[1].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[1].Ratio) == 0.5f);

    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Requirement - (Failure & Success)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, false),
            NS::Condition::Result(g_pCondition, true, 0.4f)
        ),
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful == false);
    CHECK(Approx(result.Score) == 20001.0f);
    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 2);
    CHECK(result.RequirementResults[0].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 0.0f);
    CHECK(result.RequirementResults[1].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[1].Ratio) == 0.4f);

    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Requirement - (Success & Success & Failure)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true, 0.4f),
            NS::Condition::Result(g_pCondition, true, 0.5f),
            NS::Condition::Result(g_pCondition, false)
        ),
        NS::Score::Result::ConditionResults{}
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful == false);
    CHECK(Approx(result.Score) == 30001.0f);
    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 3);
    CHECK(result.RequirementResults[0].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 0.4f);
    CHECK(result.RequirementResults[1].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[1].Ratio) == 0.5f);
    CHECK(result.RequirementResults[2].Condition == g_pCondition);
    CHECK(Approx(result.RequirementResults[2].Ratio) == 0.0f);

    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Score::Result - Construct - Preference - (Success)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true)
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == NS::MaxScore);
    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());

    REQUIRE(result.PreferenceResults.size() == 1);
    CHECK(result.PreferenceResults[0].Condition == g_pCondition);
}

TEST_CASE("Score::Result - Construct - Preference - (Success & Success)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true),
            NS::Condition::Result(g_pCondition, 0.5f)
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == 100000.75f);
    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());

    REQUIRE(result.PreferenceResults.size() == 2);
    CHECK(result.PreferenceResults[0].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 1.0f);
    CHECK(result.PreferenceResults[1].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[1].Ratio) == 0.5f);
}

TEST_CASE("Score::Result - Construct - Preference - (Failure)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, false)
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(result.RequirementResults.empty());

    CHECK(Approx(result.Score) == 100000.0f);
    REQUIRE(result.PreferenceResults.size() == 1);
    CHECK(result.PreferenceResults[0].Condition == g_pCondition);
}

TEST_CASE("Score::Result - Construct - Preference - (Success & Failure)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true),
            NS::Condition::Result(g_pCondition, false, 0.5f)
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == 100000.75f);
    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());

    REQUIRE(result.PreferenceResults.size() == 2);
    CHECK(result.PreferenceResults[0].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 1.0f);
    CHECK(result.PreferenceResults[1].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[1].Ratio) == 0.5f);
}

TEST_CASE("Score::Result - Construct - Preference - (Failure & Success)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, false),
            NS::Condition::Result(g_pCondition, true, 0.4f)
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == 100000.203125f);
    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());

    REQUIRE(result.PreferenceResults.size() == 2);
    CHECK(result.PreferenceResults[0].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 0.0f);
    CHECK(result.PreferenceResults[1].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[1].Ratio) == 0.4f);
}

TEST_CASE("Score::Result - Construct - Preference - (Success & Success & Failure)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true, 0.4f),
            NS::Condition::Result(g_pCondition, true, 0.5f),
            NS::Condition::Result(g_pCondition, false)
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == 100000.296875f);
    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());

    REQUIRE(result.PreferenceResults.size() == 3);
    CHECK(result.PreferenceResults[0].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 0.4f);
    CHECK(result.PreferenceResults[1].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[1].Ratio) == 0.5f);
    CHECK(result.PreferenceResults[2].Condition == g_pCondition);
    CHECK(Approx(result.PreferenceResults[2].Ratio) == 0.0f);
}

TEST_CASE("Score::Result - Construct - Requirement & Preference - (Success)") {
    NS::Score::Result const                 result(
        NS::Score::Result::ConditionResults{},
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true, 0.5f)
        ),
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, true, 0.4f)
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);
    CHECK(Approx(result.Score) == 50000.3984375f);
    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 1);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 0.5f);

    REQUIRE(result.PreferenceResults.size() == 1);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 0.4f);
}

TEST_CASE("Score::Result - Compare") {
    // Same
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::Result(
                NS::Score::Result::ConditionResults{},
                NS::Score::Result::ConditionResults{},
                NS::Score::Result::ConditionResults{}
            ),
            NS::Score::Result(
                NS::Score::Result::ConditionResults{},
                NS::Score::Result::ConditionResults{},
                NS::Score::Result::ConditionResults{}
            ),
            true
        ) == 0
    );

    // Unsuccessful vs. Success
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::Result(
                NS::Score::Result::ConditionResults{},
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, false)
                ),
                NS::Score::Result::ConditionResults{}
            ),
            NS::Score::Result(
                NS::Score::Result::ConditionResults{},
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, true)
                ),
                NS::Score::Result::ConditionResults{}
            )
        ) == 0
    );

    // Score (requirement)
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::Result(
                NS::Score::Result::ConditionResults{},
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, true, 0.5f)
                ),
                NS::Score::Result::ConditionResults{}
            ),
            NS::Score::Result(
                NS::Score::Result::ConditionResults{},
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, true)
                ),
                NS::Score::Result::ConditionResults{}
            )
        ) == 0
    );

    // Score (preference)
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::Result(
                NS::Score::Result::ConditionResults{},
                NS::Score::Result::ConditionResults{},
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, false)
                )
            ),
            NS::Score::Result(
                NS::Score::Result::ConditionResults{},
                NS::Score::Result::ConditionResults{},
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, true)
                )
            )
        ) == 0
    );
}

TEST_CASE("Score::Result - Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::Score::Result(
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, true, 0.1f)
                ),
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, true, 0.2f)
                ),
                CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
                    NS::Condition::Result(g_pCondition, true, 0.3f)
                )
            ),
            [](std::string const &output) {
                UNSCOPED_INFO(output);

                // Required to display the content
                CHECK(true);
            }
        ) == 0
    );
}

NS::Score::Result CreateResult(
    bool isSuccessful,
    std::optional<float> ratio=std::nullopt,
    bool isApplicable=true
) {
    NS::Condition::Result                   requirement(
        [&isSuccessful, &ratio](void) {
            if(ratio)
                return NS::Condition::Result(g_pCondition, isSuccessful, *ratio);

            return NS::Condition::Result(g_pCondition, isSuccessful);
        }()
    );

    return NS::Score::Result(
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            NS::Condition::Result(g_pCondition, isApplicable)
        ),
        CommonHelpers::Stl::CreateVector<NS::Condition::Result>(
            std::move(requirement)
        ),
        NS::Score::Result::ConditionResults{}
    );
}

NS::Score::ResultGroup::ResultPtr CreateResultPtr(
    bool isSuccessful,
    std::optional<float> ratio=std::nullopt,
    bool isApplicable=true
) {
    return std::make_shared<NS::Score::Result>(CreateResult(isSuccessful, ratio, isApplicable));
}

TEST_CASE("Score::ResultGroup - Construct - (Errors)") {
    CHECK_THROWS_MATCHES(NS::Score::ResultGroup(NS::Score::ResultGroup::ResultPtrs{}), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("results"));
    CHECK_THROWS_MATCHES(NS::Score::ResultGroup(CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(NS::Score::ResultGroup::ResultPtr())), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("results"));
    CHECK_THROWS_MATCHES(NS::Score::ResultGroup(CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(CreateResultPtr(true)), true, -1.0f, 0, 0), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("score"));
    CHECK_THROWS_MATCHES(NS::Score::ResultGroup(CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(CreateResultPtr(true)), true, NS::MaxScore + 1.0f, 0, 0), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("score"));
}

TEST_CASE("Score::ResultGroup - Construct - (Success)") {
    NS::Score::ResultGroup const            group(
        CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
            CreateResultPtr(true)
        )
    );

    CHECK(group.IsSuccessful);
    CHECK(Approx(group.AverageScore) == 100001.0f);
    CHECK(group.NumResults == 1);
    CHECK(group.NumFailures == 0);
    REQUIRE(group.Results.size() == 1);
    CHECK(group.Results[0]->IsSuccessful);
}

TEST_CASE("Score::ResultGroup - Construct - (Success & Success)") {
    NS::Score::ResultGroup const            group(
        CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
            CreateResultPtr(true),
            CreateResultPtr(true, 0.5f)
        )
    );

    CHECK(group.IsSuccessful);
    CHECK(Approx(group.AverageScore) == 75001.0f);
    CHECK(group.NumResults == 2);
    CHECK(group.NumFailures == 0);
    REQUIRE(group.Results.size() == 2);
    CHECK(group.Results[0]->IsSuccessful);
    CHECK(Approx(group.Results[0]->Score) == 100001.0f);
    CHECK(group.Results[1]->IsSuccessful);
    CHECK(Approx(group.Results[1]->Score) == 50001.0f);
}

TEST_CASE("Score::ResultGroup - Construct - (Success & Not Applicable & Success)") {
    NS::Score::ResultGroup const            group(
        CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
            CreateResultPtr(true),
            CreateResultPtr(false, 0.0f, false),
            CreateResultPtr(true, 0.5f)
        )
    );

    CHECK(group.IsSuccessful);
    CHECK(Approx(group.AverageScore) == 75001.0f);
    CHECK(group.NumResults == 2);
    CHECK(group.NumFailures == 0);

    REQUIRE(group.Results.size() == 3);

    CHECK(group.Results[0]->IsApplicable);
    CHECK(group.Results[0]->IsSuccessful);
    CHECK(Approx(group.Results[0]->Score) == 100001.0f);

    CHECK(group.Results[1]->IsApplicable == false);
    CHECK(Approx(group.Results[1]->Score) == 0.0f);

    CHECK(group.Results[2]->IsApplicable);
    CHECK(group.Results[2]->IsSuccessful);
    CHECK(Approx(group.Results[2]->Score) == 50001.0f);
}

TEST_CASE("Score::ResultGroup - Construct - (Not Applicable & Not Applicable)") {
    NS::Score::ResultGroup const            group(
        CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
            CreateResultPtr(true, 1.0f, false),
            CreateResultPtr(false, 0.0f, false)
        )
    );

    CHECK(group.IsSuccessful);
    CHECK(Approx(group.AverageScore) == 0.0f);
    CHECK(group.NumResults == 0);
    CHECK(group.NumFailures == 0);

    REQUIRE(group.Results.size() == 2);

    CHECK(group.Results[0]->IsApplicable == false);
    CHECK(group.Results[0]->IsSuccessful == false);

    CHECK(group.Results[1]->IsApplicable == false);
    CHECK(group.Results[0]->IsSuccessful == false);
}

TEST_CASE("Score::ResultGroup - Construct - (Failure)") {
    NS::Score::ResultGroup const            group(
        CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
            CreateResultPtr(false)
        )
    );

    CHECK(group.IsSuccessful == false);

    // This is 1.0 because there aren't any preference failures
    CHECK(Approx(group.AverageScore) == 1.0f);
    CHECK(group.NumResults == 1);
    CHECK(group.NumFailures == 1);
    REQUIRE(group.Results.size() == 1);
    CHECK(group.Results[0]->IsSuccessful == false);
}

TEST_CASE("Score::ResultGroup - Construct - (Success & Failure)") {
    NS::Score::ResultGroup const            group(
        CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
            CreateResultPtr(true),
            CreateResultPtr(false)
        )
    );

    CHECK(group.IsSuccessful == false);

    // This is 1.0 because there aren't any preference failures
    CHECK(Approx(group.AverageScore) == 50001.0f);
    CHECK(group.NumResults == 2);
    CHECK(group.NumFailures == 1);

    REQUIRE(group.Results.size() == 2);
    CHECK(group.Results[0]->IsSuccessful);
    CHECK(group.Results[1]->IsSuccessful == false);
}

TEST_CASE("Score::ResultGroup - Construct - (Failure & Success)") {
    NS::Score::ResultGroup const            group(
        CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
            CreateResultPtr(false),
            CreateResultPtr(true, 0.5f)
        )
    );

    CHECK(group.IsSuccessful == false);

    // This is 1.0 because there aren't any preference failures
    CHECK(Approx(group.AverageScore) == 25001.0f);
    CHECK(group.NumResults == 2);
    CHECK(group.NumFailures == 1);

    REQUIRE(group.Results.size() == 2);
    CHECK(group.Results[0]->IsSuccessful == false);
    CHECK(group.Results[1]->IsSuccessful);
}

TEST_CASE("Score::ResultGroup - Compare") {
    // Same
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true)
                )
            ),
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true)
                )
            ),
            true
        ) == 0
    );

    // Success vs. Unsuccessful
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true),
                    CreateResultPtr(true),
                    CreateResultPtr(true),
                    CreateResultPtr(true),
                    CreateResultPtr(true),
                    CreateResultPtr(false)
                )
            ),
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true)
                )
            )
        ) == 0
    );

    // More failures vs. less failures
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true),
                    CreateResultPtr(true),
                    CreateResultPtr(true),
                    CreateResultPtr(false),
                    CreateResultPtr(true),
                    CreateResultPtr(true),
                    CreateResultPtr(false)
                )
            ),
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(false)
                )
            )
        ) == 0
    );

    // Different Averages
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true),
                    CreateResultPtr(true, 0.5f)
                )
            ),
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true)
                )
            )
        ) == 0
    );

    // Different num results (good average)
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true)
                )
            ),
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true),
                    CreateResultPtr(true)
                )
            )
        ) == 0
    );

    // Different num results (bad average)
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true, 0.2f),
                    CreateResultPtr(true, 0.2f)
                )
            ),
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true, 0.2f)
                )
            )
        ) == 0
    );
}

TEST_CASE("Score::ResultGroup - Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::Score::ResultGroup(
                CommonHelpers::Stl::CreateVector<NS::Score::ResultGroup::ResultPtr>(
                    CreateResultPtr(true, 0.2f),
                    CreateResultPtr(false, 0.3f)
                )
            ),
            [](std::string const &value) {
                UNSCOPED_INFO(value);

                // Ensure the value is displayed
                CHECK(true);
            }
        ) == 0
    );
}

TEST_CASE("Score - Construct - Empty") {
    NS::Score const                         score;

    CHECK(score.IsSuccessful);
    CHECK(score.HasSuffix() == false);
}

TEST_CASE("Score - Construct - Errors") {
    CHECK_THROWS_MATCHES(
        NS::Score(
            NS::Score(
                NS::Score(),
                CreateResult(true),
                false
            ),
            CreateResult(true),
            false
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("score")
    );

    CHECK_THROWS_MATCHES(
        NS::Score(
            NS::Score(
                NS::Score(),
                NS::Condition::Result(g_pCondition, true),
                false
            ),
            NS::Condition::Result(g_pCondition, true),
            false
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("score")
    );
}

TEST_CASE("Score - Construct - Single - (Success)") {
    NS::Score const                         score(NS::Score(), NS::Condition::Result(g_pCondition, true), false);

    CHECK(score.IsSuccessful);
    CHECK(score.HasSuffix());
}

TEST_CASE("Score - Construct - Single - (Failure)") {
    NS::Score const                         score(NS::Score(), NS::Condition::Result(g_pCondition, false), false);

    CHECK(score.IsSuccessful == false);
    CHECK(score.HasSuffix());
}

NS::Score CreateResultGroup(NS::Score score, std::vector<bool> results) {
    size_t                                  remaining(results.size());

    for(bool result : results) {
        score = NS::Score(score, NS::Condition::Result(g_pCondition, result), --remaining == 0).Commit();
    }

    return score;
}

TEST_CASE("Score - Construct - Group/Single - (Success & Success)") {
    NS::Score const                         score(
        CreateResultGroup(
            NS::Score(),
            std::vector<bool>{ true, true }
        ),
        NS::Condition::Result(g_pCondition, true),
        false
    );

    CHECK(score.IsSuccessful);
    CHECK(score.HasSuffix());
}

TEST_CASE("Score - Construct - Group/Single - (Success & Failure)") {
    NS::Score const                         score(
        CreateResultGroup(
            NS::Score(),
            std::vector<bool>{ true, true }
        ),
        NS::Condition::Result(g_pCondition, false),
        false
    );

    CHECK(score.IsSuccessful == false);
    CHECK(score.HasSuffix());
}

TEST_CASE("Score - Construct - Group/Single - (Failure & Success)") {
    NS::Score const                         score(
        CreateResultGroup(
            NS::Score(),
            std::vector<bool>{ false }
        ),
        NS::Condition::Result(g_pCondition, true),
        false
    );

    CHECK(score.IsSuccessful == false);
    CHECK(score.HasSuffix());
}

TEST_CASE("Score - Construct - Group/Single - (Failure & Failure)") {
    NS::Score const                         score(
        CreateResultGroup(
            NS::Score(),
            std::vector<bool>{ false }
        ),
        NS::Condition::Result(g_pCondition, false),
        false
    );

    CHECK(score.IsSuccessful == false);
    CHECK(score.HasSuffix());
}

TEST_CASE("Score - Enumeration") {
    // ----------------------------------------------------------------------
    using ResultGroups                      = std::vector<NS::Score::ResultGroup const *>;
    using Results                           = std::vector<NS::Score::Result const *>;
    // ----------------------------------------------------------------------

    ResultGroups                            groups;
    Results                                 results;

    size_t                                  maxNumGroups(std::numeric_limits<size_t>::max());
    size_t                                  maxNumResults(std::numeric_limits<size_t>::max());

    auto const                              groupsFunc(
        [&groups, &maxNumGroups](NS::Score::ResultGroup const &group) -> bool {
            groups.emplace_back(&group);
            return groups.size() != maxNumGroups;
        }
    );

    auto const                              resultsFunc(
        [&results, &maxNumResults](NS::Score::Result const &result) -> bool {
            results.emplace_back(&result);
            return results.size() != maxNumResults;
        }
    );

    SECTION("Empty") {
        NS::Score const                     score;

        CHECK(score.EnumResultGroups(groupsFunc));
        CHECK(groups.empty());

        CHECK(score.EnumResults(resultsFunc));
        CHECK(results.empty());

        CHECK(score.EnumAllResults(resultsFunc));
        CHECK(results.empty());
    }

    SECTION("Single Result") {
        NS::Score const                     score(
            NS::Score(),
            NS::Condition::Result(g_pCondition, true),
            false
        );

        CHECK(score.EnumResultGroups(groupsFunc));
        CHECK(groups.empty());

        SECTION("Results") {
            CHECK(score.EnumResults(resultsFunc));
            CHECK(results.size() == 1);
        }

        SECTION("All Results") {
            CHECK(score.EnumAllResults(resultsFunc));
            CHECK(results.size() == 1);
        }
    }

    SECTION("Single Group") {
        NS::Score const                     score(
            CreateResultGroup(
                NS::Score(),
                std::vector<bool>{ true, false }
            )
        );

        CHECK(score.EnumResultGroups(groupsFunc));
        REQUIRE(groups.size() == 1);
        CHECK(groups[0]->Results.size() == 2);

        CHECK(score.EnumResults(resultsFunc));
        CHECK(results.empty());

        SECTION("Standard") {
            CHECK(score.EnumAllResults(resultsFunc));
            REQUIRE(results.size() == 2);
            CHECK(results[0]->IsSuccessful);
            CHECK(results[1]->IsSuccessful == false);
        }

        SECTION("Early Termination") {
            maxNumResults = 1;

            CHECK(score.EnumAllResults(resultsFunc) == false);
            REQUIRE(results.size() == 1);
            CHECK(results[0]->IsSuccessful);
        }
    }

    SECTION("Single Group, Single Result") {
        NS::Score const                     score(
            CreateResultGroup(
                NS::Score(),
                std::vector<bool>{ true, false }
            ),
            NS::Condition::Result(g_pCondition, true),
            false
        );

        CHECK(score.EnumResultGroups(groupsFunc));
        REQUIRE(groups.size() == 1);
        CHECK(groups[0]->Results.size() == 2);

        SECTION("EnumResults") {
            CHECK(score.EnumResults(resultsFunc));
            REQUIRE(results.size() == 1);
            CHECK(results[0]->IsSuccessful);
        }

        SECTION("EnumAllResults") {
            SECTION("Standard") {
                CHECK(score.EnumAllResults(resultsFunc));
                REQUIRE(results.size() == 3);
                CHECK(results[0]->IsSuccessful);
                CHECK(results[1]->IsSuccessful == false);
                CHECK(results[2]->IsSuccessful);
            }

            SECTION("Early Termination") {
                maxNumResults = 2;

                CHECK(score.EnumAllResults(resultsFunc) == false);
                REQUIRE(results.size() == 2);
                CHECK(results[0]->IsSuccessful);
                CHECK(results[1]->IsSuccessful == false);
            }
        }
    }

    SECTION("Groups") {
        NS::Score const                     score(
            CreateResultGroup(
                CreateResultGroup(
                    NS::Score(),
                    std::vector<bool>{ true, true, false }
                ),
                std::vector<bool>{ false, true }
            )
        );

        SECTION("EnumResultGroups") {
            SECTION("Standard") {
                CHECK(score.EnumResultGroups(groupsFunc));
                REQUIRE(groups.size() == 2);
                CHECK(groups[0]->Results.size() == 3);
                CHECK(groups[1]->Results.size() == 2);
            }

            SECTION("Early Terminator") {
                maxNumGroups = 1;

                CHECK(score.EnumResultGroups(groupsFunc) == false);
                REQUIRE(groups.size() == 1);
                CHECK(groups[0]->Results.size() == 3);
            }
        }

        CHECK(score.EnumResults(resultsFunc));
        CHECK(results.empty());

        SECTION("EnumAllResults") {
            SECTION("Standard") {
                CHECK(score.EnumAllResults(resultsFunc));
                REQUIRE(results.size() == 5);
                CHECK(results[0]->IsSuccessful);
                CHECK(results[1]->IsSuccessful);
                CHECK(results[2]->IsSuccessful == false);
                CHECK(results[3]->IsSuccessful == false);
                CHECK(results[4]->IsSuccessful);
            }

            SECTION("Early Termination") {
                maxNumResults = 4;

                CHECK(score.EnumAllResults(resultsFunc) == false);
                REQUIRE(results.size() == 4);
                CHECK(results[0]->IsSuccessful);
                CHECK(results[1]->IsSuccessful);
                CHECK(results[2]->IsSuccessful == false);
                CHECK(results[3]->IsSuccessful == false);
            }
        }
    }
}

TEST_CASE("Score - Compare - Single Items") {
    NS::Score const                         result(
        NS::Score(),
        NS::Condition::Result(g_pCondition, true),
        false
    );

    NS::Score const                         group(
        CreateResultGroup(
            NS::Score(),
            std::vector<bool>{ true }
        )
    );

    CHECK(CommonHelpers::TestHelpers::CompareTest(result, result, true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(group, group, true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(result, group) == 0);
}

TEST_CASE("Score - Compare - Not Equal") {
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score(
                NS::Score(),
                NS::Condition::Result(g_pCondition, false),
                false
            ),
            CreateResultGroup(
                NS::Score(),
                std::vector<bool>{ true }
            )
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            CreateResultGroup(
                NS::Score(),
                std::vector<bool>{ false }
            ),
            NS::Score(
                NS::Score(),
                NS::Condition::Result(g_pCondition, true),
                false
            )
        ) == 0
    );
}

TEST_CASE("Score - Compare - More Groups") {
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::Score(
                NS::Score(),
                NS::Condition::Result(g_pCondition, true),
                false
            ),
            CreateResultGroup(
                CreateResultGroup(
                    NS::Score(),
                    std::vector<bool>{ true }
                ),
                std::vector<bool>{ true }
            )
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            CreateResultGroup(
                CreateResultGroup(
                    NS::Score(),
                    std::vector<bool>{ true }
                ),
                std::vector<bool>{ false }
            ),
            NS::Score(
                NS::Score(),
                NS::Condition::Result(g_pCondition, true),
                false
            )
        ) == 0
    );
}

TEST_CASE("Score - Commit") {
    // ----------------------------------------------------------------------
    using ResultGroups                      = std::vector<NS::Score::ResultGroup const *>;
    // ----------------------------------------------------------------------

    CHECK_THROWS_MATCHES(NS::Score().Commit(), std::logic_error, Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid operation"));

    ResultGroups                            groups;
    auto const                              func(
        [&groups](NS::Score::ResultGroup const &group) {
            groups.emplace_back(&group);
            return true;
        }
    );

    NS::Score                               score(
        NS::Score(),
        NS::Condition::Result(g_pCondition, true),
        true
    );

    SECTION("Before Commit") {
        CHECK(score.EnumResultGroups(func));
        CHECK(groups.empty());
    }

    SECTION("After Commit") {
        NS::Score const                     newScore(score.Commit());

        CHECK(newScore.EnumResultGroups(func));
        REQUIRE(groups.size() == 1);
        CHECK(groups[0]->Results.size() == 1);
    }
}

TEST_CASE("Score - Copy") {
    NS::Score().Copy();

    CHECK_THROWS_MATCHES(NS::Score(NS::Score(), NS::Condition::Result(g_pCondition, true), false).Copy(), std::logic_error, Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid operation"));
}

TEST_CASE("Score - Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            CreateResultGroup(
                CreateResultGroup(
                    NS::Score(),
                    std::vector<bool>{ true, true, false }
                ),
                std::vector<bool>{ false, true }
            ),
            [](std::string const &value) {
                UNSCOPED_INFO(value);
                CHECK(true);
            }
        ) == 0
    );
}
