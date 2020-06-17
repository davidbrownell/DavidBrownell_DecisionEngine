/////////////////////////////////////////////////////////////////////////
///
///  \file          EngineImpl.cpp
///  \brief         See EngineImpl.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-29 15:54:04
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
#include "EngineImpl.h"

#include "CalculatedResultSystem.h"
#include "CalculatedWorkingSystem.h"
#include "Fingerprinter.h"
#include "ResultSystem.h"
#include "System.h"
#include "WorkingSystem.h"

namespace DecisionEngine {
namespace Core {
namespace Components {
namespace EngineImpl {

// ----------------------------------------------------------------------
// |
// |  Internal Function Prototypes
// |
// ----------------------------------------------------------------------
bool Sorter(SystemPtr const &p1, SystemPtr const &p2);

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
SystemPtrs ExecuteRound(
    Fingerprinter &fingerprinter,
    Observer &observer,
    size_t maxNumPendingSystems,
    size_t maxNumChildrenPerGeneration,
    size_t maxNumIterations,
    bool continueProcessingSystemWithFailures,
    std::atomic<bool> const &isCancelled,
    WorkingSystemPtr pInitial,
    std::optional<std::tuple<ThreadPool &, DynamicScoreFunctor const &>> const &dynamicScoreInfo
) {
    // ----------------------------------------------------------------------
    struct Internal {
        static bool ProcessResultSystem(
            Fingerprinter &fingerprinter,
            Observer &observer,
            size_t iteration,
            size_t maxNumIterations,
            System &system
        ) {
            assert(system.Type == System::TypeValue::Result);
            assert(dynamic_cast<CalculatedResultSystem *>(&system));

            CalculatedResultSystem &        result(static_cast<CalculatedResultSystem &>(system));

            if(fingerprinter.ShouldProcess(result) == false)
                return true;

            CalculatedResultSystem::ResultSystemUniquePtr                   pResult(result.Commit());

            assert(pResult);

            if(fingerprinter.ShouldProcess(*pResult) == false)
                return true;

            return observer.OnResultSystem(iteration, maxNumIterations, std::move(pResult));
        }
    };
    // ----------------------------------------------------------------------

    ENSURE_ARGUMENT(maxNumPendingSystems);
    ENSURE_ARGUMENT(maxNumChildrenPerGeneration);
    ENSURE_ARGUMENT(maxNumIterations);
    ENSURE_ARGUMENT(pInitial);

    SystemPtrs                              pending;

    for(size_t iteration = 0; iteration < maxNumIterations; ++iteration) {
        if(isCancelled.load())
            break;

        if(observer.OnBegin(iteration, maxNumIterations) == false)
            break;

        FINALLY([&observer, &iteration, &maxNumIterations](void) { observer.OnEnd(iteration, maxNumIterations); });

        // Get the initial WorkingSystem
        while(!pInitial && pending.empty() == false) {
            System &                        system(**pending.begin());

            if(system.Type == System::TypeValue::Result) {
                if(
                    Internal::ProcessResultSystem(
                        fingerprinter,
                        observer,
                        iteration,
                        maxNumIterations,
                        system
                    ) == false
                )
                    break;
            }
            else if(system.Type == System::TypeValue::Working) {
                if(system.Completion == System::CompletionValue::Concrete) {
                    assert(std::dynamic_pointer_cast<WorkingSystem>(*pending.begin()));
                    pInitial = std::static_pointer_cast<WorkingSystem>(*pending.begin());
                }
                else if(system.Completion == System::CompletionValue::Calculated) {
                    assert(dynamic_cast<CalculatedWorkingSystem *>(&system));
                    assert(fingerprinter.ShouldProcess(system));

                    pInitial = static_cast<CalculatedWorkingSystem &>(system).Commit();
                }
                else
                    assert(!"Unexpected CompletionValue");
            }
            else
                assert(!"Unexpected TypeValue");

            pending.pop_front();
        }

        if(!pInitial)
            break;

        FINALLY([&pInitial](void) { pInitial.reset(); });

        if(fingerprinter.ShouldProcess(*pInitial) == false)
            continue;

        // Generate the work
        if(observer.OnGeneratingWork(iteration, maxNumIterations, *pInitial) == false)
            break;

        SystemPtrs                          generated(pInitial->GenerateChildren(maxNumChildrenPerGeneration));

        {
            FINALLY([&observer, &iteration, &maxNumIterations, &pInitial, &generated](void) { observer.OnGeneratedWork(iteration, maxNumIterations, *pInitial, generated); });

            assert(generated.empty() == false);
            assert(generated.size() <= maxNumChildrenPerGeneration);

            if(pInitial->IsComplete() == false)
                generated.emplace_back(pInitial);

            std::sort(generated.begin(), generated.end(), Sorter);

            // Remove the ResultSystems that are at the front of the sorted queue (if any)
            bool                            should_continue(true);

            while(generated.empty() == false && (*generated.cbegin())->Type == System::TypeValue::Result) {
                if(
                    Internal::ProcessResultSystem(
                        fingerprinter,
                        observer,
                        iteration,
                        maxNumIterations,
                        **generated.begin()
                    )
                ) {
                    should_continue = false;
                    break;
                }

                generated.pop_front();
            }

            if(should_continue == false)
                break;

            if(generated.empty())
                continue;

            // Remove all generated work that has failed (if necessary)
            if(continueProcessingSystemWithFailures == false && (*generated.crbegin())->GetScore().IsSuccessful == false) {
                // If here, we know that the last generated system failed. Find
                // the first system that failed so that we can call the observer
                // with them all at the same time.

                // It would be nice to use a binary search here, but we don't have a full
                // key to use for comparison (which is required for a stable sort).
                SystemPtrs::const_reverse_iterator      iter(generated.crbegin() + 1);

                while(iter != generated.crend() && (*iter)->GetScore().IsSuccessful == false)
                    ++iter;

                // Convert the reverse iterator into a forward iterator
                SystemPtrs::const_iterator const        iFirstFailure(generated.cend() - std::distance(generated.crbegin(), iter));

                assert(iFirstFailure != generated.cbegin());
                assert(std::all_of(generated.cbegin(), iFirstFailure, [](SystemPtr const &ptr) { return ptr->GetScore().IsSuccessful; }));
                assert(std::all_of(iFirstFailure, generated.cend(), [](SystemPtr const &ptr) { return ptr->GetScore().IsSuccessful == false; }));

                observer.OnFailedSystems(
                    iteration,
                    maxNumIterations,
                    *pInitial,
                    iFirstFailure,
                    generated.cend()
                );

                generated.erase(iFirstFailure, generated.cend());

                if(generated.empty())
                    continue;
            }

            // Remove by fingerprinter
            if(dynamic_cast<NoopFingerprinter *>(&fingerprinter) == nullptr) {
                SystemPtrs::const_iterator  iter(generated.begin());

                while(iter != generated.end()) {
                    if(fingerprinter.ShouldProcess(**iter) == false)
                        iter = generated.erase(iter);
                    else
                        ++iter;
                }

                if(generated.empty())
                    continue;
            }
        }

        // Merge the generated work with the pending work
        {
            SystemPtrsContainer             removed;

            if(observer.OnMergingWork(iteration, maxNumIterations, *pInitial, generated, pending) == false)
                break;

            FINALLY([&observer, &iteration, &maxNumIterations, &pInitial, &pending, &removed](void) { observer.OnMergedWork(iteration, maxNumIterations, *pInitial, pending, std::move(removed)); });

            // Apply the dynamic score
            std::tie(pending, removed) = Merge(
                maxNumPendingSystems,
                SystemPtrsContainer{ std::move(generated), std::move(pending) },
                dynamicScoreInfo
            );
        }
    }

    return pending;
}

std::tuple<SystemPtrs, SystemPtrsContainer> Merge(
    size_t maxNumSystems,
    SystemPtrsContainer items,
    std::optional<std::tuple<ThreadPool &, DynamicScoreFunctor const &>> const &dynamicScoreInfo/*=std::nullopt*/
) {
    // ----------------------------------------------------------------------
    struct Internal {
        static bool AreValidItems(SystemPtrsContainer const &items) {
            return items.empty() == false
                && std::all_of(
                    items.cbegin(),
                    items.cend(),
                    [](SystemPtrs const &values) {
                        return values.empty()
                            || std::all_of(
                                values.cbegin(),
                                values.cend(),
                                [](SystemPtr const &ptr) {
                                    return static_cast<bool>(ptr);
                                }
                            );
                    }
                );
        }

        static SystemPtrs & FindGreatest(SystemPtrsContainer items, SystemPtrs const *pIgnore=nullptr) {
            SystemPtrs *                    pGreatest(nullptr);

            for(SystemPtrs & potential : items) {
                if(potential.empty())
                    continue;

                if(&potential == pIgnore)
                    continue;

                if(pGreatest == nullptr || Sorter(*potential.cbegin(), *pGreatest->cbegin()))
                    pGreatest = &potential;
            }

            assert(pGreatest != nullptr);

            return *pGreatest;
        }
    };
    // ----------------------------------------------------------------------

    ENSURE_ARGUMENT(maxNumSystems);
    ENSURE_ARGUMENT(items, Internal::AreValidItems(items));

    if(dynamicScoreInfo) {
        // ----------------------------------------------------------------------
        using Futures                       = std::vector<CommonHelpers::ThreadPoolFuture<bool>>;

        struct DynamicScoreInternal {
            static bool Execute(DynamicScoreFunctor const &scoreFunc, SystemPtrs &ptrs) {
                bool                        hasChanged(false);

                for(SystemPtr & ptr : ptrs) {
                    Score                   newScore(scoreFunc(*ptr, ptr->GetScore()));

                    if(newScore != ptr->GetScore()) {
                        ptr->UpdateScore(std::move(newScore));
                        hasChanged = true;
                    }
                }

                if(hasChanged)
                    std::sort(ptrs.begin(), ptrs.end(), Sorter);

                return hasChanged;
            }
        };
        // ----------------------------------------------------------------------

        ThreadPool &                        pool(std::get<0>(*dynamicScoreInfo));
        DynamicScoreFunctor const &         scoreFunc(std::get<1>(*dynamicScoreInfo));
        Futures                             futures;

        futures.reserve(items.size());

        for(SystemPtrs &ptrs : items) {
            futures.emplace_back(
                pool.enqueue_task(
                    [&scoreFunc, &ptrs](void) {
                        return DynamicScoreInternal::Execute(scoreFunc, ptrs);
                    }
                )
            );
        }

        for(auto & future : futures)
            future.wait();
    }

#if (defined DEBUG)
    for(auto const &systemPtrs : items) {
        assert(std::is_sorted(systemPtrs.cbegin(), systemPtrs.cend(), Sorter));
    }
#endif // DEBUG

    size_t                                  numSystemPtrsRemaining(std::accumulate(items.cbegin(), items.cend(), static_cast<size_t>(0), [](size_t total, SystemPtrs const &ptrs) { return total + ptrs.size(); }));
    size_t                                  numSystemsRemaining(std::min(maxNumSystems, std::accumulate(items.cbegin(), items.cend(), static_cast<size_t>(0), [](size_t total, SystemPtrs const &ptrs) { return total + (ptrs.empty() ? 0 : 1); })));

    assert(numSystemPtrsRemaining);
    assert(numSystemsRemaining);

    SystemPtrs                              results;
    SystemPtrs *                            pGreatest(&Internal::FindGreatest(items));

    for(;;) {
        if(numSystemPtrsRemaining == 0 || results.size() == maxNumSystems)
            break;

        SystemPtrs *                        pNextGreatest(nullptr);
        SystemPtrs &                        greatest(*pGreatest);
        SystemPtrs::const_iterator          iEndCopy(greatest.cbegin());

        std::advance(iEndCopy, std::min(numSystemsRemaining, greatest.size()));

        // If there are multiple containers remaining, find the container
        // that has the greatest value. No need to do this if there is only
        // 1 container remaining.
        if(numSystemPtrsRemaining != 1) {
            pNextGreatest = &Internal::FindGreatest(items, &greatest);

            iEndCopy = std::upper_bound(
                greatest.cbegin(),
                iEndCopy,
                *pNextGreatest->cbegin(),
                Sorter
            );
        }

        size_t const                        toCopy(static_cast<size_t>(std::distance(greatest.cbegin(), iEndCopy)));

        assert(toCopy);
        assert(toCopy <= greatest.size());
        assert(toCopy <= numSystemsRemaining);

        numSystemsRemaining -= toCopy;

        // Move the items from the results container
        for(SystemPtrs::iterator iter = greatest.begin(); iter != iEndCopy; ++iter)
            results.emplace_back(std::move(*iter));

        // Remove the items from the original container
        greatest.erase(greatest.begin(), iEndCopy);

        if(greatest.empty())
            --numSystemPtrsRemaining;

        if(pNextGreatest)
            pGreatest = pNextGreatest;
    }

    assert(std::is_sorted(results.cbegin(), results.cend(), Sorter));

    // Remove the empty items
    SystemPtrsContainer::iterator           iter(items.begin());

    while(iter != items.end()) {
        if(iter->empty())
            iter = items.erase(iter);
        else
            ++iter;
    }

    return std::make_tuple(std::move(results), std::move(items));
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
bool Sorter(SystemPtr const &p1, SystemPtr const &p2) {
    // Higher potential is better than lower potential
    return *p1 > *p2;
}

} // namespace EngineImpl
} // namespace Components
} // namespace Core
} // namespace DecisionEngine
