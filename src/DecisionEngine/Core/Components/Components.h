/////////////////////////////////////////////////////////////////////////
///
///  \file          Components.h
///  \brief         Contains the Components namespace
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-20 22:27:02
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
#pragma once

#include <CommonHelpers/Compare.h>
#include <CommonHelpers/Constructor.h>
#include <CommonHelpers/Copy.h>
#include <CommonHelpers/Finally.h>
#include <CommonHelpers/Move.h>
#include <CommonHelpers/Misc.h>
#include <CommonHelpers/Stl.h>
#include <CommonHelpers/ThreadPool.h>
#include <CommonHelpers/ThreadSafeCounter.h>

#include <BoostHelpers/Serialization.h>
#include <BoostHelpers/SharedObject.h>

#include <boost/algorithm/string/join.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>

#include <boost/format.hpp>

#include <boost/serialization/deque.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

// This file must be included after all of the serialization helpers
// and archives used across by every object in this directory have
// been included.
#include <BoostHelpers/Serialization.suffix.h>

namespace DecisionEngine {
namespace Core {

/////////////////////////////////////////////////////////////////////////
///  \namespace     Components
///  \brief         Functionality common to all variations of the DecisionEngine.
///
namespace Components {

// ----------------------------------------------------------------------
// |  Public Types
using ThreadPool                            = CommonHelpers::ComplexThreadPool;

// ----------------------------------------------------------------------
// |  Public Data
static float const constexpr                MaxScore = 100001.0;

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
