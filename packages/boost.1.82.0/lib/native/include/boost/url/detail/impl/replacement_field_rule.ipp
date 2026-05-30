//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

#ifndef BOOST_URL_DETAIL_IMPL_REPLACEMENT_FIELD_RULE_IPP
#define BOOST_URL_DETAIL_IMPL_REPLACEMENT_FIELD_RULE_IPP

#include <boost/url/detail/replacement_field_rule.hpp>
#include <boost/url/grammar/alnum_chars.hpp>
#include <boost/url/grammar/alpha_chars.hpp>
#include <boost/url/grammar/delim_rule.hpp>
#include <boost/url/grammar/lut_chars.hpp>
#include <boost/url/grammar/optional_rule.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/token_rule.hpp>
#include <boost/url/grammar/tuple_rule.hpp>

namespace boost {
namespace urls {
namespace detail {

auto
replacement_field_rule_t::
parse(
    char const*& it,
    char const* const end) const noexcept ->
        result<value_type>
{
    static constexpr auto replacement_field_rules =
        grammar::tuple_rule(
            // open
            grammar::squelch(
                grammar::delim_rule('{')),
            // id
            grammar::optional_rule(arg_id_rule),
            // format options
            grammar::optional_rule(
                grammar::tuple_rule(
                    grammar::squelch(
                        grammar::delim_rule(':')),
                    format_spec_rule)),
            // close
            grammar::squelch(
                grammar::delim_rule('}')));
    auto it0 = it;
    auto rv = grammar::parse(it, end, replacement_field_rules);
    if (!rv)
    {
        BOOST_URL_RETURN_EC(
            grammar::error::mismatch);
    }
    return string_view(it0, it);
}

auto
identifier_rule_t::
parse(
    char const*& it,
    char const* const end) const noexcept
        -> result<value_type>
{
    static constexpr auto identifier_rules =
        grammar::tuple_rule(
            grammar::delim_rule(
                grammar::alpha_chars +
                grammar::lut_chars('_')),
            grammar::optional_rule(
                grammar::token_rule(
                    grammar::alnum_chars +
                    grammar::lut_chars('_'))));
    char const* it0 = it;
    auto rv = grammar::parse(it, end, identifier_rules);
    if (!rv)
    {
        BOOST_URL_RETURN_EC(
            grammar::error::mismatch);
    }
    return string_view(it0, it);
}

auto
format_spec_rule_t::
parse(
    char const*& it,
    char const* const end) const noexcept
        -> result<value_type>
{
    if (it == end)
        return {};

    // any tokens allowed in fmt specs
    static constexpr auto fmt_specs_token_rule =
        grammar::optional_rule(
            grammar::token_rule(
                grammar::vchars +
                grammar::lut_chars(' ')
                - "{}"));

    // internal ids in the fmt specs
    // "{" [arg_id] "}"
    static constexpr auto internal_id_rule =
        grammar::tuple_rule(
            grammar::squelch(
                grammar::delim_rule('{')),
            grammar::optional_rule(
                arg_id_rule),
            grammar::squelch(
                grammar::delim_rule('}')));

    auto start = it;
    // consume fmt_spec chars
    while (grammar::parse(it, end, fmt_specs_token_rule))
    {
        auto it0 = it;
        // consume internal id
        if (!grammar::parse(it, end, internal_id_rule))
        {
            // rewind
            it = it0;
            break;
        }
    }

    return string_view(start, it);
}

} // detail
} // urls
} // boost

#endif
