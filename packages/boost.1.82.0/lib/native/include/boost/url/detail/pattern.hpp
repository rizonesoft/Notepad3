//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

#ifndef BOOST_URL_DETAIL_PATTERN_HPP
#define BOOST_URL_DETAIL_PATTERN_HPP

#include <boost/url/error_types.hpp>
#include <boost/url/url_base.hpp>
#include <boost/url/string_view.hpp>

// This file includes functions and classes
// to parse uri templates or format strings

namespace boost {
namespace urls {
namespace detail {

class format_args;

struct pattern
{
    string_view scheme;
    string_view user;
    string_view pass;
    string_view host;
    string_view port;
    string_view path;
    string_view query;
    string_view frag;

    bool has_authority = false;
    bool has_user = false;
    bool has_pass = false;
    bool has_port = false;
    bool has_query = false;
    bool has_frag = false;

    BOOST_URL_DECL
    void
    apply(
        url_base& u,
        format_args const& args) const;
};

BOOST_URL_DECL
result<pattern>
parse_pattern(
    string_view s);

} // detail
} // url
} // boost

#endif
