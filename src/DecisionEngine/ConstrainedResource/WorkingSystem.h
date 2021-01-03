/////////////////////////////////////////////////////////////////////////
///
///  \file          WorkingSystem.h
///  \brief         Contains the WorkingSystem object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 11:10:12
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

#include "ConstrainedResource.h"
#include "PermutationGenerator.h"
#include "PermutationGeneratorFactory.h"
#include "Request.h"
#include "Resource.h"

#include <DecisionEngine/Core/Components/WorkingSystem.h>

namespace DecisionEngine {
namespace ConstrainedResource {

/////////////////////////////////////////////////////////////////////////
///  \class         WorkingSystem
///  \brief         Contains a Resource and zero or more applied Requests.
///
class WorkingSystem : public Core::Components::WorkingSystem {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using PermutationGeneratorFactoryPtr    = std::shared_ptr<PermutationGeneratorFactory>;

    using Score                             = Core::Components::Score;
    using Index                             = Core::Components::Index;

    /////////////////////////////////////////////////////////////////////////
    ///  \class         ImmutableState
    ///  \brief         Contains information created when instantiating the original
    ///                 WorkingSystem; this information remains the same for all
    ///                 generated children.
    ///
    class ImmutableState {
    public:
        // ----------------------------------------------------------------------
        // |  Public Data
        RequestPtrsContainerPtr const                   RequestsContainer;
        PermutationGeneratorFactoryPtr const            OptionalPermutationGeneratorFactory;

        // ----------------------------------------------------------------------
        // |  Public Methods
        ImmutableState(RequestPtrsContainerPtr pRequestsContainer);
        ImmutableState(RequestPtrsContainerPtr pRequestsContainer, PermutationGeneratorFactoryPtr pPermutationGeneratorFactory);

#define ARGS                                MEMBERS(RequestsContainer, OptionalPermutationGeneratorFactory)

        NON_COPYABLE(ImmutableState);
        MOVE(ImmutableState);
        COMPARE(ImmutableState);
        SERIALIZATION(ImmutableState, ARGS);

#undef ARGS

    private:
        // ----------------------------------------------------------------------
        // |  Private Method
        ImmutableState(RequestPtrsContainerPtr pRequestsContainer, PermutationGeneratorFactoryPtr optionalPermutationGeneratorFactory, bool);
    };

    /////////////////////////////////////////////////////////////////////////
    ///  \class         CurrentState
    ///  \brief         Contains information about the WorkingSystem in its
    ///                 current state; this information will be unique for each
    ///                 WorkingSystem.
    ///
    class CurrentState {
    public:
        // ----------------------------------------------------------------------
        // |  Public Data
        ResourcePtr const                   Resource;
        size_t const                        RequestOffset;

        // ----------------------------------------------------------------------
        // |  Public Methods
        CurrentState(ResourcePtr pResource, size_t requestOffset);

#define ARGS                                MEMBERS(Resource, RequestOffset)

        NON_COPYABLE(CurrentState);
        MOVE(CurrentState, ARGS);
        COMPARE(CurrentState, ARGS);
        SERIALIZATION(CurrentState, ARGS);

#undef ARGS
    };

    using ImmutableStatePtr                 = std::shared_ptr<ImmutableState>;
    using CurrentStatePtr                   = std::shared_ptr<CurrentState>;

    /////////////////////////////////////////////////////////////////////////
    ///  \class         TransitionState
    ///  \brief         State used to transition from a CalculatedWorkingSystem
    ///                 object to a WorkingSystem object.
    ///
    class TransitionState {
    public:
        // ----------------------------------------------------------------------
        // |  Public Types
        using CurrentStatePtr               = WorkingSystem::CurrentStatePtr;
        using ApplyStatePtr                 = Resource::ApplyStatePtr;

        // ----------------------------------------------------------------------
        // |  Public Data
        CurrentStatePtr const               CurrentState;
        ApplyStatePtr const                 OptionalApplyState;
        RequestPtrsPtr const                OptionalPermutedRequests;

        // ----------------------------------------------------------------------
        // |  Public Methods
        TransitionState(CurrentStatePtr pCurrentState, ApplyStatePtr pApplyState);
        TransitionState(CurrentStatePtr pCurrentState, ApplyStatePtr pApplyState, RequestPtrsPtr pPermutedRequests);
        TransitionState(CurrentStatePtr pCurrentState, RequestPtrsPtr pPermutedRequests);

        ~TransitionState(void) = default;

#define ARGS                                MEMBERS(CurrentState, OptionalApplyState, OptionalPermutedRequests)

        NON_COPYABLE(TransitionState);
        MOVE(TransitionState, ARGS);
        COMPARE(TransitionState, ARGS);
        SERIALIZATION(TransitionState, ARGS);

#undef ARGS
    };

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Types (used in public declarations)
    // |
    // ----------------------------------------------------------------------

