/////////////////////////////////////////////////////////////////////////
///
///  \file          PermutationGenerator_UnitTest.cpp
///  \brief         Unit test for PermutationGenerator
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:36:09
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
#include "../PermutationGenerator.h"
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

class Request {};

} // namespace ConstrainedResource
} // namespace DecisionEngine

class MyPermutationGenerator : public NS::PermutationGenerator {
public:
    // ----------------------------------------------------------------------
    // |  Public Types
    enum class GenerateType {
        Valid,
        Empty,
        InvalidPointer,
        InvalidRequestsPtr,
        TooMany
    };

private:
    // ----------------------------------------------------------------------
    // |  Private Data (used in public declarations)
    GenerateType const                      _generateType;
    bool const                              _autoComplete;

public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    MyPermutationGenerator(
        GenerateType generateType,
        bool autoComplete=true,
        size_t maxNumTotalPermutations=2
    ) :
        NS::PermutationGenerator(maxNumTotalPermutations),
        _generateType(generateType),
        _autoComplete(autoComplete)
    {}

    ~MyPermutationGenerator(void) override = default;

#define ARGS                                MEMBERS(_generateType, _autoComplete), BASES(NS::PermutationGenerator)

    NON_COPYABLE(MyPermutationGenerator);
    MOVE(MyPermutationGenerator, ARGS);
    COMPARE(MyPermutationGenerator, ARGS);
    SERIALIZATION(MyPermutationGenerator, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::PermutationGenerator)));

#undef ARGS

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    RequestPtrsPtrs GenerateImpl(RequestPtrs const &requests, size_t maxNumPermutations) override {
        if(_autoComplete)
            _isActive = false;

        if(_generateType == GenerateType::Valid)
            return RequestPtrsPtrs{ std::make_shared<RequestPtrs>(requests) };
        if(_generateType == GenerateType::Empty)
            return RequestPtrsPtrs();
        if(_generateType == GenerateType::InvalidPointer)
            return RequestPtrsPtrs{ std::make_shared<RequestPtrs>() };
        if(_generateType == GenerateType::InvalidRequestsPtr)
            return RequestPtrsPtrs{ std::make_shared<RequestPtrs>(RequestPtrs{ RequestPtr() }) };
        if(_generateType == GenerateType::TooMany) {
            RequestPtrsPtrs                 results;

            while(maxNumPermutations--)
                results.emplace_back(std::make_shared<RequestPtrs>(requests));

            results.emplace_back(std::make_shared<RequestPtrs>(requests));

            return results;
        }
        else
            assert(!"Unrecognized GenerateType");

        return RequestPtrsPtrs();
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyPermutationGenerator);

TEST_CASE("Standard") {
    MyPermutationGenerator                              generator(MyPermutationGenerator::GenerateType::Valid);
    MyPermutationGenerator::RequestPtrs const           requests{ std::make_shared<NS::Request>(), std::make_shared<NS::Request>() };

    CHECK(generator.IsComplete() == false);
    CHECK(generator.Generate(requests, 1).size() == 1);
    CHECK(generator.IsComplete());
}

TEST_CASE("Invalid Construction") {
    CHECK_THROWS_MATCHES(
        MyPermutationGenerator(
            MyPermutationGenerator::GenerateType::Valid,
            true,
            0
        ),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("maxNumTotalPermutations")
    );
};

TEST_CASE("Generate") {
    MyPermutationGenerator::RequestPtrs const           requests{ std::make_shared<NS::Request>(), std::make_shared<NS::Request>() };

    SECTION("Valid") {
        MyPermutationGenerator                          generator(MyPermutationGenerator::GenerateType::Valid);

        CHECK(generator.IsComplete() == false);

        MyPermutationGenerator::RequestPtrsPtrs const   results(generator.Generate(requests, 1));

        REQUIRE(results.size() == 1);
        CHECK(*results[0] == requests);

        // Attempts to invoke Generate after the generator is complete should result in errors
        CHECK(generator.IsComplete());
        CHECK_THROWS_MATCHES(
            generator.Generate(requests, 1),
            std::runtime_error,
            Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid operation")
        );
    }

    SECTION("Valid - Closed by base") {
        MyPermutationGenerator                          generator(MyPermutationGenerator::GenerateType::Valid, false, 2);

        CHECK(generator.IsComplete() == false);

        generator.Generate(requests, 1);
        CHECK(generator.IsComplete() == false);

        generator.Generate(requests, 1);
        CHECK(generator.IsComplete());
    }

    SECTION("Invalid - Empty") {
        MyPermutationGenerator                          generator(MyPermutationGenerator::GenerateType::Empty);

        CHECK_THROWS_MATCHES(
            generator.Generate(requests, 1),
            std::runtime_error,
            Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid RequestPtrsPtrs")
        );
    }

    SECTION("Invalid - Invalid Pointer") {
        MyPermutationGenerator                          generator(MyPermutationGenerator::GenerateType::InvalidPointer);

        CHECK_THROWS_MATCHES(
            generator.Generate(requests, 1),
            std::runtime_error,
            Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid RequestPtrsPtrs")
        );
    }

    SECTION("Invalid - Invalid Request Ptr") {
        MyPermutationGenerator                          generator(MyPermutationGenerator::GenerateType::InvalidRequestsPtr);

        CHECK_THROWS_MATCHES(
            generator.Generate(requests, 1),
            std::runtime_error,
            Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid RequestPtrsPtrs")
        );
    }

    SECTION("Invalid - Too many") {
        MyPermutationGenerator                          generator(MyPermutationGenerator::GenerateType::TooMany);

        CHECK_THROWS_MATCHES(
            generator.Generate(requests, 1),
            std::runtime_error,
            Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid RequestPtrsPtrs")
        );
    }
}

TEST_CASE("Compare") {
    MyPermutationGenerator                  generator(MyPermutationGenerator::GenerateType::Valid);

    CHECK(CommonHelpers::TestHelpers::CompareTest(generator, MyPermutationGenerator(MyPermutationGenerator::GenerateType::Valid), true) == 0);

    // Complete the generator
    MyPermutationGenerator::RequestPtrs const           requests{ std::make_shared<NS::Request>(), std::make_shared<NS::Request>() };

    generator.Generate(requests, 1);

    CHECK(CommonHelpers::TestHelpers::CompareTest(generator, MyPermutationGenerator(MyPermutationGenerator::GenerateType::Valid)) == 0);
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializePtrTest(
            std::make_unique<MyPermutationGenerator>(MyPermutationGenerator::GenerateType::Valid),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
