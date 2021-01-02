/////////////////////////////////////////////////////////////////////////
///
///  \file          Index.cpp
///  \brief         See Index.h
///
///  \author        David Brownell <db@DavidBrownell.com>
///  \date          2020-05-23 14:45:23
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
#include "Index.h"

namespace DecisionEngine {
namespace Core {
namespace Components {

// ----------------------------------------------------------------------
// |
// |  Index
// |
// ----------------------------------------------------------------------
Index::Index(void)
{}

Index::Index(value_type index) :
    _suffix(std::move(index))
{}

Index::Index(Index const &index, value_type suffix) :
    _pIndexes(index._pIndexes),
    _suffix(std::move(suffix))
{
    ENSURE_ARGUMENT(index, index.HasSuffix() == false);
}

// static
int Index::Compare(Index const &a, Index const &b) {
    // ----------------------------------------------------------------------
    class Iterator {
    public:
        Iterator(Indexes const *pIndexes, value_type const *pSuffix) :
            _ptr(pIndexes ? pIndexes->data() : nullptr),
            _pIndexesEnd(pIndexes ? pIndexes->data() + pIndexes->size() : nullptr),
            _pSuffixBegin(pSuffix),
            _pSuffixEnd(pSuffix ? pSuffix + 1 : nullptr)
        {
            if(_ptr == _pIndexesEnd)
                _ptr = _pSuffixBegin;
        }

        NON_COPYABLE(Iterator);
        NON_MOVABLE(Iterator);

        value_type const & operator *(void) const {
            if(AtEnd())
                throw std::logic_error("Invalid operation");

            return *_ptr;
        }

        Iterator & operator++(void) {
            if(AtEnd())
                throw std::logic_error("Invalid operator");

            ++_ptr;

            if(_ptr == _pIndexesEnd)
                _ptr = _pSuffixBegin;

            return *this;
        }

        bool AtEnd(void) const {
            return _ptr == _pSuffixEnd;
        };

    private:
        value_type const *                  _ptr;
        value_type const * const            _pIndexesEnd;
        value_type const * const            _pSuffixBegin;
        value_type const * const            _pSuffixEnd;
    };
    // ----------------------------------------------------------------------

    if(static_cast<void const *>(&a) == static_cast<void const *>(&b))
        return 0;

    Iterator                                iThis(a._pIndexes.get(), a._suffix ? &*a._suffix : nullptr);
    Iterator                                iThat(b._pIndexes.get(), b._suffix ? &*b._suffix : nullptr);

    // Support left-stable sorting when > is used as the sorting operator. This
    // implies that right-trending indexes will be < left-trending indexes, which
    // is the opposite of what would normally be expected.

    while(iThis.AtEnd() == false && iThat.AtEnd() == false) {
        value_type const &                  vThis(*iThis);
        value_type const &                  vThat(*iThat);

        // Higher values imply a smaller sort
        if(vThis > vThat)
            return -1;
        else if(vThis < vThat)
            return 1;

        ++iThis;
        ++iThat;
    }

    if(iThis.AtEnd() && iThat.AtEnd())
        return 0;

    // The item with the smaller number of indexes is considered to be < than the other
    return iThis.AtEnd() ? -1 : 1;
}

bool Index::operator==(Index const &other) const {
    return Compare(*this, other) == 0;
}

bool Index::operator!=(Index const &other) const {
    return Compare(*this, other) != 0;
}

bool Index::operator <(Index const &other) const {
    return Compare(*this, other) < 0;
}

bool Index::operator<=(Index const &other) const {
    return Compare(*this, other) <= 0;
}

bool Index::operator >(Index const &other) const {
    return Compare(*this, other) > 0;
}

bool Index::operator>=(Index const &other) const {
    return Compare(*this, other) >= 0;
}

std::string Index::ToString(void) const /*override*/ {
    std::vector<std::string>                strings;

    if(_pIndexes) {
        for(auto const &index : *_pIndexes)
            strings.emplace_back(std::to_string(index));
    }

    if(_suffix) {
        strings.emplace_back(
            boost::str(
                boost::format("(%1%)") % *_suffix
            )
        );
    }

    return boost::str(
        boost::format(
            "Index(%1%)"
        )
        % boost::algorithm::join(strings, ",")
    );
}

bool Index::AtRoot(void) const {
    return !_pIndexes && !_suffix;
}

size_t Index::Depth(void) const {
    return (_pIndexes ? _pIndexes->size() : 0)
        + (_suffix ? 1 : 0);
}

bool Index::HasSuffix(void) const {
    return static_cast<bool>(_suffix);
}

Index Index::Commit(void) {
    if(HasSuffix() == false)
        throw std::logic_error("Invalid operation");

    Indexes                                 indexes;

    if(_pIndexes) {
        indexes.reserve(_pIndexes->size() + 1);
        std::copy(_pIndexes->cbegin(), _pIndexes->cend(), std::back_inserter(indexes));
    }

    indexes.emplace_back(std::move(*_suffix));

    return Index(std::make_shared<Indexes>(std::move(indexes)));
}

Index Index::Copy(void) const {
    if(HasSuffix())
        throw std::logic_error("Invalid operation");

    return Index(_pIndexes);
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
Index::Index(IndexesPtr pIndexes) :
    _pIndexes(
        std::move(
            [&pIndexes](void) -> IndexesPtr & {
                ENSURE_ARGUMENT(pIndexes, pIndexes && pIndexes->empty() == false);
                return pIndexes;
            }()
        )
    )
{}

} // namespace Components
} // namespace Core
} // namespace DecisionEngine
