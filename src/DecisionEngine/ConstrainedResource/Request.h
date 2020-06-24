/////////////////////////////////////////////////////////////////////////
///
///  \file          Request.h
///  \brief         Contains the Request object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-22 22:17:21
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

namespace DecisionEngine {
namespace ConstrainedResource {

/////////////////////////////////////////////////////////////////////////
///  \class         Request
///  \brief         A request that can be potentially fulfilled by one or more
///                 Resources.
///
class Request {
public:
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
    Request(
        std::string name,
        ConditionPtrsPtr optionalApplicabilityConditions=ConditionPtrsPtr(),
        ConditionPtrsPtr optionalRequirementConditions=ConditionPtrsPtr(),
        ConditionPtrsPtr optionalPreferenceConditions=ConditionPtrsPtr()
    );

    virtual ~Request(void) = default;

#define ARGS                                MEMBERS(Name, OptionalApplicabilityConditions, OptionalRequirementConditions, OptionalPreferenceConditions)

    NON_COPYABLE(Request);
    MOVE(Request, ARGS);
    COMPARE(Request, ARGS);
    SERIALIZATION(Request, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC_BASE));

#undef ARGS

    std::string const & ToString(void) const;
};

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::Request);
