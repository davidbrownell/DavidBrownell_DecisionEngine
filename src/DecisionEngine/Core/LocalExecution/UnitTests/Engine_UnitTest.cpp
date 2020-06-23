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
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../Engine.h"
#include <catch.hpp>

#include <DecisionEngine/Core/Components/Condition.h>

#include <boost/algorithm/string/replace.hpp>

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

        // Replace size_t::max with -1 for consistency across x64 and x86
        std::string const                   maxValue(std::to_string(std::numeric_limits<size_t>::max()));

        for(std::string &str : _strings)
            boost::algorithm::replace_all(str, maxValue, "-1");

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

    bool OnIterationFailedSystems(size_t round, size_t task, size_t numTasks, size_t iteration, size_t numIterations, SystemPtrs::const_iterator begin, SystemPtrs::const_iterator end) override {
        AddString(boost::format("OnIterationFailedSystems: %1%, %2%, %3%, %4%, %5%, %6%") % round % task % numTasks % iteration % numIterations % std::distance(begin, end));
        return true;
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
    using IndexesType                       = std::vector<Components::Index::value_type>;

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
        bool const                          enumeratedAll(
            index.Enumerate(
                [this, &iter](Components::Index::value_type value) {
                    if(value != *iter)
                        return false;

                    ++iter;
                    return iter != Indexes.end();
                }
            )
        );

        float                               ratio(
            [this, &iter, &enumeratedAll](void) {
                if(enumeratedAll || iter == Indexes.end())
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
    size_t const                            ChildrenPerIteration;
    size_t const                            ChildrenToGenerate;
    MyConditionPtr const                    Condition;

    // ----------------------------------------------------------------------
    // |  Public Methods
    MyWorkingSystem(size_t childrenPerIteration, MyConditionPtr pCondition) :
        MyWorkingSystem(
            std::move(childrenPerIteration),
            std::move(pCondition),
            Components::Score(),
            Components::Index()
        )
    {}

    MyWorkingSystem(size_t childrenPerIteration, MyConditionPtr pCondition, Components::Score score, Components::Index index) :
        Components::WorkingSystem(std::move(score), std::move(index)),
        _childIndex(0),
        ChildrenPerIteration(
            std::move(
                [&childrenPerIteration](void) -> size_t & {
                    ENSURE_ARGUMENT(childrenPerIteration);
                    return childrenPerIteration;
                }()
            )
        ),
        ChildrenToGenerate(10),
        Condition(
            std::move(
                [&pCondition](void) -> MyConditionPtr & {
                    ENSURE_ARGUMENT(pCondition);
                    return pCondition;
                }()
            )
        )
    {}

#define ARGS                                MEMBERS(_childIndex, ChildrenPerIteration, ChildrenToGenerate, Condition), BASES(Components::WorkingSystem)

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
        return _childIndex == ChildrenToGenerate;
    }

private:
    // ----------------------------------------------------------------------
    // |  Private Methods
    virtual SystemPtrs GenerateChildrenImpl(size_t maxNumChildren) override {
        // ----------------------------------------------------------------------
        using CreateSystemFunc              = std::function<LocalExecution::Engine::SystemPtr (Components::Score, Components::Index)>;
        // ----------------------------------------------------------------------

        size_t                              toGenerate(std::min(std::min(ChildrenPerIteration, ChildrenToGenerate -_childIndex), maxNumChildren));

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
                            ChildrenPerIteration,
                            Condition,
                            std::move(score).Commit(),
                            std::move(index).Commit()
                        );
                    };
            }()
        );

        SystemPtrs                          results;

        while(toGenerate--) {
            Components::Index               newIndex(GetIndex(), _childIndex);
            Components::Score               newScore(GetScore(), Condition->Apply(newIndex), isFinal);

            results.emplace_back(createSystemFunc(std::move(newScore), std::move(newIndex)));
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
        bool isDeterministic,
        boost::optional<size_t> numConcurrentTasks=boost::none
    ) :
        LocalExecution::Configuration(
            false,
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

std::vector<Components::Index::value_type> GetIndexes(Components::ResultSystem const &resultParam) {
    assert(dynamic_cast<MyResultSystem const *>(&resultParam));

    MyResultSystem const &                  result(static_cast<MyResultSystem const &>(resultParam));
    std::vector<Components::Index::value_type>                     indexes;

    result.GetIndex().Enumerate(
        [&indexes](Components::Index::value_type value) {
            indexes.emplace_back(std::move(value));
            return true;
        }
    );

    return indexes;
}

TEST_CASE("Deterministic: 0 - 10 - 10") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{0}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((4))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((5))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((6))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((7))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((8))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{0}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationFailedSystems: 0, 0, 1, 0, -1, 9",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((3))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((4))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((5))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((6))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((7))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((8))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 5 - 10 - 10") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{5}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((4))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,1,0)),Index((5))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((6))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((7))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((8))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{5}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationFailedSystems: 0, 0, 1, 0, -1, 9",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((3))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((4))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,1,0)),Index((5))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((6))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((7))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((8))),MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())", "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 0 - 10 - 1") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{0}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                 "OnIterationBegin: 0, 0, 1, 0, -1",
                 "OnIterationEnd: 0, 0, 1, 0, -1",
                 "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,1,0)),Index((0)))]",
                 "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                 "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                 "OnRoundEnd: 0, pending []",
                 "OnTaskBegin: 0, 0, 1",
                 "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{0}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                 "OnIterationBegin: 0, 0, 1, 0, -1",
                 "OnIterationEnd: 0, 0, 1, 0, -1",
                 "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,1,0)),Index((0)))]",
                 "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                 "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                 "OnRoundEnd: 0, pending []",
                 "OnTaskBegin: 0, 0, 1",
                 "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 5 - 10 - 1") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{5}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationBegin: 0, 0, 1, 2, -1",
                "OnIterationBegin: 0, 0, 1, 3, -1",
                "OnIterationBegin: 0, 0, 1, 4, -1",
                "OnIterationBegin: 0, 0, 1, 5, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 2, -1",
                "OnIterationEnd: 0, 0, 1, 3, -1",
                "OnIterationEnd: 0, 0, 1, 4, -1",
                "OnIterationEnd: 0, 0, 1, 5, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((4)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,1,0)),Index((5)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0)))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1)))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2)))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3)))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((4)))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0)))], pending []",
                "OnIterationMergingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1)))], pending [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0)))]",
                "OnIterationMergingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2)))], pending [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1)))]",
                "OnIterationMergingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3)))], pending [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2)))]",
                "OnIterationMergingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((4)))], pending [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3)))]",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((4)))]",
                "OnRoundMergedWork: 0, pending [MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((4)))], removed []",
                "OnRoundMergingWork: 0, pending [[MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((0))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((1))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((2))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((3))),MyCalculatedResultSystem(Score(Suffix(Result(1,1,1.00),1),Pending(1,1.00,1,0)),Index((4)))]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{5}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationBegin: 0, 0, 1, 2, -1",
                "OnIterationBegin: 0, 0, 1, 3, -1",
                "OnIterationBegin: 0, 0, 1, 4, -1",
                "OnIterationBegin: 0, 0, 1, 5, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 2, -1",
                "OnIterationEnd: 0, 0, 1, 3, -1",
                "OnIterationEnd: 0, 0, 1, 4, -1",
                "OnIterationEnd: 0, 0, 1, 5, -1",
                "OnIterationFailedSystems: 0, 0, 1, 0, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 1, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 2, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 3, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 4, -1, 1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((0)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((1)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((2)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((3)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,0,1.00),1),Pending(0,1.00,1,1)),Index((4)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyCalculatedResultSystem(Score(Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,1,0)),Index((5)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 0,0 - 10 - 10") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{0,0}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0, 0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(4))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(5))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(6))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(7))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(8))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{0, 0}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0, 0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                 "OnIterationBegin: 0, 0, 1, 0, -1",
                 "OnIterationBegin: 0, 0, 1, 1, -1",
                 "OnIterationEnd: 0, 0, 1, 0, -1",
                 "OnIterationEnd: 0, 0, 1, 1, -1",
                 "OnIterationFailedSystems: 0, 0, 1, 0, -1, 9",
                 "OnIterationFailedSystems: 0, 0, 1, 1, -1, 9",
                 "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(1)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(2)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(3)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(4)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(5)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(6)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(7)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(8)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(9))]",
                 "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(4))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(5))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(6))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(7))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(8))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1", "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 0,5 - 10 - 10") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{0,5}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0,5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(4))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(0,(5))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(6))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(7))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(8))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{0,5}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0,5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationFailedSystems: 0, 0, 1, 0, -1, 9",
                "OnIterationFailedSystems: 0, 0, 1, 1, -1, 9",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(1)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(2)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(3)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(4)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(5)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(6)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(7)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(8)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(9))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(4))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(0,(5))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(6))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(7))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(8))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 0,0 - 10 - 1") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{0, 0}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0, 0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(0,(0)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{0, 0}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0, 0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                 "OnIterationBegin: 0, 0, 1, 0, -1",
                 "OnIterationBegin: 0, 0, 1, 1, -1",
                 "OnIterationEnd: 0, 0, 1, 0, -1",
                 "OnIterationEnd: 0, 0, 1, 1, -1",
                 "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))]",
                 "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(0,(0)))]",
                 "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                 "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                 "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                 "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                 "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                 "OnRoundEnd: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                 "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                 "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]]",
                 "OnTaskBegin: 0, 0, 1",
                 "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 0,5 - 10 - 1") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{0, 5}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0, 5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationBegin: 0, 0, 1, 2, -1",
                "OnIterationBegin: 0, 0, 1, 3, -1",
                "OnIterationBegin: 0, 0, 1, 4, -1",
                "OnIterationBegin: 0, 0, 1, 5, -1",
                "OnIterationBegin: 0, 0, 1, 6, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 2, -1",
                "OnIterationEnd: 0, 0, 1, 3, -1",
                "OnIterationEnd: 0, 0, 1, 4, -1",
                "OnIterationEnd: 0, 0, 1, 5, -1",
                "OnIterationEnd: 0, 0, 1, 6, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(4)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(0,(5)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0)))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1)))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2)))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3)))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(4)))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0)))]",
                "OnIterationMergingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1)))]",
                "OnIterationMergingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2)))]",
                "OnIterationMergingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(4)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3)))]",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(4)))]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(4)))], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(0,(4)))]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{0, 5}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{0, 5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationBegin: 0, 0, 1, 2, -1",
                "OnIterationBegin: 0, 0, 1, 3, -1",
                "OnIterationBegin: 0, 0, 1, 4, -1",
                "OnIterationBegin: 0, 0, 1, 5, -1",
                "OnIterationBegin: 0, 0, 1, 6, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 2, -1",
                "OnIterationEnd: 0, 0, 1, 3, -1",
                "OnIterationEnd: 0, 0, 1, 4, -1",
                "OnIterationEnd: 0, 0, 1, 5, -1",
                "OnIterationEnd: 0, 0, 1, 6, -1",
                "OnIterationFailedSystems: 0, 0, 1, 1, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 2, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 3, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 4, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 5, -1, 1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(0)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(1)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(2)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(3)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(0,(4)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(0,(5)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationGeneratingWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(0))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 5,0 - 10 - 10") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{5,0}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5, 0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                 "OnIterationBegin: 0, 0, 1, 0, -1",
                 "OnIterationBegin: 0, 0, 1, 1, -1",
                 "OnIterationEnd: 0, 0, 1, 0, -1",
                 "OnIterationEnd: 0, 0, 1, 1, -1",
                 "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]",
                 "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(4))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(5))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(6))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(7))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(8))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(9)))]",
                 "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                 "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                 "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], removed []",
                 "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], pending []",
                 "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                 "OnRoundEnd: 0, pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]",
                 "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], removed []",
                 "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]]",
                 "OnTaskBegin: 0, 0, 1",
                 "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{5, 0}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5, 0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationFailedSystems: 0, 0, 1, 0, -1, 9",
                "OnIterationFailedSystems: 0, 0, 1, 1, -1, 9",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(0)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(1)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(2)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(3)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(4)),MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(6)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(7)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(8)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(9))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(4))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(5))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(6))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(7))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(8))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 5,5 - 10 - 10") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{5,5}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5,5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(4))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(5,(5))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(6))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(7))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(8))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(6)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(7)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(8)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(9))]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(10, MyCondition::Create(MyCondition::IndexesType{5,5}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5,5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationFailedSystems: 0, 0, 1, 0, -1, 9",
                "OnIterationFailedSystems: 0, 0, 1, 1, -1, 9",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(0)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(1)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(2)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(3)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(4)),MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(6)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(7)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(8)),MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(9))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(4))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(5,(5))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(6))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(7))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(8))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(9)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending []",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 5,0 - 10 - 1") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{5, 0}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5, 0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationBegin: 0, 0, 1, 2, -1",
                "OnIterationBegin: 0, 0, 1, 3, -1",
                "OnIterationBegin: 0, 0, 1, 4, -1",
                "OnIterationBegin: 0, 0, 1, 5, -1",
                "OnIterationBegin: 0, 0, 1, 6, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 2, -1",
                "OnIterationEnd: 0, 0, 1, 3, -1",
                "OnIterationEnd: 0, 0, 1, 4, -1",
                "OnIterationEnd: 0, 0, 1, 5, -1",
                "OnIterationEnd: 0, 0, 1, 6, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1))]",
                "OnIterationGeneratedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2))]",
                "OnIterationGeneratedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3))]",
                "OnIterationGeneratedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnIterationGeneratedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))]",
                "OnIterationGeneratedWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(5,(0)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0))], pending []",
                "OnIterationMergingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1))], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0))]",
                "OnIterationMergingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2))], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1))]",
                "OnIterationMergingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3))], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2))]",
                "OnIterationMergingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3))]",
                "OnIterationMergingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{5, 0}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5, 0});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationBegin: 0, 0, 1, 2, -1",
                "OnIterationBegin: 0, 0, 1, 3, -1",
                "OnIterationBegin: 0, 0, 1, 4, -1",
                "OnIterationBegin: 0, 0, 1, 5, -1",
                "OnIterationBegin: 0, 0, 1, 6, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 2, -1",
                "OnIterationEnd: 0, 0, 1, 3, -1",
                "OnIterationEnd: 0, 0, 1, 4, -1",
                "OnIterationEnd: 0, 0, 1, 5, -1",
                "OnIterationEnd: 0, 0, 1, 6, -1",
                "OnIterationFailedSystems: 0, 0, 1, 0, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 1, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 2, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 3, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 4, -1, 1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(0))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(1))]",
                "OnIterationGeneratedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(2))]",
                "OnIterationGeneratedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(3))]",
                "OnIterationGeneratedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(4))]",
                "OnIterationGeneratedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))]",
                "OnIterationGeneratedWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(5,(0)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

