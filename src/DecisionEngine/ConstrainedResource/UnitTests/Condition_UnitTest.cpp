/////////////////////////////////////////////////////////////////////////
///
///  \file          Condition_UnitTest.cpp
///  \brief         Unit test for Condition.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-23 09:36:14
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
#include "../Condition.h"
#include <catch.hpp>

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::ConstrainedResource;

namespace DecisionEngine {
namespace ConstrainedResource {

class Request {};
class Resource {};

} // namespace ConstrainedResource
} // namespace DecisionEngine

class MyCondition : public NS::Condition {
private:
    // ----------------------------------------------------------------------
    // |  Private Data (used in public declarations)
    bool const                              _returnSuccess;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    CREATE(MyCondition);

    template <typename PrivateConstructorTagT>
    MyCondition(PrivateConstructorTagT tag, bool returnSuccess) :
        NS::Condition(tag, "MyCondition", 100),
        _returnSuccess(std::move(returnSuccess))
    {}

    ~MyCondition(void) override = default;

#define ARGS                                MEMBERS(_returnSuccess), BASES(NS::Condition)

    NON_COPYABLE(MyCondition);
    MOVE(MyCondition, ARGS);
    COMPARE(MyCondition, ARGS);
    SERIALIZATION(MyCondition, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_POLYMORPHIC(DecisionEngine::Core::Components::Condition)));

#undef ARGS
    Result Apply(Request const &, Resource const &) const override {
        return Result(SharedFromThis(), _returnSuccess);
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyCondition);

TEST_CASE("Apply") {
    NS::Request const                       request;
    NS::Resource const                      resource;

    CHECK(MyCondition::Create(true)->Apply(request, resource).IsSuccessful);
    CHECK(MyCondition::Create(false)->Apply(request, resource).IsSuccessful == false);
}

TEST_CASE("Compare") {
    CHECK(CommonHelpers::TestHelpers::CompareTest(*MyCondition::Create(true), *MyCondition::Create(true), true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(*MyCondition::Create(false), *MyCondition::Create(true)) == 0);
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializePtrTest(
            MyCondition::Create(true),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
