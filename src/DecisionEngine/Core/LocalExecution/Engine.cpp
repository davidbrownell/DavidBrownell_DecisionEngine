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
#include "FingerprinterFactory.h"

#include <DecisionEngine/Core/Components/CalculatedWorkingSystem.h>
#include <DecisionEngine/Core/Components/Fingerprinter.h>
#include <DecisionEngine/Core/Components/WorkingSystem.h>

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
///  \brief         Observer for the events associated with a single task.
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
    std::atomic<bool>                       _isCancelled;

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
        size_t numTasks
    ) :
        _observer(observer),
        _round(std::move(round)),
        _task(std::move(task)),
        _numTasks(std::move(numTasks)),
        _isCancelled(false)
    {}

    ~TaskObserver(void) override = default;

    NON_COPYABLE(TaskObserver);

#define ARGS                                MEMBERS(_observer, _round, _task, _numTasks)

    MOVE(TaskObserver, ARGS, FLAGS(MOVE_NO_ASSIGNMENT))

#undef ARGS

    bool IsCancelled(void) const {
        return _isCancelled;
    }

    // EngineImpl::Observer Methods
    bool OnBegin(size_t iteration, size_t maxIterations) override {
        if(_observer.OnIterationBegin(_round, _task, _numTasks, iteration, maxIterations) == false) {
            _isCancelled = true;
            return false;
        }

        return true;
    }

    void OnEnd(size_t iteration, size_t maxIterations) override {
        _observer.OnIterationEnd(_round, _task, _numTasks, iteration, maxIterations);
    }

    bool OnGeneratingWork(size_t iteration, size_t maxIterations, WorkingSystem const &active) override {
        if(_observer.OnIterationGeneratingWork(_round, _task, _numTasks, iteration, maxIterations, active) == false) {
            _isCancelled = true;
            return false;
        }

        return true;
    }

    void OnGeneratedWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &generated) override {
        _observer.OnIterationGeneratedWork(_round, _task, _numTasks, iteration, maxIterations, active, generated);
    }

    bool OnMergingWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &generated, SystemPtrs const &pending) override {
        if(_observer.OnIterationMergingWork(_round, _task, _numTasks, iteration, maxIterations, active, generated, pending) == false) {
            _isCancelled = true;
            return false;
        }

        return true;
    }

    void OnMergedWork(size_t iteration, size_t maxIterations, WorkingSystem const &active, SystemPtrs const &pending, SystemPtrsContainer removed) override {
        _observer.OnIterationMergedWork(_round, _task, _numTasks, iteration, maxIterations, active, pending, std::move(removed));
    }

    bool OnFailedSystems(size_t iteration, size_t maxIterations, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) override {
        if(_observer.OnIterationFailedSystems(_round, _task, _numTasks, iteration, maxIterations, begin, end) == false) {
            _isCancelled = true;
            return false;
        }

        return true;
    }

    bool OnSuccessfulSystems(size_t iteration, size_t maxIterations, ResultSystemUniquePtrs results) override {
        assert(results.empty() == false);

        if(_observer.OnIterationResultSystems(_round, _task, _numTasks, iteration, maxIterations, std::move(results)) == false) {
            _isCancelled = true;
            return false;
        }

        return true;
    }
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
    // ----------------------------------------------------------------------
    using Fingerprinter                                 = Components::Fingerprinter;

    using ProcessWorkingItemsFuncArgs                   = std::tuple<size_t, size_t, size_t, SystemPtr>;
    using ProcessWorkingItemsFuncArgsContainer          = std::vector<ProcessWorkingItemsFuncArgs>;
    // ----------------------------------------------------------------------

    // Create the fingerprinter based on configuration information
    std::unique_ptr<Fingerprinter>          pFingerprinter;
    Fingerprinter &                         fingerprinter(
        [&config, &pFingerprinter](void) -> Fingerprinter & {
            FingerprinterFactory * const    pFingerprinterFactory(reinterpret_cast<FingerprinterFactory *>(config.QueryInterface(FingerprinterFactory::ID)));

            if(pFingerprinterFactory != nullptr)
                pFingerprinter = pFingerprinterFactory->Create();
            else
                pFingerprinter = std::make_unique<Components::NoopFingerprinter>();

            return *pFingerprinter;
        }()
    );

    // Create the function used to determine if time has expired
    std::function<bool (void)> const        hasTimeExpiredFunc(
        [&timeout](void) -> std::function<bool (void)> {
            if(timeout) {
                std::chrono::steady_clock::time_point const                 now(std::chrono::steady_clock::now());
                std::chrono::steady_clock::time_point const                 endTime(now + *timeout);

                ENSURE_ARGUMENT(timeout, now <= endTime);

                return
                    [endTime=std::move(endTime)](void) {
                        return std::chrono::steady_clock::now() >= endTime;
                    };
            }

            return [](void) { return false; };
        }()
    );

    // Create the function used to process working systems
    std::atomic<bool>                       isCancelled(false);
    auto const                              executeTaskFunc(
        [
            &config,
            &observer,
            &fingerprinter,
            &isCancelled
        ](
            size_t round,
            size_t taskIndex,
            size_t numTasks,
            SystemPtr pSystem
        ) -> SystemPtrs {
            assert(pSystem->Type == Components::System::TypeValue::Working);

            WorkingSystemPtr                pWorkingSystem(
                [&pSystem](void) {
                    if(pSystem->Completion == Components::System::CompletionValue::Calculated) {
                        assert(std::dynamic_pointer_cast<Components::CalculatedWorkingSystem>(pSystem));
                        return std::static_pointer_cast<Components::CalculatedWorkingSystem>(pSystem)->Commit();
                    }
                    else if(pSystem->Completion == Components::System::CompletionValue::Concrete) {
                        assert(std::dynamic_pointer_cast<Components::WorkingSystem>(pSystem));
                        return std::static_pointer_cast<Components::WorkingSystem>(pSystem);
                    }

                    assert(!"Unexpected CompletionValue");
                    return WorkingSystemPtr();
                }()
            );
            assert(pWorkingSystem);

            if(observer.OnTaskBegin(round, taskIndex, numTasks) == false)
                return SystemPtrs();

            FINALLY([&observer, round, taskIndex, numTasks](void) { observer.OnTaskEnd(round, taskIndex, numTasks); });

            try {
                TaskObserver                taskObserver(
                    observer,
                    round,
                    taskIndex,
                    numTasks
                );

                SystemPtrs                  results(
                    Components::EngineImpl::ExecuteTask(
                        fingerprinter,
                        taskObserver,
                        config.GetMaxNumPendingSystems(*pWorkingSystem),
                        config.GetMaxNumChildrenPerGeneration(*pWorkingSystem),
                        config.GetMaxNumIterationsPerRound(*pWorkingSystem),
                        config.ContinueProcessingSystemsWithFailures,
                        std::move(pWorkingSystem)
                    )
                );

                if(taskObserver.IsCancelled())
                    isCancelled = true;

                return results;
            }
            catch(std::exception const &ex) {
                observer.OnTaskError(round, taskIndex, numTasks, ex);
                return SystemPtrs();
            }
        }
    );
    auto const                              executeTaskFuncImpl(
        [&executeTaskFunc](ProcessWorkingItemsFuncArgs const &args) {
            return std::apply(executeTaskFunc, args);
        }
    );

    // Execute the rounds
    Components::ThreadPool                  pool(
        [&config](void) {
            if(config.NumConcurrentTasks)
                return Components::ThreadPool(*config.NumConcurrentTasks);

            return Components::ThreadPool();
        }()
    );
    size_t                                  round(0);

    while(
        isCancelled == false
        && pending.empty() == false
        && hasTimeExpiredFunc() == false
    ) {
        {
            if(observer.OnRoundBegin(round, pending) == false)
                isCancelled = true;

            if(isCancelled)
                continue;

            FINALLY([&observer, &round, &pending](void) { observer.OnRoundEnd(round, pending); });

            // Create the tasks
            size_t const                    numTasks(std::min(pool.NumThreads, pending.size()));

            assert(numTasks);

            ProcessWorkingItemsFuncArgsContainer        allTaskArgs;

            allTaskArgs.reserve(numTasks);

            for(size_t taskIndex = 0; taskIndex < numTasks; ++taskIndex) {
                SystemPtr                   pTaskSystem(pending.front());

                pending.pop_front();

                allTaskArgs.emplace_back(
                    ProcessWorkingItemsFuncArgs(
                        round,
                        taskIndex,
                        numTasks,
                        std::move(pTaskSystem)
                    )
                );
            }

            // Execute the tasks
            SystemPtrsContainer             taskResults(pool.parallel(allTaskArgs, executeTaskFuncImpl));

            assert(taskResults.size() == numTasks);

            if(pending.empty() == false)
                taskResults.emplace_back(std::move(pending));

            // Determine if there is work to complete
            bool const                      hasResults(std::any_of(taskResults.cbegin(), taskResults.cend(), [](SystemPtrs const &ptrs) { return ptrs.empty() == false; }));

            if(hasResults == false)
                continue;

            if(observer.OnRoundMergingWork(round, taskResults) == false) {
                isCancelled = true;
                continue;
            }

            // Merge the results
            {
                SystemPtrsContainer         removed;

                FINALLY([&observer, &round, &pending, &removed](void) { observer.OnRoundMergedWork(round, pending, std::move(removed)); });

                std::tie(pending, removed) = Components::EngineImpl::Merge(config.GetMaxNumPendingSystems(), std::move(taskResults));
            }
        }

        ++round;
    }

    if(pending.empty())
        return ExecuteResultValue::Completed;
    else if(isCancelled)
        return ExecuteResultValue::ExitViaObserver;
    else
        return ExecuteResultValue::Timeout;
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

