/////////////////////////////////////////////////////////////////////////
///
///  \file          CalculatedResultSystem.h
///  \brief         Contains the CalculatedResultSystem object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-25 09:47:03
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
#pragma once

#include "ConstrainedResource.h"
#include "Request.h"
#include "Resource.h"
#include "ResultSystem.h"

#include <DecisionEngine/Core/Components/CalculatedResultSystem.h>

namespace DecisionEngine {
namespace ConstrainedResource {

/////////////////////////////////////////////////////////////////////////
///  \class         CalculatedResultSystem
///  \brief         Object that can be converted into a ResultSystem for
///                 constrained resource systems.
///
class CalculatedResultSystem : public Core::Components::CalculatedResultSystem {
private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    ResourcePtr const                       _pResource;
    Resource::ApplyStatePtr const           _pApplyState;
    RequestPtrsContainerPtr const           _pRequestsContainer;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using Score                             = Core::Components::Score;
    using Index                             = Core::Components::Index;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    CalculatedResultSystem(
        ResourcePtr pResource,
        Resource::ApplyStatePtr pApplyState,
        RequestPtrsContainerPtr pRequestsContainer,
        Score score,
        Index index
    );
    ~CalculatedResultSystem(void) override = default;

#define ARGS                                MEMBERS(_pResource, _pApplyState, _pRequestsContainer), BASES(Core::Components::CalculatedResultSystem)

    NON_COPYABLE(CalculatedResultSystem);
    MOVE(CalculatedResultSystem, ARGS);
    COMPARE(CalculatedResultSystem, ARGS);
    SERIALIZATION(CalculatedResultSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(Core::Components::System)));

#undef ARGS

    std::string ToString(void) const override;

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    ResultSystemUniquePtr CommitImpl(Score score, Index index) override;
};

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::CalculatedResultSystem);
