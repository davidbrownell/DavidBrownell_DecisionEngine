/////////////////////////////////////////////////////////////////////////
///
///  \file          Fingerprinter_UnitTest.cpp
///  \brief         Unit test for Fingerprinter.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-29 23:27:15
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
#include "../Fingerprinter.h"
#include <catch.hpp>

namespace DecisionEngine {
namespace Core {
namespace Components {

class System {};

} // namespace Components
} // namespace Core
} // namespace DecisionEngine

namespace NS                                = DecisionEngine::Core::Components;

TEST_CASE("NoopFingerprinter") {
    NS::NoopFingerprinter                   f;

    CHECK(f.ShouldProcess(NS::System()));
}
