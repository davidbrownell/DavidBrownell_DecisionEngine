/////////////////////////////////////////////////////////////////////////
///
///  \file          WorkingSystem.cpp
///  \brief         See WorkingSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 19:07:22
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
#include "WorkingSystem.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |
// |  WorkingSystem
// |
// ----------------------------------------------------------------------
WorkingSystem::WorkingSystem(void) :
    WorkingSystem(Score(), Index())
{}

WorkingSystem::WorkingSystem(Score score, Index index) :
    System(
        TypeValue::Working,
        CompletionValue::Concrete,
        std::move(score),
        std::move(index)
    )
{}

WorkingSystem::SystemPtrs WorkingSystem::GenerateChildren(size_t maxNumChildren) {
    ENSURE_ARGUMENT(maxNumChildren);

    SystemPtrs                              results(GenerateChildrenImpl(maxNumChildren));

    if(
        results.empty()
        || results.size() > maxNumChildren
        || std::all_of(results.cbegin(), results.cend(), [](SystemPtr const &ptr) { return static_cast<bool>(ptr); }) == false
    )
        throw std::logic_error("Invalid results");

    return results;
}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
