/////////////////////////////////////////////////////////////////////////
///
///  \file          ResultSystem.h
///  \brief         Contains the ResultSystem object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-24 20:00:54
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

#include "System.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

/////////////////////////////////////////////////////////////////////////
///  \class         ResultSystem
///  \brief         Represents a system in a completed state. Derived classes
///                 must be created to communicate the attributes of the
///                 system itself.
///
class ResultSystem : public System {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    ResultSystem(Score score, Index index);
    ~ResultSystem(void) override = default;

#define ARGS                                BASES(System)

    NON_COPYABLE(ResultSystem);
    MOVE(ResultSystem, ARGS);
    COMPARE(ResultSystem, ARGS);
    SERIALIZATION(ResultSystem, ARGS, FLAGS(SERIALIZATION_DATA_ONLY));

#undef ARGS
};

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
