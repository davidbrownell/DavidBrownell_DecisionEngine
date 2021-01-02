/////////////////////////////////////////////////////////////////////////
///
///  \file          ConstrainedResource.h
///  \brief         Contains the ConstrainedResource namespace
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-22 22:07:50
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

#include <DecisionEngine/Core/Components/Components.h>

namespace DecisionEngine {

/////////////////////////////////////////////////////////////////////////
///  \namespace     ConstrainedResource
///  \brief         Application of the DecisionEngine algorithm for problem
///                 domains where the System is comprised of limited Resource(s),
///                 and Request(s) are applied to them in order. Use this algorithm
///                 when any of these conditions are true:
///
///                     - The number of Request(s) is fixed at execution time
///                     - A Resource does not contain enough information to come
///                       to a resolution in isolation (Requests are necessary).
///
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |  Forward Declarations
class Condition;
class Request;
class Resource;

// ----------------------------------------------------------------------
// |
// |  Public Types
// |
// ----------------------------------------------------------------------
using ConditionPtr                          = std::shared_ptr<Condition>;
using ConditionPtrs                         = std::vector<ConditionPtr>;
using ConditionPtrsPtr                      = std::shared_ptr<ConditionPtrs>;

using RequestPtr                            = std::shared_ptr<Request>;
using RequestPtrs                           = std::vector<RequestPtr>;
using RequestPtrsPtr                        = std::shared_ptr<RequestPtrs>;
using RequestPtrsContainer                  = std::vector<RequestPtrs>;
using RequestPtrsContainerPtr               = std::shared_ptr<RequestPtrsContainer>;

using ResourcePtr                           = std::shared_ptr<Resource>;
using ResourcePtrs                          = std::vector<ResourcePtr>;

} // namespace ConstrainedResource
} // namespace DecisionEngine
