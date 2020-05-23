/////////////////////////////////////////////////////////////////////////
///
///  \file          Score.cpp
///  \brief         See Score.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-20 22:05:42
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
#include "Score.h"
#include "Components.h"

#include <CommonHelpers/Stl.h>

#include <cassert>

namespace DecisionEngine {
namespace Core {
namespace Components {

namespace {

static float const constexpr                GoodThreshold = MaxScore * 0.80f;

// GroupList{A|B}T will either be `Score::ResultGroup` or `Score::PendingData`
template <typename GroupLikeAT, typename GroupLikeBT>
int CompareGroups(GroupLikeAT const &a, GroupLikeBT const &b) {
    if(static_cast<void const *>(&a) == static_cast<void const *>(&b))
        return 0;

    if(a.IsSuccessful != b.IsSuccessful)
        return a.IsSuccessful == false ? -1 : 1;

    if(a.NumFailures != b.NumFailures)
        return a.NumFailures > b.NumFailures ? -1 : 1;

    float const                             diff(a.AverageScore - b.AverageScore);

    if(diff != 0.0f)
        return diff < 0.0f ? -1 : 1;

    if(a.NumResults != b.NumResults) {
        // If here, we are looking at 2 groups with equal potential. If the score
        // is a "good" one, the group with the higher number of matches has a high good
        // potential. If the score is not a "good" score, give the group with the lower
        // number of results the opportunity to get better over time.
        if(a.AverageScore >= GoodThreshold)
            return a.NumResults < b.NumResults ? -1 : 1;

        return a.NumResults > b.NumResults ? -1 : 1;
    }

    return 0;
}

} // anonymous namespace

// ----------------------------------------------------------------------
// |
// |  Score::Result
// |
// ----------------------------------------------------------------------
Score::Result::Result(
    ConditionResults applicabilityResults,
    ConditionResults requirementResults,
    ConditionResults preferenceResults
) :
    IsApplicable(false), // Placeholder
    IsSuccessful(false), // Placeholder
    Score(0.0f), // Placeholder
    ApplicabilityResults(std::move(applicabilityResults)),
    RequirementResults(std::move(requirementResults)),
    PreferenceResults(std::move(preferenceResults)) {
    // ----------------------------------------------------------------------
    struct Internal {
        static float CalculateScore(ConditionResults const &results) {
            float                           score(0.0f);
            unsigned long                   maxPossibleScore(0);

            for(auto const &result : results) {
                score += result.Ratio * result.Condition->MaxScore;
                maxPossibleScore += result.Condition->MaxScore;
            }

            return maxPossibleScore != 0 ? score / maxPossibleScore : 1.0f;
        }
    };
    // ----------------------------------------------------------------------

    make_mutable(IsApplicable) = std::all_of(ApplicabilityResults.cbegin(), ApplicabilityResults.cend(), [](Condition::Result const &cr) { return cr.IsSuccessful; });

    if(IsApplicable == false)
        return;

    // The result is successful if all of the requirements are successful
    make_mutable(IsSuccessful) = std::all_of(RequirementResults.cbegin(), RequirementResults.cend(), [](Condition::Result const &cr) { return cr.IsSuccessful; });

    // Calculate the final score based on the requirement and preference results.
    // Take special care to ensure that the preferences are never treated as more
    // important than the requirements.

    // The integer part of the final score is based on the requirements and the
    // mantissa is based on the preferences. With this, we can enforce the rule that
    // states "it is always more important to do better on the requirements than it
    // it is on the preferences."
    float const                             requirementsScore(Internal::CalculateScore(RequirementResults));
    float const                             preferencesScore(Internal::CalculateScore(PreferenceResults));

    assert(requirementsScore >= 0.0f && requirementsScore <= 1.0f);
    assert(preferencesScore >= 0.0f && preferencesScore <= 1.0f);

    make_mutable(Score) = static_cast<unsigned long>(requirementsScore * (MaxScore - 1)) + preferencesScore;
    assert(Score <= MaxScore);
}

// TODO std::string Score::Result::ToString(void) const;

// ----------------------------------------------------------------------
// |
// |  Score::ResultGroup
// |
// ----------------------------------------------------------------------
namespace {

auto ConstructResultGroupTuple(Score::ResultGroup::ResultPtrs results) {
    float                                   totalScore(0.0f);
    unsigned long                           numResults(0);
    unsigned long                           numFailures(0);

    for(auto const &pResult : results) {
        if(!pResult)
            continue;

        if(pResult->IsApplicable == false)
            continue;

        ++numResults;
        totalScore += pResult->Score;

        if(pResult->IsSuccessful == false)
            ++numFailures;
    }

    return std::make_tuple(
        std::move(results),
        numFailures == 0,
        numResults ? totalScore / numResults : totalScore,
        numResults,
        numFailures
    );
}

} // anonymous namespace

Score::ResultGroup::ResultGroup(ResultPtrs results) :
    ResultGroup(ConstructResultGroupTuple(std::move(results)))
{}

Score::ResultGroup::ResultGroup(
    ResultPtrs results,
    bool isSuccessful,
    float averageScore,
    unsigned long numResults,
    unsigned long numFailures
) :
    ResultGroup(std::make_tuple(std::move(results), isSuccessful, averageScore, numResults, numFailures))
{}

// static
int Score::ResultGroup::Compare(ResultGroup const &a, ResultGroup const &b) {
    return CompareGroups(a, b);
}

bool Score::ResultGroup::operator==(ResultGroup const &other) const {
    return Compare(*this, other) == 0;
}

bool Score::ResultGroup::operator!=(ResultGroup const &other) const {
    return Compare(*this, other) != 0;
}

bool Score::ResultGroup::operator <(ResultGroup const &other) const {
    return Compare(*this, other) < 0;
}

bool Score::ResultGroup::operator<=(ResultGroup const &other) const {
    return Compare(*this, other) <= 0;
}

bool Score::ResultGroup::operator >(ResultGroup const &other) const {
    return Compare(*this, other) > 0;
}

bool Score::ResultGroup::operator>=(ResultGroup const &other) const {
    return Compare(*this, other) >= 0;
}

// TODO std::string Score::ResultGroup::ToString(void) const;

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
Score::ResultGroup::ResultGroup(std::tuple<ResultPtrs, bool, float, unsigned long, unsigned long> args) :
    IsSuccessful(std::move(std::get<1>(args))),
    AverageScore(
        std::move(
            [&args](void) -> float & {
                float &                     score(std::get<2>(args));

                ENSURE_ARGUMENT(score, score >= 0.0f && score <= MaxScore);
                return score;
            }()
        )
    ),
    NumResults(std::move(std::get<3>(args))),
    NumFailures(std::move(std::get<4>(args))),
    Results(
        std::move(
            [&args](void) -> ResultPtrs & {
                ResultPtrs &                results(std::get<0>(args));

                ENSURE_ARGUMENT(results, results.empty() == false);
                ENSURE_ARGUMENT(results, std::all_of(results.cbegin(), results.cend(), [](ResultPtr const &ptr) { return static_cast<bool>(ptr); }));

                return results;
            }()
        )
    )
{}

// ----------------------------------------------------------------------
// |
// |  Score::PendingData
// |
// ----------------------------------------------------------------------
Score::PendingData::PendingData(ResultPtrs const *pOptionalResults, Result const *pOptionalResult) :
    IsSuccessful(false), // Placeholder
    AverageScore(0.0f), // Placeholder
    NumResults(0), // Placeholder
    NumFailures(0) // Placeholder
{
    assert(pOptionalResults == nullptr || (pOptionalResults->empty() == false && std::all_of(pOptionalResults->cbegin(), pOptionalResults->cend(), [](ResultPtr const &ptr) { return static_cast<bool>(ptr); })));

    float                                   totalScore(0.0f);
    unsigned long                           numResults(0);
    unsigned long                           numFailures(0);

    auto const                              processResultsFunc(
        [&totalScore, &numResults, &numFailures](Result const &r) {
            if(r.IsApplicable == false)
                return;

            ++numResults;

            if(r.IsSuccessful == false)
                ++numFailures;

            totalScore += r.Score;
        }
    );

    if(pOptionalResults) {
        for(auto const &ptr : *pOptionalResults) {
            processResultsFunc(*ptr);
        }
    }

    if(pOptionalResult)
        processResultsFunc(*pOptionalResult);

    float                                   averageScore(numResults ? totalScore / numResults : totalScore);

    assert(averageScore >= 0.0f && averageScore <= MaxScore);

    make_mutable(IsSuccessful) = numFailures == 0;
    make_mutable(AverageScore) = std::move(averageScore);
    make_mutable(NumResults) = std::move(numResults);
    make_mutable(NumFailures) = std::move(numFailures);
}

// ----------------------------------------------------------------------
// |
// |  Score::SuffixInfo
// |
// ----------------------------------------------------------------------
Score::SuffixInfo::SuffixInfo(Result result, bool completesGroup) :
    _result(std::move(result)),
    _isMoved(false),
    CompletesGroup(std::move(completesGroup))
{}

Score::Result Score::SuffixInfo::Move(void) {
    if(_isMoved)
        throw std::logic_error("invalid operation");

    Result                                  result(std::move(_result));

    _isMoved = true;
    return result;
}

Score::Result const & Score::SuffixInfo::GetResult(void) const {
    if(_isMoved)
        throw std::logic_error("invalid operation");

    return _result;
}

// ----------------------------------------------------------------------
// |
// |  Score
// |
// ----------------------------------------------------------------------
Score::Score(void) :
    Score(
        ResultGroupPtrsPtr(),
        ResultPtrsPtr(),
        std::unique_ptr<SuffixInfo>()
    )
{}

Score::Score(Score const &score, Result suffix, bool completesGroup) :
    Score(
        score._pResultGroups,
        score._pResults,
        std::make_unique<SuffixInfo>(std::move(suffix), completesGroup)
    )
{}

Score::Score(Score const &score, Condition::Result suffix, bool completesGroup) :
    Score(
        score._pResultGroups,
        score._pResults,
        std::make_unique<SuffixInfo>(
            Result(
                Score::Result::ConditionResults(),
                CommonHelpers::Stl::CreateVector<Condition::Result>(std::move(suffix)),
                Score::Result::ConditionResults()
            ),
            completesGroup
        )
    )
{}

// static
int Score::Compare(Score const &a, Score const &b) {
    if(a.IsSuccessful != b.IsSuccessful)
        return a.IsSuccessful == false ? -1 : 1;

    ResultGroupPtrs::const_iterator         pThisGroupPtr;
    ResultGroupPtrs::const_iterator         pThisGroupEnd;
    ResultGroupPtrs::const_iterator         pThatGroupPtr;
    ResultGroupPtrs::const_iterator         pThatGroupEnd;

    if(a._pResultGroups) {
        pThisGroupPtr = a._pResultGroups->cbegin();
        pThisGroupEnd = a._pResultGroups->cend();
    }

    if(b._pResultGroups) {
        pThatGroupPtr = b._pResultGroups->cbegin();
        pThatGroupEnd = b._pResultGroups->cend();
    }

    while(pThisGroupPtr != pThisGroupEnd && pThatGroupPtr != pThatGroupEnd) {
        int const                           result(CompareGroups(**pThisGroupPtr, **pThatGroupPtr));

        if(result != 0)
            return result;

        ++pThisGroupPtr;
        ++pThatGroupPtr;
    }

    if(pThisGroupPtr != pThisGroupEnd) {
        int const                           result(CompareGroups(**pThisGroupPtr, b._pendingData));

        if(result != 0)
            return result;

        ++pThisGroupPtr;

        bool const                          isThisSuccessful(pThisGroupPtr != pThisGroupEnd ? (*pThisGroupPtr)->IsSuccessful : a._pendingData.IsSuccessful);

        return isThisSuccessful == false ? -1 : 1;
    }

    if(pThatGroupPtr != pThatGroupEnd) {
        int const                           result(CompareGroups(a._pendingData, **pThatGroupPtr));

        if(result != 0)
            return result;

        ++pThatGroupPtr;

        bool const                          isThatSuccessful(pThatGroupPtr != pThatGroupEnd ? (*pThatGroupPtr)->IsSuccessful : b._pendingData.IsSuccessful);

        return isThatSuccessful ? -1 : 1;
    }

    return CompareGroups(a._pendingData, b._pendingData);
}

bool Score::operator==(Score const &other) const {
    return Compare(*this, other) == 0;
}

bool Score::operator!=(Score const &other) const {
    return Compare(*this, other) != 0;
}

bool Score::operator <(Score const &other) const {
    return Compare(*this, other) < 0;
}

bool Score::operator<=(Score const &other) const {
    return Compare(*this, other) <= 0;
}

bool Score::operator >(Score const &other) const {
    return Compare(*this, other) > 0;
}

bool Score::operator>=(Score const &other) const {
    return Compare(*this, other) >= 0;
}

// TODO std::string Score::ToString(void) const;

bool Score::HasSuffix(void) const {
    return static_cast<bool>(_suffix);
}

// This method should only be called when the object was created with a suffix
Score Score::Commit(void) {
    if(HasSuffix() == false)
        throw std::logic_error("Invalid operation");

    ResultPtrs                              results;

    if(_pResults) {
        results.reserve(_pResults->size() + 1);
        std::copy(_pResults->cbegin(), _pResults->cend(), std::back_inserter(results));
    }

    results.emplace_back(std::make_shared<Result>(_suffix->Move()));

    if(_suffix->CompletesGroup == false)
        return Score(_pResultGroups, std::make_shared<ResultPtrs>(std::move(results)));

    ResultGroupPtrs                         groups;

    if(_pResultGroups) {
        groups.reserve(_pResultGroups->size() + 1);
        std::copy(_pResultGroups->cbegin(), _pResultGroups->cend(), std::back_inserter(groups));
    }

    groups.emplace_back(
        std::make_shared<ResultGroup>(
            std::move(results),
            _pendingData.IsSuccessful,
            _pendingData.AverageScore,
            _pendingData.NumResults,
            _pendingData.NumFailures
        )
    );

    return Score(std::make_shared<ResultGroupPtrs>(std::move(groups)));
}

// This method should only be called when the object was created without a suffix
Score Score::Copy(void) const {
    if(HasSuffix())
        throw std::logic_error("Invalid operation");

    if(_pResults)
        return Score(_pResultGroups, _pResults);

    if(_pResultGroups)
        return Score(_pResultGroups);

    return Score();
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
Score::Score(ResultGroupPtrsPtr pResultGroups) :
    Score(
        std::move(
            [&pResultGroups](void) -> ResultGroupPtrsPtr & {
                assert(pResultGroups && pResultGroups->empty() == false && std::all_of(pResultGroups->cbegin(), pResultGroups->cend(), [](ResultGroupPtr const &ptr) { return static_cast<bool>(ptr); }));
                return pResultGroups;
            }()
        ),
        ResultPtrsPtr(),
        std::unique_ptr<SuffixInfo>()
    )
{}

Score::Score(ResultGroupPtrsPtr pResultGroups, ResultPtrsPtr pResults) :
    Score(
        std::move(pResultGroups),
        std::move(
            [&pResults](void) -> ResultPtrsPtr & {
                assert(pResults && pResults->empty() == false && std::all_of(pResults->cbegin(), pResults->cend(), [](ResultPtr const &ptr) { return static_cast<bool>(ptr); }));
                return pResults;
            }()
        ),
        std::unique_ptr<SuffixInfo>()
    )
{}

Score::Score(ResultGroupPtrsPtr pResultGroups, ResultPtrsPtr pResults, std::unique_ptr<SuffixInfo> suffix) :
    IsSuccessful(false), // Placeholder
    _pResultGroups(std::move(pResultGroups)),
    _pResults(std::move(pResults)),
    _suffix(std::move(suffix)),
    _pendingData(
        _pResults.get(),
        _suffix ? &_suffix->GetResult() : nullptr
    )
{
    make_mutable(IsSuccessful) =
        [this](void) {
            if(_pResultGroups) {
                if(std::all_of(_pResultGroups->cbegin(), _pResultGroups->cend(), [](ResultGroupPtr const &pGroup) { return pGroup->IsSuccessful; }) == false)
                    return false;
            }

            if(_pResults) {
                if(std::all_of(_pResults->cbegin(), _pResults->cend(), [](ResultPtr const &ptr) { return ptr->IsApplicable == false || ptr->IsSuccessful; }) == false)
                    return false;
            }

            if(_suffix) {
                Result const &              result(_suffix->GetResult());

                if(result.IsApplicable && result.IsSuccessful == false)
                    return false;
            }

            return true;
        }();
}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