TEST_CASE("Deterministic: 5,5 - 10 - 1") {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(10, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{5, 5}, false))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5, 5});

        // This string is too big for MSVC
#if (!defined _MSC_VER || defined __clang__)
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationBegin: 0, 0, 1, 10, -1",
                "OnIterationBegin: 0, 0, 1, 11, -1",
                "OnIterationBegin: 0, 0, 1, 2, -1",
                "OnIterationBegin: 0, 0, 1, 3, -1",
                "OnIterationBegin: 0, 0, 1, 4, -1",
                "OnIterationBegin: 0, 0, 1, 5, -1",
                "OnIterationBegin: 0, 0, 1, 6, -1",
                "OnIterationBegin: 0, 0, 1, 7, -1",
                "OnIterationBegin: 0, 0, 1, 8, -1",
                "OnIterationBegin: 0, 0, 1, 9, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 10, -1",
                "OnIterationEnd: 0, 0, 1, 11, -1",
                "OnIterationEnd: 0, 0, 1, 2, -1",
                "OnIterationEnd: 0, 0, 1, 3, -1",
                "OnIterationEnd: 0, 0, 1, 4, -1",
                "OnIterationEnd: 0, 0, 1, 5, -1",
                "OnIterationEnd: 0, 0, 1, 6, -1",
                "OnIterationEnd: 0, 0, 1, 7, -1",
                "OnIterationEnd: 0, 0, 1, 8, -1",
                "OnIterationEnd: 0, 0, 1, 9, -1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1))]",
                "OnIterationGeneratedWork: 0, 0, 1, 10, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(4)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 11, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(5,(5)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2))]",
                "OnIterationGeneratedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3))]",
                "OnIterationGeneratedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnIterationGeneratedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))]",
                "OnIterationGeneratedWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 7, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 8, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 9, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 10, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 11, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 7, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 8, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 9, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 10, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(4))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 7, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 8, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergedWork: 0, 0, 1, 9, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0))], pending []",
                "OnIterationMergingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1))], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0))]",
                "OnIterationMergingWork: 0, 0, 1, 10, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(4)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnIterationMergingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2))], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1))]",
                "OnIterationMergingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3))], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2))]",
                "OnIterationMergingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3))]",
                "OnIterationMergingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending [MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnIterationMergingWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnIterationMergingWork: 0, 0, 1, 7, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnIterationMergingWork: 0, 0, 1, 8, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnIterationMergingWork: 0, 0, 1, 9, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3)))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(4))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(4))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(0))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(1))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(2))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(3))),MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,50001.00),1),Pending(1,75001.00,2,0)),Index(5,(4))),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(0)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(1)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(2)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(3)),MyWorkingSystem(Score([Result(1,1,1.00)],Pending(1,1.00,1,0)),Index(4))]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
