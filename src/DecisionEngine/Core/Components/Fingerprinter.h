/////////////////////////////////////////////////////////////////////////
///
///  \file          Fingerprinter.h
///  \brief         Contains the Fingerprinter and NoopFingerprinter
///                 objects
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-29 23:27:59
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

#include "Components.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |  Forward Declarations
class System;

/////////////////////////////////////////////////////////////////////////
///  \class         Fingerprinter
///  \brief         Functionality that is able to uniquely identify the semantics
///                 of a System and use that information to eliminate Systems that
///                 are semantically equal; this process avoids the processing of
///                 duplicate Systems.
///
class Fingerprinter {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using System                            = DecisionEngine::Core::Components::System;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    virtual ~Fingerprinter(void) = default;

    virtual bool ShouldProcess(System const &system) = 0;
};

/////////////////////////////////////////////////////////////////////////
///  \class         NoopFingerprinter
///  \brief         Fingerprinter that doesn't do anything.
///
class NoopFingerprinter : public Fingerprinter {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    ~NoopFingerprinter(void) override = default;

    bool ShouldProcess(System const &system) override;
};

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
