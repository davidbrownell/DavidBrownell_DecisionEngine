/////////////////////////////////////////////////////////////////////////
///
///  \file          FingerprinterFactory.cpp
///  \brief         See FingerprinterFactory.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-17 23:54:41
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
#include "FingerprinterFactory.h"

#include <DecisionEngine/Core/Components/Fingerprinter.h>

namespace DecisionEngine {
namespace Core {
namespace LocalExecution {

// ----------------------------------------------------------------------
// |
// |  FingerprinterFactory
// |
// ----------------------------------------------------------------------
FingerprinterFactory::FingerprinterUniquePtr FingerprinterFactory::Create(void) {
    FingerprinterUniquePtr                  result(CreateImpl());

    if(!result)
        throw std::runtime_error("Invalid result");

    return result;
}

// FE0B5B2D3FFA4ED6B4EDB4DC0ABE35C0

// static
boost::uuids::uuid const FingerprinterFactory::ID = { 0xFE, 0x0B, 0x5B, 0x2D, 0x3F, 0xFA, 0x4E, 0xD6, 0xB4, 0xED, 0xB4, 0xDC, 0x0A, 0xBE, 0x35, 0xC0 };

} // namespace LocalExecution
} // namespace Core
} // namespace DecisionEngine
