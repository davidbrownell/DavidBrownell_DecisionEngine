/////////////////////////////////////////////////////////////////////////
///
///  \file          Engine.cpp
///  \brief         See Engine.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-06 20:30:12
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
#include "Engine.h"

namespace DecisionEngine {
namespace Core {
namespace LocalExecution {
namespace Engine {
namespace Details {

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Internal Types
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
namespace {

/////////////////////////////////////////////////////////////////////////
///  \class         TaskObserver
///  \brief         BugBug
///
class TaskObserver : public Components::EngineImpl::Observer {
private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    ResultObserver &                        _observer;
    size_t const                            _round;
    size_t const                            _task;
    size_t const                            _numTasks;

    // BugBug: Is this necessary anymore?
    std::atomic<bool> &                     _isCancelled;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    TaskObserver(
        ResultObserver &observer,
        size_t round,
        size_t task,
        size_t numTasks,
        std::atomic<bool> &isCancelled
    ) :
        _observer(observer),
        _round(std::move(round)),
        _task(std::move(task)),
        _numTasks(std::move(numTasks)),
        _isCancelled(isCancelled)
    {}

    NON_COPYABLE(TaskObserver);

#define ARGS                                MEMBERS(_observer, _round, _task, _numTasks, _isCancelled)

    MOVE(TaskObserver, ARGS, FLAGS(MOVE_NO_ASSIGNMENT))

#undef ARGS

    // EngineImpl::Observer Methods
    bool OnBegin(size_t iteration, size_t maxIterations) override {
        if(_observer.OnIterationBegin(_round, _task, _numTasks, iteration, maxIterations) == false) {
            _isCancelled.store(true);
            return false;
        }

        return true;
    }

    void OnEnd(size_t iteration, size_t maxIterations) override {
        _observer.OnIterationEnd(_round, _task, _numTasks, iteration, maxIterations);
    }

    bool OnResultSystem(size_t iteration, size_t maxIterations, ResultSystemUniquePtr pResult) override {
        assert(pResult);

        if(_observer.OnIterationResultSystem(_round, _task, _numTasks, iteration, maxIterations, std::move(pResult)) == false) {
            _isCancelled.store(true);
            return false;
        }

        return true;
    }

    bool OnGeneratingWork(size_t iteration, size_t maxIterations, WorkingSystem const &active) override {
        if(_observer.OnIterationGeneratingWork(_round, _task, _numTasks, iteration, maxIterations, active) == false) {
            _isCancelled.store(true);
            return false;
        }

        return true;
    }

    void OnGeneratedWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &generated) override {
        _observer.OnIterationGeneratedWork(_round, _task, _numTasks, iteration, maxIterations, active, generated);
    }

    bool OnMergingWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &generated, SystemPtrs const &pending) override {
        if(_observer.OnIterationMergingWork(_round, _task, _numTasks, iteration, maxIterations, active, generated, pending) == false) {
            _isCancelled.store(true);
            return false;
        }

        return true;
    }

    void OnMergedWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &pending, SystemPtrsContainer removed) override {
        _observer.OnIterationMergedWork(_round, _task, _numTasks, iteration, maxIterations, active, pending, std::move(removed));
    }

