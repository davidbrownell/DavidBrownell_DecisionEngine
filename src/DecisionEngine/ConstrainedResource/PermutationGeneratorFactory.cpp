/////////////////////////////////////////////////////////////////////////
///
///  \file          PermutationGeneratorFactory.cpp
///  \brief         See PermutationGeneratorFactory.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:41:00
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
#include "PermutationGeneratorFactory.h"
#include "PermutationGenerator.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  PermutationGeneratorFactory
// |
// ----------------------------------------------------------------------
PermutationGeneratorFactory::PermutationGeneratorFactory(size_t maxNumTotalPermutations) :
    _maxNumTotalPermutations(
        std::move(
            [&maxNumTotalPermutations](void) -> size_t & {
                ENSURE_ARGUMENT(maxNumTotalPermutations);
                return maxNumTotalPermutations;
            }()
        )
    )
{}

PermutationGeneratorFactory::PermutationGeneratorUniquePtr PermutationGeneratorFactory::Create(void) const {
    PermutationGeneratorUniquePtr           result(CreateImpl(_maxNumTotalPermutations));

    if(!result)
        throw std::runtime_error("Invalid PermutationGeneratorUniquePtr");

    return result;
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::PermutationGeneratorFactory);