#endif
    }

    SECTION("Mismatches are failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(1, MyCondition::Create(MyCondition::IndexesType{5, 5}, true))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5, 5});
        CHECK(
            observer.GetStrings() ==
            std::vector<std::string>{
                "OnIterationBegin: 0, 0, 1, 0, -1",
                "OnIterationBegin: 0, 0, 1, 1, -1",
                "OnIterationBegin: 0, 0, 1, 10, -1",
                "OnIterationBegin: 0, 0, 1, 11, -1",
                "OnIterationBegin: 0, 0, 1, 2, -1",
                "OnIterationBegin: 0, 0, 1, 3, -1",
                "OnIterationBegin: 0, 0, 1, 4, -1",
                "OnIterationBegin: 0, 0, 1, 5, -1",
                "OnIterationBegin: 0, 0, 1, 6, -1",
                "OnIterationBegin: 0, 0, 1, 7, -1",
                "OnIterationBegin: 0, 0, 1, 8, -1",
                "OnIterationBegin: 0, 0, 1, 9, -1",
                "OnIterationEnd: 0, 0, 1, 0, -1",
                "OnIterationEnd: 0, 0, 1, 1, -1",
                "OnIterationEnd: 0, 0, 1, 10, -1",
                "OnIterationEnd: 0, 0, 1, 11, -1",
                "OnIterationEnd: 0, 0, 1, 2, -1",
                "OnIterationEnd: 0, 0, 1, 3, -1",
                "OnIterationEnd: 0, 0, 1, 4, -1",
                "OnIterationEnd: 0, 0, 1, 5, -1",
                "OnIterationEnd: 0, 0, 1, 6, -1",
                "OnIterationEnd: 0, 0, 1, 7, -1",
                "OnIterationEnd: 0, 0, 1, 8, -1",
                "OnIterationEnd: 0, 0, 1, 9, -1",
                "OnIterationFailedSystems: 0, 0, 1, 0, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 1, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 10, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 2, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 3, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 4, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 6, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 7, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 8, -1, 1",
                "OnIterationFailedSystems: 0, 0, 1, 9, -1, 1",
                "OnIterationGeneratedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(0))]",
                "OnIterationGeneratedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(1))]",
                "OnIterationGeneratedWork: 0, 0, 1, 10, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(4)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 11, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,1,100001.00),1),Pending(1,100001.00,2,0)),Index(5,(5)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(2))]",
                "OnIterationGeneratedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(3))]",
                "OnIterationGeneratedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,0,1.00)],Pending(0,1.00,1,1)),Index(4))]",
                "OnIterationGeneratedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))]",
                "OnIterationGeneratedWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(0)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 7, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(1)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 8, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(2)))]",
                "OnIterationGeneratedWork: 0, 0, 1, 9, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyCalculatedResultSystem(Score([Result(1,1,100001.00)],Suffix(Result(1,0,50001.00),1),Pending(0,75001.00,2,1)),Index(5,(3)))]",
                "OnIterationGeneratingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 10, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 11, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())",
                "OnIterationGeneratingWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 7, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 8, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationGeneratingWork: 0, 0, 1, 9, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))",
                "OnIterationMergedWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 10, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 7, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 8, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergedWork: 0, 0, 1, 9, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), pending [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnIterationMergingWork: 0, 0, 1, 0, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 1, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 10, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 2, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 3, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 4, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 5, -1, active MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index()), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)),MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], pending []",
                "OnIterationMergingWork: 0, 0, 1, 6, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 7, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 8, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnIterationMergingWork: 0, 0, 1, 9, -1, active MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5)), generated [MyWorkingSystem(Score([Result(1,1,100001.00)],Pending(1,100001.00,1,0)),Index(5))], pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundBegin: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundEnd: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]",
                "OnRoundMergedWork: 0, pending [MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())], removed []",
                "OnRoundMergingWork: 0, pending [[MyWorkingSystem(Score(Pending(1,100001.00,0,0)),Index())]]",
                "OnTaskBegin: 0, 0, 1",
                "OnTaskEnd: 0, 0, 1"
            }
        );
    }
}

