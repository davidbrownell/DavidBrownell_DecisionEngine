/////////////////////////////////////////////////////////////////////////
///
///  \file          EngineImpl.h
///  \brief         Contains the EngineImpl namespace
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-29 15:37:55
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

#include "Components.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |  Forward Declarations
class Fingerprinter;
class Score;
class System;
class ResultSystem;
class WorkingSystem;

/////////////////////////////////////////////////////////////////////////
///  \namespace     EngineImpl
///  \brief         Functionality common when running locally or distributed
///                 across multiple machines.
///
namespace EngineImpl {

using SystemPtr                             = std::shared_ptr<System>;
using SystemPtrs                            = std::deque<SystemPtr>;
using SystemPtrsContainer                   = std::vector<SystemPtrs>;

using ResultSystemUniquePtr                 = std::unique_ptr<ResultSystem>;
using WorkingSystemPtr                      = std::shared_ptr<WorkingSystem>;

/////////////////////////////////////////////////////////////////////////
///  \class         Observer
///  \brief         Observes events generated during `ExecuteRound`.
///
class Observer {
public:
    // ----------------------------------------------------------------------
    // |  Public Types
    using SystemPtr                         = DecisionEngine::Core::Components::EngineImpl::SystemPtrs;

    // ----------------------------------------------------------------------
    // |  Public Methods
    virtual bool OnBegin(size_t iteration, size_t maxIterations) = 0;
    virtual void OnEnd(size_t iteration, size_t maxItrations) = 0;

    virtual bool OnResultSystem(size_t iteration, size_t maxIterations, ResultSystemUniquePtr pResult) = 0;

    virtual bool OnGeneratingWork(size_t iteration, size_t maxIterations, WorkingSystem const &active) = 0;
    virtual void OnGeneratedWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &generated) = 0;

    virtual bool OnMergingWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &generated, SystemPtrs const &pending) = 0;
    virtual void OnMergedWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &pending, SystemPtrsContainer removed) = 0;

    virtual void OnFailedSystems(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) = 0;

protected:
    // ----------------------------------------------------------------------
    // |  Protected Methods
    ~Observer(void) = default;
};

/////////////////////////////////////////////////////////////////////////
///  \typedef       DynamicScoreFunctor
///  \brief         Function that can be used to update the Score of a System
///                 based on some criteria (such as momentum, instinct, etc.).
///                 If applied, the algorithm execution is no longer deterministic.
///
using DynamicScoreFunctor                   = std::function<Score (System const &, Score const &)>;

/////////////////////////////////////////////////////////////////////////
///  \fn            ExecuteRound
///  \brief         Executes a round of System generation, where the number of
///                 iterations is specified by the caller.
///
SystemPtrs ExecuteRound(
    Fingerprinter &fingerprinter,
    Observer &observer,
    size_t maxNumPendingSystems,
    size_t maxNumChildrenPerGeneration,
    size_t maxNumIterations,
    bool continueProcessingSystemWithFailures,
    std::atomic<bool> const &isCancelled,
    WorkingSystemPtr pInitial,
    std::optional<std::tuple<ThreadPool &, DynamicScoreFunctor const &>> const &dynamicScoreInfo=std::nullopt
);

/////////////////////////////////////////////////////////////////////////
///  \fn            Merge
///  \brief         Merges systems into a sorted lists, limiting the result
///                 size to a maximum number of items.
///
///  \returns       std::tuple<
///                     Sorted items,
///                     input items with empty SystemPtrs containers removed
///                 >
///
std::tuple<SystemPtrs, SystemPtrsContainer> Merge(
    size_t maxNumsystems,
    SystemPtrsContainer items,
    std::optional<std::tuple<ThreadPool &, DynamicScoreFunctor const &>> const &dynamicScoreInfo=std::nullopt
);

} // namespace EngineImpl
} // namespace Components
} // namespace Core
} // namespace DecisionEngine
