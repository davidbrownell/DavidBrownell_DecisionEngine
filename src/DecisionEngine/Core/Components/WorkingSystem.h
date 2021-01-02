/////////////////////////////////////////////////////////////////////////
///
///  \file          WorkingSystem.h
///  \brief         Contains the WorkingSystem object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 18:57:32
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
#pragma once

#include "System.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

/////////////////////////////////////////////////////////////////////////
///  \class         WorkingSystem
///  \brief         Represents a system in a pending/not-yet-completed state.
///                 Derived WorkingSystems can generate child systems based on
///                 their internal state.
///
class WorkingSystem : public System {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using Index                             = DecisionEngine::Core::Components::Index;
    using Score                             = DecisionEngine::Core::Components::Score;

    using SystemPtr                         = std::shared_ptr<System>;
    using SystemPtrs                        = std::deque<SystemPtr>;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    WorkingSystem(void);
    WorkingSystem(Score score, Index index);
    ~WorkingSystem(void) override = default;

#define ARGS                                BASES(System)

    NON_COPYABLE(WorkingSystem);
    MOVE(WorkingSystem, ARGS);
    COMPARE(WorkingSystem, ARGS);
    SERIALIZATION(WorkingSystem, ARGS, FLAGS(SERIALIZATION_DATA_ONLY));

#undef ARGS

    SystemPtrs GenerateChildren(size_t maxNumChildren);
    virtual bool IsComplete(void) const = 0;

protected:
    // ----------------------------------------------------------------------
    // |
    // |  Protected Types
    // |
    // ----------------------------------------------------------------------

    /////////////////////////////////////////////////////////////////////////
    ///  \class         InternalStateImpl
    ///  \brief         Helper type that can be useful for derived classes
    ///                 implementing re-entrant GenerateChildrenImpl functionality.
    ///
    template <typename T>
    class InternalStateImpl {
    public:
        // ----------------------------------------------------------------------
        // |  Public Data
        Index::value_type const             Index;
        T                                   State;

        // ----------------------------------------------------------------------
        // |  Public Methods
        InternalStateImpl(Index::value_type index, T state);

#define ARGS                                MEMBERS(Index, State)

        NON_COPYABLE(InternalStateImpl);
        MOVE(InternalStateImpl, ARGS);
        COMPARE(InternalStateImpl, ARGS);
        SERIALIZATION(InternalStateImpl, ARGS);

#undef ARGS
    };

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    virtual SystemPtrs GenerateChildrenImpl(size_t maxNumChildren) = 0;
};

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Implementation
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
template <typename T>
WorkingSystem::InternalStateImpl<T>::InternalStateImpl(Index::value_type index, T state) :
    Index(std::move(index)),
    State(std::move(state))
{}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
