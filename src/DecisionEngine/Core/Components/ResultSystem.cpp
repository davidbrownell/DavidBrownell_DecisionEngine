/////////////////////////////////////////////////////////////////////////
///
///  \file          ResultSystem.cpp
///  \brief         See ResultSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 20:13:30
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
#include "ResultSystem.h"

#include <boost/format.hpp>

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |
// |  ResultSystem
// |
// ----------------------------------------------------------------------
ResultSystem::ResultSystem(Score score, Index index) :
    System(
        TypeValue::Result,
        CompletionValue::Concrete,
        std::move(score),
        std::move(index)
    )
{}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
