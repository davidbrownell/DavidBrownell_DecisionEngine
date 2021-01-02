/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedResultSystem.cpp
///  \brief         See CalculatedResultSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 20:44:34
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
#include "CalculatedResultSystem.h"
#include "ResultSystem.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |
// |  CalculatedResultSystem
// |
// ----------------------------------------------------------------------
CalculatedResultSystem::CalculatedResultSystem(Score score, Index index) :
    System(
        TypeValue::Result,
        CompletionValue::Calculated,
        std::move(score),
        std::move(index)
    )
{}

CalculatedResultSystem::ResultSystemUniquePtr CalculatedResultSystem::Commit(void) {
    ResultSystemUniquePtr                   pResult(CommitImpl(_score.Commit(), _index.Commit()));

    if(!pResult)
        throw std::logic_error("Invalid result");

    return pResult;
}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
