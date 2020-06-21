/////////////////////////////////////////////////////////////////////////
///
///  \file          Engine_UnitTest.cpp
///  \brief         Unit test for Engine
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-06-06 00:35:29
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
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../Engine.h"
#include <catch.hpp>

#include <DecisionEngine/Core/Components/Condition.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

namespace Components                        = DecisionEngine::Core::Components;
namespace LocalExecution                    = DecisionEngine::Core::LocalExecution;

// ----------------------------------------------------------------------
// |
// |  Internal Types and Methods
// |
// ----------------------------------------------------------------------
class MyObserver : public LocalExecution::Engine::Observer {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    ~MyObserver(void) override = default;

    std::vector<std::string> const & GetStrings(bool sort=true) {
        if(sort)
            std::sort(_strings.begin(), _strings.end());

        return _strings;
    }

    bool OnRoundBegin(size_t round, SystemPtrs const &pending) override {
        AddString(boost::format("OnRoundBegin: %1%, %2%") % round % ToString("pending ", pending));
        return true;
    }

    void OnRoundEnd(size_t round, SystemPtrs const &pending) override {
        AddString(boost::format("OnRoundEnd: %1%, %2%") % round % ToString("pending ", pending));
    }

    bool OnRoundMergingWork(size_t round, SystemPtrsContainer const &pending) override {
        AddString(boost::format("OnRoundMergingWork: %1%, %2%") % round % ToString("pending ", pending));
        return true;
    }

    void OnRoundMergedWork(size_t round, SystemPtrs const &pending, SystemPtrsContainer removed) override {
        AddString(boost::format("OnRoundMergedWork: %1%, %2%, %3%") % round % ToString("pending ", pending) % ToString("removed ", removed));
    }

    bool OnTaskBegin(size_t round, size_t task, size_t numTasks) override {
        AddString(boost::format("OnTaskBegin: %1%, %2%, %3%") % round % task % numTasks);
        return true;
    }

    void OnTaskEnd(size_t round, size_t task, size_t numTasks) override {
        AddString(boost::format("OnTaskEnd: %1%, %2%, %3%") % round % task % numTasks);
    }

    void OnTaskError(size_t round, size_t task, size_t numTasks, std::exception const &ex) override {
        AddString(boost::format("OnTaskError: %1%, %2%, %3%, %4%") % round % task % numTasks % ex.what());
    }

    bool OnIterationBegin(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations) override {
        AddString(boost::format("OnIterationBegin: %1%, %2%, %3%, %4%, %5%") % round % task % numTasks % iteration % numIterations);
        return true;
    }

    void OnIterationEnd(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations) override {
        AddString(boost::format("OnIterationEnd: %1%, %2%, %3%, %4%, %5%") % round % task % numTasks % iteration % numIterations);
    }

    bool OnIterationGeneratingWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active) override {
        AddString(boost::format("OnIterationGeneratingWork: %1%, %2%, %3%, %4%, %5%, %6%") % round % task % numTasks % iteration % numIterations % ToString("active ", active));
        return true;
    }

    void OnIterationGeneratedWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &generated) override {
        AddString(boost::format("OnIterationGeneratedWork: %1%, %2%, %3%, %4%, %5%, %6%, %7%") % round % task % numTasks % iteration % numIterations % ToString("active ", active) % ToString("generated ", generated));
    }

    bool OnIterationMergingWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &generated, SystemPtrs const &pending) override {
        AddString(boost::format("OnIterationMergingWork: %1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%") % round % task % numTasks % iteration % numIterations % ToString("active ", active) % ToString("generated ", generated) % ToString("pending ", pending));
        return true;
    }

    void OnIterationMergedWork(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs const &pending, SystemPtrsContainer removed) override {
        AddString(boost::format("OnIterationMergedWork: %1%, %2%, %3%, %4%, %5%, %6%, %7%, %8%") % round % task % numTasks % iteration % numIterations % ToString("active ", active) % ToString("pending ", pending) % ToString("removed ", removed));
    }

    void OnIterationFailedSystems(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, WorkingSystem const &active, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) override {
        AddString(boost::format("OnIterationFailedSystems: %1%, %2%, %3%, %4%, %5%, %6%, %7%") % round % task % numTasks % iteration % numIterations % ToString("active ", active) % std::distance(begin, end));
    }

