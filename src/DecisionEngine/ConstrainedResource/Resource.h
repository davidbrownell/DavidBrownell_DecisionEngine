/////////////////////////////////////////////////////////////////////////
///
///  \file          Resource.h
///  \brief         Contains the Resource object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-23 22:28:44
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

#include "Condition.h"
#include "ConstrainedResource.h"

#include <DecisionEngine/Core/Components/Score.h>

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |  Forward Declarations
class Request;

/////////////////////////////////////////////////////////////////////////
///  \class         Resource
///  \brief         Base class for an object that is able to fulfill one
///                 or more Requests.
///
class Resource : public BoostHelpers::SharedObject {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using Request                           = DecisionEngine::ConstrainedResource::Request;
    using ResourcePtr                       = DecisionEngine::ConstrainedResource::ResourcePtr;

    using ConditionPtrsPtr                  = DecisionEngine::ConstrainedResource::ConditionPtrsPtr;

    /////////////////////////////////////////////////////////////////////////
    ///  \class         State
    ///  \brief         Information created during calls to 'Evaluate' that
    ///                 can be later be provided during calls to 'Apply' when
    ///                 creating new 'Resource' objects. This class is configured
    ///                 so that it is created and owned by a specific Resource
    ///                 instance.
    ///
    class State {
    private:
        // ----------------------------------------------------------------------
        // |  Private Data (used in public declarations)
        ResourcePtr const                   _resource;

    public:
        // ----------------------------------------------------------------------
        // |  Public Methods
        State(Resource const &resource);
        virtual ~State(void) = default;

#define ARGS                                MEMBERS(_resource)

        NON_COPYABLE(State);
        MOVE(State, ARGS);
        COMPARE(State, ARGS);
        SERIALIZATION(State, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC_BASE));

#undef ARGS

    private:
        // Relationships
        friend class Resource;
    };

    using ApplyStatePtr                     = std::shared_ptr<State>;
    using ContinuationStatePtr              = std::shared_ptr<State>;

    /////////////////////////////////////////////////////////////////////////
    ///  \class         Evaluation
    ///  \brief         The results of applying a Request to a Resource.
    ///
    class Evaluation {
    public:
        // ----------------------------------------------------------------------
        // |  Public Types
        using Score                         = Core::Components::Score;
        using ApplyStatePtr                 = Resource::ApplyStatePtr;

        // ----------------------------------------------------------------------
        // |  Public Data
        Score::Result                       Result;
        ApplyStatePtr                       ApplyState;

        // ----------------------------------------------------------------------
        // |  Public Methods
        Evaluation(Score::Result result, ApplyStatePtr pApplyState);

        // ApplyState is not necessary for unsuccessful results when the algorithm
        // is configured to not continue processing Resources that contain errors.
        Evaluation(Score::Result result);

#define ARGS                                MEMBERS(Result, ApplyState)

        NON_COPYABLE(Evaluation);
        MOVE(Evaluation, ARGS);
        COMPARE(Evaluation, ARGS);
        SERIALIZATION(Evaluation, ARGS);

#undef ARGS
    };

    using Evaluations                       = std::vector<Evaluation>;
    using EvaluateResult                    = std::tuple<Evaluations, ContinuationStatePtr>;

    // ----------------------------------------------------------------------
    // |
    // |  Public Data
    // |
    // ----------------------------------------------------------------------
    std::string const                       Name;
    ConditionPtrsPtr const                  OptionalApplicabilityConditions;
    ConditionPtrsPtr const                  OptionalRequirementConditions;
    ConditionPtrsPtr const                  OptionalPreferenceConditions;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    CREATE(Resource);

    template <typename PrivateConstructorTagT>
    Resource(
        PrivateConstructorTagT tag,
        std::string name,
        ConditionPtrsPtr optionalApplicabilityConditions=ConditionPtrsPtr(),
        ConditionPtrsPtr optionalRequirementConditions=ConditionPtrsPtr(),
        ConditionPtrsPtr optionalPreferenceConditions=ConditionPtrsPtr()
    );

    template <typename PrivateConstructorTagT>
    Resource(PrivateConstructorTagT tag, Resource const &other);

    virtual ~Resource(void) = default;

