/////////////////////////////////////////////////////////////////////////
///
///  \file          PermutationGeneratorFactory_UnitTest.cpp
///  \brief         Unit test for PermutationGeneratorFactory
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:36:30
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
#include "../PermutationGeneratorFactory.h"
#include <catch.hpp>

#include "../PermutationGenerator.h"

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::ConstrainedResource;

// ----------------------------------------------------------------------
// |
// |  Internal Types
// |
// ----------------------------------------------------------------------
class MyPermutationGenerator : public NS::PermutationGenerator {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    using NS::PermutationGenerator::PermutationGenerator;

#define ARGS                                BASES(NS::PermutationGenerator)

    NON_COPYABLE(MyPermutationGenerator);
    MOVE(MyPermutationGenerator, ARGS);
    COMPARE(MyPermutationGenerator, ARGS);
    SERIALIZATION(MyPermutationGenerator, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::PermutationGenerator)));

#undef ARGS

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    RequestPtrsPtrs GenerateImpl(RequestPtrs const &, size_t) override {
        return RequestPtrsPtrs();
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyPermutationGenerator);

class MyPermutationGeneratorFactory : public NS::PermutationGeneratorFactory {
private:
    // ----------------------------------------------------------------------
    // |  Private Data (used in public declarations)
    bool const                              _returnValid;

public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    MyPermutationGeneratorFactory(bool returnValid, size_t maxNumTotalPermutations) :
        NS::PermutationGeneratorFactory(maxNumTotalPermutations),
        _returnValid(returnValid)
    {}

    ~MyPermutationGeneratorFactory(void) override = default;

#define ARGS                                MEMBERS(_returnValid), BASES(NS::PermutationGeneratorFactory)

    NON_COPYABLE(MyPermutationGeneratorFactory);
    MOVE(MyPermutationGeneratorFactory, ARGS);
    COMPARE(MyPermutationGeneratorFactory, ARGS);
    SERIALIZATION(MyPermutationGeneratorFactory, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(NS::PermutationGeneratorFactory)));

#undef ARGS

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    PermutationGeneratorUniquePtr CreateImpl(size_t maxNumTotalPermutations) const override {
        if(_returnValid)
            return std::make_unique<MyPermutationGenerator>(maxNumTotalPermutations);

        return PermutationGeneratorUniquePtr();
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyPermutationGeneratorFactory);

TEST_CASE("Construct") {
    MyPermutationGeneratorFactory(true, 10);

    CHECK_THROWS_MATCHES(
        MyPermutationGeneratorFactory(true, 0),
        std::invalid_argument,
        Catch::Matchers::Exception::ExceptionMessageMatcher("maxNumTotalPermutations")
    );
}

TEST_CASE("Valid creation") {
    CHECK(MyPermutationGeneratorFactory(true, 10).Create());

    CHECK_THROWS_MATCHES(
        MyPermutationGeneratorFactory(false, 10).Create(),
        std::runtime_error,
        Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid PermutationGeneratorUniquePtr")
    );
}

TEST_CASE("Compare") {
    CHECK(CommonHelpers::TestHelpers::CompareTest(MyPermutationGeneratorFactory(true, 10), MyPermutationGeneratorFactory(true, 10), true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(MyPermutationGeneratorFactory(false, 10), MyPermutationGeneratorFactory(true, 10)) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(MyPermutationGeneratorFactory(true, 1), MyPermutationGeneratorFactory(true, 10)) == 0);
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializePtrTest(
            std::make_unique<MyPermutationGeneratorFactory>(true, 10),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