void SmallDeterministicTest(size_t numGenerations, size_t childrenPerGeneration, bool failuresAreErrors) {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(numGenerations, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(childrenPerGeneration, MyCondition::Create(MyCondition::IndexesType{5, 4, 3, 2, 1}, failuresAreErrors))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{5, 4, 3, 2, 1});
    }
}

void MediumDeterministicTest(size_t numGenerations, size_t childrenPerGeneration, bool failuresAreErrors) {
    LocalExecution::Engine::ExecuteResultValue          result;
    LocalExecution::Engine::ResultSystemUniquePtr       pResult;
    Configuration                                       configuration(numGenerations, true);
    MyObserver                                          observer;

    SECTION("Mismatches are not failures") {
        std::tie(result, pResult) = LocalExecution::Engine::Execute(
            configuration,
            observer,
            MyWorkingSystem(childrenPerGeneration, MyCondition::Create(MyCondition::IndexesType{1, 2, 3, 4, 5, 6, 7}, failuresAreErrors))
        );

        CHECK(result == LocalExecution::Engine::ExecuteResultValue::Completed);
        REQUIRE(pResult);
        CHECK(GetIndexes(*pResult) == std::vector<Components::Index::value_type>{1, 2, 3, 4, 5, 6, 7});
    }
}

#if (defined NDEBUG)

