/////////////////////////////////////////////////////////////////////////
///
///  \file          Score.h
///  \brief         Contains the Score object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-19 21:18:11
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

#include "Condition.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

/////////////////////////////////////////////////////////////////////////
///  \class         Score
///  \brief         The result of applying one more more `Conditions` to
///                 a system in its current state. Different `Scores`
///                 obtained by measuring systems in different states are
///                 comparable, regardless of how close the system is to
///                 completion. These comparisons can be used to determine
///                 the overall potential of the system to reach a desired
///                 solution or state.
///
class Score {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------

    /////////////////////////////////////////////////////////////////////////
    ///  \class         Result
    ///  \brief         The result of applying one or more `Conditions` to the
    ///                 system in its current state. A `Score` is made up of
    ///                 zero or more `Results`, organized into groups.
    ///
    class Result {
    public:
        // ----------------------------------------------------------------------
        // |  Public Types
        using ConditionResults              = std::vector<Condition::Result>;

        // ----------------------------------------------------------------------
        // |  Public Data
        bool const                          IsApplicable;
        bool const                          IsSuccessful;
        float                               Score;

        ConditionResults const              ApplicabilityResults;
        ConditionResults const              RequirementResults;
        ConditionResults const              PreferenceResults;

        // ----------------------------------------------------------------------
        // |  Public Methods
        Result(
            ConditionResults applicabilityResults,
            ConditionResults requirementResults,
            ConditionResults preferenceResults
        );

#define ARGS                                MEMBERS(IsApplicable, IsSuccessful, Score, ApplicabilityResults, RequirementResults, PreferenceResults)

        NON_COPYABLE(Result);
        MOVE(Result, ARGS);
        SERIALIZATION(Result, ARGS);

        // Comparison doesn't depend on the specific ConditionResults
        COMPARE(Result, MEMBERS(IsApplicable, IsSuccessful, Score));

#undef ARGS

        std::string ToString(void) const;
    };

    /////////////////////////////////////////////////////////////////////////
    ///  \class         ResultGroup
    ///  \brief         One or more `Results`, organized into priority groups.
    ///                 Groups are always ordered together, meaning two systems
    ///                 with two groups will always sort according to the first
    ///                 group if one is greater than the other, regardless of the
    ///                 sort order of the second group. This is different from
    ///                 `Results` within a single group, where each individual
    ///                 `Result` contributes to the comparison regardless of
    ///                 its order within the group itself.
    ///
    class ResultGroup {
    public:
        // ----------------------------------------------------------------------
        // |  Public Types
        using ResultPtr                     = std::shared_ptr<Result>;
        using ResultPtrs                    = std::vector<ResultPtr>;

        // ----------------------------------------------------------------------
        // |  Public Data
        bool const                          IsSuccessful;
        float const                         AverageScore;
        unsigned long const                 NumResults;
        unsigned long const                 NumFailures;
        ResultPtrs const                    Results;

        // ----------------------------------------------------------------------
        // |  Public Methods
        ResultGroup(ResultPtrs results);

        // This ctor is invoked when the other data has already
        // been calculated.
        ResultGroup(
            ResultPtrs results,
            bool isSuccessful,
            float averageScore,
            unsigned long numResults,
            unsigned long numFailures
        );

#define ARGS                                MEMBERS(IsSuccessful, AverageScore, NumResults, NumFailures, Results)

        NON_COPYABLE(ResultGroup);
        MOVE(ResultGroup, ARGS);
        SERIALIZATION(ResultGroup, ARGS);

#undef ARGS

        // Comparison is too complicated for the default implementation
        static int Compare(ResultGroup const &a, ResultGroup const &b);

        bool operator==(ResultGroup const &other) const;
        bool operator!=(ResultGroup const &other) const;
        bool operator <(ResultGroup const &other) const;
        bool operator<=(ResultGroup const &other) const;
        bool operator >(ResultGroup const &other) const;
        bool operator>=(ResultGroup const &other) const;

        std::string ToString(void) const;

