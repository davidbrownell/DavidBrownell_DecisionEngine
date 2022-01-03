/////////////////////////////////////////////////////////////////////////
///
///  \file          System.cpp
///  \brief         See System.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-23 22:18:56
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
#include "System.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |
// |  System
// |
// ----------------------------------------------------------------------
System::System(TypeValue type, CompletionValue completion, Score score, Index index) :
    _score(std::move(score)),
    _index(std::move(index)),
    Type(std::move(type)),
    Completion(std::move(completion))
{
    if(Completion == CompletionValue::Calculated) {
        ENSURE_ARGUMENT(score, _score.HasSuffix());
        ENSURE_ARGUMENT(index, _index.HasSuffix());
    }
    else if(Completion == CompletionValue::Concrete) {
        ENSURE_ARGUMENT(score, _score.HasSuffix() == false);
        ENSURE_ARGUMENT(index, _index.HasSuffix() == false);
    }
    else
        assert(!"Unexpected CompletionValue");
}

System & System::UpdateScore(Score score) {
    if(Completion == CompletionValue::Calculated) {
        ENSURE_ARGUMENT(score, score.HasSuffix());
    }
    else if(Completion == CompletionValue::Concrete) {
        ENSURE_ARGUMENT(score, score.HasSuffix() == false);
    }
    else
        assert(!"Unexpected CompletionValue");

    _score = std::move(score);
    return *this;
}

Score const & System::GetScore(void) const {
    return _score;
}

Index const & System::GetIndex(void) const {
    return _index;
}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::Core::Components::System);