private:
    // ----------------------------------------------------------------------
    // |  Private Data
    std::mutex                              _stringsMutex;
    std::vector<std::string>                _strings;

    // ----------------------------------------------------------------------
    // |  Private Methods
    void AddString(boost::format const &fmt) {
        std::scoped_lock<decltype(_stringsMutex)> const                     lock(_stringsMutex); UNUSED(lock);

        _strings.emplace_back(boost::str(fmt));
    }

    static std::string ToString(char const *desc, LocalExecution::Engine::SystemPtrs const &ptrs) {
        std::vector<std::string>            strings;

        strings.reserve(ptrs.size());

        for(LocalExecution::Engine::SystemPtr const &ptr : ptrs) {
            strings.emplace_back(ptr->ToString());
        }

        return boost::str(
            boost::format("%1%[%2%]") % desc % boost::algorithm::join(strings, ",")
        );
    }

    static std::string ToString(char const *desc, LocalExecution::Engine::SystemPtrsContainer const &allPtrs) {
        std::vector<std::string>            strings;

        strings.reserve(allPtrs.size());

        for(LocalExecution::Engine::SystemPtrs const & ptrs : allPtrs) {
            strings.emplace_back(ToString("", ptrs));
        }

        return boost::str(
            boost::format("%1%[%2%]") % desc % boost::algorithm::join(strings, ",")
        );
    }

    static std::string ToString(char const *desc, Components::WorkingSystem const &system) {
        return boost::str(boost::format("%1%%2%") % desc % system.ToString());
    }
};

class MyCondition : public Components::Condition {
public:
    // ----------------------------------------------------------------------
    // |  Public Types
    using IndexesType                       = std::vector<size_t>;

    // ----------------------------------------------------------------------
    // |  Public Data
    IndexesType const                       Indexes;
    bool const                              MismatchesAreFailures;

    // ----------------------------------------------------------------------
    // |  Public Methods
    CREATE(MyCondition);

    template <typename PrivateConstructorTagT>
    MyCondition(PrivateConstructorTagT tag, IndexesType indexes, bool mismatchesAreFailures) :
        Components::Condition(tag, "MyCondition", 10000),
        Indexes(
            std::move(
                [&indexes](void) -> IndexesType & {
                    ENSURE_ARGUMENT(indexes, indexes.empty() == false);
                    return indexes;
                }()
            )
        ),
        MismatchesAreFailures(mismatchesAreFailures)
    {}

#define ARGS                                MEMBERS(Indexes, MismatchesAreFailures), BASES(Components::Condition)

    NON_COPYABLE(MyCondition);
    MOVE(MyCondition, ARGS);
    COMPARE(MyCondition, ARGS);
    SERIALIZATION(MyCondition, ARGS, FLAGS(SERIALIZATION_SHARED_OBJECT, SERIALIZATION_POLYMORPHIC(Components::Condition)));

#undef ARGS

    Result Apply(Components::Index const &index) const {
        IndexesType::const_iterator         iter(Indexes.begin());

        index.Enumerate(
            [this, &iter](Components::Index::value_type value) {
                if(value != *iter)
                    return false;

                ++iter;
                return iter != Indexes.end();
            }
        );

        float                               ratio(
            [this, &iter](void) {
                if(iter == Indexes.end())
                    return 1.0f;

                if(Indexes.size() == 1)
                    return 0.0f;

                return static_cast<float>(std::distance(Indexes.begin(), iter)) / static_cast<float>(Indexes.size());
            }()
        );

        return Result(
            SharedFromThis(),
            ratio == 1.0f || MismatchesAreFailures == false,
            ratio
        );
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyCondition);

class MyResultSystem : public Components::ResultSystem {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    using Components::ResultSystem::ResultSystem;

#define ARGS                                BASES(Components::ResultSystem)

    NON_COPYABLE(MyResultSystem);
    MOVE(MyResultSystem, ARGS);
    COMPARE(MyResultSystem, ARGS);
    SERIALIZATION(MyResultSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(Components::System)));

#undef ARGS

    std::string ToString(void) const override {
        return boost::str(
            boost::format("MyResultSystem(%s,%s)")
                % GetScore().ToString()
                % GetIndex().ToString()
        );
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyResultSystem);

class MyCalculatedResultSystem : public Components::CalculatedResultSystem {
public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    using Components::CalculatedResultSystem::CalculatedResultSystem;

#define ARGS                                BASES(Components::CalculatedResultSystem)

