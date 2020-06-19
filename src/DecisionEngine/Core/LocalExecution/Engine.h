/////////////////////////////////////////////////////////////////////////
///
///  \file          Engine.h
///  \brief         Contains the Engine namespace
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-05 23:47:58
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

#include "Configuration.h"
#include "LocalExecution.h"

#include <DecisionEngine/Core/Components/EngineImpl.h>
#include <DecisionEngine/Core/Components/ResultSystem.h>
#include <DecisionEngine/Core/Components/WorkingSystem.h>

namespace DecisionEngine {
namespace Core {
namespace LocalExecution {

/////////////////////////////////////////////////////////////////////////
///  \namespace     Engine
///  \brief         BugBug
///
namespace Engine {

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Public Types
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
using SystemPtr                             = Components::EngineImpl::SystemPtr;
using SystemPtrs                            = Components::EngineImpl::SystemPtrs;
using SystemPtrsContainer                   = Components::EngineImpl::SystemPtrsContainer;

using ResultSystemUniquePtr                 = Components::EngineImpl::ResultSystemUniquePtr;
using ResultSystemUniquePtrs                = std::vector<ResultSystemUniquePtr>;

using WorkingSystemPtr                      = Components::EngineImpl::WorkingSystemPtr;
using WorkingSystem                         = typename WorkingSystemPtr::element_type;

/////////////////////////////////////////////////////////////////////////
///  \class         Observer
///  \brief         Observes events generated by Execute, where a round contains
///                 one or more iterations.
///
class Observer {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using SystemPtr                         = DecisionEngine::Core::LocalExecution::Engine::SystemPtr;
    using SystemPtrs                        = DecisionEngine::Core::LocalExecution::Engine::SystemPtrs;
    using SystemPtrsContainer               = DecisionEngine::Core::LocalExecution::Engine::SystemPtrsContainer;

    using WorkingSystem                     = DecisionEngine::Core::LocalExecution::Engine::WorkingSystem;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    virtual ~Observer(void) = default;

    virtual bool OnRoundBegin(size_t round, SystemPtrs const &pending) = 0;
    virtual void OnRoundEnd(size_t round, SystemPtrs const &pending) = 0;

    virtual bool OnRoundMergingWork(size_t round, SystemPtrsContainer const &pending) = 0;
    virtual void OnRoundMergedWork(size_t round, SystemPtrs const &pending, SystemPtrsContainer removed) = 0;

    virtual bool OnTaskBegin(size_t round, size_t task, size_t numTasks) = 0;
    virtual void OnTaskEnd(size_t round, size_t task, size_t numTasks) = 0;

    virtual void OnTaskError(size_t round, size_t task, size_t numTasks, std::exception const &ex) = 0;

    virtual bool OnIterationBegin(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations) = 0;
    virtual void OnIterationEnd(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations) = 0;

    virtual bool OnIterationGeneratingWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active) = 0;
    virtual void OnIterationGeneratedWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &generated) = 0;

    virtual bool OnIterationMergingWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &generated, SystemPtrs const &pending) = 0;
    virtual void OnIterationMergedWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &pending, SystemPtrsContainer removed) = 0;

    virtual void OnIterationFailedSystems(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) = 0;
};

/////////////////////////////////////////////////////////////////////////
///  \class         ResultObserver
///  \brief         Observes ResultSystems as well as standard events.
///
class ResultObserver : public Observer {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using ResultSystemUniquePtr             = DecisionEngine::Core::LocalExecution::Engine::ResultSystemUniquePtr;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    ~ResultObserver(void) override = default;

    virtual bool OnIterationResultSystem(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, ResultSystemUniquePtr pResult) = 0;
};

/////////////////////////////////////////////////////////////////////////
///  \enum          ExecuteResultValue
///  \brief         Result returned by one of the overloaded Execute functions.
///
enum class ExecuteResultValue {
    Completed=1,                            /// The algorithm ran its course
    Timeout,                                /// The algorithm terminated as it exceeded the given timeout period
    ExitViaObserver                         /// An observer callback returned false
};

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Public Methods
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
///  \fn            Execute
///  \brief         Returns a single result.
///
std::tuple<ExecuteResultValue, ResultSystemUniquePtr> Execute(
    Configuration &config,
    Observer &observer,
    WorkingSystem const &initial,
    std::optional<std::chrono::steady_clock::duration> const &timeout=std::nullopt
);

