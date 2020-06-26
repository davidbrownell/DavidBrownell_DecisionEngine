/////////////////////////////////////////////////////////////////////////
///
///  \file          Resource_UnitTest.cpp
///  \brief         Unit test for Resource.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-23 22:27:57
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
#include "../Resource.h"
#include <catch.hpp>

#include "../Request.h"

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::ConstrainedResource;
namespace Components                        = DecisionEngine::Core::Components;

// ----------------------------------------------------------------------
// |
// |  Internal Types and Methods
// |
// ----------------------------------------------------------------------
class MyCondition : public NS::Condition {
private:
    // ----------------------------------------------------------------------
    // |  Private Data (used in public declarations)
    bool const                              _result;

public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    CREATE(MyCondition);

    template <typename PrivateConstructorTagT>
    MyCondition(PrivateConstructorTagT tag, bool result) :
        NS::Condition(tag, "MyCondition", 100),
        _result(result)
    {}

    ~MyCondition(void) override = default;

#define ARGS                                MEMBERS(_result), BASES(NS::Condition)

    NON_COPYABLE(MyCondition);
    MOVE(MyCondition, ARGS);
    COMPARE(MyCondition, ARGS);
    SERIALIZATION(MyCondition, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_POLYMORPHIC(Components::Condition)));
    SERIALIZATION_POLYMORPHIC_ADDITIONAL_VOID_CASTS(MyCondition, NS::Condition);

#undef ARGS

    Result Apply(Request const &, Resource const &) const override {
        return Result(SharedFromThis(), _result);
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyCondition);

#if (defined __clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

static NS::ConditionPtr const               sg_pCondition(MyCondition::Create(true));

#if (defined __clang__)
#   pragma clang diagnostic pop
#endif

Components::Score::Result CreateScoreResult(bool value) {
    return Components::Score::Result(
        Components::Score::Result::ConditionResults(),
        CommonHelpers::Stl::CreateVector<Components::Score::Result::ConditionResults::value_type>(Components::Condition::Result(sg_pCondition, value)),
        Components::Score::Result::ConditionResults()
    );
}

NS::Resource::Evaluations CreateEvaluations(NS::Resource const &resource, std::vector<bool> const &values) {
    NS::Resource::Evaluations               results;

    results.reserve(values.size());

    for(bool value : values) {
        results.emplace_back(
            NS::Resource::Evaluation(
                CreateScoreResult(value),
                std::make_shared<NS::Resource::State>(resource)
            )
        );
    }

    return results;
}

class MyResource : public NS::Resource {
public:
    // ----------------------------------------------------------------------
    // |  Public Types
    enum class OperationType {
        Valid,
        EvaluateEmpty,
        EvaluateTooMany,
        ApplyInvalid
    };

private:
    // ----------------------------------------------------------------------
    // |  Private Data (used in public declarations)
    OperationType const                     _operation;

public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    CREATE(MyResource);

    template <typename PrivateContructorTagT>
    MyResource(
        PrivateContructorTagT tag,
        OperationType operation,
        std::string name,
        ConditionPtrsPtr optionalApplicabilityConditions=ConditionPtrsPtr(),
        ConditionPtrsPtr optionalRequirementConditions=ConditionPtrsPtr(),
        ConditionPtrsPtr optionalPreferenceConditions=ConditionPtrsPtr()
    ) :
        NS::Resource(tag, std::move(name), std::move(optionalApplicabilityConditions), std::move(optionalRequirementConditions), std::move(optionalPreferenceConditions)),
        _operation(operation)
    {}

    template <typename PrivateConstructorTagT>
    MyResource(
        PrivateConstructorTagT tag,
        MyResource const &other
    ) :
        NS::Resource(tag, other),
        _operation(other._operation)
    {}

    ~MyResource(void) override = default;

#define ARGS                                MEMBERS(_operation), BASES(NS::Resource)

    NON_COPYABLE(MyResource);
    MOVE(MyResource, ARGS);
    COMPARE(MyResource, ARGS);
    SERIALIZATION(MyResource, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_POLYMORPHIC(NS::Resource)));

