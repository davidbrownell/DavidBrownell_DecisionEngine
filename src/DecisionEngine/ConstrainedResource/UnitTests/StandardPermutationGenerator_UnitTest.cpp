/////////////////////////////////////////////////////////////////////////
///
///  \file          StandardPermutationGenerator_UnitTest.cpp
///  \brief         Unit test for StandardPermutationGenerator.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:44:18
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2020-22
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../StandardPermutationGenerator.h"
#include <catch.hpp>

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::ConstrainedResource;

// ----------------------------------------------------------------------
// |
// |  Internal Types
// |
// ----------------------------------------------------------------------
namespace DecisionEngine {
namespace ConstrainedResource {

class Request {
public:
    // ----------------------------------------------------------------------
    // |  Public Data
    unsigned long const                     Id;

    // ----------------------------------------------------------------------
    // |  Public Methods
    Request(unsigned long id) : Id(id) {}
};

} // namespace ConstrainedResource
} // namespace DecisionEngine

TEST_CASE("Standard") {
    NS::StandardPermutationGenerator::RequestPtrs const                     requests{ std::make_shared<NS::Request>(1), std::make_shared<NS::Request>(2), std::make_shared<NS::Request>(3) };
    NS::StandardPermutationGenerator::RequestPtrsPtrs const                 results(NS::StandardPermutationGenerator().Generate(requests, 10000));

    REQUIRE(results.size() == 6);

    REQUIRE(results[0]->size() == 3);
    CHECK((*results[0])[0]->Id == 1);
    CHECK((*results[0])[1]->Id == 2);
    CHECK((*results[0])[2]->Id == 3);

    REQUIRE(results[1]->size() == 3);
    CHECK((*results[1])[0]->Id == 1);
    CHECK((*results[1])[1]->Id == 3);
    CHECK((*results[1])[2]->Id == 2);

    REQUIRE(results[2]->size() == 3);
    CHECK((*results[2])[0]->Id == 2);
    CHECK((*results[2])[1]->Id == 1);
    CHECK((*results[2])[2]->Id == 3);

    REQUIRE(results[3]->size() == 3);
    CHECK((*results[3])[0]->Id == 2);
    CHECK((*results[3])[1]->Id == 3);
    CHECK((*results[3])[2]->Id == 1);

    REQUIRE(results[4]->size() == 3);
    CHECK((*results[4])[0]->Id == 3);
    CHECK((*results[4])[1]->Id == 1);
    CHECK((*results[4])[2]->Id == 2);

    REQUIRE(results[5]->size() == 3);
    CHECK((*results[5])[0]->Id == 3);
    CHECK((*results[5])[1]->Id == 2);
    CHECK((*results[5])[2]->Id == 1);
}

TEST_CASE("Factory") {
    CHECK(NS::StandardPermutationGeneratorFactory(10).Create());
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::StandardPermutationGenerator(10),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );

    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::StandardPermutationGeneratorFactory(10),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