bool CollectionResultObserver::OnIterationFailedSystems(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) /*override*/ {
    return _observer.OnIterationFailedSystems(round, task, numTasks, iteration, numIterations, begin, end);
}

// ResultObserver methods
bool CollectionResultObserver::OnIterationResultSystems(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, ResultSystemUniquePtrs theseResults) /*override*/ {
    UNUSED(round);
    UNUSED(task);
    UNUSED(numTasks);
    UNUSED(iteration);
    UNUSED(numIterations);

    ENSURE_ARGUMENT(theseResults.empty() == false);

    bool                                    shouldContinue(
        [this, &theseResults](void) {
            if(results.size() + theseResults.size() > _maxNumResults) {
                size_t const                toRemove(results.size() + theseResults.size() - _maxNumResults);

                assert(toRemove);
                assert(toRemove < theseResults.size());

                ResultSystemUniquePtrs::iterator        iter(theseResults.begin());

                std::advance(iter, theseResults.size() - toRemove);
                theseResults.erase(iter, theseResults.end());

                assert(theseResults.empty() == false);

                return false;
            }

            if(results.size() + theseResults.size() == _maxNumResults)
                return false;

            return true;
        }()
    );

    _applyResultFunc(_mxResults, results, std::move(theseResults));

    return shouldContinue;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// static
void CollectionResultObserver::SingleThreadedApplyResultSystem(std::mutex &, ResultSystemUniquePtrs &results, ResultSystemUniquePtrs theseResults) {
    for(auto &pResult : theseResults)
        results.emplace_back(std::move(pResult));
}

// static
void CollectionResultObserver::MultiThreadedApplyResultSystem(std::mutex &m, ResultSystemUniquePtrs &results, ResultSystemUniquePtrs theseResults) {
    std::scoped_lock<decltype(m)>           lock(m); UNUSED(lock);

    SingleThreadedApplyResultSystem(m, results, std::move(theseResults));
}

// ----------------------------------------------------------------------
// |
// |  Public Methods
// |
// ----------------------------------------------------------------------
ExecuteResultValue ExecuteImpl(Configuration &config, ResultObserver &observer, SystemPtrs working, std::optional<std::chrono::steady_clock::duration> const &timeout) {
    ENSURE_ARGUMENT(working, working.empty() == false);
    ENSURE_ARGUMENT(working, std::all_of(working.cbegin(), working.cend(), [](SystemPtr const &ptr) { return static_cast<bool>(ptr); }));
    ENSURE_ARGUMENT(timeout, !timeout || timeout->count());

    if(config.IsDeterministic)
        return DeterministicExecuteImpl(config, observer, std::move(working), timeout);

    throw std::runtime_error("TODO: Need non-deterministic solution");
}

} // namespace Details
} // namespace Engine
} // namespace LocalExecution
} // namespace Core
} // namespace DecisionEngine