#undef ARGS

    using NS::Resource::CalculateResult;

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    EvaluateResult EvaluateImpl(Request const &, size_t maxNumEvaluations) const override {
        if(_operation == OperationType::Valid || _operation == OperationType::ApplyInvalid)
            return EvaluateResult(
                CreateEvaluations(*this, { true, false }),
                std::make_shared<State>(*this)
            );

        if(_operation == OperationType::EvaluateEmpty)
            return EvaluateResult(
                Evaluations(),
                std::make_shared<State>(*this)
            );

        else if(_operation == OperationType::EvaluateTooMany) {
            std::vector<bool>               values;

            values.resize(maxNumEvaluations + 1);

            return EvaluateResult(
                CreateEvaluations(*this, values),
                std::make_shared<State>(*this)
            );
        }

        assert(!"Invalid OperationType");
        return EvaluateResult(Evaluations(), ContinuationStatePtr());
    }

    EvaluateResult EvaluateImpl(Request const &request, size_t maxNumEvaluations, State &) const override {
        return EvaluateImpl(request, maxNumEvaluations);
    }

    ResourcePtr ApplyImpl(State const &) const override {
        if(_operation == OperationType::ApplyInvalid)
            return ResourcePtr();

        return MyResource::Create(*this);
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyResource);

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
TEST_CASE("Resource - Construct - Simple") {
    std::shared_ptr<MyResource> const       resource(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "MyResource"
        )
    );

    CHECK(resource->Name == "MyResource");
    CHECK(!resource->OptionalApplicabilityConditions);
    CHECK(!resource->OptionalRequirementConditions);
    CHECK(!resource->OptionalPreferenceConditions);
}

TEST_CASE("Resource - Construct - Applicability") {
    std::shared_ptr<MyResource> const       resource(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "MyResource",
            std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition })
        )
    );

    CHECK(resource->Name == "MyResource");
    REQUIRE(resource->OptionalApplicabilityConditions);
    REQUIRE(resource->OptionalApplicabilityConditions->size() == 1);
    REQUIRE((*resource->OptionalApplicabilityConditions)[0].get() == sg_pCondition.get());

    CHECK(!resource->OptionalRequirementConditions);
    CHECK(!resource->OptionalPreferenceConditions);
}

TEST_CASE("Resource - Construct - Requirements") {
    std::shared_ptr<MyResource> const       resource(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "MyResource",
            NS::ConditionPtrsPtr(),
            std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition })
        )
    );

    CHECK(resource->Name == "MyResource");
    CHECK(!resource->OptionalApplicabilityConditions);

    REQUIRE(resource->OptionalRequirementConditions);
    REQUIRE(resource->OptionalRequirementConditions->size() == 1);
    REQUIRE((*resource->OptionalRequirementConditions)[0].get() == sg_pCondition.get());

    CHECK(!resource->OptionalPreferenceConditions);
}

TEST_CASE("Resource - Construct - Preferences") {
    std::shared_ptr<MyResource> const       resource(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "MyResource",
            NS::ConditionPtrsPtr(),
            NS::ConditionPtrsPtr(),
            std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition })
        )
    );

    CHECK(resource->Name == "MyResource");
    CHECK(!resource->OptionalApplicabilityConditions);
    CHECK(!resource->OptionalRequirementConditions);

    REQUIRE(resource->OptionalPreferenceConditions);
    REQUIRE(resource->OptionalPreferenceConditions->size() == 1);
    REQUIRE((*resource->OptionalPreferenceConditions)[0].get() == sg_pCondition.get());
}

TEST_CASE("Resource - Construct - Requirements and Preferences") {
    std::shared_ptr<MyResource> const       resource(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "MyResource",
            NS::ConditionPtrsPtr(),
            std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition, sg_pCondition }),
            std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition })
        )
    );

    CHECK(resource->Name == "MyResource");
    CHECK(!resource->OptionalApplicabilityConditions);

    REQUIRE(resource->OptionalRequirementConditions);
    REQUIRE(resource->OptionalRequirementConditions->size() == 2);
    REQUIRE((*resource->OptionalRequirementConditions)[0].get() == sg_pCondition.get());
    REQUIRE((*resource->OptionalRequirementConditions)[1].get() == sg_pCondition.get());

    REQUIRE(resource->OptionalPreferenceConditions);
    REQUIRE(resource->OptionalPreferenceConditions->size() == 1);
    REQUIRE((*resource->OptionalPreferenceConditions)[0].get() == sg_pCondition.get());
}