    NON_COPYABLE(MyCalculatedResultSystem);
    MOVE(MyCalculatedResultSystem, ARGS);
    COMPARE(MyCalculatedResultSystem, ARGS);
    SERIALIZATION(MyCalculatedResultSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(Components::System)));

#undef ARGS

    std::string ToString(void) const override {
        return boost::str(
            boost::format("MyCalculatedResultSystem(%s,%s)")
                % GetScore().ToString()
                % GetIndex().ToString()
        );
    }

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    ResultSystemUniquePtr CommitImpl(Score score, Index index) override {
        return std::make_unique<MyResultSystem>(std::move(score), std::move(index));
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyCalculatedResultSystem);

class MyWorkingSystem : public Components::WorkingSystem {
private:
    // ----------------------------------------------------------------------
    // |  Private Data (used in public declarations)
    size_t                                  _childIndex;

public:
    // ----------------------------------------------------------------------
    // |  Public Types
    using MyConditionPtr                    = std::shared_ptr<MyCondition>;

    // ----------------------------------------------------------------------
    // |  Public Data
    size_t const                            MaxNumChildren;
    MyConditionPtr const                    Condition;

    // ----------------------------------------------------------------------
    // |  Public Methods
    MyWorkingSystem(size_t maxNumChildren, MyConditionPtr pCondition) :
        MyWorkingSystem(
            std::move(maxNumChildren),
            std::move(pCondition),
            Components::Score(),
            Components::Index()
        )
    {}

    MyWorkingSystem(size_t maxNumChildren, MyConditionPtr pCondition, Components::Score score, Components::Index index) :
        Components::WorkingSystem(std::move(score), std::move(index)),
        _childIndex(0),
        MaxNumChildren(
            std::move(
                [&maxNumChildren](void) -> size_t & {
                    ENSURE_ARGUMENT(maxNumChildren);
                    return maxNumChildren;
                }()
            )
        ),
        Condition(
            std::move(
                [&pCondition](void) -> MyConditionPtr & {
                    ENSURE_ARGUMENT(pCondition);
                    return pCondition;
                }()
            )
        )
    {}

#define ARGS                                MEMBERS(_childIndex, MaxNumChildren, Condition), BASES(Components::WorkingSystem)

    NON_COPYABLE(MyWorkingSystem);
    MOVE(MyWorkingSystem, ARGS);
    COMPARE(MyWorkingSystem, ARGS);
    SERIALIZATION(MyWorkingSystem, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(Components::System)));

#undef ARGS

    std::string ToString(void) const override {
        return boost::str(
            boost::format("MyWorkingSystem(%s,%s)")
                % GetScore().ToString()
                % GetIndex().ToString()
        );
    }

