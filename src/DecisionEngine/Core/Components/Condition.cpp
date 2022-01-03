/////////////////////////////////////////////////////////////////////////
///
///  \file          Condition.cpp
///  \brief         See Condition.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-08 21:17:23
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
#include "Condition.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |
// |  Condition::Result
// |
// ----------------------------------------------------------------------
Condition::Result::Result(ConditionPtr pCondition, bool isSuccessful, std::optional<std::string> reason /*=std::nullopt*/) :
    Result(std::move(pCondition), std::move(isSuccessful), isSuccessful ? 1.0f : 0.0f, std::move(reason))
{}

Condition::Result::Result(ConditionPtr pCondition, float ratio, std::optional<std::string> reason /*=std::nullopt*/) :
    Result(std::move(pCondition), ratio > 0.0f, std::move(ratio), std::move(reason))
{}

Condition::Result::Result(ConditionPtr pCondition, bool isSuccessful, float ratio, std::optional<std::string> reason /*=std::nullopt*/) :
    Condition(
        std::move(
            [&pCondition](void) -> ConditionPtr & {
                ENSURE_ARGUMENT(pCondition);
                return pCondition;
            }()
        )
    ),
    IsSuccessful(std::move(isSuccessful)),
    Ratio(
        std::move(
            [&ratio](void) -> float & {
                ENSURE_ARGUMENT(ratio, ratio >= 0.0f && ratio <= 1.0f);
                return ratio;
            }()
        )
    ),
    Reason(
        [&reason](void) -> std::string {
            if(reason) {
                ENSURE_ARGUMENT(reason, reason->empty() == false);
                return std::move(*reason);
            }

            return std::string();
        }()
    )
{}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DEFINE(DecisionEngine::Core::Components::Condition);