TEST_CASE("Resource - Construct - Errors") {
    // name
    CHECK_THROWS_MATCHES(
        MyResource::Create(
            MyResource::OperationType::Valid,
            ""
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("name")
    );

    // optionalApplicabilityConditions
    CHECK_THROWS_MATCHES(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "Name",
            std::make_shared<NS::ConditionPtrs>()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalApplicabilityConditions")
    );

    CHECK_THROWS_MATCHES(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "Name",
            std::make_shared<NS::ConditionPtrs>(std::vector<NS::ConditionPtr>(1))
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalApplicabilityConditions")
    );

    // optionalRequirementConditions
    CHECK_THROWS_MATCHES(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "Name",
            NS::ConditionPtrsPtr(),
            std::make_shared<NS::ConditionPtrs>()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalRequirementConditions")
    );

    CHECK_THROWS_MATCHES(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "Name",
            NS::ConditionPtrsPtr(),
            std::make_shared<NS::ConditionPtrs>(std::vector<NS::ConditionPtr>(1))
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalRequirementConditions")
    );

    // optionalPreferenceConditions
    CHECK_THROWS_MATCHES(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "Name",
            NS::ConditionPtrsPtr(),
            NS::ConditionPtrsPtr(),
            std::make_shared<NS::ConditionPtrs>()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalPreferenceConditions")
    );

    CHECK_THROWS_MATCHES(
        MyResource::Create(
            MyResource::OperationType::Valid,
            "Name",
            NS::ConditionPtrsPtr(),
            NS::ConditionPtrsPtr(),
            std::make_shared<NS::ConditionPtrs>(std::vector<NS::ConditionPtr>(1))
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("optionalPreferenceConditions")
    );
}

TEST_CASE("Resource - Eval") {
    NS::Request const                       request("MyRequest");

    SECTION("Valid") {
        std::shared_ptr<MyResource> const   resource(MyResource::Create(MyResource::OperationType::Valid, "MyResource"));

        // Standard
        resource->Evaluate(request, 10);

        // Invalid number of evaluations
        CHECK_THROWS_MATCHES(
            resource->Evaluate(request, 0),
            std::invalid_argument,
            Catch::Matchers::Exception::ExceptionMessageMatcher("maxNumEvaluations")
        );
    }

    CHECK_THROWS_MATCHES(
        MyResource::Create(MyResource::OperationType::EvaluateEmpty, "MyResource")->Evaluate(request, 1),
        std::runtime_error,
        Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid Evaluations")
    );

    CHECK_THROWS_MATCHES(
        MyResource::Create(MyResource::OperationType::EvaluateTooMany, "MyResource")->Evaluate(request, 1),
        std::runtime_error,
        Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid Evaluations")
    );
}

TEST_CASE("Resource - Eval with state") {
    NS::Request const                       request("MyRequest");

    SECTION("Valid") {
        std::shared_ptr<MyResource> const   resource(MyResource::Create(MyResource::OperationType::Valid, "MyResource"));
        NS::Resource::State                 state(*resource);

        // Standard
        resource->Evaluate(request, 10, state);

        // Invalid number of evaluations
        CHECK_THROWS_MATCHES(
            resource->Evaluate(request, 0, state),
            std::invalid_argument,
            Catch::Matchers::Exception::ExceptionMessageMatcher("maxNumEvaluations")
        );

        NS::Resource::State             wrongState(*MyResource::Create(MyResource::OperationType::Valid, "DifferentResource"));

        CHECK_THROWS_MATCHES(
            resource->Evaluate(request, 1, wrongState),
            std::invalid_argument,
            Catch::Matchers::Exception::ExceptionMessageMatcher("continuationState")
        );
    }

    SECTION("Empty") {
        std::shared_ptr<MyResource> const   resource(MyResource::Create(MyResource::OperationType::EvaluateEmpty, "MyResource"));
        NS::Resource::State                 state(*resource);

        CHECK_THROWS_MATCHES(
            resource->Evaluate(request, 1, state),
            std::runtime_error,
            Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid Evaluations")
        );
    }

    SECTION("Too Many") {
        std::shared_ptr<MyResource> const   resource(MyResource::Create(MyResource::OperationType::EvaluateTooMany, "MyResource"));
        NS::Resource::State                 state(*resource);

        CHECK_THROWS_MATCHES(
            resource->Evaluate(request, 1, state),
            std::runtime_error,
            Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid Evaluations")
        );
    }
}

TEST_CASE("Resource - Apply") {
    SECTION("Valid") {
        std::shared_ptr<MyResource> const   resource(MyResource::Create(MyResource::OperationType::Valid, "MyResource"));
        NS::Resource::State                 state(*resource);

        CHECK(resource->Apply(state));

        NS::Resource::State                 wrongState(*MyResource::Create(MyResource::OperationType::Valid, "DifferentResource"));

        CHECK_THROWS_MATCHES(
            resource->Apply(wrongState),
            std::invalid_argument,
            Catch::Matchers::Exception::ExceptionMessageMatcher("applyState")
        );
    }

    SECTION("Invalid") {
        std::shared_ptr<MyResource> const   resource(MyResource::Create(MyResource::OperationType::ApplyInvalid, "MyResource"));
        NS::Resource::State                 state(*resource);

        CHECK_THROWS_MATCHES(
            resource->Apply(state),
            std::runtime_error,
            Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid ResourcePtr")
        );
    }
}

TEST_CASE("Resource - Compare") {
    CHECK(CommonHelpers::TestHelpers::CompareTest(*MyResource::Create(MyResource::OperationType::Valid, "a"), *MyResource::Create(MyResource::OperationType::Valid, "a"), true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(*MyResource::Create(MyResource::OperationType::Valid, "a"), *MyResource::Create(MyResource::OperationType::Valid, "z")) == 0);

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            *MyResource::Create(MyResource::OperationType::Valid, "a"),
            *MyResource::Create(MyResource::OperationType::Valid, "a", std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }))
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            *MyResource::Create(MyResource::OperationType::Valid, "a"),
            *MyResource::Create(MyResource::OperationType::Valid, "a", NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }))
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            *MyResource::Create(MyResource::OperationType::Valid, "a"),
            *MyResource::Create(MyResource::OperationType::Valid, "a", NS::ConditionPtrsPtr(), NS::ConditionPtrsPtr(), std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ sg_pCondition }))
        ) == 0
    );
}

