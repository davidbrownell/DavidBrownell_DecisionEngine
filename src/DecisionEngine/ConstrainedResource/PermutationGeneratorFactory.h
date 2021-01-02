/////////////////////////////////////////////////////////////////////////
///
///  \file          PermutationGeneratorFactory.h
///  \brief         Contains the PermutationGeneratorFactory and
///                 PermutationGeneratorFactoryImpl objects.
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-24 14:37:07
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

namespace DecisionEngine {
namespace ConstrainedResource {

// ----------------------------------------------------------------------
// |  Forward Declarations
class PermutationGenerator;

/////////////////////////////////////////////////////////////////////////
///  \class         PermutationGeneratorFactory
///  \brief         Factory for PermutationGenerators
///
class PermutationGeneratorFactory {
protected:
    // ----------------------------------------------------------------------
    // |
    // |  Protected Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    size_t const                            _maxNumTotalPermutations;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using PermutationGeneratorUniquePtr     = std::unique_ptr<PermutationGenerator>;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    PermutationGeneratorFactory(size_t maxNumTotalPermutations);
    virtual ~PermutationGeneratorFactory(void) = default;

#define ARGS                                MEMBERS(_maxNumTotalPermutations)

    NON_COPYABLE(PermutationGeneratorFactory);
    MOVE(PermutationGeneratorFactory, ARGS);
    COMPARE(PermutationGeneratorFactory, ARGS);
    SERIALIZATION(PermutationGeneratorFactory, ARGS, FLAGS(SERIALIZATION_ABSTRACT));

#undef ARGS

    PermutationGeneratorUniquePtr Create(void) const;

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    virtual PermutationGeneratorUniquePtr CreateImpl(size_t maxNumTotalPermutations) const = 0;
};

/////////////////////////////////////////////////////////////////////////
///  \class         PermutationGeneratorFactoryImpl
///  \brief         Helper implementation to create a concrete PermutationGenerator instances.
///
template <typename PermutationGeneratorT>
class PermutationGeneratorFactoryImpl : public PermutationGeneratorFactory {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    using PermutationGeneratorFactory::PermutationGeneratorFactory;
    ~PermutationGeneratorFactoryImpl(void) override = default;

#define ARGS                                BASES(PermutationGeneratorFactory)

    NON_COPYABLE(PermutationGeneratorFactoryImpl);
    MOVE(PermutationGeneratorFactoryImpl, ARGS);
    COMPARE(PermutationGeneratorFactoryImpl, ARGS);
    SERIALIZATION(PermutationGeneratorFactoryImpl, ARGS, FLAGS(SERIALIZATION_DATA_ONLY));

#undef ARGS

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    PermutationGeneratorUniquePtr CreateImpl(size_t maxNumTotalPermutations) const override;
};

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// |
// |  Implementation
// |
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// |
// |  PermutationGeneratorFactoryImpl
// |
// ----------------------------------------------------------------------
template <typename PermutationGeneratorT>
typename PermutationGeneratorFactoryImpl<PermutationGeneratorT>::PermutationGeneratorUniquePtr PermutationGeneratorFactoryImpl<PermutationGeneratorT>::CreateImpl(size_t maxNumTotalPermuations) const /*override*/ {
    return std::make_unique<PermutationGeneratorT>(maxNumTotalPermuations);
}

} // namespace ConstrainedResource
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::ConstrainedResource::PermutationGeneratorFactory);
