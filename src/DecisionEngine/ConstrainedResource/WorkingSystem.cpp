/////////////////////////////////////////////////////////////////////////
///
///  \file          WorkingSystem.cpp
///  \brief         See WorkingSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 11:09:43
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
#include "WorkingSystem.h"

#include "CalculatedResultSystem.h"
#include "CalculatedWorkingSystem.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  WorkingSystem::ImmutableState
// |
// ----------------------------------------------------------------------
WorkingSystem::ImmutableState::ImmutableState(RequestPtrsContainerPtr pRequestsContainer) :
    ImmutableState(
        std::move(pRequestsContainer),
        PermutationGeneratorFactoryPtr(),
        true
    )
{}

WorkingSystem::ImmutableState::ImmutableState(RequestPtrsContainerPtr pRequestsContainer, PermutationGeneratorFactoryPtr pPermutationGeneratorFactory) :
    ImmutableState(
        std::move(pRequestsContainer),
        [&pPermutationGeneratorFactory](void) -> PermutationGeneratorFactoryPtr & {
            ENSURE_ARGUMENT(pPermutationGeneratorFactory);
            return pPermutationGeneratorFactory;
        }(),
        true
    )
{}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
WorkingSystem::ImmutableState::ImmutableState(RequestPtrsContainerPtr pRequestsContainer, PermutationGeneratorFactoryPtr optionalPermutationGeneratorFactory, bool) :
    RequestsContainer(
        std::move(
            [&pRequestsContainer](void) -> RequestPtrsContainerPtr & {
                ENSURE_ARGUMENT(
                    pRequestsContainer,
                    pRequestsContainer
                    && pRequestsContainer->empty() == false
                    && std::all_of(
                        pRequestsContainer->cbegin(),
                        pRequestsContainer->cend(),
                        [](RequestPtrs const &values) {
                            return values.empty() == false
                                && std::all_of(
                                    values.cbegin(),
                                    values.cend(),
                                    [](RequestPtr const &ptr) {
                                        return static_cast<bool>(ptr);
                                    }
                                );
                        }
                    )
                );
                return pRequestsContainer;
            }()
        )
    ),
    OptionalPermutationGeneratorFactory(std::move(optionalPermutationGeneratorFactory))
{}

// ----------------------------------------------------------------------
// |
// |  WorkingSystem::CurrentState
// |
// ----------------------------------------------------------------------
WorkingSystem::CurrentState::CurrentState(ResourcePtr pResource, size_t requestOffset) :
    Resource(
        std::move(
            [&pResource](void) -> ResourcePtr & {
                ENSURE_ARGUMENT(pResource);
                return pResource;
            }()
        )
    ),
    RequestOffset(std::move(requestOffset))
{}

// ----------------------------------------------------------------------
// |
// |  WorkingSystem::TransitionState
// |
// ----------------------------------------------------------------------
WorkingSystem::TransitionState::TransitionState(CurrentStatePtr pCurrentState, ApplyStatePtr pApplyState) :
    CurrentState(
        std::move(
            [&pCurrentState](void) -> CurrentStatePtr & {
                ENSURE_ARGUMENT(pCurrentState);
                return pCurrentState;
            }()
        )
    ),
    OptionalApplyState(
        std::move(
            [&pApplyState](void) -> ApplyStatePtr & {
                ENSURE_ARGUMENT(pApplyState);
                return pApplyState;
            }()
        )
    )
{}

WorkingSystem::TransitionState::TransitionState(CurrentStatePtr pCurrentState, ApplyStatePtr pApplyState, RequestPtrsPtr pPermutedRequests) :
    CurrentState(
        std::move(
            [&pCurrentState](void) -> CurrentStatePtr & {
                ENSURE_ARGUMENT(pCurrentState);
                return pCurrentState;
            }()
        )
    ),
    OptionalApplyState(
        std::move(
            [&pApplyState](void) -> ApplyStatePtr & {
                ENSURE_ARGUMENT(pApplyState);
                return pApplyState;
            }()
        )
    ),
    OptionalPermutedRequests(
        std::move(
            [&pPermutedRequests](void) -> RequestPtrsPtr & {
                ENSURE_ARGUMENT(pPermutedRequests);
                return pPermutedRequests;
            }()
        )
    )
{}

