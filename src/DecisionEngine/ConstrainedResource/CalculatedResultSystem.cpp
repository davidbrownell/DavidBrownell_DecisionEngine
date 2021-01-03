/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedResultSystem.cpp
///  \brief         See CalculatedResultSystem.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-25 09:47:38
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
#include "CalculatedResultSystem.h"

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |
// |  CalculatedResultSystem
// |
// ----------------------------------------------------------------------
CalculatedResultSystem::CalculatedResultSystem(
    ResourcePtr pResource,
    Resource::ApplyStatePtr pApplyState,
    RequestPtrsContainerPtr pRequestsContainer,
    Score score,
    Index index
) :
    Core::Components::CalculatedResultSystem(std::move(score), std::move(index)),
    _pResource(
        std::move(
            [&pResource](void) -> ResourcePtr & {
                ENSURE_ARGUMENT(pResource);
                return pResource;
            }()
        )
    ),
    _pApplyState(
        std::move(
            [&pApplyState](void) -> Resource::ApplyStatePtr & {
                ENSURE_ARGUMENT(pApplyState);
                return pApplyState;
            }()
        )
    ),
    _pRequestsContainer(
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

std::string CalculatedResultSystem::ToString(void) const /*override*/ {
    return boost::str(
        boost::format("ConstrainedResource::CalculatedResultSystem(%1%,%2%)")
            % GetScore().ToString()
            % GetIndex().ToString()
    );
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
CalculatedResultSystem::ResultSystemUniquePtr CalculatedResultSystem::CommitImpl(Score score, Index index) /*override*/ {
    return std::make_unique<ResultSystem>(
        _pResource->Apply(*_pApplyState),
        _pRequestsContainer,
        std::move(score),
        std::move(index)
    );
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::ConstrainedResource::CalculatedResultSystem);