#define ARGS                                MEMBERS(Name, OptionalApplicabilityConditions, OptionalRequirementConditions, OptionalPreferenceConditions), BASES(BoostHelpers::SharedObject)

    NON_COPYABLE(Resource);
    MOVE(Resource, ARGS);
    COMPARE(Resource, ARGS);
    SERIALIZATION(Resource, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_ABSTRACT));

#undef ARGS

    std::string const & ToString(void) const;

    EvaluateResult Evaluate(Request const &request, size_t maxNumEvaluations) const;
    EvaluateResult Evaluate(Request const &request, size_t maxNumEvaluations, State &continuationState) const;

    ResourcePtr Apply(State &applyState) const;

protected:
    // ----------------------------------------------------------------------
    // |
    // |  Protected Methods
    // |
    // ----------------------------------------------------------------------
    static Core::Components::Score::Result CalculateResult(Request const &request, Resource const &resource);

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    virtual EvaluateResult EvaluateImpl(Request const &request, size_t maxNumEvaluations) const = 0;
    virtual EvaluateResult EvaluateImpl(Request const &request, size_t maxNumEvaluations, State &continuationState) const = 0;

    virtual ResourcePtr ApplyImpl(State const &applyState) const = 0;
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

// ----------------------------------------------------------------------
// |
// |  Resource
// |
// ----------------------------------------------------------------------
namespace Details {

inline bool IsValidResourceOptionalConditionPtrsPtr(ConditionPtrsPtr const &value) {
    if(!value)
        return true;

    return value->empty() == false
        && std::all_of(value->cbegin(), value->cend(), [](ConditionPtr const &ptr) { return static_cast<bool>(ptr); })
    ;
}

} // namespace Details

template <typename PrivateConstructorTagT>
Resource::Resource(
    PrivateConstructorTagT tag,
    std::string name,
    ConditionPtrsPtr optionalApplicabilityConditions/*=ConditionPtrsPtr()*/,
    ConditionPtrsPtr optionalRequirementConditions/*=ConditionPtrsPtr()*/,
    ConditionPtrsPtr optionalPreferenceConditions/*=ConditionPtrsPtr()*/
) :
    BoostHelpers::SharedObject(tag),
    Name(
        std::move(
            [&name](void) -> std::string & {
                ENSURE_ARGUMENT(name, name.empty() == false);
                return name;
            }()
        )
    ),
    OptionalApplicabilityConditions(
        std::move(
            [&optionalApplicabilityConditions](void) -> ConditionPtrsPtr & {
                ENSURE_ARGUMENT(optionalApplicabilityConditions, Details::IsValidResourceOptionalConditionPtrsPtr(optionalApplicabilityConditions));
                return optionalApplicabilityConditions;
            }()
        )
    ),
    OptionalRequirementConditions(
        std::move(
            [&optionalRequirementConditions](void) -> ConditionPtrsPtr & {
                ENSURE_ARGUMENT(optionalRequirementConditions, Details::IsValidResourceOptionalConditionPtrsPtr(optionalRequirementConditions));
                return optionalRequirementConditions;
            }()
        )
    ),
    OptionalPreferenceConditions(
        std::move(
            [&optionalPreferenceConditions](void) -> ConditionPtrsPtr & {
                ENSURE_ARGUMENT(optionalPreferenceConditions, Details::IsValidResourceOptionalConditionPtrsPtr(optionalPreferenceConditions));
                return optionalPreferenceConditions;
            }()
        )
    )
{}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
template <typename PrivateConstructorTagT>
Resource::Resource(PrivateConstructorTagT tag, Resource const &other) :
    BoostHelpers::SharedObject(tag),
    Name(other.Name),
    OptionalApplicabilityConditions(other.OptionalApplicabilityConditions),
    OptionalRequirementConditions(other.OptionalRequirementConditions),
    OptionalPreferenceConditions(other.OptionalPreferenceConditions)
{}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::Resource);
SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::Resource::State);