WorkingSystem::TransitionState::TransitionState(CurrentStatePtr pCurrentState, RequestPtrsPtr pPermutedRequests) :
    CurrentState(
        std::move(
            [&pCurrentState](void) -> CurrentStatePtr & {
                ENSURE_ARGUMENT(pCurrentState);
                return pCurrentState;
            }()
        )
    ),
    OptionalPermutedRequests(
        std::move(
            [&pPermutedRequests](void) -> RequestPtrsPtr & {
                ENSURE_ARGUMENT(pPermutedRequests);
                return pPermutedRequests;
            }()
        )
    )
{}

// ----------------------------------------------------------------------
// |
// |  WorkingSystem
// |
// ----------------------------------------------------------------------
WorkingSystem::WorkingSystem(RequestPtrsContainerPtr pRequestsContainer, ResourcePtr pResource) :
    _pInitialState(std::make_shared<ImmutableState>(std::move(pRequestsContainer))),
    _pCurrentState(std::make_shared<CurrentState>(std::move(pResource), 0)),
    _state(InitializedType())
{
    FinalConstruct();
}

WorkingSystem::WorkingSystem(RequestPtrsContainerPtr pRequestsContainer, ResourcePtr pResource, PermutationGeneratorFactoryPtr pPermutationGeneratorFactory) :
    _pInitialState(std::make_shared<ImmutableState>(std::move(pRequestsContainer), std::move(pPermutationGeneratorFactory))),
    _pCurrentState(std::make_shared<CurrentState>(std::move(pResource), 0)),
    _state(InitializedType())
{
    FinalConstruct();
}

WorkingSystem::WorkingSystem(RequestPtrs pRequests, ResourcePtr pResource) :
    WorkingSystem(
        std::make_shared<RequestPtrsContainer>(RequestPtrsContainer{ std::move(pRequests) }),
        std::move(pResource)
    )
{}

WorkingSystem::WorkingSystem(RequestPtrs pRequests, ResourcePtr pResource, PermutationGeneratorFactoryPtr pPermutationGeneratorFactory) :
    WorkingSystem(
        std::make_shared<RequestPtrsContainer>(RequestPtrsContainer{ std::move(pRequests) }),
        std::move(pResource),
        std::move(pPermutationGeneratorFactory)
    )
{}

WorkingSystem::WorkingSystem(RequestPtr pRequest, ResourcePtr pResource) :
    WorkingSystem(
        RequestPtrs{ std::move(pRequest) },
        std::move(pResource)
    )
{}

WorkingSystem::WorkingSystem(RequestPtr pRequest, ResourcePtr pResource, PermutationGeneratorFactoryPtr pPermutationGeneratorFactory) :
    WorkingSystem(
        RequestPtrs{ std::move(pRequest) },
        std::move(pResource),
        std::move(pPermutationGeneratorFactory)
    )
{}

// Ctor called by CalculatedWorkingSystem
WorkingSystem::WorkingSystem(ImmutableStatePtr pImmutableState, TransitionState transition, Score score, Index index) :
    Core::Components::WorkingSystem(std::move(score), std::move(index)),
    _pInitialState(
        std::move(
            [&pImmutableState](void) -> ImmutableStatePtr & {
                ENSURE_ARGUMENT(pImmutableState);
                return pImmutableState;
            }()
        )
    ),
    _state(InitializedType())
{
    if(transition.OptionalApplyState)
        make_mutable(_pCurrentState) = std::make_shared<CurrentState>(
            transition.CurrentState->Resource->Apply(*transition.OptionalApplyState),
            transition.CurrentState->RequestOffset + 1
        );
    else
        make_mutable(_pCurrentState) = transition.CurrentState;

    if(transition.OptionalPermutedRequests)
        _state = transition.OptionalPermutedRequests;

    FinalConstruct();
}

