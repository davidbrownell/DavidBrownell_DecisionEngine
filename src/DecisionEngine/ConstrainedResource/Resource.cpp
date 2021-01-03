/////////////////////////////////////////////////////////////////////////
///
///  \file          Resource.cpp
///  \brief         See Resource.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-23 22:28:26
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
#include "Resource.h"
#include "Request.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  Resource::State
// |
// ----------------------------------------------------------------------
Resource::State::State(Resource const &resource) :
    _resource(resource.SharedFromThis())
{}

// ----------------------------------------------------------------------
// |
// |  Resource::Evaluation
// |
// ----------------------------------------------------------------------
Resource::Evaluation::Evaluation(Score::Result result, ApplyStatePtr pApplyState) :
    Result(std::move(result)),
    ApplyState(
        std::move(
            [&pApplyState](void) -> ApplyStatePtr & {
                ENSURE_ARGUMENT(pApplyState);
                return pApplyState;
            }()
        )
    )
{}

Resource::Evaluation::Evaluation(Score::Result result) :
    Result(
        std::move(
            [&result](void) -> Score::Result & {
                ENSURE_ARGUMENT(result, result.IsSuccessful == false);
                return result;
            }()
        )
    )
{}

// ----------------------------------------------------------------------
// |
// |  Resource
// |
// ----------------------------------------------------------------------
std::string const & Resource::ToString(void) const {
    return Name;
}

Resource::EvaluateResult Resource::Evaluate(Request const &request, size_t maxNumEvaluations) const {
    ENSURE_ARGUMENT(maxNumEvaluations);

    EvaluateResult                          result(EvaluateImpl(request, maxNumEvaluations));
    Evaluations const &                     evaluations(std::get<0>(result));

    if(evaluations.empty() || evaluations.size() > maxNumEvaluations)
        throw std::runtime_error("Invalid Evaluations");

    return result;
}

Resource::EvaluateResult Resource::Evaluate(Request const &request, size_t maxNumEvaluations, State &continuationState) const {
    ENSURE_ARGUMENT(maxNumEvaluations);
    ENSURE_ARGUMENT(continuationState, continuationState._resource.get() == this);

    EvaluateResult                          result(EvaluateImpl(request, maxNumEvaluations, continuationState));
    Evaluations const &                     evaluations(std::get<0>(result));

    if(evaluations.empty() || evaluations.size() > maxNumEvaluations)
        throw std::runtime_error("Invalid Evaluations");

    return result;
}

ResourcePtr Resource::Apply(State &applyState) const {
    ENSURE_ARGUMENT(applyState, applyState._resource.get() == this);

    ResourcePtr                             result(ApplyImpl(applyState));

    if(!result)
        throw std::runtime_error("Invalid ResourcePtr");

    return result;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// static
Core::Components::Score::Result Resource::CalculateResult(Request const &request, Resource const &resource) {
    // ----------------------------------------------------------------------
    using Result                            = Core::Components::Score::Result;
    using ConditionResults                  = Result::ConditionResults;
    using ConditionResult                   = ConditionResults::value_type;
    // ----------------------------------------------------------------------

    auto const                              applyConditionsFunc(
        [&request, &resource](ConditionPtrsPtr const * const * pppConditionPtrs, size_t numConditionPtrs) {
            assert(pppConditionPtrs);
            assert(numConditionPtrs);

            ConditionPtrsPtr const * const * const      pppConditionPtrsEnd(pppConditionPtrs + numConditionPtrs);
            ConditionResults                            results;

            while(pppConditionPtrs != pppConditionPtrsEnd) {
                if(**pppConditionPtrs) {
                    ConditionPtrs const &   conditionPtrs(***pppConditionPtrs);

                    results.reserve(results.size() + conditionPtrs.size());

                    for(auto const &pCondition : conditionPtrs)
                        results.emplace_back(pCondition->Apply(request, resource));
                }

                ++pppConditionPtrs;
            }

            return results;
        }
    );

    // Calculate applicability
    ConditionPtrsPtr const * const          ppApplicability[] = { &request.OptionalApplicabilityConditions, &resource.OptionalApplicabilityConditions };

    ConditionResults                        applicabilityResults(applyConditionsFunc(ppApplicability, sizeof(ppApplicability) / sizeof(*ppApplicability)));
    ConditionResults                        requirementResults;
    ConditionResults                        preferenceResults;

    // Only calculate requirements and preferences if everything is applicable
    if(std::all_of(applicabilityResults.cbegin(), applicabilityResults.cend(), [](ConditionResult const &result) { return result.IsSuccessful; })) {
        ConditionPtrsPtr const * const      ppRequirements[] = { &request.OptionalRequirementConditions, &resource.OptionalRequirementConditions };
        ConditionPtrsPtr const * const      ppPreferences[] = { &request.OptionalPreferenceConditions, &resource.OptionalPreferenceConditions };

        requirementResults = applyConditionsFunc(ppRequirements, sizeof(ppRequirements) / sizeof(*ppRequirements));
        preferenceResults = applyConditionsFunc(ppPreferences, sizeof(ppPreferences) / sizeof(*ppPreferences));
    }

    return Result(
        std::move(applicabilityResults),
        std::move(requirementResults),
        std::move(preferenceResults)
    );
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::Resource);
SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::Resource::State);