    private:
        // ----------------------------------------------------------------------
        // |  Private Methods
        ResultGroup(std::tuple<ResultPtrs, bool, float, unsigned long, unsigned long> args);
    };

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Data
    // |
    // ----------------------------------------------------------------------
    bool const                              IsSuccessful;

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Types (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    using ResultPtr                         = std::shared_ptr<Result>;
    using ResultPtrs                        = std::vector<ResultPtr>;
    using ResultPtrsPtr                     = std::shared_ptr<ResultPtrs>;

    using ResultGroupPtr                    = std::shared_ptr<ResultGroup>;
    using ResultGroupPtrs                   = std::vector<ResultGroupPtr>;
    using ResultGroupPtrsPtr                = std::shared_ptr<ResultGroupPtrs>;

    /////////////////////////////////////////////////////////////////////////
    ///  \class         PendingData
    ///  \brief         Stores information about the system in its current state.
    ///                 This cached comparison info prevents the results from
    ///                 being repeated processed during comparison calculations.
    ///
    class PendingData {
    public:
        // ----------------------------------------------------------------------
        // |  Public Data
        bool const                          IsSuccessful;
        float const                         AverageScore;
        unsigned long const                 NumResults;
        unsigned long const                 NumFailures;

        // ----------------------------------------------------------------------
        // |  Public Methods
        PendingData(ResultPtrs const *pOptionalResults, Result const *pOptionalResult);

#define ARGS                                MEMBERS(IsSuccessful, AverageScore, NumResults, NumFailures)

        NON_COPYABLE(PendingData);
        MOVE(PendingData, ARGS);
        COMPARE(PendingData, ARGS);
        SERIALIZATION(PendingData, ARGS);

#undef ARGS

        std::string ToString(void) const;
    };

    /////////////////////////////////////////////////////////////////////////
    ///  \class         SuffixInfo
    ///  \brief         Contains information that is used when the `Score` is
    ///                 committed.
    ///
    class SuffixInfo {
    private:
        // ----------------------------------------------------------------------
        // |  Private Data (used in public declarations)
        Score::Result                       _result;
        bool                                _isMoved;

    public:
        // ----------------------------------------------------------------------
        // |  Public Data
        bool const                          CompletesGroup;

        // ----------------------------------------------------------------------
        // |  Public Methods
        SuffixInfo(Result result, bool completesGroup);

#define ARGS                                MEMBERS(_result, _isMoved, CompletesGroup)

        NON_COPYABLE(SuffixInfo);
        MOVE(SuffixInfo, ARGS);
        COMPARE(SuffixInfo, ARGS);
        SERIALIZATION(SuffixInfo, ARGS);

#undef ARGS

        Result Move(void);
        Result const & GetResult(void) const;

        std::string ToString(void) const;
    };

    // ----------------------------------------------------------------------
    // |
    // |  Private Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    ResultGroupPtrsPtr const                _pResultGroups;
    ResultPtrsPtr const                     _pResults;

    std::unique_ptr<SuffixInfo>             _suffix;

    PendingData const                       _pendingData;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    Score(void);
    Score(Result suffix, bool completesGroup);
    Score(Condition::Result suffix, bool completesGroup);
    Score(Score const &score, Result suffix, bool completesGroup);
    Score(Score const &score, Condition::Result suffix, bool completesGroup);

#define ARGS                                MEMBERS(IsSuccessful, _pResultGroups, _pResults, _suffix, _pendingData)

    NON_COPYABLE(Score);
    MOVE(Score, ARGS);
    SERIALIZATION(Score, ARGS);

#undef ARGS

    // Comparison is too complicated for the default implementation
    static int Compare(Score const &a, Score const &b);

    bool operator==(Score const &other) const;
    bool operator!=(Score const &other) const;
    bool operator <(Score const &other) const;
    bool operator<=(Score const &other) const;
    bool operator >(Score const &other) const;
    bool operator>=(Score const &other) const;

    std::string ToString(void) const;

    bool HasSuffix(void) const;

    template <typename FunctionT>
    // bool (ResultGroup const &);
    bool EnumResultGroups(FunctionT const &func) const;

    template <typename FunctionT>
    // bool (Result const &);
    bool EnumResults(FunctionT const &func) const;

    template <typename FunctionT>
    // bool (Result const &)
    bool EnumAllResults(FunctionT const &func) const;

    // This method should only be called when the object was created with a suffix
    Score Commit(void);

    // This method should only be called when the object was created without a suffix
    Score Copy(void) const;

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    Score(ResultGroupPtrsPtr pResultGroups);
    Score(ResultGroupPtrsPtr pResultGroups, ResultPtrsPtr pResults);
    Score(ResultGroupPtrsPtr pResultGroups, ResultPtrsPtr pResults, std::unique_ptr<SuffixInfo> suffix);
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
template <typename FunctionT>
// bool (ResultGroup const &);
bool Score::EnumResultGroups(FunctionT const &func) const {
    if(_pResultGroups) {
        for(auto const &pResultGroup : *_pResultGroups) {
            if(func(*pResultGroup) == false)
                return false;
        }
    }

    return true;
}

template <typename FunctionT>
// bool (Result const &);
bool Score::EnumResults(FunctionT const &func) const {
    if(_pResults) {
        for(auto const &pResult : *_pResults) {
            if(func(*pResult) == false)
                return false;
        }
    }

    if(_suffix && func(_suffix->GetResult()) == false)
        return false;

    return true;
}

template <typename FunctionT>
// bool (Result const &)
bool Score::EnumAllResults(FunctionT const &func) const {
    auto const                              groupCallback(
        [&func](ResultGroup const &group) -> bool {
            for(auto const &pResult : group.Results) {
                if(func(*pResult) == false)
                    return false;
            }

            return true;
        });

    if(EnumResultGroups(groupCallback) == false)
        return false;

    if(EnumResults(func) == false)
        return false;

    return true;
}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
