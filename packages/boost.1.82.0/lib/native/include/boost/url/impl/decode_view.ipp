//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

#ifndef BOOST_URL_IMPL_DECODE_VIEW_IPP
#define BOOST_URL_IMPL_DECODE_VIEW_IPP

#include <boost/url/decode_view.hpp>
#include <ostream>

namespace boost {
namespace urls {

namespace detail {

template <class T>
int
decoded_strcmp(decode_view s0, T s1)
{
    auto const n0 = s0.size();
    auto const n1 = s1.size();
    auto n = (std::min)(n0, n1);
    auto it0 = s0.begin();
    auto it1 = s1.begin();
    while (n--)
    {
        const char c0 = *it0++;
        const char c1 = *it1++;
        if (c0 == c1)
            continue;
        return 1 - 2 * (static_cast<unsigned char>(c0)
                      < static_cast<unsigned char>(c1));
    }
    return 1 - (n0 == n1) - 2 * (n0 < n1);
}

} // detail

//------------------------------------------------

auto
decode_view::
iterator::
operator*() const noexcept ->
    reference
{
    if (space_as_plus_ &&
        *pos_ == '+')
        return ' ';
    if (*pos_ != '%')
        return *pos_;
    auto d0 = grammar::hexdig_value(pos_[1]);
    auto d1 = grammar::hexdig_value(pos_[2]);
    return static_cast<char>(
        ((static_cast<
              unsigned char>(d0) << 4) +
         (static_cast<
             unsigned char>(d1))));
}

// unchecked constructor
decode_view::
decode_view(
    string_view s,
    std::size_t n,
    encoding_opts opt) noexcept
    : p_(s.data())
    , n_(s.size())
    , dn_(n)
    , space_as_plus_(
        opt.space_as_plus)
{
}

int
decode_view::
compare(string_view other) const noexcept
{
    return detail::decoded_strcmp(*this, other);
}

int
decode_view::
compare(decode_view other) const noexcept
{
    return detail::decoded_strcmp(*this, other);
}

void
decode_view::
write(std::ostream& os) const
{
    auto it = begin();
    auto const end_ = end();
    while(it != end_)
        os.put(*it++);
}

void
decode_view::
remove_prefix( size_type n )
{
    auto it = begin();
    auto n0 = n;
    while (n)
    {
        ++it;
        --n;
    }
    n_ -= (it.base() - begin().base());
    dn_ -= n0;
    p_ = it.base();
}

void
decode_view::
remove_suffix( size_type n )
{
    auto it = end();
    auto n0 = n;
    while (n)
    {
        --it;
        --n;
    }
    n_ -= (end().base() - it.base());
    dn_ -= n0;
}

bool
decode_view::
starts_with( string_view s ) const noexcept
{
    if (s.size() > size())
        return false;
    auto it0 = begin();
    auto it1 = s.begin();
    std::size_t n = s.size();
    while (n)
    {
        if (*it0 != *it1)
            return false;
        ++it0;
        ++it1;
        --n;
    }
    return true;
}

bool
decode_view::
ends_with( string_view s ) const noexcept
{
    if (s.size() > size())
        return false;
    auto it0 = end();
    auto it1 = s.end();
    std::size_t n = s.size();
    --it0;
    --it1;
    while (n - 1)
    {
        if (*it0 != *it1)
            return false;
        --it0;
        --it1;
        --n;
    }
    return *it0 == *it1;
}

bool
decode_view::
starts_with( char ch ) const noexcept
{
    return
        !empty() &&
        front() == ch;
}

bool
decode_view::
ends_with( char ch ) const noexcept
{
    return
        !empty() &&
        back() == ch;
}

decode_view::const_iterator
decode_view::
find( char ch ) const noexcept
{
    auto it = begin();
    auto end = this->end();
    while (it != end)
    {
        if (*it == ch)
            return it;
        ++it;
    }
    return it;
}

decode_view::const_iterator
decode_view::
rfind( char ch ) const noexcept
{
    if (empty())
        return end();
    auto it = end();
    auto begin = this->begin();
    --it;
    while (it != begin)
    {
        if (*it == ch)
            return it;
        --it;
    }
    if (*it == ch)
        return it;
    return end();
}

} // urls
} // boost

#endif
