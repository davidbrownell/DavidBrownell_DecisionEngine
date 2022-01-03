/////////////////////////////////////////////////////////////////////////
///
///  \file          StandardPermutationGenerator.cpp
///  \brief         See StandardPermutationGenerator.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:55:49
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
#include "StandardPermutationGenerator.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  StandardPermutationGenerator
// |
// ----------------------------------------------------------------------
StandardPermutationGenerator::StandardPermutationGenerator(size_t maxNumPermutations/*=std::numeric_limits<size_t>::max()*/) :
    PermutationGenerator(std::move(maxNumPermutations))
{}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
StandardPermutationGenerator::RequestPtrsPtrs StandardPermutationGenerator::GenerateImpl(RequestPtrs const &requests, size_t maxNumPermutations) /*override*/ {
    assert(requests.empty() == false && std::all_of(requests.cbegin(), requests.cend(), [](RequestPtr const &ptr) { return static_cast<bool>(ptr); }));
    assert(maxNumPermutations);
    assert(IsComplete() == false);

    if(_indexes.empty()) {
        size_t const                        numRequests(requests.size());

        _indexes.reserve(numRequests);

        for(size_t index = 0; index < numRequests; ++index)
            _indexes.emplace_back(index);
    }

    assert(_indexes.size() == requests.size());

    RequestPtrsPtrs                         results;

    while(maxNumPermutations--) {
        RequestPtrs                         theseResults;

        theseResults.reserve(_indexes.size());

        for(auto index : _indexes) {
            assert(index < requests.size());
            theseResults.emplace_back(requests[index]);
        }

        results.emplace_back(std::make_shared<RequestPtrs>(std::move(theseResults)));

        if(std::next_permutation(_indexes.begin(), _indexes.end()) == false) {
            assert(_isActive);
            _isActive = false;

            break;
        }
    }

    return results;
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::StandardPermutationGenerator);
SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::StandardPermutationGeneratorFactory);
