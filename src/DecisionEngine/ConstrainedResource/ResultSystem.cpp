/////////////////////////////////////////////////////////////////////////
///
///  \file          ResultSystem.cpp
///  \brief         See ResultSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-25 09:34:21
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
#include "ResultSystem.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  ResultSystem
// |
// ----------------------------------------------------------------------
ResultSystem::ResultSystem(ResourcePtr pResource, RequestPtrsContainerPtr pRequestsContainer, Score score, Index index) :
    Core::Components::ResultSystem(std::move(score), std::move(index)),
    Resource(
        std::move(
            [&pResource](void) -> ResourcePtr & {
                ENSURE_ARGUMENT(pResource);
                return pResource;
            }()
        )
    ),
    Requests(
        std::move(
            [&pRequestsContainer](void) -> RequestPtrsContainerPtr & {
                ENSURE_ARGUMENT(
                    pRequestsContainer,
                    pRequestsContainer
                    && pRequestsContainer->empty() == false
                    && std::all_of(
                        pRequestsContainer->cbegin(),
                        pRequestsContainer->cend(),
                        [](RequestPtrs const &ptrs) {
                            return ptrs.empty() == false
                                && std::all_of(
                                    ptrs.cbegin(),
                                    ptrs.cend(),
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
    )
{}

std::string ResultSystem::ToString(void) const /*override*/ {
    return boost::str(
        boost::format("ConstrainedResource::ResultSystem(%1%,%2%)")
            % GetScore().ToString()
            % GetIndex().ToString()
    );
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::ResultSystem);
