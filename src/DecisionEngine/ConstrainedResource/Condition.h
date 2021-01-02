/////////////////////////////////////////////////////////////////////////
///
///  \file          Condition.h
///  \brief         Contains the Condition object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-22 22:40:27
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

#include <DecisionEngine/Core/Components/Condition.h>

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |  Forward Declarations
class Request;
class Resource;

/////////////////////////////////////////////////////////////////////////
///  \class         Condition
///  \brief         Evaluates the potential application of a Request to a
///                 Resource. The condition's Result can be expressed as
///                 being successful or unsuccessful.
///
class Condition : public Core::Components::Condition {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using Request                           = DecisionEngine::ConstrainedResource::Request;
    using Resource                          = DecisionEngine::ConstrainedResource::Resource;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    using Core::Components::Condition::Condition;

    ~Condition(void) override = default;

#define ARGS                                BASES(Core::Components::Condition)

    NON_COPYABLE(Condition);
    MOVE(Condition, ARGS);
    COMPARE(Condition, ARGS);
    SERIALIZATION(Condition, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_DATA_ONLY));

#undef ARGS

    virtual Result Apply(Request const &request, Resource const &resource) const = 0;
};

} // namespace ConstrainedResource
} // namespace DecisionEngine
