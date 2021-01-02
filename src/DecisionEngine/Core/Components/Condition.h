/////////////////////////////////////////////////////////////////////////
///
///  \file          Condition.h
///  \brief         Contains the Condition object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-08 20:28:58
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

#include "Components.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

/////////////////////////////////////////////////////////////////////////
///  \class         Condition
///  \brief         Criteria that can be applied to a `Request` or a `Resource`,
///                 resulting in one or more `Condition::Result` items.
///                 `Conditions` are named and associated with a maximum score
///                 (where the score is an arbitrary value meaningful only
///                 to the `Condition` itself).
///
class Condition : public BoostHelpers::SharedObject {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------

    /////////////////////////////////////////////////////////////////////////
    ///  \class         Result
    ///  \brief         IsThe result of applying a `Condition` to a `Request` or
    ///                 a `Resource`. `Results` are successful or unsuccessful,
    ///                 where a Ratio can be used to indicate partial success.
    ///                 The Ratio is an arbitrary value meaningful only to the
    ///                 `Result` itself.
    ///
    class Result {
    public:
        // ----------------------------------------------------------------------
        // |  Public Types
        using ConditionPtr                  = std::shared_ptr<Condition>;

        // ----------------------------------------------------------------------
        // |  Public Data
        ConditionPtr const                  Condition;
        bool const                          IsSuccessful;
        float const                         Ratio;
        std::string const                   Reason;

        // ----------------------------------------------------------------------
        // |  Public Methods
        Result(ConditionPtr pCondition, bool isSuccessful, std::optional<std::string> reason=std::nullopt);
        Result(ConditionPtr pCondition, float ratio, std::optional<std::string> reason=std::nullopt);
        Result(ConditionPtr pCondition, bool isSuccessful, float ratio, std::optional<std::string> reason=std::nullopt);

#define ARGS                                MEMBERS(Condition, IsSuccessful, Ratio, Reason)

        NON_COPYABLE(Result);
        MOVE(Result, ARGS);
        COMPARE(Result, ARGS);
        SERIALIZATION(Result, ARGS);

#undef ARGS
    };

    // ----------------------------------------------------------------------
    // |
    // |  Public Data
    // |
    // ----------------------------------------------------------------------
    std::string const                       Name;
    unsigned short const                    MaxScore;

    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    CREATE(Condition);

    template <typename PrivateConstructorTagT>
    Condition(PrivateConstructorTagT tag, std::string name, unsigned short maxScore);

    virtual ~Condition(void) = default;

#define ARGS                                MEMBERS(Name, MaxScore), BASES(BoostHelpers::SharedObject)

    NON_COPYABLE(Condition);
    MOVE(Condition, ARGS);
    COMPARE(Condition, ARGS);
    SERIALIZATION(Condition, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_POLYMORPHIC_BASE));

#undef ARGS
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
template <typename PrivateConstructorTagT>
Condition::Condition(PrivateConstructorTagT tag, std::string name, unsigned short maxScore) :
    BoostHelpers::SharedObject(tag),
    Name(
        std::move(
            [&name](void) -> std::string & {
                ENSURE_ARGUMENT(name, name.empty() == false);
                return name;
            }()
        )
    ),
    MaxScore(
        std::move(
            [&maxScore](void) -> unsigned short & {
                ENSURE_ARGUMENT(maxScore);
                return maxScore;
            }()
        )
    )
{}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine

SERIALIZATION_POLYMORPHIC_DECLARE(DecisionEngine::Core::Components::Condition);