template <typename WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT>
std::tuple<ExecuteResultValue, ResultSystemUniquePtr> Execute(
    Configuration &config,
    Observer &observer,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT begin,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT end,
    std::optional<std::chrono::steady_clock::duration> const &timeout=std::nullopt
);

/////////////////////////////////////////////////////////////////////////
///  \fn            Execute
///  \brief         Returns multiple results.
///
std::tuple<ExecuteResultValue, ResultSystemUniquePtrs> Execute(
    Configuration &config,
    Observer &observer,
    WorkingSystem const &initial,
    size_t maxNumResults,
    std::optional<std::chrono::steady_clock::duration> const &timeout=std::nullopt
);

template <typename WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT>
std::tuple<ExecuteResultValue, ResultSystemUniquePtrs> Execute(
    Configuration &config,
    Observer &observer,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT begin,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT end,
    size_t maxNumResults,
    std::optional<std::chrono::steady_clock::duration> const &timeout=std::nullopt
);

/////////////////////////////////////////////////////////////////////////
///  \fn            Execute
///  \brief         Returns results via an observer.
///
ExecuteResultValue Execute(
    Configuration &config,
    ResultObserver &observer,
    WorkingSystem const &initial,
    std::optional<std::chrono::steady_clock::duration> const &timeout=std::nullopt
);

template <typename WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT>
ExecuteResultValue Execute(
    Configuration &config,
    ResultObserver &observer,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT begin,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT end,
    std::optional<std::chrono::steady_clock::duration> const &timeout=std::nullopt
);

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Implementation
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
namespace Details {

// ----------------------------------------------------------------------
// |
// |  Public Types
// |
// ----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
///  \class         CollectionResultObserver
///  \brief         Observer that writes results to an internal collection.
///
class CollectionResultObserver : public ResultObserver {
public:
    // ----------------------------------------------------------------------
    // |  Public Data
    ResultSystemUniquePtrs                  results;

    // ----------------------------------------------------------------------
    // |  Public Methods
    CollectionResultObserver(Observer &observer, size_t maxNumResults, bool isMultithreaded);
    ~CollectionResultObserver(void) override = default;

    NON_COPYABLE(CollectionResultObserver);
    NON_MOVABLE(CollectionResultObserver);

    // Observer Methods
    bool OnRoundBegin(size_t round, SystemPtrs const &pending) override;
    void OnRoundEnd(size_t round, SystemPtrs const &pending) override;

    bool OnRoundMergingWork(size_t round, SystemPtrsContainer const &pending) override;
    void OnRoundMergedWork(size_t round, SystemPtrs const &pending, SystemPtrsContainer removed) override;

    bool OnTaskBegin(size_t round, size_t task, size_t numTasks) override;
    void OnTaskEnd(size_t round, size_t task, size_t numTasks) override;

    void OnTaskError(size_t round, size_t task, size_t numTasks, std::exception const &ex) override;

    bool OnIterationBegin(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations) override;
    void OnIterationEnd(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations) override;

    bool OnIterationGeneratingWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active) override;
    void OnIterationGeneratedWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &generated) override;

    bool OnIterationMergingWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &generated, SystemPtrs const &pending) override;
    void OnIterationMergedWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &pending, SystemPtrsContainer removed) override;

    void OnIterationFailedSystems(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) override;

    // ResultObserver methods
    bool OnIterationResultSystem(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, ResultSystemUniquePtr pResult) override;

private:
    // ----------------------------------------------------------------------
    // |  Private Types
    using ApplyResultSystemPtr              = size_t (*)(std::mutex &, ResultSystemUniquePtrs &, ResultSystemUniquePtr);

    // ----------------------------------------------------------------------
    // |  Private Data
    Observer &                              _observer;
    size_t const                            _maxNumResults;
    ApplyResultSystemPtr const              _applyResultFunc;

    std::mutex                              _mxResults;

    // ----------------------------------------------------------------------
    // |  Private Methods
    static size_t SingleThreadedApplyResultSystem(std::mutex &m, ResultSystemUniquePtrs &results, ResultSystemUniquePtr pResult);
    static size_t MultiThreadedApplyResultSystem(std::mutex &m, ResultSystemUniquePtrs &results, ResultSystemUniquePtr pResult);
};

