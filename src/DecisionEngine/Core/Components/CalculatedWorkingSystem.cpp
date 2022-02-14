/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedWorkingSystem.cpp
///  \brief         See CalculatedWorkingSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 20:52:13
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
#include "CalculatedWorkingSystem.h"
#include "WorkingSystem.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |
// |  CalculatedWorkingSystem
// |
// ----------------------------------------------------------------------
CalculatedWorkingSystem::CalculatedWorkingSystem(Score score, Index index) :
    System(
        TypeValue::Working,
        CompletionValue::Calculated,
        std::move(score),
        std::move(index)
    )
{}

CalculatedWorkingSystem::WorkingSystemPtr CalculatedWorkingSystem::Commit(void) {
    WorkingSystemPtr                        pResult(CommitImpl(_score.Commit(), _index.Commit()));

    if(!pResult)
        throw std::logic_error("Invalid result");

    return pResult;
}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
