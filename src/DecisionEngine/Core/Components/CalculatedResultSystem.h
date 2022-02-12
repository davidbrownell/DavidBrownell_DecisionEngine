/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedResultSystem.h
///  \brief         Contains the CalculatedResultSystem object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 20:39:57
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

#include "System.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |  Forward Declarations
class ResultSystem;

/////////////////////////////////////////////////////////////////////////
///  \class         CalculatedResultSystem
///  \brief         A `ResultSystem` whose state has yet to be applied. Once
///                 a `CalculatedResultSystem` is needed, it can by lazily converted
///                 into a `ResultSystem` object.
///
class CalculatedResultSystem : public System {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using Index                             = DecisionEngine::Core::Components::Index;
    using Score                             = DecisionEngine::Core::Components::Score;

    using ResultSystemUniquePtr             = std::unique_ptr<ResultSystem>;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    CalculatedResultSystem(Score score, Index index);
    ~CalculatedResultSystem(void) override = default;

#define ARGS                                BASES(System)

    NON_COPYABLE(CalculatedResultSystem)
    MOVE(CalculatedResultSystem, ARGS);
    COMPARE(CalculatedResultSystem, ARGS);
    SERIALIZATION(CalculatedResultSystem, ARGS, FLAGS(SERIALIZATION_DATA_ONLY));

#undef ARGS

    ResultSystemUniquePtr Commit(void);

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    virtual ResultSystemUniquePtr CommitImpl(Score score, Index index) = 0;
};

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