    bool IsComplete(void) const override {
        return _childIndex == MaxNumChildren;
    }

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    virtual SystemPtrs GenerateChildrenImpl(size_t maxNumChildren) override {
        // ----------------------------------------------------------------------
        using CreateSystemFunc              = std::function<LocalExecution::Engine::SystemPtr (Components::Score, Components::Index)>;
        // ----------------------------------------------------------------------

        size_t                              toGenerate(std::min(MaxNumChildren -_childIndex, maxNumChildren));

        assert(toGenerate);

        bool const                          isFinal(GetIndex().Depth() + 1 == Condition->Indexes.size());
        CreateSystemFunc const              createSystemFunc(
            [this, isFinal](void) -> CreateSystemFunc {
                if(isFinal) {
                    return
                        [](Components::Score score, Components::Index index) -> LocalExecution::Engine::SystemPtr {
                            return std::make_shared<MyCalculatedResultSystem>(std::move(score), std::move(index));
                        };
                }

                return
                    [this](Components::Score score, Components::Index index) -> LocalExecution::Engine::SystemPtr {
                        return std::make_shared<MyWorkingSystem>(
                            MaxNumChildren,
                            Condition,
                            std::move(score).Commit(),
                            std::move(index).Commit()
                        );
                    };
            }()
        );

        SystemPtrs                          results;

        while(toGenerate--) {
            results.emplace_back(
                createSystemFunc(
                    Components::Score(GetScore(), Condition->Apply(GetIndex()), isFinal),
                    Components::Index(GetIndex(), _childIndex)
                )
            );

            ++_childIndex;
        }

        return results;
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(MyWorkingSystem);

class Configuration : public LocalExecution::Configuration {
private:
    // ----------------------------------------------------------------------
    // |  Private Data (used in public declarations)
    size_t const                            _maxNumChildrenPerGeneration;

public:
    // ----------------------------------------------------------------------
    // |  Public Methods
    Configuration(
        size_t maxNumChildrenPerGeneration,
        bool continueProcessingSystemsWithFailures,
        bool isDeterministic,
        boost::optional<size_t> numConcurrentTasks=boost::none
    ) :
        LocalExecution::Configuration(
            std::move(continueProcessingSystemsWithFailures),
            std::move(isDeterministic),
            std::move(numConcurrentTasks)
        ),
        _maxNumChildrenPerGeneration(std::move(maxNumChildrenPerGeneration))
    {}

#define ARGS                                MEMBERS(_maxNumChildrenPerGeneration), BASES(LocalExecution::Configuration)

    NON_COPYABLE(Configuration);
    MOVE(Configuration, ARGS);
    COMPARE(Configuration, ARGS);
    SERIALIZATION(Configuration, ARGS, FLAGS(SERIALIZATION_POLYMORPHIC(LocalExecution::Configuration)));

#undef ARGS

    size_t GetMaxNumPendingSystems(void) const override {
        return std::numeric_limits<size_t>::max();
    }

    size_t GetMaxNumPendingSystems(WorkingSystem const &) const override {
        return std::numeric_limits<size_t>::max();
    }

    size_t GetMaxNumChildrenPerGeneration(WorkingSystem const &) const override {
        return _maxNumChildrenPerGeneration;
    }

    size_t GetMaxNumIterationsPerRound(WorkingSystem const &) const override {
        return std::numeric_limits<size_t>::max();
    }
};

SERIALIZATION_POLYMORPHIC_DECLARE_AND_DEFINE(Configuration);

std::vector<size_t> GetIndexes(Components::ResultSystem const &resultParam) {
    assert(dynamic_cast<MyResultSystem const *>(&resultParam));

    MyResultSystem const &                  result(static_cast<MyResultSystem const &>(resultParam));
    std::vector<size_t>                     indexes;

    result.GetIndex().Enumerate(
        [&indexes](Components::Index::value_type value) {
            indexes.emplace_back(std::move(value));
            return true;
        }
    );

    return indexes;
}

TEST_CASE("0 - 10 - 10") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(
        10,
        true,
        true
    );

    MyObserver                                          observer;

    std::tie(result, pResult) = LocalExecution::Engine::Execute(
        configuration,
        observer,
        MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{0}, false))
    );

    CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
    CHECK(GetIndexes(*pResult) == std::vector<size_t>{0});
    CHECK(
        observer.GetStrings() ==
        std::vector<std::string>{
            "OnIterationBegin: 0, 0, 1, 0, 18446744073709551615",
            "OnIterationBegin: 0, 0, 1, 1, 18446744073709551615",
            "OnIterationEnd: 0, 0, 1, 0, 18446744073709551615",
            "OnIterationEnd: 0, 0, 1, 1, 18446744073709551615",
            "OnIterationGeneratedWork: 0, 0, 1, 0, 18446744073709551615, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated []",
            "OnIterationGeneratingWork: 0, 0, 1, 0, 18446744073709551615, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
            "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
            "OnRoundEnd: 0, pending []",
            "OnTaskBegin: 0, 0, 1",
            "OnTaskEnd: 0, 0, 1"
        }
    );
}

TEST_CASE("5 - 10 - 10") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(
        10,
        true,
        true
    );

    MyObserver                                          observer;

    std::tie(result, pResult) = LocalExecution::Engine::Execute(
        configuration,
        observer,
        MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{5}, false))
    );

    CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
    CHECK(GetIndexes(*pResult) == std::vector<size_t>{5});
    CHECK(
        observer.GetStrings() ==
        std::vector<std::string>{
            "OnIterationBegin: 0, 0, 1, 0, 18446744073709551615",
            "OnIterationBegin: 0, 0, 1, 1, 18446744073709551615",
            "OnIterationEnd: 0, 0, 1, 0, 18446744073709551615",
            "OnIterationEnd: 0, 0, 1, 1, 18446744073709551615",
            "OnIterationGeneratedWork: 0, 0, 1, 0, 18446744073709551615, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated []",
            "OnIterationGeneratingWork: 0, 0, 1, 0, 18446744073709551615, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
            "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
            "OnRoundEnd: 0, pending []",
            "OnTaskBegin: 0, 0, 1",
            "OnTaskEnd: 0, 0, 1"
        }
    );
}
