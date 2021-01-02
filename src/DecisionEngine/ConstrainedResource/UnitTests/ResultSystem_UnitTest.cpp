/////////////////////////////////////////////////////////////////////////
///
///  \file          ResultSystem_UnitTest.cpp
///  \brief         Unit test for ResultSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-25 09:33:43
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
#include "../ResultSystem.h"
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
        return ResourcePtr();
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyResource);

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
TEST_CASE("Valid") {
    NS::ResultSystem const                  system(
        MyResource::Create("Resource"),
        std::make_shared<NS::RequestPtrsContainer>(
            NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
        ),
        Components::Score(),
        Components::Index()
    );

    CHECK(system.ToString() == "ConstrainedResource::ResultSystem(Score(Pending(1,100001.00,0,0)),Index())");
}

TEST_CASE("Construct Errors") {
    // Invalid resource
    CHECK_THROWS_MATCHES(
        NS::ResultSystem(
            NS::ResourcePtr(),
            std::make_shared<NS::RequestPtrsContainer>(
                NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
            ),
            Components::Score(),
            Components::Index()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pResource")
    );

    // Invalid Request ptr container
    CHECK_THROWS_MATCHES(
        NS::ResultSystem(
            MyResource::Create("Resource"),
            NS::RequestPtrsContainerPtr(),
            Components::Score(),
            Components::Index()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pRequestsContainer")
    );

    // Empty RequestsPtrsContainers
    CHECK_THROWS_MATCHES(
        NS::ResultSystem(
            MyResource::Create("Resource"),
            std::make_shared<NS::RequestPtrsContainer>(),
            Components::Score(),
            Components::Index()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pRequestsContainer")
    );

    // Empty RequestsPtrsContainer
    CHECK_THROWS_MATCHES(
        NS::ResultSystem(
            MyResource::Create("Resource"),
            std::make_shared<NS::RequestPtrsContainer>(NS::RequestPtrsContainer{ NS::RequestPtrs() }),
            Components::Score(),
            Components::Index()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pRequestsContainer")
    );

    // Invalid RequestsPtrsContainer element
    CHECK_THROWS_MATCHES(
        NS::ResultSystem(
            MyResource::Create("Resource"),
            std::make_shared<NS::RequestPtrsContainer>(NS::RequestPtrsContainer{ NS::RequestPtrs{ NS::RequestPtr() } }),
            Components::Score(),
            Components::Index()
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("pRequestsContainer")
    );
}

TEST_CASE("Compare") {
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::ResultSystem(
                MyResource::Create("Resource"),
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                Components::Score(),
                Components::Index()
            ),
            NS::ResultSystem(
                MyResource::Create("Resource"),
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                Components::Score(),
                Components::Index()
            ),
            true
        ) == 0
    );

    // Different Resource
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::ResultSystem(
                MyResource::Create("Resource1"),
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                Components::Score(),
                Components::Index()
            ),
            NS::ResultSystem(
                MyResource::Create("Resource2"),
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                Components::Score(),
                Components::Index()
            )
        ) == 0
    );

    // Different Requests
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            NS::ResultSystem(
                MyResource::Create("Resource"),
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                Components::Score(),
                Components::Index()
            ),
            NS::ResultSystem(
                MyResource::Create("Resource"),
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2"), std::make_shared<NS::Request>("Request3") } }
                ),
                Components::Score(),
                Components::Index()
            )
        ) == 0
    );
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::ResultSystem(
                MyResource::Create("Resource"),
                std::make_shared<NS::RequestPtrsContainer>(
                    NS::RequestPtrsContainer{ NS::RequestPtrs{ std::make_shared<NS::Request>("Request1"), std::make_shared<NS::Request>("Request2") } }
                ),
                Components::Score(),
                Components::Index()
            ),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
