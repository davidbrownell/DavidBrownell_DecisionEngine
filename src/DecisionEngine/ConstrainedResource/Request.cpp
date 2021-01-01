/////////////////////////////////////////////////////////////////////////
///
///  \file          Request.cpp
///  \brief         See Request.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-22 22:28:47
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
#include "Request.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  Request
// |
// ----------------------------------------------------------------------
namespace {

bool IsValidOptionalConditionPtrsPtr(ConditionPtrsPtr const &conditions) {
    if(!conditions)
        return true;

    return
        conditions->empty() == false
        && std::all_of(conditions->cbegin(), conditions->cend(), [](ConditionPtr const &ptr) { return static_cast<bool>(ptr); });
}

} // anonymous namespace

Request::Request(
    std::string name,
    ConditionPtrsPtr optionalApplicabilityConditions/*=ConditionPtrsPtr()*/,
    ConditionPtrsPtr optionalRequirementConditions/*=ConditionPtrsPtr()*/,
    ConditionPtrsPtr optionalPreferenceConditions/*=ConditionPtrsPtr()*/
) :
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
                ENSURE_ARGUMENT(optionalApplicabilityConditions, IsValidOptionalConditionPtrsPtr(optionalApplicabilityConditions));
                return optionalApplicabilityConditions;
            }()
        )
    ),
    OptionalRequirementConditions(
        std::move(
            [&optionalRequirementConditions](void) -> ConditionPtrsPtr & {
                ENSURE_ARGUMENT(optionalRequirementConditions, IsValidOptionalConditionPtrsPtr(optionalRequirementConditions));
                return optionalRequirementConditions;
            }()
        )
    ),
    OptionalPreferenceConditions(
        std::move(
            [&optionalPreferenceConditions](void) -> ConditionPtrsPtr & {
                ENSURE_ARGUMENT(optionalPreferenceConditions, IsValidOptionalConditionPtrsPtr(optionalPreferenceConditions));
                return optionalPreferenceConditions;
            }()
        )
    )
{}

std::string const & Request::ToString(void) const {
    return Name;
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::Request);
