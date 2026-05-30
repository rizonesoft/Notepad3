//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

#ifndef BOOST_URL_GRAMMAR_IMPL_ERROR_IPP
#define BOOST_URL_GRAMMAR_IMPL_ERROR_IPP

#include <boost/url/grammar/error.hpp>

namespace boost {
namespace urls {
namespace grammar {

namespace detail {

const char*
error_cat_type::
name() const noexcept
{
    return "boost.url.grammar";
}

std::string
error_cat_type::
message(int code) const
{
    return message(code, nullptr, 0);
}

char const*
error_cat_type::
message(
    int code,
    char*,
    std::size_t) const noexcept
{
    switch(static_cast<error>(code))
    {
    default:
case error::need_more: return "need more";
case error::mismatch: return "mismatch";
case error::invalid: return "invalid";
case error::end_of_range: return "end of range";
case error::leftover: return "leftover";
case error::out_of_range: return "out of range";
    }
}

error_condition
error_cat_type::
default_error_condition(
    int ev) const noexcept
{
    switch(static_cast<error>(ev))
    {
case error::invalid:
case error::out_of_range:
        return condition::fatal;
    default:
        return {ev, *this};
    }
}

//------------------------------------------------

const char*
condition_cat_type::
name() const noexcept
{
    return "boost.url.grammar";
}

std::string
condition_cat_type::
message(int code) const
{
    return message(code, nullptr, 0);
}

char const*
condition_cat_type::
message(
    int code, char*, std::size_t) const noexcept
{
    switch(static_cast<condition>(code))
    {
    default:
    case condition::fatal:
        return "fatal condition";
    }
}

} // detail

} // grammar
} // urls
} // boost

#endif
