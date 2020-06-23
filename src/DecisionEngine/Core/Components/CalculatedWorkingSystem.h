/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedWorkingSystem.h
///  \brief         Contains the CalculatedWorkingSystem object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 20:48:49
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

#include "System.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |  Forward Declaration
class WorkingSystem;

/////////////////////////////////////////////////////////////////////////
///  \class         CalculatedWorkingSystem
///  \brief         A `WorkingSystem` whose state has yet to be applied. Once
///                 a `CalculatedWorkingSystem` is needed, it can by lazily converted
///                 into a `WorkingSystem` object.
///
class CalculatedWorkingSystem : public System {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using Index                             = DecisionEngine::Core::Components::Index;
    using Score                             = DecisionEngine::Core::Components::Score;

    using WorkingSystemPtr                  = std::shared_ptr<WorkingSystem>;

    // ----------------------------------------------------------------------
    // |
    // |  Public Results
    // |
    // ----------------------------------------------------------------------
    CalculatedWorkingSystem(Score score, Index index);
    ~CalculatedWorkingSystem(void) override = default;

#define ARGS                                BASES(System)

    NON_COPYABLE(CalculatedWorkingSystem);
    MOVE(CalculatedWorkingSystem, ARGS);
    COMPARE(CalculatedWorkingSystem, ARGS);
    SERIALIZATION(CalculatedWorkingSystem, ARGS, FLAGS(SERIALIZATION_DATA_ONLY));

#undef ARGS

    WorkingSystemPtr Commit(void);

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    virtual WorkingSystemPtr CommitImpl(Score score, Index index) = 0;
};

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