TEST_CASE("Resource - Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializePtrTest(
            MyResource::Create(
                MyResource::OperationType::Valid,
                "MyResource",
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

TEST_CASE("Resource - CalculateResult - Request Requirement") {
    Components::Score::Result const         result(
        MyResource::CalculateResult(
            NS::Request(
                "Request",
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(true) })
            ),
            *MyResource::Create(
                MyResource::OperationType::Valid,
                "Resource"
            )
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);

    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 1);
    CHECK(result.RequirementResults[0].IsSuccessful);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 1.0f);

    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Resource - CalculateResult - Resource Requirement") {
    Components::Score::Result const         result(
        MyResource::CalculateResult(
            NS::Request("Request"),
            *MyResource::Create(
                MyResource::OperationType::Valid,
                "Resource",
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(true) })
            )
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);

    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 1);
    CHECK(result.RequirementResults[0].IsSuccessful);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 1.0f);

    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Resource - CalculateRequest - Request and Resource Requirement") {
    Components::Score::Result const         result(
        MyResource::CalculateResult(
            NS::Request(
                "Request",
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(false) })
            ),
            *MyResource::Create(
                MyResource::OperationType::Valid,
                "Resource",
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(true) })
            )
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful == false);

    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 2);
    CHECK(result.RequirementResults[0].IsSuccessful == false);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 0.0f);

    CHECK(result.RequirementResults[1].IsSuccessful);
    CHECK(Approx(result.RequirementResults[1].Ratio) == 1.0f);

    CHECK(result.PreferenceResults.empty());
}

TEST_CASE("Resource - CalculateResult - Request Preference") {
    Components::Score::Result const         result(
        MyResource::CalculateResult(
            NS::Request(
                "Request",
                NS::ConditionPtrsPtr(),
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(true) })
            ),
            *MyResource::Create(
                MyResource::OperationType::Valid,
                "Resource"
            )
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);

    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());

    REQUIRE(result.PreferenceResults.size() == 1);
    CHECK(result.PreferenceResults[0].IsSuccessful);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 1.0f);
}

TEST_CASE("Resource - CalculateResult - Resource Preference") {
    Components::Score::Result const         result(
        MyResource::CalculateResult(
            NS::Request("Request"),
            *MyResource::Create(
                MyResource::OperationType::Valid,
                "Resource",
                NS::ConditionPtrsPtr(),
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(true) })
            )
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);

    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());

    REQUIRE(result.PreferenceResults.size() == 1);
    CHECK(result.PreferenceResults[0].IsSuccessful);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 1.0f);
}

TEST_CASE("Resource - CalculateRequest - Request and Resource Preference") {
    Components::Score::Result const         result(
        MyResource::CalculateResult(
            NS::Request(
                "Request",
                NS::ConditionPtrsPtr(),
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(false) })
            ),
            *MyResource::Create(
                MyResource::OperationType::Valid,
                "Resource",
                NS::ConditionPtrsPtr(),
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(true) })
            )
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);

    CHECK(result.ApplicabilityResults.empty());
    CHECK(result.RequirementResults.empty());

    REQUIRE(result.PreferenceResults.size() == 2);
    CHECK(result.PreferenceResults[0].IsSuccessful == false);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 0.0f);

    CHECK(result.PreferenceResults[1].IsSuccessful);
    CHECK(Approx(result.PreferenceResults[1].Ratio) == 1.0f);
}

