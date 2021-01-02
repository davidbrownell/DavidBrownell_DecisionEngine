/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedWorkingSystem.cpp
///  \brief         See CalculatedWorkingSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 15:36:38
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
#include "CalculatedWorkingSystem.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  CalculatedWorkingSystem
// |
// ----------------------------------------------------------------------
CalculatedWorkingSystem::CalculatedWorkingSystem(WorkingSystem::ImmutableStatePtr pImmutableState, WorkingSystem::TransitionState transition, Score score, Index index) :
    Core::Components::CalculatedWorkingSystem(std::move(score), std::move(index)),
    _pImmutableState(
        std::move(
            [&pImmutableState](void) -> WorkingSystem::ImmutableStatePtr & {
                ENSURE_ARGUMENT(pImmutableState);
                return pImmutableState;
            }()
        )
    ),
    _transitionState(std::move(transition))
{}

std::string CalculatedWorkingSystem::ToString(void) const /*override*/ {
    return boost::str(
        boost::format("ConstrainedResource::CalculatedWorkingSystem(%1%,%2%)")
            % GetScore().ToString()
            % GetIndex().ToString()
    );
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
CalculatedWorkingSystem::WorkingSystemPtr CalculatedWorkingSystem::CommitImpl(Score score, Index index) /*override*/ {
    return std::make_shared<WorkingSystem>(
        std::move(_pImmutableState),
        std::move(_transitionState),
        std::move(score),
        std::move(index)
    );
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::CalculatedWorkingSystem);
