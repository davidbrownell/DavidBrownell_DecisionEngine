/////////////////////////////////////////////////////////////////////////
///
///  \file          PermutationGenerator.cpp
///  \brief         See PermutationsGenerator.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:27:34
///
///  \note
///
///  \bug
///
/////////////////////////////////////////////////////////////////////////
///
///  \attention
///  Copyright David Brownell 2020-22
///  Distributed under the Boost Software License, Version 1.0. See
///  accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt.
///
/////////////////////////////////////////////////////////////////////////
#include "PermutationGenerator.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  PermutationGenerator
// |
// ----------------------------------------------------------------------
PermutationGenerator::PermutationGenerator(size_t maxNumTotalPermutations) :
    _permutationsRemaining(
        std::move(
            [&maxNumTotalPermutations](void) -> size_t & {
                ENSURE_ARGUMENT(maxNumTotalPermutations);
                return maxNumTotalPermutations;
            }()
        )
    ),
    _isActive(true)
{}

bool PermutationGenerator::IsComplete(void) const {
    return _isActive == false;
}

PermutationGenerator::RequestPtrsPtrs PermutationGenerator::Generate(RequestPtrs const &requests, size_t maxNumPermutations) {
    ENSURE_ARGUMENT(requests, requests.empty() == false && std::all_of(requests.cbegin(), requests.cend(), [](RequestPtr const &ptr) { return static_cast<bool>(ptr); }));
    ENSURE_ARGUMENT(maxNumPermutations);

    if(IsComplete())
        throw std::runtime_error("Invalid operation");

    size_t const                            permutationsToGenerate(std::min(_permutationsRemaining, maxNumPermutations));
    RequestPtrsPtrs                         results(GenerateImpl(requests, permutationsToGenerate));

    if(
        results.empty()
        || results.size() > permutationsToGenerate
        || std::any_of(
            results.cbegin(),
            results.cend(),
            [](RequestPtrsPtr const &ptrs) {
                return !ptrs
                    || ptrs->empty()
                    || std::any_of(
                        ptrs->cbegin(),
                        ptrs->cend(),
                        [](RequestPtr const &ptr) {
                            return static_cast<bool>(ptr) == false;
                        }
                    );
            }
        )
    )
        throw std::runtime_error("Invalid RequestPtrsPtrs");

    _permutationsRemaining -= permutationsToGenerate;
    if(_permutationsRemaining == 0)
        _isActive = false;

    return results;
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::PermutationGenerator);
