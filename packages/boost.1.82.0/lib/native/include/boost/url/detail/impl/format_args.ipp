//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

#ifndef BOOST_URL_DETAIL_IMPL_FORMAT_ARGS_IPP
#define BOOST_URL_DETAIL_IMPL_FORMAT_ARGS_IPP

#include <boost/url/detail/format_args.hpp>
#include <boost/url/detail/replacement_field_rule.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/unsigned_rule.hpp>

namespace boost {
namespace urls {
namespace detail {

std::size_t
get_uvalue( string_view a )
{
    string_view str(a);
    auto rv = grammar::parse(
        str, grammar::unsigned_rule<std::size_t>{});
    if (rv)
        return *rv;
    return 0;
}

std::size_t
get_uvalue( char a )
{
    string_view str(&a, 1);
    return get_uvalue(str);
}

char const*
formatter<string_view>::
parse(format_parse_context& ctx)
{
    char const* it = ctx.begin();
    char const* end = ctx.end();
    BOOST_ASSERT(it != end);

    // fill / align
    if (end - it > 2)
    {
        if (*it != '{' &&
            *it != '}' &&
            (*(it + 1) == '<' ||
             *(it + 1) == '>' ||
             *(it + 1) == '^'))
        {
            fill = *it;
            align = *(it + 1);
            it += 2;
        }
    }

    // align
    if (align == '\0' &&
        (*it == '<' ||
         *it == '>' ||
         *it == '^'))
    {
        align = *it++;
    }

    // width
    char const* it0 = it;
    constexpr auto width_rule =
        grammar::variant_rule(
             grammar::unsigned_rule<std::size_t>{},
             grammar::tuple_rule(
                 grammar::squelch(
                     grammar::delim_rule('{')),
                 grammar::optional_rule(
                    arg_id_rule),
                 grammar::squelch(
                     grammar::delim_rule('}'))));
    auto rw = grammar::parse(it, end, width_rule);
    if (!rw)
    {
        // rewind
        it = it0;
    }
    else if (align != '\0')
    {
        // width is ignored when align is '\0'
        if (rw->index() == 0)
        {
            // unsigned_rule
            width = variant2::get<0>(*rw);
        }
        else
        {
            // arg_id: store the id idx or string
            auto& arg_id = variant2::get<1>(*rw);
            if (!arg_id)
            {
                // empty arg_id, use and consume
                // the next arg idx
                width_idx = ctx.next_arg_id();
            }
            else if (arg_id->index() == 0)
            {
                // string identifier
                width_name = variant2::get<0>(*arg_id);
            }
            else
            {
                // integer identifier: use the
                // idx of this format_arg
                width_idx = variant2::get<1>(*arg_id);
            }
        }
    }

    // type is parsed but doesn't have to
    // be stored for strings
    if (*it == 'c' ||
        *it == 's')
    {
        ++it;
    }

    // we should have arrived at the end now
    if (*it != '}')
    {
        urls::detail::throw_invalid_argument();
    }

    return it;
}

std::size_t
formatter<string_view>::
measure(
    string_view str,
    measure_context& ctx,
    grammar::lut_chars const& cs) const
{
    std::size_t w = width;
    if (width_idx != std::size_t(-1) ||
        !width_name.empty())
    {
        get_width_from_args(
            width_idx, width_name, ctx.args(), w);
    }

    std::size_t n = ctx.out();
    if (str.size() < w)
        n += measure_one(fill, cs) * (w - str.size());

    return n + encoded_size(str, cs);
}

char*
formatter<string_view>::
format(string_view str, format_context& ctx, grammar::lut_chars const& cs) const
{
    std::size_t w = width;
    if (width_idx != std::size_t(-1) ||
        !width_name.empty())
    {
        get_width_from_args(
            width_idx, width_name, ctx.args(), w);
    }

    std::size_t lpad = 0;
    std::size_t rpad = 0;
    if (str.size() < w)
    {
        std::size_t pad = w - str.size();
        switch (align)
        {
        case '<':
            rpad = pad;
            break;
        case '>':
            lpad = pad;
            break;
        case '^':
            lpad = w / 2;
            rpad = pad - lpad;
            break;
        }
    }

    // unsafe `encode`, assuming `out` has
    // enough capacity
    char* out = ctx.out();
    for (std::size_t i = 0; i < lpad; ++i)
        encode_one(out, fill, cs);
    for (char c: str)
        encode_one(out, c, cs);
    for (std::size_t i = 0; i < rpad; ++i)
        encode_one(out, fill, cs);
    return out;
}

void
get_width_from_args(
    std::size_t arg_idx,
    string_view arg_name,
    format_args args,
    std::size_t& w)
{
    // check arg_id
    format_arg warg;
    if (arg_idx != std::size_t(-1))
    {
        // identifier
        warg = args.get(arg_idx);
    }
    else
    {
        // unsigned integer
        warg = args.get(arg_name);
    }

    // get unsigned int value from that format arg
    w = warg.value();
}

char const*
integer_formatter_impl::
parse(format_parse_context& ctx)
{
    char const* it = ctx.begin();
    char const* end = ctx.end();
    BOOST_ASSERT(it != end);

    // fill / align
    if (end - it > 2)
    {
        if (*it != '{' &&
            *it != '}' &&
            (*(it + 1) == '<' ||
             *(it + 1) == '>' ||
             *(it + 1) == '^'))
        {
            fill = *it;
            align = *(it + 1);
            it += 2;
        }
    }

    // align
    if (align == '\0' &&
        (*it == '<' ||
         *it == '>' ||
         *it == '^'))
    {
        align = *it++;
    }

    // sign
    if (*it == '+' ||
        *it == '-' ||
        *it == ' ')
    {
        sign = *it++;
    }

    // #
    if (*it == '#')
    {
        // alternate form not supported
        ++it;
    }

    // 0
    if (*it == '0')
    {
        zeros = *it++;
    }

    // width
    char const* it0 = it;
    constexpr auto width_rule = grammar::variant_rule(
        grammar::unsigned_rule<std::size_t>{},
        grammar::tuple_rule(
            grammar::squelch(
                grammar::delim_rule('{')),
            grammar::optional_rule(
                arg_id_rule),
            grammar::squelch(
                grammar::delim_rule('}'))));
    auto rw = grammar::parse(it, end, width_rule);
    if (!rw)
    {
        // rewind
        it = it0;
    }
    else if (align != '\0')
    {
        // width is ignored when align is '\0'
        if (rw->index() == 0)
        {
            // unsigned_rule
            width = variant2::get<0>(*rw);
        }
        else
        {
            // arg_id: store the id idx or string
            auto& arg_id = variant2::get<1>(*rw);
            if (!arg_id)
            {
                // empty arg_id, use and consume
                // the next arg idx
                width_idx = ctx.next_arg_id();
            }
            else if (arg_id->index() == 0)
            {
                // string identifier
                width_name = variant2::get<0>(*arg_id);
            }
            else
            {
                // integer identifier: use the
                // idx of this format_arg
                width_idx = variant2::get<1>(*arg_id);
            }
        }
    }

    // type is parsed but doesn't have to
    // be stored for strings
    if (*it == 'd')
    {
        // we don't include other presentation
        // modes for integers as they are not
        // recommended or generally used in
        // urls
        ++it;
    }

    // we should have arrived at the end now
    if (*it != '}')
    {
        urls::detail::throw_invalid_argument();
    }

    return it;
}

std::size_t
integer_formatter_impl::
measure(
    long long int v,
    measure_context& ctx,
    grammar::lut_chars const& cs) const
{
    std::size_t dn = 0;
    std::size_t n = 0;
    if (v < 0)
    {
        dn += measure_one('-', cs);
        ++n;
        v *= -1;
    }
    else if (sign != '-')
    {
        dn += measure_one(sign, cs);
        ++n;
    }
    while (v > 0)
    {
        int d = v % 10;
        v /= 10;
        dn += measure_one('0' + static_cast<char>(d), cs);
        ++n;
    }

    std::size_t w = width;
    if (width_idx != std::size_t(-1) ||
        !width_name.empty())
    {
        get_width_from_args(
            width_idx, width_name, ctx.args(), w);
    }
    if (w > n)
    {
        if (!zeros)
            dn += measure_one(fill, cs) * (w - n);
        else
            dn += measure_one('0', cs) * (w - n);
    }
    return ctx.out() + dn;
}

std::size_t
integer_formatter_impl::
measure(
    unsigned long long int v,
    measure_context& ctx,
    grammar::lut_chars const& cs) const
{
    std::size_t dn = 0;
    std::size_t n = 0;
    if (sign != '-')
    {
        dn += measure_one(sign, cs);
        ++n;
    }
    while (v != 0)
    {
        int d = v % 10;
        v /= 10;
        dn += measure_one('0' + static_cast<char>(d), cs);
        ++n;
    }

    std::size_t w = width;
    if (width_idx != std::size_t(-1) ||
        !width_name.empty())
    {
        get_width_from_args(
            width_idx, width_name, ctx.args(), w);
    }
    if (w > n)
    {
        if (!zeros)
            dn += measure_one(fill, cs) * (w - n);
        else
            dn += measure_one('0', cs) * (w - n);
    }
    return ctx.out() + dn;
}

char*
integer_formatter_impl::
format(
    long long int v,
    format_context& ctx,
    grammar::lut_chars const& cs) const
{
    // get n digits
    long long int v0 = v;
    long long int p = 1;
    std::size_t n = 0;
    if (v < 0)
    {
        v *= - 1;
        ++n;
    }
    else if (sign != '-')
    {
        ++n;
    }
    while (v > 0)
    {
        if (v >= 10)
            p *= 10;
        v /= 10;
        ++n;
    }
    static constexpr auto m =
        std::numeric_limits<long long int>::digits10;
    BOOST_ASSERT(n <= m + 1);
    ignore_unused(m);

    // get pad
    std::size_t w = width;
    if (width_idx != std::size_t(-1) ||
        !width_name.empty())
    {
        get_width_from_args(
            width_idx, width_name, ctx.args(), w);
    }
    std::size_t lpad = 0;
    std::size_t rpad = 0;
    if (w > n)
    {
        std::size_t pad = w - n;
        if (zeros)
        {
            lpad = pad;
        }
        else
        {
            switch (align)
            {
            case '<':
                rpad = pad;
                break;
            case '>':
                lpad = pad;
                break;
            case '^':
                lpad = pad / 2;
                rpad = pad - lpad;
                break;
            }
        }
    }

    // write
    v = v0;
    char* out = ctx.out();
    if (!zeros)
    {
        for (std::size_t i = 0; i < lpad; ++i)
            encode_one(out, fill, cs);
    }
    if (v < 0)
    {
        encode_one(out, '-', cs);
        v *= -1;
        --n;
    }
    else if (sign != '-')
    {
        encode_one(out, sign, cs);
        --n;
    }
    if (zeros)
    {
        for (std::size_t i = 0; i < lpad; ++i)
            encode_one(out, '0', cs);
    }
    while (n)
    {
        unsigned long long int d = v / p;
        encode_one(out, '0' + static_cast<char>(d), cs);
        --n;
        v %= p;
        p /= 10;
    }
    if (!zeros)
    {
        for (std::size_t i = 0; i < rpad; ++i)
            encode_one(out, fill, cs);
    }
    return out;
}

char*
integer_formatter_impl::
format(
unsigned long long int v,
format_context& ctx,
grammar::lut_chars const& cs) const
{
    // get n digits
    unsigned long long int v0 = v;
    unsigned long long int p = 1;
    std::size_t n = 0;
    if (sign != '-')
    {
        ++n;
    }
    while (v > 0)
    {
        if (v >= 10)
            p *= 10;
        v /= 10;
        ++n;
    }
    static constexpr auto m =
        std::numeric_limits<unsigned long long int>::digits10;
    BOOST_ASSERT(n <= m + 1);
    ignore_unused(m);

    // get pad
    std::size_t w = width;
    if (width_idx != std::size_t(-1) ||
        !width_name.empty())
    {
        get_width_from_args(
            width_idx, width_name, ctx.args(), w);
    }
    std::size_t lpad = 0;
    std::size_t rpad = 0;
    if (w > n)
    {
        std::size_t pad = w - n;
        if (zeros)
        {
            lpad = pad;
        }
        else
        {
            switch (align)
            {
            case '<':
                rpad = pad;
                break;
            case '>':
                lpad = pad;
                break;
            case '^':
                lpad = pad / 2;
                rpad = pad - lpad;
                break;
            }
        }
    }

    // write
    v = v0;
    char* out = ctx.out();
    if (!zeros)
    {
        for (std::size_t i = 0; i < lpad; ++i)
            encode_one(out, fill, cs);
    }
    if (sign != '-')
    {
        encode_one(out, sign, cs);
        --n;
    }
    if (zeros)
    {
        for (std::size_t i = 0; i < lpad; ++i)
            encode_one(out, '0', cs);
    }
    while (n)
    {
        unsigned long long int d = v / p;
        encode_one(out, '0' + static_cast<char>(d), cs);
        --n;
        v %= p;
        p /= 10;
    }
    if (!zeros)
    {
        for (std::size_t i = 0; i < rpad; ++i)
            encode_one(out, fill, cs);
    }
    return out;
}

} // detail
} // urls
} // boost

#endif
