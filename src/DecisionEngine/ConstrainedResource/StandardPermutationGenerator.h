/////////////////////////////////////////////////////////////////////////
///
///  \file          StandardPermutationGenerator.h
///  \brief         Contains the StandardPermutationGenerator object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:45:04
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

#include "ConstrainedResource.h"
#include "PermutationGenerator.h"
#include "PermutationGeneratorFactory.h"

namespace DecisionEngine {
namespace ConstrainedResource {

/////////////////////////////////////////////////////////////////////////
///  \class         StandardPermutationGenerator
///  \brief         PermutationGenerator that relies on a simple mathematical
///                 permutation algorithm; it has no special knowledge of the
///                 Requests themselves.
///
class StandardPermutationGenerator : public PermutationGenerator {
private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    std::vector<size_t>                     _indexes;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    StandardPermutationGenerator(size_t maxNumPermutations=std::numeric_limits<size_t>::max());
    ~StandardPermutationGenerator(void) override = default;

#define ARGS                                MEMBERS(_indexes), BASES(PermutationGenerator)

    NON_COPYABLE(StandardPermutationGenerator);
    MOVE(StandardPermutationGenerator, ARGS);
    COMPARE(StandardPermutationGenerator, ARGS);
    SERIALIZATION(StandardPermutationGenerator, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(PermutationGenerator)));

#undef ARGS

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    RequestPtrsPtrs GenerateImpl(RequestPtrs const &requests, size_t maxNumPermutations) override;
};

/////////////////////////////////////////////////////////////////////////
///  \class         StandardPermutationGeneratorFactory
///  \brief         Factory that generates StandardPermutationGenerator instances
///
class StandardPermutationGeneratorFactory : public PermutationGeneratorFactoryImpl<StandardPermutationGenerator>
{
public:
    // ----------------------------------------------------------------------
    // |  Public Types
    using BaseType                          = PermutationGeneratorFactoryImpl<StandardPermutationGenerator>;

    // ----------------------------------------------------------------------
    // |  Public Methods
    using BaseType::BaseType;

#define ARGS                                BASES(BaseType)

    NON_COPYABLE(StandardPermutationGeneratorFactory);
    MOVE(StandardPermutationGeneratorFactory, ARGS);
    COMPARE(StandardPermutationGeneratorFactory, ARGS);
    SERIALIZATION(StandardPermutationGeneratorFactory, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(PermutationGeneratorFactory)));

#undef ARGS
};

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::StandardPermutationGenerator);
SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::StandardPermutationGeneratorFactory);