// ----------------------------------------------------------------------
// |
// |  Public Methods
// |
// ----------------------------------------------------------------------
ExecuteResultValue ExecuteImpl(Configuration &config, ResultObserver &observer, SystemPtrs working, std::optional<std::chrono::steady_clock::duration> const &timeout);

inline void EmptyDeleter(void const *) {}

} // namespace Details

inline std::tuple<ExecuteResultValue, ResultSystemUniquePtr> Execute(
    Configuration &config,
    Observer &observer,
    WorkingSystem const &initial,
    std::optional<std::chrono::steady_clock::duration> const &timeout/*=std::nullopt*/
) {
    WorkingSystemPtr                        pInitial(&make_mutable(initial), Details::EmptyDeleter);

    return Execute(config, observer, &pInitial, &pInitial + 1, timeout);
}

template <typename WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT>
std::tuple<ExecuteResultValue, ResultSystemUniquePtr> Execute(
    Configuration &config,
    Observer &observer,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT begin,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT end,
    std::optional<std::chrono::steady_clock::duration> const &timeout/*=std::nullopt*/
) {
    std::tuple<ExecuteResultValue, ResultSystemUniquePtrs>                  result(
        Execute(
            config,
            observer,
            begin,
            end,
            1,
            timeout
        )
    );

    if(std::get<1>(result).size() >= 1)
        return std::make_tuple(ExecuteResultValue::Completed, std::move(std::get<1>(result)[0]));

    return std::make_tuple(std::get<0>(result), ResultSystemUniquePtr());
}

inline std::tuple<ExecuteResultValue, ResultSystemUniquePtrs> Execute(
    Configuration &config,
    Observer &observer,
    WorkingSystem const &initial,
    size_t maxNumResults,
    std::optional<std::chrono::steady_clock::duration> const &timeout/*=std::nullopt*/
) {
    WorkingSystemPtr                        pInitial(&make_mutable(initial), Details::EmptyDeleter);

    return Execute(config, observer, &pInitial, &pInitial + 1, maxNumResults, timeout);
}

template <typename WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT>
std::tuple<ExecuteResultValue, ResultSystemUniquePtrs> Execute(
    Configuration &config,
    Observer &observer,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT begin,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT end,
    size_t maxNumResults,
    std::optional<std::chrono::steady_clock::duration> const &timeout/*=std::nullopt*/
) {
    Details::CollectionResultObserver       cro(observer, maxNumResults, config.NumConcurrentTasks > 1);
    ExecuteResultValue                      result(
        Execute(
            config,
            cro,
            begin,
            end,
            timeout
        )
    );

    if(result != ExecuteResultValue::Completed && cro.results.size() >= maxNumResults)
        result = ExecuteResultValue::Completed;

    std::sort(
        cro.results.begin(),
        cro.results.end(),
        [](ResultSystemUniquePtr const &p1, ResultSystemUniquePtr const &p2) {
            return *p1 > *p2;
        }
    );

    if(cro.results.size() != maxNumResults)
        cro.results.resize(maxNumResults);

    return std::make_tuple(std::move(result), config.Finalize(std::move(cro.results)));
}

inline ExecuteResultValue Execute(
    Configuration &config,
    ResultObserver &observer,
    WorkingSystem const &initial,
    std::optional<std::chrono::steady_clock::duration> const &timeout/*=std::nullopt*/
) {
    WorkingSystemPtr                        pInitial(&make_mutable(initial), Details::EmptyDeleter);

    return Execute(config, observer, &pInitial, &pInitial + 1, timeout);
}

template <typename WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT>
ExecuteResultValue Execute(
    Configuration &config,
    ResultObserver &observer,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT begin,
    WorkingSystemOrCalculatedWorkingSystemPtrInputIteratorT end,
    std::optional<std::chrono::steady_clock::duration> const &timeout/*=std::nullopt*/
) {
    SystemPtrs                              ptrs;

    while(begin != end) {
        SystemPtr                           pSystem(std::static_pointer_cast<Components::System>(*begin));

        ptrs.emplace_back(std::move(pSystem));
        ++begin;
    }

    return Details::ExecuteImpl(config, observer, std::move(ptrs), timeout);
}

} // namespace Engine
} // namespace LocalExecution
} // namespace Core
} // namespace DecisionEngine
