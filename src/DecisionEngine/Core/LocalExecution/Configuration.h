/////////////////////////////////////////////////////////////////////////
///
///  \file          Configuration.h
///  \brief         Contains the Configuration object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-06 00:38:18
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

#include "LocalExecution.h"

namespace DecisionEngine {
namespace Core {

namespace Components {

// ----------------------------------------------------------------------
// |  Forward Declarations
class ResultSystem;
class WorkingSystem;

} // namespace Components

namespace LocalExecution {

/////////////////////////////////////////////////////////////////////////
///  \class         Configuration
///  \brief         Configuration values for the invocation of the DecisionEngine
///                 when executing locally.
///
class Configuration {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using ResultSystem                      = Components::ResultSystem;
    using ResultSystemUniquePtr             = std::unique_ptr<ResultSystem>;
    using ResultSystemUniquePtrs            = std::vector<ResultSystemUniquePtr>;

    using WorkingSystem                     = Components::WorkingSystem;

    // ----------------------------------------------------------------------
    // |
    // |  Public Data
    // |
    // ----------------------------------------------------------------------

    // These values will not change, regardless of where the system is in terms of
    // completion.
    bool const                              ContinueProcessingSystemsWithFailures;
    bool const                              IsDeterministic;
    boost::optional<size_t> const           NumConcurrentTasks;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    Configuration(
        bool continueProcessingSystemsWithFailures,
        bool isDeterministic,
        boost::optional<size_t> numConcurrentTasks=boost::none
    );

    virtual ~Configuration(void) = default;

    NON_COPYABLE(Configuration);

#define ARGS                                MEMBERS(ContinueProcessingSystemsWithFailures, IsDeterministic, NumConcurrentTasks)

    MOVE(Configuration, ARGS);
    COMPARE(Configuration, ARGS);
    SERIALIZATION(Configuration, ARGS, FLAGS(SERIALIZATION_ABSTRACT));

#undef ARGS

    virtual size_t GetMaxNumPendingSystems(void) const = 0;
    virtual size_t GetMaxNumPendingSystems(WorkingSystem const &system) const = 0;
    virtual size_t GetMaxNumChildrenPerGeneration(WorkingSystem const &system) const = 0;
    virtual size_t GetMaxNumIterationsPerRound(WorkingSystem const &system) const = 0;

    /////////////////////////////////////////////////////////////////////////
    ///  \fn            Finalize
    ///  \brief         Opportunity to modify the results before they are returned.
    ///
    virtual ResultSystemUniquePtrs Finalize(ResultSystemUniquePtrs results);

    /////////////////////////////////////////////////////////////////////////
    ///  \fn            QueryInterface
    ///  \brief         Dynamic retrieval of interfaces that are not defined here.
    ///                 Will return a value pointer or nullptr if the id was not
    ///                 recognized.
    ///
    virtual void * QueryInterface(boost::uuids::uuid const &id) const;
};

} // namespace LocalExecution
} // namespace Core
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::Core::LocalExecution::Configuration);