TEST_CASE("Resource - CalculateRequest - Request Preference and Resource Requirement") {
    Components::Score::Result const         result(
        MyResource::CalculateResult(
            NS::Request(
                "Request",
                NS::ConditionPtrsPtr(),
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(false) })
            ),
            *MyResource::Create(
                MyResource::OperationType::Valid,
                "Resource",
                NS::ConditionPtrsPtr(),
                std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(true) })
            )
        )
    );

    CHECK(result.IsApplicable);
    CHECK(result.IsSuccessful);

    CHECK(result.ApplicabilityResults.empty());

    REQUIRE(result.RequirementResults.size() == 1);
    CHECK(result.RequirementResults[0].IsSuccessful);
    CHECK(Approx(result.RequirementResults[0].Ratio) == 1.0f);

    REQUIRE(result.PreferenceResults.size() == 1);
    CHECK(result.PreferenceResults[0].IsSuccessful == false);
    CHECK(Approx(result.PreferenceResults[0].Ratio) == 0.0f);
}

TEST_CASE("Resource - CalculateRequest - Applicability") {
    SECTION("Applicable") {
        Components::Score::Result const         result(
            MyResource::CalculateResult(
                NS::Request(
                    "Request",
                    std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(true) })
                ),
                *MyResource::Create(
                    MyResource::OperationType::Valid,
                    "Resource",
                    NS::ConditionPtrsPtr(),
                    std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(false) })
                )
            )
        );

        CHECK(result.IsApplicable);
        CHECK(result.IsSuccessful == false);

        REQUIRE(result.ApplicabilityResults.size() == 1);
        CHECK(result.ApplicabilityResults[0].IsSuccessful);
        CHECK(Approx(result.ApplicabilityResults[0].Ratio) == 1.0f);

        REQUIRE(result.RequirementResults.size() == 1);
        CHECK(result.RequirementResults[0].IsSuccessful == false);
        CHECK(Approx(result.RequirementResults[0].Ratio) == 0.0f);

        CHECK(result.PreferenceResults.empty());
    }

    SECTION("Not Applicable") {
        Components::Score::Result const         result(
            MyResource::CalculateResult(
                NS::Request(
                    "Request",
                    std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(false) })
                ),
                *MyResource::Create(
                    MyResource::OperationType::Valid,
                    "Resource",
                    NS::ConditionPtrsPtr(),
                    std::make_shared<NS::ConditionPtrs>(NS::ConditionPtrs{ MyCondition::Create(false) })
                )
            )
        );

        CHECK(result.IsApplicable == false);
        CHECK(result.IsSuccessful);

        REQUIRE(result.ApplicabilityResults.size() == 1);
        CHECK(result.ApplicabilityResults[0].IsSuccessful == false);
        CHECK(Approx(result.ApplicabilityResults[0].Ratio) == 0.0f);

        CHECK(result.RequirementResults.empty());
        CHECK(result.PreferenceResults.empty());
    }
}

TEST_CASE("Resource::State - Compare") {
    std::shared_ptr<MyResource>             resource1(MyResource::Create(MyResource::OperationType::Valid, "Resource1"));
    std::shared_ptr<MyResource>             resource2(MyResource::Create(MyResource::OperationType::Valid, "Resource2"));

    CHECK(CommonHelpers::TestHelpers::CompareTest(*resource1, *resource1, true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(*resource1, *resource2) == 0);
}

TEST_CASE("Resource::State - Serialization") {
    std::shared_ptr<MyResource>             resource(MyResource::Create(MyResource::OperationType::Valid, "MyResource"));

    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::Resource::State(*resource),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}

TEST_CASE("Resource::Evaluation") {
    std::shared_ptr<MyResource>             resource(MyResource::Create(MyResource::OperationType::Valid, "MyResource"));
    std::shared_ptr<NS::Resource::State>    state(std::make_shared<NS::Resource::State>(*resource));

    SECTION("Construct") {
        NS::Resource::Evaluation(CreateScoreResult(true), state);
        NS::Resource::Evaluation(CreateScoreResult(false), state);
        NS::Resource::Evaluation(CreateScoreResult(false));

        CHECK_THROWS_MATCHES(
            NS::Resource::Evaluation(CreateScoreResult(true)),
            std::invalid_argument,
            Catch::Matchers::Exception::ExceptionMessageMatcher("result")
        );
    }

    NS::Resource::Evaluation const          eval(CreateScoreResult(true), state);

    CHECK(CommonHelpers::TestHelpers::CompareTest(eval, eval, true) == 0);
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            eval,
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
