/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedResultSystem_UnitTest.cpp
///  \brief         Unit test for CalculatedResultSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-25 09:46:10
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
#include "../CalculatedResultSystem.h"
#include <catch.hpp>

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::ConstrainedResource;
namespace Components                        = DecisionEngine::Core::Components;

// ----------------------------------------------------------------------
// |
// |  Internal Types
// |
// ----------------------------------------------------------------------
class MyResource : public NS::Resource {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    CREATE(MyResource);

    using NS::Resource::Resource;

#define ARGS                                BASES(NS::Resource)

    NON_COPYABLE(MyResource);
    MOVE(MyResource, ARGS);
    COMPARE(MyResource, ARGS);
    SERIALIZATION(MyResource, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_POLYMORPHIC(NS::Resource)));

#undef ARGS

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    EvaluateResult EvaluateImpl(Request const &, size_t) const override {
        return EvaluateResult(Evaluations(), ContinuationStatePtr());
    }

    EvaluateResult EvaluateImpl(Request const &, size_t, State &) const override {
        return EvaluateResult(Evaluations(), ContinuationStatePtr());
    }

    ResourcePtr ApplyImpl(State const &) const override {
        return MyResource::Create(*this);
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyResource);

Components::Score CreateScore(void) {
    return Components::Score(
        Components::Score::Result(
            Components::Score::Result::ConditionResults(),
            Components::Score::Result::ConditionResults(),
            Components::Score::Result::ConditionResults()
        ),
        false
    );
}

Components::Index CreateIndex(void) {
    return Components::Index(1);
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
TEST_CASE("Valid") {
    std::shared_ptr<MyResource>             resource(MyResource::Create("Resource"));
    std::shared_ptr<NS::Resource::State>    state(std::make_shared<NS::Resource::State>(*resource));
    NS::CalculatedResultSystem const        system(
        resource,
        state,
        std::make_shared<NS::RequestPtrsContainer>(
            NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
        ),
        CreateScore(),
        CreateIndex()
    );

    CHECK(system.ToString() == "ConstrainedResource::CalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),0),Pending(1,100001.00,1,0)),Index((1)))");
}

TEST_CASE("Construct Errors") {
    // Invalid resource
    CHECK_THROWS_MATCHES(
        NS::CalculatedResultSystem(
            NS::ResourcePtr(),
            NS::Resource::ApplyStatePtr(),
            std::make_shared<NS::RequestPtrsContainer>(
                NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
            ),
            CreateScore(),
            CreateIndex()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pResource")
    );

    std::shared_ptr<MyResource>             resource(MyResource::Create("Resource"));

    // Invalid state
    CHECK_THROWS_MATCHES(
        NS::CalculatedResultSystem(
            resource,
            NS::Resource::ApplyStatePtr(),
            std::make_shared<NS::RequestPtrsContainer>(
                NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
            ),
            CreateScore(),
            CreateIndex()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pApplyState")
    );

    std::shared_ptr<NS::Resource::State>    state(std::make_shared<NS::Resource::State>(*resource));

    // Invalid Request ptr container
    CHECK_THROWS_MATCHES(
        NS::CalculatedResultSystem(
            resource,
            state,
            NS::RequestPtrsContainerPtr(),
            CreateScore(),
            CreateIndex()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pRequestsContainer")
    );

    // Empty RequestsPtrsContainers
    CHECK_THROWS_MATCHES(
        NS::CalculatedResultSystem(
            resource,
            state,
            std::make_shared<NS::RequestPtrsContainer>(),
            CreateScore(),
            CreateIndex()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pRequestsContainer")
    );

    // Empty RequestsPtrsContainer
    CHECK_THROWS_MATCHES(
        NS::CalculatedResultSystem(
            resource,
            state,
            std::make_shared<NS::RequestPtrsContainer>(NS::RequestPtrsContainer{ NS::RequestPtrs() }),
            CreateScore(),
            CreateIndex()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pRequestsContainer")
    );

    // Invalid RequestsPtrsContainer element
    CHECK_THROWS_MATCHES(
        NS::CalculatedResultSystem(
            resource,
            state,
            std::make_shared<NS::RequestPtrsContainer>(NS::RequestPtrsContainer{ NS::RequestPtrs{ NS::RequestPtr() } }),
            CreateScore(),
            CreateIndex()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pRequestsContainer")
    );
}

TEST_CASE("Commit") {
    std::shared_ptr<MyResource>             resource(MyResource::Create("Resource"));
    std::shared_ptr<NS::Resource::State>    state(std::make_shared<NS::Resource::State>(*resource));

    CHECK(
        NS::CalculatedResultSystem(
            resource,
            state,
            std::make_shared<NS::RequestPtrsContainer>(
                NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
            ),
            CreateScore(),
            CreateIndex()
        ).Commit()
    );
}

TEST_CASE("Compare") {
    std::shared_ptr<MyResource>             resource(MyResource::Create("Resource"));
    std::shared_ptr<NS::Resource::State>    state(std::make_shared<NS::Resource::State>(*resource));

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::CalculatedResultSystem(
                resource,
                state,
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                CreateScore(),
                CreateIndex()
            ),
            NS::CalculatedResultSystem(
                resource,
                state,
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                CreateScore(),
                CreateIndex()
            ),
            true
        ) == 0
    );

    // Different Resource
    std::shared_ptr<MyResource>             resource2(MyResource::Create("Resource2"));
    std::shared_ptr<NS::Resource::State>    state2(std::make_shared<NS::Resource::State>(*resource2));

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::CalculatedResultSystem(
                resource,
                state,
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                CreateScore(),
                CreateIndex()
            ),
            NS::CalculatedResultSystem(
                resource2,
                state2,
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                CreateScore(),
                CreateIndex()
            )
        ) == 0
    );

    // Different Requests
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::CalculatedResultSystem(
                resource,
                state,
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                CreateScore(),
                CreateIndex()
            ),
            NS::CalculatedResultSystem(
                resource,
                state,
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2"), std::make_shared<NS::Request>("Request3") } }
                ),
                CreateScore(),
                CreateIndex()
            )
        ) == 0
    );
}

TEST_CASE("Serialization") {
    std::shared_ptr<MyResource>             resource(MyResource::Create("Resource"));
    std::shared_ptr<NS::Resource::State>    state(std::make_shared<NS::Resource::State>(*resource));

    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::CalculatedResultSystem(
                resource,
                state,
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                CreateScore(),
                CreateIndex()
            ),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