std::string WorkingSystem::ToString(void) const /*override*/ {
    return boost::str(
        boost::format("ConstrainedResource::WorkingSystem(%1%,%2%)")
            % GetScore().ToString()
            % GetIndex().ToString()
    );
}

bool WorkingSystem::IsComplete(void) const /*override*/ {
    return boost::get<CompletedType>(&_state) != nullptr;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
void WorkingSystem::FinalConstruct(void) {
    size_t                                  requestOffset(_pCurrentState->RequestOffset);
    RequestPtrsContainer::const_iterator    pRequests(_pInitialState->RequestsContainer->cbegin());

    while(requestOffset >= pRequests->size()) {
        requestOffset -= pRequests->size();
        ++pRequests;
    }

    assert(pRequests != _pInitialState->RequestsContainer->cend());
    assert(requestOffset < pRequests->size());

    make_mutable(_requestsIndex) = static_cast<size_t>(std::distance(_pInitialState->RequestsContainer->cbegin(), pRequests));
    make_mutable(_requestIndex) = requestOffset;
    make_mutable(_atLastRequests) = _requestsIndex == _pInitialState->RequestsContainer->size() - 1;
    make_mutable(_atLastRequest) = _requestsIndex == pRequests->size() - 1;
}

WorkingSystem::SystemPtrs WorkingSystem::GenerateChildrenImpl(size_t maxNumChildren) /*override*/ {
    assert(maxNumChildren);
    assert(IsComplete() == false);

    // ----------------------------------------------------------------------
    struct Internal {
        static void ApplyPermutations(
            WorkingSystem &ws,
            size_t maxNumChildren,
            SystemPtrs &results,
            RequestPtrs const &requests,
            ActivePermutationsInfo::PermutationGeneratorPtr pPermutationGenerator,
            size_t permutationIndex
        ) {
            assert(maxNumChildren);
            assert(pPermutationGenerator);

            PermutationGenerator::RequestPtrsPtrs       permutations(pPermutationGenerator->Generate(requests, maxNumChildren));

            for(auto &permutation : permutations)
                results.emplace_back(
                    std::make_shared<CalculatedWorkingSystem>(
                        ws._pInitialState,
                        TransitionState(ws._pCurrentState, std::move(permutation)),
                        ws.GetScore().Copy(),
                        Index(ws.GetIndex(), permutationIndex++)
                    )
                );

            if(pPermutationGenerator->IsComplete() == false)
                ws._state = ActivePermutationsInfo(std::move(pPermutationGenerator), permutationIndex);
            else
                ws._state = CompletedType();
        }

        static void ApplyEvaluations(
            WorkingSystem &ws,
            size_t maxNumChildren,
            SystemPtrs &results,
            Request const &request,
            Resource::ContinuationStatePtr pOptionalContinuationState,
            size_t evaluationIndex
        ) {
            assert(maxNumChildren);

            Resource::EvaluateResult        result(
                [&ws, &request, &maxNumChildren, &pOptionalContinuationState](void) {
                    if(pOptionalContinuationState)
                        return ws._pCurrentState->Resource->Evaluate(
                            request,
                            maxNumChildren,
                            *pOptionalContinuationState
                        );

                    return ws._pCurrentState->Resource->Evaluate(request, maxNumChildren);
                }()
            );

            Resource::Evaluations &                     evaluations(std::get<0>(result));
            Resource::ContinuationStatePtr &            pContinuationState(std::get<1>(result));

            assert(evaluations.empty() == false);

            for(auto &evaluation : evaluations) {
                Score                       newScore(ws.GetScore(), std::move(evaluation.Result), ws._atLastRequest);
                Index                       newIndex(ws.GetIndex(), evaluationIndex++);

                if(ws._atLastRequest && ws._atLastRequests) {
                    // All done; create a ResultSystem
                    results.emplace_back(
                        std::make_shared<CalculatedResultSystem>(
                            ws._pCurrentState->Resource,
                            std::move(evaluation.ApplyState),
                            ws._pInitialState->RequestsContainer,
                            std::move(newScore),
                            std::move(newIndex)
                        )
                    );
                }
                else if(auto *requestPtrs = boost::get<RequestPtrsPtr>(&ws._state)) {
                    // If here, we just completed a Request within a permutation. Pass the permutation on
                    // if there are more Requests remaining.

                    if(ws._atLastRequest == false) {
                        results.emplace_back(
                            std::make_shared<CalculatedWorkingSystem>(
                                ws._pInitialState,
                                TransitionState(
                                    ws._pCurrentState,
                                    std::move(evaluation.ApplyState),
                                    *requestPtrs
                                ),
                                std::move(newScore),
                                std::move(newIndex)
                            )
                        );
                    }
                }
                else {
                    // Pass on the results. An invocation to the resulting CalculatedWorkingSystem will
                    // determine what should come next.
                    results.emplace_back(
                        std::make_shared<CalculatedWorkingSystem>(
                            ws._pInitialState,
                            TransitionState(
                                ws._pCurrentState,
                                std::move(evaluation.ApplyState)
                            ),
                            std::move(newScore),
                            std::move(newIndex)
                        )
                    );
                }
            }

            if(pContinuationState)
                ws._state = ContinuationInfo(std::move(pContinuationState), evaluationIndex);
            else
                ws._state = CompletedType();
        }
    };
    // ----------------------------------------------------------------------

    SystemPtrs                              results;

    if(auto *ppRequestPtrs = boost::get<RequestPtrsPtr>(&_state)) {
        // If here, we are extracting results for a request that is part of a permutation
        RequestPtrs const &                 requests(**ppRequestPtrs);
        Request const &                     request(*requests[_requestIndex]);

        Internal::ApplyEvaluations(
            *this,
            maxNumChildren,
            results,
            request,
            Resource::ContinuationStatePtr(),
            0
        );
    }
    else {
        RequestPtrs const &                 requests((*_pInitialState->RequestsContainer)[_requestsIndex]);
        Request const &                     request(*requests[_requestIndex]);

        if(boost::get<InitializedType>(&_state)) {
            // This is the first time that this WorkingSystem has been asked to generate children.
            // Either generate the permutation to be used for this system or jump straight to evaluation.
            if(requests.size() == 1 || !_pInitialState->OptionalPermutationGeneratorFactory) {
                // No permutation necessary
                Internal::ApplyEvaluations(
                    *this,
                    maxNumChildren,
                    results,
                    request,
                    Resource::ContinuationStatePtr(),
                    0
                );
            }
            else {
                // Generate Permutations
                Internal::ApplyPermutations(
                    *this,
                    maxNumChildren,
                    results,
                    requests,
                    _pInitialState->OptionalPermutationGeneratorFactory->Create(),
                    0
                );
            }
        }
        else if(auto *activePermutationInfo = boost::get<ActivePermutationsInfo>(&_state)) {
            // Generate more permutations
            Internal::ApplyPermutations(
                *this,
                maxNumChildren,
                results,
                requests,
                std::move(activePermutationInfo->PermutationGenerator),
                std::move(activePermutationInfo->PermutationIndex)
            );
        }
        else if(auto *continuationInfo = boost::get<ContinuationInfo>(&_state)) {
            // Generate more evaluations
            Internal::ApplyEvaluations(
                *this,
                maxNumChildren,
                results,
                request,
                std::move(continuationInfo->ContinuationState),
                std::move(continuationInfo->EvaluationIndex)
            );
        }
        else
            assert(!"Unexpected state");
    }

    return results;
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::WorkingSystem);
