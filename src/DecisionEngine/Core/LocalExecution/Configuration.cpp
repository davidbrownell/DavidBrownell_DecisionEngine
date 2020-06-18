/////////////////////////////////////////////////////////////////////////
///
///  \file          Configuration.cpp
///  \brief         See Configuration.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-06 00:49:54
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
#include "Configuration.h"

#include <DecisionEngine/Core/Components/ResultSystem.h>

namespace DecisionEngine {
namespace Core {
namespace LocalExecution {

// ----------------------------------------------------------------------
// |
// |  Configuration
// |
// ----------------------------------------------------------------------
Configuration::Configuration(
    size_t numConcurrentTasks,
    bool continueProcessingSystemsWithFailures,
    bool isDeterministic
) :
    NumConcurrentTasks(
        std::move(
            [&numConcurrentTasks](void) -> size_t & {
                ENSURE_ARGUMENT(numConcurrentTasks);
                return numConcurrentTasks;
            }()
        )
    ),
    ContinueProcessingSystemsWithFailures(std::move(continueProcessingSystemsWithFailures)),
    IsDeterministic(std::move(isDeterministic))
{}

// virtual
Configuration::ResultSystemUniquePtrs Configuration::Finalize(ResultSystemUniquePtrs results) {
    // Don't do anything by default
    return results;
}

// virtual
void * Configuration::QueryInterface(boost::uuids::uuid const &) const {
    return nullptr;
}

} // namespace LocalExecution
} // namespace Core
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::Core::LocalExecution::Configuration);
