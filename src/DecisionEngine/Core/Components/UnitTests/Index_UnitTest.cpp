/////////////////////////////////////////////////////////////////////////
///
///  \file          Index_UnitTest.cpp
///  \brief         Unit test for Index.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-23 10:53:31
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
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_CONSOLE_WIDTH 200
#include "../Index.h"
#include <catch.hpp>

#include <CommonHelpers/TestHelpers.h>
#include <BoostHelpers/TestHelpers.h>

namespace NS                                = DecisionEngine::Core::Components;

TEST_CASE("Default Ctor") {
    NS::Index const                         index;

    CHECK(index.HasSuffix() == false);
    CHECK(index.AtRoot());
    CHECK(index.Depth() == 0);
}

TEST_CASE("Index Ctor") {
    NS::Index const                         index(10);

    CHECK(index.HasSuffix());
    CHECK(index.AtRoot() == false);
    CHECK(index.Depth() == 1);
}

TEST_CASE("Indexes Ctor") {
    CHECK_THROWS_MATCHES(NS::Index(NS::Index(10), 20), std::invalid_argument, Catch::Matchers::Exception::ExceptionMessageMatcher("index"));

    NS::Index const                         index(
        NS::Index(10).Commit(),
        20
    );

    CHECK(index.HasSuffix());
    CHECK(index.AtRoot() == false);
    CHECK(index.Depth() == 2);
}

NS::Index CreateIndex(NS::Index index, std::vector<NS::Index::value_type> values) {
    for(auto const & value: values) {
        index = NS::Index(index, value).Commit();
    }

    return index;
}

TEST_CASE("ToString") {
    CHECK(NS::Index().ToString() == "Index()");
    CHECK(NS::Index(10).ToString() == "Index((10))");
    CHECK(NS::Index(CreateIndex(NS::Index(), {1})).ToString() == "Index(1)");
    CHECK(NS::Index(CreateIndex(NS::Index(), {1}), 2).ToString() == "Index(1,(2))");
    CHECK(NS::Index(CreateIndex(NS::Index(), {1, 2, 3}), 4).ToString() == "Index(1,2,3,(4))");
}

TEST_CASE("Compare") {
    // Equal
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Index(), NS::Index(), true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(NS::Index(1), NS::Index(1), true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(CreateIndex(NS::Index(), {1}), NS::Index(1), true) == 0);
    CHECK(CommonHelpers::TestHelpers::CompareTest(CreateIndex(NS::Index(), {1, 2, 3}), CreateIndex(NS::Index(), {1, 2, 3}), true) == 0);
}

TEST_CASE("Compare Not Equal") {
    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            CreateIndex(NS::Index(), {1}),
            CreateIndex(NS::Index(), {0})
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            CreateIndex(NS::Index(), {1, 2, 3}),
            CreateIndex(NS::Index(), {1, 2, 3, 4})
        ) == 0
    );

    CHECK(
        CommonHelpers::TestHelpers::CompareTest(
            CreateIndex(NS::Index(), {0, 2}),
            CreateIndex(NS::Index(), {0, 1})
        ) == 0
    );
}

TEST_CASE("Enumeration") {
    // ----------------------------------------------------------------------
    using Indexes                           = std::vector<NS::Index::value_type>;
    // ----------------------------------------------------------------------

    Indexes                                 indexes;
    size_t                                  maxNumIndexes(std::numeric_limits<size_t>::max());

    auto const                              func(
        [&indexes, &maxNumIndexes](NS::Index::value_type const &value) {
            indexes.push_back(value);
            return indexes.size() < maxNumIndexes;
        }
    );

    SECTION("None") {
        CHECK(NS::Index().Enumerate(func));
        CHECK(indexes.empty());
    }

    SECTION("Single Suffix") {
        CHECK(NS::Index(1).Enumerate(func));
        CHECK(indexes == Indexes{ 1 });
    }

    SECTION("Multi Indexes") {
        CHECK(CreateIndex(NS::Index(), {1, 2}).Enumerate(func));
        CHECK(indexes == Indexes{ 1, 2 });
    }

    SECTION("Multi Indexes with Suffix") {
        CHECK(NS::Index(CreateIndex(NS::Index(), {1, 2}), 3).Enumerate(func));
        CHECK(indexes == Indexes{1, 2, 3});
    }

    SECTION("Cancellation") {
        maxNumIndexes = 2;

        CHECK(NS::Index(CreateIndex(NS::Index(), {1, 2}), 3).Enumerate(func) == false);
        CHECK(indexes == Indexes{1, 2});
    }
}

TEST_CASE("Commit") {
    CHECK_THROWS_MATCHES(NS::Index().Commit(), std::logic_error, Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid operation"));

    NS::Index                               index(1);

    CHECK(index.HasSuffix());
    CHECK(index.Commit().HasSuffix() == false);
}

TEST_CASE("Copy") {
    NS::Index const                         index(CreateIndex(NS::Index(), {1}));

    CHECK(index.HasSuffix() == false);
    CHECK(index.Copy().HasSuffix() == false);

    CHECK_THROWS_MATCHES(NS::Index(1).Copy(), std::logic_error, Catch::Matchers::Exception::ExceptionMessageMatcher("Invalid operation"));
}

TEST_CASE("Serialization") {
    CHECK(
        BoostHelpers::TestHelpers::SerializeTest(
            NS::Index(
                CreateIndex(NS::Index(), {1, 2, 3}),
                4
            ),
            [](std::string const &output) {
                UNSCOPED_INFO(output);
                CHECK(true);
            }
        ) == 0
    );
}