    // Note that this object must be default constructible and copyable to support serialization via boost::variants
    class ActivePermutationsInfo {
    public:
        // ----------------------------------------------------------------------
        // |  Public Types
        using PermutationGeneratorPtr       = std::shared_ptr<PermutationGenerator>;

        // ----------------------------------------------------------------------
        // |  Public Data
        PermutationGeneratorPtr             PermutationGenerator;
        size_t                              PermutationIndex;

        // ----------------------------------------------------------------------
        // |  Public Methods
        ActivePermutationsInfo(void) = default;

#define ARGS                                MEMBERS(PermutationGenerator, PermutationIndex)

        CONSTRUCTOR(ActivePermutationsInfo, ARGS);
        COPY(ActivePermutationsInfo, ARGS);
        MOVE(ActivePermutationsInfo, ARGS);
        COMPARE(ActivePermutationsInfo, ARGS);
        SERIALIZATION(ActivePermutationsInfo, ARGS);

#undef ARGS
    };

    // Note that this object must be default constructible and copyable to support serialization via boost::variants
    class ContinuationInfo {
    public:
        // ----------------------------------------------------------------------
        // |  Public Data
        Resource::ContinuationStatePtr      ContinuationState;
        size_t                              EvaluationIndex;

        // ----------------------------------------------------------------------
        // |  Public Methods
        ContinuationInfo(void) = default;

#define ARGS                                MEMBERS(ContinuationState, EvaluationIndex)

        CONSTRUCTOR(ContinuationInfo, ARGS);
        COPY(ContinuationInfo, ARGS);
        MOVE(ContinuationInfo, ARGS);
        COMPARE(ContinuationInfo, ARGS);
        SERIALIZATION(ContinuationInfo, ARGS);

#undef ARGS
    };

    // The internal state can be in one of the following mutually exclusive states:
    //
    //      1) Permutations remain
    //      2) Requests remain in the current permutation
    //      3) There are additional evaluations left when applying the Request to the Resource

    using InitializedType                   = bool;
    using CompletedType                     = float;

    // Note that InternalState is a boost::variant (rather than a std::variant) to support serialization.
    // boost::variant requires that types support:
    //      - Default construction
    ///     - Assignment

    using InternalState =
        boost::variant<
            InitializedType,                // Initializing
            ActivePermutationsInfo,         // (1)
            RequestPtrsPtr,                 // (2)
            ContinuationInfo,               // (3)
            CompletedType                   // Completed
        >;

    // ----------------------------------------------------------------------
    // |
    // |  Private Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    ImmutableStatePtr const                 _pInitialState;
    CurrentStatePtr const                   _pCurrentState;
    InternalState                           _state;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    WorkingSystem(RequestPtrsContainerPtr pRequestsContainer, ResourcePtr pResource);
    WorkingSystem(RequestPtrsContainerPtr pRequestsContainer, ResourcePtr pResource, PermutationGeneratorFactoryPtr pPermutationGeneratorFactory);
    WorkingSystem(RequestPtrs pRequests, ResourcePtr pResource);
    WorkingSystem(RequestPtrs pRequests, ResourcePtr pResource, PermutationGeneratorFactoryPtr pPermutationGeneratorFactory);
    WorkingSystem(RequestPtr pRequest, ResourcePtr pResource);
    WorkingSystem(RequestPtr pRequest, ResourcePtr pResource, PermutationGeneratorFactoryPtr pPermutationGeneratorFactory);

    // Ctor called by CalculatedWorkingSystem
    WorkingSystem(ImmutableStatePtr pImmutableState, TransitionState transition, Score score, Index index);

    ~WorkingSystem(void) override = default;

#define ARGS                                MEMBERS(_pInitialState, _pCurrentState, _state), BASES(Core::Components::WorkingSystem)

    NON_COPYABLE(WorkingSystem);
    MOVE(WorkingSystem, ARGS);
    COMPARE(WorkingSystem, ARGS);
    SERIALIZATION(WorkingSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(Core::Components::System)));

#undef ARGS

    std::string ToString(void) const override;

    bool IsComplete(void) const override;

private:
    // ----------------------------------------------------------------------
    // |  Relationships
    friend class CommonHelpers::TypeTraits::Access;

    // ----------------------------------------------------------------------
    // |
    // |  Private Data
    // |
    // ----------------------------------------------------------------------

    // Cached data; default values are provided for deserialization scenarios and set
    // to accurate values in FinalConstruct.
    size_t const                            _requestsIndex = 0;             // Set in FinalConstruct
    size_t const                            _requestIndex = 0;              // Set in FinalConstruct
    bool const                              _atLastRequests = false;        // Set in FinalConstruct
    bool const                              _atLastRequest = false;         // Set in FinalConstruct

    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    void FinalConstruct(void);

    SystemPtrs GenerateChildrenImpl(size_t maxNumChildren) override;
};

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::WorkingSystem);
