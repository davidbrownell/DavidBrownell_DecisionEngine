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

#include <BoostHelpers/TestHelpers.h>
#include <CommonHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::ConstrainedResource;

// TODO
