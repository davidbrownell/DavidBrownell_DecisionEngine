/////////////////////////////////////////////////////////////////////////
///
///  \file          ResultSystem.h
///  \brief         Contains the ResultSystem object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-25 09:34:48
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
#pragma once

#include "ConstrainedResource.h"
#include "Request.h"
#include "Resource.h"

#include <DecisionEngine/Core/Components/ResultSystem.h>

namespace DecisionEngine {
namespace ConstrainedResource {

/////////////////////////////////////////////////////////////////////////
///  \class         ResultSystem
///  \brief         A ResultSystem for constrained systems.
///
class ResultSystem : public Core::Components::ResultSystem {
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
    // |  Public Data
    // |
    // ----------------------------------------------------------------------
    ResourcePtr const                       Resource;
    RequestPtrsContainerPtr const           Requests;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    ResultSystem(ResourcePtr pResource, RequestPtrsContainerPtr pRequestsContainer, Score score, Index index);
    ~ResultSystem(void) override = default;

#define ARGS                                MEMBERS(Resource, Requests), BASES(Core::Components::ResultSystem)

    NON_COPYABLE(ResultSystem);
    MOVE(ResultSystem, ARGS);
    COMPARE(ResultSystem, ARGS);
    SERIALIZATION(ResultSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(Core::Components::System)));

#undef ARGS

    std::string ToString(void) const override;
};

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::ResultSystem);
