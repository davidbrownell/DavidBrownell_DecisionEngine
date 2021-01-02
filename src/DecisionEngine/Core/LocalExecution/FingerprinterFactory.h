/////////////////////////////////////////////////////////////////////////
///
///  \file          FingerprinterFactory.h
///  \brief         Contains the FingerprinterFactory object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-17 23:48:45
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

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |  Forward Declarations
class Fingerprinter;

} // namespace Components

namespace LocalExecution {

/////////////////////////////////////////////////////////////////////////
///  \class         FingerprinterFactory
///  \brief         Creates a Fingerprinter instances.
///
class FingerprinterFactory {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using FingerprinterUniquePtr            = std::unique_ptr<Components::Fingerprinter>;

    // ----------------------------------------------------------------------
    // |
    // |  Public Data
    // |
    // ----------------------------------------------------------------------
    static boost::uuids::uuid const         ID; // Configuration ID for QueryInterface

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    virtual ~FingerprinterFactory(void) = default;

    FingerprinterUniquePtr Create(void);

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    virtual FingerprinterUniquePtr CreateImpl(void) = 0;
};

} // namespace LocalExecution
} // namespace Core
} // namespace DecisionEngine
