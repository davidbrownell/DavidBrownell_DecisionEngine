/////////////////////////////////////////////////////////////////////////
///
///  \file          FingerprinterFactory_UnitTest.cpp
///  \brief         Unit test for FingerprinterFactory
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-17 23:47:55
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
#include "../FingerprinterFactory.h"
#include <catch.hpp>

#include <DecisionEngine/Core/Components/Fingerprinter.h>

namespace NS                                = DecisionEngine::Core::LocalExecution;

class MyFingerprinterFactory : public NS::FingerprinterFactory {
public:
    // ----------------------------------------------------------------------
    // |  Public Data
    bool const                              IsError;

    // ----------------------------------------------------------------------
    // |  Public Methods
    MyFingerprinterFactory(bool isError) : IsError(isError) {}
    ~MyFingerprinterFactory(void) override = default;

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    FingerprinterUniquePtr CreateImpl(void) override {
        if(IsError)
            return FingerprinterUniquePtr();

        return std::make_unique<DecisionEngine::Core::Components::NoopFingerprinter>();
    }
};

TEST_CASE("Standard") {
    CHECK(MyFingerprinterFactory(false).Create());
    CHECK_THROWS_MATCHES(MyFingerprinterFactory(true).Create(), std::runtime_error, Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid result"));
}
