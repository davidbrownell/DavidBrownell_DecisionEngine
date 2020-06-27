/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedWorkingSystem.h
///  \brief         Contains the CalculatedWorkingSystem object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 15:37:09
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
#pragma once

#include "WorkingSystem.h"

#include <DecisionEngine/Core/Components/CalculatedWorkingSystem.h>

namespace DecisionEngine {
namespace ConstrainedResource {

/////////////////////////////////////////////////////////////////////////
///  \class         CalculatedWorkingSystem
///  \brief         Object that can be converted into a WorkingSystem for
///                 constrained resource systems.
///
class CalculatedWorkingSystem : public Core::Components::CalculatedWorkingSystem {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Private Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    WorkingSystem::ImmutableStatePtr        _pImmutableState;
    WorkingSystem::TransitionState          _transitionState;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    CalculatedWorkingSystem(WorkingSystem::ImmutableStatePtr pImmutableState, WorkingSystem::TransitionState transition, Score score, Index index);
    ~CalculatedWorkingSystem(void) override = default;

#define ARGS                                MEMBERS(_pImmutableState, _transitionState), BASES(Core::Components::CalculatedWorkingSystem)

    NON_COPYABLE(CalculatedWorkingSystem);
    MOVE(CalculatedWorkingSystem, ARGS);
    COMPARE(CalculatedWorkingSystem, ARGS);
    SERIALIZATION(CalculatedWorkingSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(Core::Components::System)));

#undef ARGS

    std::string ToString(void) const override;

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    WorkingSystemPtr CommitImpl(Score score, Index index) override;
};

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::CalculatedWorkingSystem);
