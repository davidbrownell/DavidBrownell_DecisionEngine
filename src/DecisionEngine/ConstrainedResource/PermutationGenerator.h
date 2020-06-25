/////////////////////////////////////////////////////////////////////////
///
///  \file          PermutationGenerator.h
///  \brief         Contains the PermutationGenerator object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:23:09
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

#include "ConstrainedResource.h"

namespace DecisionEngine {
namespace ConstrainedResource {

/////////////////////////////////////////////////////////////////////////
///  \class         PermutationGenerator
///  \brief         Abstract class for an algorithm that generates permutations
///                 for Requests within a group.
///
class PermutationGenerator {
protected:
    // ----------------------------------------------------------------------
    // |
    // |  Protected Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------

    // Stored as _isActive rather than _isComplete so that a completed generator
    // compares as < an active generator.
    bool                                    _isActive;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using RequestPtr                        = DecisionEngine::ConstrainedResource::RequestPtr;
    using RequestPtrs                       = DecisionEngine::ConstrainedResource::RequestPtrs;
    using RequestPtrsPtr                    = DecisionEngine::ConstrainedResource::RequestPtrsPtr;

    using RequestPtrsPtrs                   = std::vector<RequestPtrsPtr>;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    PermutationGenerator(void);
    virtual ~PermutationGenerator(void) = default;

#define ARGS                                MEMBERS(_isActive)

    NON_COPYABLE(PermutationGenerator);
    MOVE(PermutationGenerator, ARGS);
    COMPARE(PermutationGenerator, ARGS);
    SERIALIZATION(PermutationGenerator, ARGS, FLAGS(SERIALIZATION_ABSTRACT));

#undef ARGS

    bool IsComplete(void) const;

    RequestPtrsPtrs Generate(RequestPtrs const &requests, size_t maxNumPermutations);

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    virtual RequestPtrsPtrs GenerateImpl(RequestPtrs const &requests, size_t maxNumPermutations) = 0;
};

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::PermutationGenerator);