    void OnFailedSystems(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) override {
        _observer.OnIterationFailedSystems(_round, _task, _numTasks, iteration, maxIterations, active, begin, end);
    }

protected:
    // ----------------------------------------------------------------------
    // |
    // |  Protected Methods
    // |
    // ----------------------------------------------------------------------
    ~TaskObserver(void) = default;
};

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Internal Functions
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
ExecuteResultValue DeterministicExecuteImpl(
    Configuration &config,
    ResultObserver &observer,
    SystemPtrs pending,
    std::optional<std::chrono::steady_clock::duration> const &timeout
) {
    // BugBug
}

} // anonymous namespace

// ----------------------------------------------------------------------
// |
// |  CollectionResultObserver
// |
// ----------------------------------------------------------------------
CollectionResultObserver::CollectionResultObserver(Observer &observer, size_t maxNumResults, bool isMultithreaded) :
    _observer(observer),
    _maxNumResults(
        std::move(
            [&maxNumResults](void) -> size_t & {
                ENSURE_ARGUMENT(maxNumResults);
                return maxNumResults;
            }()
        )
    ),
    _applyResultFunc(isMultithreaded ? &MultiThreadedApplyResultSystem : &SingleThreadedApplyResultSystem)
{}

// Observer Methods
bool CollectionResultObserver::OnRoundBegin(size_t round, SystemPtrs const &pending) /*override*/ {
    return _observer.OnRoundBegin(round, pending);
}

void CollectionResultObserver::OnRoundEnd(size_t round, SystemPtrs const &pending) /*override*/ {
    _observer.OnRoundEnd(round, pending);
}

bool CollectionResultObserver::OnRoundMergingWork(size_t round, SystemPtrsContainer const &pending) /*override*/ {
    return _observer.OnRoundMergingWork(round, pending);
}

void CollectionResultObserver::OnRoundMergedWork(size_t round, SystemPtrs const &pending, SystemPtrsContainer removed) /*override*/ {
    _observer.OnRoundMergedWork(round, pending, std::move(removed));
}

bool CollectionResultObserver::OnTaskBegin(size_t round, size_t task, size_t numTasks) /*override*/ {
    return _observer.OnTaskBegin(round, task, numTasks);
}

void CollectionResultObserver::OnTaskEnd(size_t round, size_t task, size_t numTasks) /*override*/ {
    _observer.OnTaskEnd(round, task, numTasks);
}

void CollectionResultObserver::OnTaskError(size_t round, size_t task, size_t numTasks, std::exception const &ex) /*override*/ {
    _observer.OnTaskError(round, task, numTasks, ex);
}

bool CollectionResultObserver::OnIterationBegin(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations) /*override*/ {
    return _observer.OnIterationBegin(round, task, numTasks, iteration, numIterations);
}

void CollectionResultObserver::OnIterationEnd(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations) /*override*/ {
    _observer.OnIterationEnd(round, task, numTasks, iteration, numIterations);
}

bool CollectionResultObserver::OnIterationGeneratingWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active) /*override*/ {
    return _observer.OnIterationGeneratingWork(round, task, numTasks, iteration, numIterations, active);
}

void CollectionResultObserver::OnIterationGeneratedWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &generated) /*override*/ {
    _observer.OnIterationGeneratedWork(round, task, numTasks, iteration, numIterations, active, generated);
}

bool CollectionResultObserver::OnIterationMergingWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &generated, SystemPtrs const &pending) /*override*/ {
    return _observer.OnIterationMergingWork(round, task, numTasks, iteration, numIterations, active, generated, pending);
}

void CollectionResultObserver::OnIterationMergedWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &pending, SystemPtrsContainer removed) /*override*/ {
    _observer.OnIterationMergedWork(round, task, numTasks, iteration, numIterations, active, pending, std::move(removed));
}

void CollectionResultObserver::OnIterationFailedSystems(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) /*override*/ {
    _observer.OnIterationFailedSystems(round, task, numTasks, iteration, numIterations, active, begin, end);
}

// ResultObserver methods
bool CollectionResultObserver::OnIterationResultSystem(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, ResultSystemUniquePtr pResult) /*override*/ {
    UNUSED(round);
    UNUSED(task);
    UNUSED(numTasks);
    UNUSED(iteration);
    UNUSED(numIterations);

    ENSURE_ARGUMENT(pResult);
    return _applyResultFunc(_mxResults, results, std::move(pResult)) < _maxNumResults;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// static
size_t CollectionResultObserver::SingleThreadedApplyResultSystem(std::mutex &, ResultSystemUniquePtrs &results, ResultSystemUniquePtr pResult) {
    results.emplace_back(std::move(pResult));
    return results.size();
}

// static
size_t CollectionResultObserver::MultiThreadedApplyResultSystem(std::mutex &m, ResultSystemUniquePtrs &results, ResultSystemUniquePtr pResult) {
    std::scoped_lock                        lock(m); UNUSED(lock);

    return SingleThreadedApplyResultSystem(m, results, std::move(pResult));
}

// ----------------------------------------------------------------------
// |
// |  Public Methods
// |
// ----------------------------------------------------------------------
ExecuteResultValue ExecuteImpl(Configuration &config, ResultObserver &observer, SystemPtrs working, std::optional<std::chrono::steady_clock::duration> const &timeout) {
    ENSURE_ARGUMENT(working, working.empty() == false);
    ENSURE_ARGUMENT(working, std::all_of(working.cbegin(), working.cend(), [](SystemPtr const &ptr) { return static_cast<bool>(ptr); }));
    ENSURE_ARGUMENT(timeout, timeout || timeout->count());

    if(config.IsDeterministic)
        return DeterministicExecuteImpl(config, observer, std::move(working), timeout);

    throw std::runtime_error("TODO: Need non-deterministic solution");
}

} // namespace Details
} // namespace Engine
} // namespace LocalExecution
} // namespace Core
} // namespace DecisionEngine
