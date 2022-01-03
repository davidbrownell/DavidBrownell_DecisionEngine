/////////////////////////////////////////////////////////////////////////
///
///  \file          Fingerprinter.cpp
///  \brief         See Fingerprinter.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-29 23:33:19
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
#include "Fingerprinter.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |
// |  NoopFingerprinter
// |
// ----------------------------------------------------------------------
bool NoopFingerprinter::ShouldProcess(System const &) /*override*/ {
    return true;
}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
