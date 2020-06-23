/////////////////////////////////////////////////////////////////////////
///
///  \file          System.h
///  \brief         Contains the System object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-23 22:07:32
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

#include "Index.h"
#include "Score.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

/////////////////////////////////////////////////////////////////////////
///  \class         System
///  \brief         Abstract base class for all Systems; Scores
///                 and Index values ensure a stable sort.
///
class System {
private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    Score                                   _score;
    Index                                   _index;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    enum class TypeValue : unsigned char {
        Working = 1,                        /// System that is not yet complete
        Result                              /// System that is complete
    };

    enum class CompletionValue : unsigned char {
        Calculated = 1,                     /// System that has not yet been commited
        Concrete                            /// System that has been committed
    };

    // ----------------------------------------------------------------------
    // |
    // |  Public Data
    // |
    // ----------------------------------------------------------------------
    TypeValue const                         Type;
    CompletionValue const                   Completion;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    System(TypeValue type, CompletionValue completion, Score score, Index index);
    virtual ~System(void) = default;

#define ARGS                                MEMBERS(_score, _index, Type, Completion)

    NON_COPYABLE(System);
    MOVE(System, ARGS);
    SERIALIZATION(System, ARGS, FLAGS(SERIALIZATION_ABSTRACT));

    // Note that the order of these arguments are very important to ensure stable comparisons
    COMPARE(System, MEMBERS(_score, Type, Completion, _index));

#undef ARGS

    virtual std::string ToString(void) const = 0;

    System & UpdateScore(Score score);
    Score const & GetScore(void) const;
    Index const & GetIndex(void) const;

private:
    // ----------------------------------------------------------------------
    // |  Relationships
    friend class CalculatedWorkingSystem;
    friend class CalculatedResultSystem;
};

} // namespace Components
} // namespace Core
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::Core::Components::System);