TEST_CASE("Deterministic: 5,4,3,2,1 - 10 - 10", "[Benchmark]") {
    BENCHMARK("Mismatches are not failures") { SmallDeterministicTest(10, 10, false); };
    BENCHMARK("Mismatches are failures") { SmallDeterministicTest(10, 10, true); };
}

TEST_CASE("Deterministic: 5,4,3,2,1 - 10 - 1", "[Benchmark]") {
    BENCHMARK("Mismatches are not failures") { SmallDeterministicTest(10, 1, false); };
    BENCHMARK("Mismatches are failures") { SmallDeterministicTest(10, 1, true); };
}

TEST_CASE("Deterministic: 1,2,3,4,5,6,7 - 10 - 10", "[Benchmark]") {
    BENCHMARK("Mismatches are not failures") { MediumDeterministicTest(10, 10, false); };
    BENCHMARK("Mismatches are failures") { MediumDeterministicTest(10, 10, true); };
}

TEST_CASE("Deterministic: 1,2,3,4,5,6,7 - 10 - 1", "[Benchmark]") {
    BENCHMARK("Mismatches are not failures") { MediumDeterministicTest(10, 1, false); };
    BENCHMARK("Mismatches are failures") { MediumDeterministicTest(10, 1, true); };
}

#else

TEST_CASE("Deterministic: 5,4,3,2,1 - 10 - 10") {
    SECTION("Mismatches are not failures") { SmallDeterministicTest(10, 10, false); }
    SECTION("Mismatches are failures") { SmallDeterministicTest(10, 10, true); }
}

TEST_CASE("Deterministic: 5,4,3,2,1 - 10 - 1") {
    SECTION("Mismatches are not failures") { SmallDeterministicTest(10, 1, false); }
    SECTION("Mismatches are failures") { SmallDeterministicTest(10, 1, true); }
}

TEST_CASE("Deterministic: 1,2,3,4,5,6,7 - 10 - 10") {
    SECTION("Mismatches are not failures") { MediumDeterministicTest(10, 10, false); }
    SECTION("Mismatches are failures") { MediumDeterministicTest(10, 10, true); }
}

TEST_CASE("Deterministic: 1,2,3,4,5,6,7 - 10 - 1") {
    SECTION("Mismatches are not failures") { MediumDeterministicTest(10, 1, false); }
    SECTION("Mismatches are failures") { MediumDeterministicTest(10, 1, true); }
}

#endif
