/////////////////////////////////////////////////////////////////////////
///
///  \file          Index.h
///  \brief         Contains the Index object
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-23 10:54:35
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

#include "Components.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

/////////////////////////////////////////////////////////////////////////
///  \class         Index
///  \brief         A representation of the system in its current state;
///                 this ensures a stable sort when scores are equal.
///
class Index {
public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Types
    // |
    // ----------------------------------------------------------------------
    using value_type                        = std::uint64_t;

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Types (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    using Indexes                           = std::vector<value_type>;
    using IndexesPtr                        = std::shared_ptr<Indexes>;

    // ----------------------------------------------------------------------
    // |
    // |  Private Data (used in public declarations)
    // |
    // ----------------------------------------------------------------------
    IndexesPtr const                        _pIndexes;
    boost::optional<value_type> const       _suffix;

public:
    // ----------------------------------------------------------------------
    // |
    // |  Public Methods
    // |
    // ----------------------------------------------------------------------
    Index(void);
    Index(value_type index);
    Index(Index const &index, value_type suffix);

#define ARGS                                MEMBERS(_pIndexes, _suffix)

    NON_COPYABLE(Index);
    MOVE(Index, ARGS);
    SERIALIZATION(Index, ARGS);

#undef ARGS

    // Comparison is too complicated for the standard implementation
    static int Compare(Index const &a, Index const &b);

    bool operator==(Index const &other) const;
    bool operator!=(Index const &other) const;
    bool operator <(Index const &other) const;
    bool operator<=(Index const &other) const;
    bool operator >(Index const &other) const;
    bool operator>=(Index const &other) const;

    std::string ToString(void) const;

    bool AtRoot(void) const;
    size_t Depth(void) const;

    bool HasSuffix(void) const;

    template <typename FunctionT>
    // bool (value_type const &);
    bool Enumerate(FunctionT const &func) const;

    // This method should only be called when the object was created with a suffix
    Index Commit(void);

    // This method should only be called when the object was created without a suffix
    Index Copy(void) const;

private:
    // ----------------------------------------------------------------------
    // |
    // |  Private Methods
    // |
    // ----------------------------------------------------------------------
    Index(IndexesPtr pIndexes);
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
// bool (value_type const &);
bool Index::Enumerate(FunctionT const &func) const {
    if(_pIndexes) {
        for(auto const &index : *_pIndexes) {
            if(func(index) == false)
                return false;
        }
    }

    if(_suffix && func(*_suffix) == false)
        return false;

    return true;
}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
