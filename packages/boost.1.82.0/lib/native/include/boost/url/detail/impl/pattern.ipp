//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

#ifndef BOOST_URL_DETAIL_IMPL_PATTERN_IPP
#define BOOST_URL_DETAIL_IMPL_PATTERN_IPP

#include <boost/url/detail/pattern.hpp>
#include <boost/url/detail/pct_format.hpp>
#include <boost/url/detail/replacement_field_rule.hpp>
#include <boost/url/rfc/detail/host_rule.hpp>
#include <boost/url/rfc/detail/path_rules.hpp>
#include <boost/url/rfc/detail/port_rule.hpp>
#include <boost/url/rfc/detail/scheme_rule.hpp>

namespace boost {
namespace urls {
namespace detail {

static constexpr auto lhost_chars = host_chars + ':';

void
pattern::
apply(
    url_base& u,
    format_args const& args) const
{
    // measure total
    struct sizes
    {
        std::size_t scheme = 0;
        std::size_t user = 0;
        std::size_t pass = 0;
        std::size_t host = 0;
        std::size_t port = 0;
        std::size_t path = 0;
        std::size_t query = 0;
        std::size_t frag = 0;
    };
    sizes n;

    format_parse_context pctx(nullptr, nullptr, 0);
    measure_context mctx(args);
    if (!scheme.empty())
    {
        pctx = {scheme, pctx.next_arg_id()};
        n.scheme = pct_vmeasure(
            grammar::alpha_chars, pctx, mctx);
        mctx.advance_to(0);
    }
    if (has_authority)
    {
        if (has_user)
        {
            pctx = {user, pctx.next_arg_id()};
            n.user = pct_vmeasure(
                user_chars, pctx, mctx);
            mctx.advance_to(0);
            if (has_pass)
            {
                pctx = {pass, pctx.next_arg_id()};
                n.pass = pct_vmeasure(
                    password_chars, pctx, mctx);
                mctx.advance_to(0);
            }
        }
        if (host.starts_with('['))
        {
            BOOST_ASSERT(host.ends_with(']'));
            pctx = {host.substr(1, host.size() - 2), pctx.next_arg_id()};
            n.host = pct_vmeasure(
                lhost_chars, pctx, mctx) + 2;
            mctx.advance_to(0);
        }
        else
        {
            pctx = {host, pctx.next_arg_id()};
            n.host = pct_vmeasure(
                host_chars, pctx, mctx);
            mctx.advance_to(0);
        }
        if (has_port)
        {
            pctx = {port, pctx.next_arg_id()};
            n.port = pct_vmeasure(
                grammar::digit_chars, pctx, mctx);
            mctx.advance_to(0);
        }
    }
    if (!path.empty())
    {
        pctx = {path, pctx.next_arg_id()};
        n.path = pct_vmeasure(
            path_chars, pctx, mctx);
        mctx.advance_to(0);
    }
    if (has_query)
    {
        pctx = {query, pctx.next_arg_id()};
        n.query = pct_vmeasure(
            query_chars, pctx, mctx);
        mctx.advance_to(0);
    }
    if (has_frag)
    {
        pctx = {frag, pctx.next_arg_id()};
        n.frag = pct_vmeasure(
            fragment_chars, pctx, mctx);
        mctx.advance_to(0);
    }
    std::size_t const n_total =
        n.scheme +
        (n.scheme != 0) * 1 + // ":"
        has_authority * 2 +   // "//"
        n.user +
        has_pass * 1 +        // ":"
        n.pass +
        has_user * 1 +        // "@"
        n.host +
        has_port * 1 +        // ":"
        n.port +
        n.path +
        has_query * 1 +       // "?"
        n.query +
        has_frag * 1 +        // "#"
        n.frag;
    u.reserve(n_total);

    // Apply
    pctx = {nullptr, nullptr, 0};
    format_context fctx(nullptr, args);
    url_base::op_t op(u);
    using parts = parts_base;
    if (!scheme.empty())
    {
        auto dest = u.resize_impl(
            parts::id_scheme,
            n.scheme + 1, op);
        pctx = {scheme, pctx.next_arg_id()};
        fctx.advance_to(dest);
        const char* dest1 = pct_vformat(
            grammar::alpha_chars, pctx, fctx);
        dest[n.scheme] = ':';
        // validate
        if (!grammar::parse({dest, dest1}, scheme_rule()))
        {
            throw_invalid_argument();
        }
    }
    if (has_authority)
    {
        if (has_user)
        {
            auto dest = u.set_user_impl(
                n.user, op);
            pctx = {user, pctx.next_arg_id()};
            fctx.advance_to(dest);
            char const* dest1 = pct_vformat(
                user_chars, pctx, fctx);
            u.impl_.decoded_[parts::id_user] =
                pct_string_view(dest, dest1 - dest)
                    ->decoded_size();
            if (has_pass)
            {
                char* destp = u.set_password_impl(
                    n.pass, op);
                pctx = {pass, pctx.next_arg_id()};
                fctx.advance_to(destp);
                dest1 = pct_vformat(
                    password_chars, pctx, fctx);
                u.impl_.decoded_[parts::id_pass] =
                    pct_string_view({destp, dest1})
                        ->decoded_size() + 1;
            }
        }
        auto dest = u.set_host_impl(
            n.host, op);
        if (host.starts_with('['))
        {
            BOOST_ASSERT(host.ends_with(']'));
            pctx = {host.substr(1, host.size() - 2), pctx.next_arg_id()};
            *dest++ = '[';
            fctx.advance_to(dest);
            char* dest1 =
                pct_vformat(lhost_chars, pctx, fctx);
            *dest1++ = ']';
            u.impl_.decoded_[parts::id_host] =
                pct_string_view(dest - 1, dest1 - dest)
                    ->decoded_size();
        }
        else
        {
            pctx = {host, pctx.next_arg_id()};
            fctx.advance_to(dest);
            char const* dest1 =
                pct_vformat(host_chars, pctx, fctx);
            u.impl_.decoded_[parts::id_host] =
                pct_string_view(dest, dest1 - dest)
                    ->decoded_size();
        }
        auto uh = u.encoded_host();
        auto h = grammar::parse(uh, host_rule).value();
        std::memcpy(
            u.impl_.ip_addr_,
            h.addr,
            sizeof(u.impl_.ip_addr_));
        u.impl_.host_type_ = h.host_type;
        if (has_port)
        {
            dest = u.set_port_impl(n.port, op);
            pctx = {port, pctx.next_arg_id()};
            fctx.advance_to(dest);
            char const* dest1 = pct_vformat(
                grammar::digit_chars, pctx, fctx);
            u.impl_.decoded_[parts::id_port] =
                pct_string_view(dest, dest1 - dest)
                    ->decoded_size() + 1;
            string_view up = {dest - 1, dest1};
            auto p = grammar::parse(up, detail::port_part_rule).value();
            if (p.has_port)
                u.impl_.port_number_ = p.port_number;
        }
    }
    if (!path.empty())
    {
        auto dest = u.resize_impl(
            parts::id_path,
            n.path, op);
        pctx = {path, pctx.next_arg_id()};
        fctx.advance_to(dest);
        auto dest1 = pct_vformat(
            path_chars, pctx, fctx);
        pct_string_view npath(dest, dest1 - dest);
        u.impl_.decoded_[parts::id_path] +=
            npath.decoded_size();
        if (!npath.empty())
        {
            u.impl_.nseg_ = std::count(
                npath.begin() + 1,
                npath.end(), '/') + 1;
        }
        // handle edge cases
        // 1) path is first component and the
        // first segment contains an unencoded ':'
        // This is impossible because the template
        // "{}" would be a host.
        if (u.scheme().empty() &&
            !u.has_authority())
        {
            auto fseg = u.encoded_segments().front();
            std::size_t nc = std::count(
                fseg.begin(), fseg.end(), ':');
            if (nc)
            {
                std::size_t diff = nc * 2;
                u.reserve(n_total + diff);
                dest = u.resize_impl(
                    parts::id_path,
                    n.path + diff, op);
                char* dest0 = dest + diff;
                std::memmove(dest0, dest, n.path);
                while (dest0 != dest)
                {
                    if (*dest0 != ':')
                    {
                        *dest++ = *dest0++;
                    }
                    else
                    {
                        *dest++ = '%';
                        *dest++ = '3';
                        *dest++ = 'A';
                        dest0++;
                    }
                }
            }
        }
        // 2) url has no authority and path
        // starts with "//"
        if (!u.has_authority() &&
            u.encoded_path().starts_with("//"))
        {
            u.reserve(n_total + 2);
            dest = u.resize_impl(
                parts::id_path,
                n.path + 2, op);
            std::memmove(dest + 2, dest, n.path);
            *dest++ = '/';
            *dest = '.';
        }
    }
    if (has_query)
    {
        auto dest = u.resize_impl(
            parts::id_query,
            n.query + 1, op);
        *dest++ = '?';
        pctx = {query, pctx.next_arg_id()};
        fctx.advance_to(dest);
        auto dest1 = pct_vformat(
            query_chars, pctx, fctx);
        pct_string_view nquery(dest, dest1 - dest);
        u.impl_.decoded_[parts::id_query] +=
            nquery.decoded_size() + 1;
        if (!nquery.empty())
        {
            u.impl_.nparam_ = std::count(
                nquery.begin(),
                nquery.end(), '&') + 1;
        }
    }
    if (has_frag)
    {
        auto dest = u.resize_impl(
            parts::id_frag,
            n.frag + 1, op);
        *dest++ = '#';
        pctx = {frag, pctx.next_arg_id()};
        fctx.advance_to(dest);
        auto dest1 = pct_vformat(
            fragment_chars, pctx, fctx);
        u.impl_.decoded_[parts::id_frag] +=
            make_pct_string_view(
                string_view(dest, dest1 - dest))
                ->decoded_size() + 1;
    }
}

// This rule represents a pct-encoded string
// that contains an arbitrary number of
// replacement ids in it
template<class CharSet>
struct pct_encoded_fmt_string_rule_t
{
    using value_type = pct_string_view;

    constexpr
    pct_encoded_fmt_string_rule_t(
        CharSet const& cs) noexcept
        : cs_(cs)
    {
    }

    template<class CharSet_>
    friend
    constexpr
    auto
    pct_encoded_fmt_string_rule(
        CharSet_ const& cs) noexcept ->
    pct_encoded_fmt_string_rule_t<CharSet_>;

    result<value_type>
    parse(
        char const*& it,
        char const* end) const noexcept
    {
        auto const start = it;
        if(it == end)
        {
            // this might be empty
            return {};
        }

        // consume some with literal rule
        // this might be an empty literal
        auto literal_rule = pct_encoded_rule(cs_);
        auto rv = literal_rule.parse(it, end);
        while (rv)
        {
            auto it0 = it;
            // consume some with replacement id
            // rule
            if (!replacement_field_rule.parse(it, end))
            {
                it = it0;
                break;
            }
            rv = literal_rule.parse(it, end);
        }

        return string_view(start, it - start);
    }

private:
    CharSet cs_;
};

template<class CharSet>
constexpr
auto
pct_encoded_fmt_string_rule(
    CharSet const& cs) noexcept ->
    pct_encoded_fmt_string_rule_t<CharSet>
{
    // If an error occurs here it means that
    // the value of your type does not meet
    // the requirements. Please check the
    // documentation!
    static_assert(
        grammar::is_charset<CharSet>::value,
        "CharSet requirements not met");

    return pct_encoded_fmt_string_rule_t<CharSet>(cs);
}

// This rule represents a regular string with
// only chars from the specified charset and
// an arbitrary number of replacement ids in it
template<class CharSet>
struct fmt_token_rule_t
{
    using value_type = pct_string_view;

    constexpr
    fmt_token_rule_t(
        CharSet const& cs) noexcept
        : cs_(cs)
    {
    }

    template<class CharSet_>
    friend
    constexpr
    auto
    fmt_token_rule(
        CharSet_ const& cs) noexcept ->
    fmt_token_rule_t<CharSet_>;

    result<value_type>
    parse(
        char const*& it,
        char const* end) const noexcept
    {
        auto const start = it;
        BOOST_ASSERT(it != end);
        /*
        // This should never happen because
        // all tokens are optional and will
        // already return `none`:
        if(it == end)
        {
            BOOST_URL_RETURN_EC(
                grammar::error::need_more);
        }
        */

        // consume some with literal rule
        // this might be an empty literal
        auto partial_token_rule =
            grammar::optional_rule(
                grammar::token_rule(cs_));
        auto rv = partial_token_rule.parse(it, end);
        while (rv)
        {
            auto it0 = it;
            // consume some with replacement id
            if (!replacement_field_rule.parse(it, end))
            {
                // no replacement and no more cs
                // before: nothing else to consume
                it = it0;
                break;
            }
            // after {...}, consume any more chars
            // in the charset
            rv = partial_token_rule.parse(it, end);
        }

        if(it == start)
        {
            // it != end but we consumed nothing
            BOOST_URL_RETURN_EC(
                grammar::error::need_more);
        }

        return string_view(start, it - start);
    }

private:
    CharSet cs_;
};

template<class CharSet>
constexpr
auto
fmt_token_rule(
    CharSet const& cs) noexcept ->
    fmt_token_rule_t<CharSet>
{
    // If an error occurs here it means that
    // the value of your type does not meet
    // the requirements. Please check the
    // documentation!
    static_assert(
        grammar::is_charset<CharSet>::value,
        "CharSet requirements not met");

    return fmt_token_rule_t<CharSet>(cs);
}

struct userinfo_template_rule_t
{
    struct value_type
    {
        string_view user;
        string_view password;
        bool has_password = false;
    };

    auto
    parse(
        char const*& it,
        char const* end
            ) const noexcept ->
        result<value_type>
    {
        static constexpr auto uchars =
            unreserved_chars +
            sub_delim_chars;
        static constexpr auto pwchars =
            uchars + ':';

        value_type t;

        // user
        static constexpr auto user_fmt_rule =
            pct_encoded_fmt_string_rule(uchars);
        auto rv = grammar::parse(
            it, end, user_fmt_rule);
        BOOST_ASSERT(rv);
        t.user = *rv;

        // ':'
        if( it == end ||
            *it != ':')
        {
            t.has_password = false;
            t.password = {};
            return t;
        }
        ++it;

        // pass
        static constexpr auto pass_fmt_rule =
            pct_encoded_fmt_string_rule(grammar::ref(pwchars));
        rv = grammar::parse(
            it, end, pass_fmt_rule);
        BOOST_ASSERT(rv);
        t.has_password = true;
        t.password = *rv;

        return t;
    }
};

constexpr userinfo_template_rule_t userinfo_template_rule{};

struct host_template_rule_t
{
    using value_type = string_view;

    auto
    parse(
        char const*& it,
        char const* end
            ) const noexcept ->
        result<value_type>
    {
        if(it == end)
        {
            // empty host
            return {};
        }

        // the host type will be ultimately
        // validated when applying the replacement
        // strings. Any chars allowed in hosts
        // are allowed here.
        if (*it != '[')
        {
            // IPv4address and reg-name have the
            // same char sets.
            constexpr auto any_host_template_rule =
                pct_encoded_fmt_string_rule(host_chars);
            auto rv = grammar::parse(
                it, end, any_host_template_rule);
            // any_host_template_rule can always
            // be empty, so it's never invalid
            BOOST_ASSERT(rv);
            return detail::to_sv(*rv);
        }
        // IP-literals need to be enclosed in
        // "[]" if using ':' in the template
        // string, because the ':' would be
        // ambiguous with the port in fmt string.
        // The "[]:" can be used in replacement
        // strings without the "[]" though.
        constexpr auto ip_literal_template_rule =
            pct_encoded_fmt_string_rule(lhost_chars);
        auto it0 = it;
        auto rv = grammar::parse(
            it, end,
            grammar::optional_rule(
                grammar::tuple_rule(
                    grammar::squelch(
                        grammar::delim_rule('[')),
                    ip_literal_template_rule,
                    grammar::squelch(
                        grammar::delim_rule(']')))));
        // ip_literal_template_rule can always
        // be empty, so it's never invalid, but
        // the rule might fail to match the
        // closing "]"
        BOOST_ASSERT(rv);
        return string_view{it0, it};
    }
};

constexpr host_template_rule_t host_template_rule{};

struct authority_template_rule_t
{
    using value_type = pattern;

    result<value_type>
    parse(
        char const*& it,
        char const* end
    ) const noexcept
    {
        pattern u;

        // [ userinfo "@" ]
        {
            auto rv = grammar::parse(
                it, end,
                grammar::optional_rule(
                    grammar::tuple_rule(
                        userinfo_template_rule,
                        grammar::squelch(
                            grammar::delim_rule('@')))));
            BOOST_ASSERT(rv);
            if(rv->has_value())
            {
                auto& r = **rv;
                u.has_user = true;
                u.user = r.user;
                u.has_pass = r.has_password;
                u.pass = r.password;
            }
        }

        // host
        {
            auto rv = grammar::parse(
                it, end,
                host_template_rule);
            // host is allowed to be empty
            BOOST_ASSERT(rv);
            u.host = *rv;
        }

        // [ ":" port ]
        {
            constexpr auto port_template_rule =
                grammar::optional_rule(
                    fmt_token_rule(grammar::digit_chars));
            auto it0 = it;
            auto rv = grammar::parse(
                it, end,
                grammar::tuple_rule(
                    grammar::squelch(
                        grammar::delim_rule(':')),
                    port_template_rule));
            if (!rv)
            {
                it = it0;
            }
            else
            {
                u.has_port = true;
                if (rv->has_value())
                {
                    u.port = **rv;
                }
            }
        }

        return u;
    }
};

constexpr authority_template_rule_t authority_template_rule{};

struct scheme_template_rule_t
{
    using value_type = string_view;

    result<value_type>
    parse(
        char const*& it,
        char const* end) const noexcept
    {
        auto const start = it;
        if(it == end)
        {
            // scheme can't be empty
            BOOST_URL_RETURN_EC(
                grammar::error::mismatch);
        }
        if(!grammar::alpha_chars(*it) &&
            *it != '{')
        {
            // expected alpha
            BOOST_URL_RETURN_EC(
                grammar::error::mismatch);
        }

        // it starts with replacement id or alpha char
        if (!grammar::alpha_chars(*it))
        {
            if (!replacement_field_rule.parse(it, end))
            {
                // replacement_field_rule is invalid
                BOOST_URL_RETURN_EC(
                    grammar::error::mismatch);
            }
        }
        else
        {
            // skip first
            ++it;
        }

        static
        constexpr
        grammar::lut_chars scheme_chars(
            "0123456789" "+-."
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz");

        // non-scheme chars might be a new
        // replacement-id or just an invalid char
        it = grammar::find_if_not(
            it, end, scheme_chars);
        while (it != end)
        {
            auto it0 = it;
            if (!replacement_field_rule.parse(it, end))
            {
                it = it0;
                break;
            }
            it = grammar::find_if_not(
                it, end, scheme_chars);
        }
        return string_view(start, it - start);
    }
};

constexpr scheme_template_rule_t scheme_template_rule{};

// This rule should consider all url types at the
// same time according to the format string
// - relative urls with no scheme/authority
// - absolute urls have no fragment
struct pattern_rule_t
{
    using value_type = pattern;

    result<value_type>
    parse(
        char const*& it,
        char const* const end
    ) const noexcept
    {
        pattern u;

        // optional scheme
        {
            auto it0 = it;
            auto rv = grammar::parse(
                it, end,
                grammar::tuple_rule(
                    scheme_template_rule,
                    grammar::squelch(
                        grammar::delim_rule(':'))));
            if(rv)
                u.scheme = *rv;
            else
                it = it0;
        }

        // hier_part (authority + path)
        // if there are less than 2 chars left,
        // we are parsing the path
        if (it == end)
        {
            // this is over, so we can consider
            // that a "path-empty"
            return u;
        }
        if(end - it == 1)
        {
            // only one char left
            // it can be a single separator "/",
            // representing an empty absolute path,
            // or a single-char segment
            if(*it == '/')
            {
                // path-absolute
                u.path = {it, 1};
                ++it;
                return u;
            }
            // this can be a:
            // - path-noscheme if there's no scheme, or
            // - path-rootless with a single char, or
            // - path-empty (and consume nothing)
            if (!u.scheme.empty() ||
                *it != ':')
            {
                // path-rootless with a single char
                // this needs to be a segment because
                // the authority needs two slashes
                // "//"
                // path-noscheme also matches here
                // because we already validated the
                // first char
                auto rv = grammar::parse(
                    it, end, urls::detail::segment_rule);
                if(! rv)
                    return rv.error();
                u.path = *rv;
            }
            return u;
        }

        // authority
        if( it[0] == '/' &&
            it[1] == '/')
        {
            // "//" always indicates authority
            it += 2;
            auto rv = grammar::parse(
                it, end,
                authority_template_rule);
            // authority is allowed to be empty
            BOOST_ASSERT(rv);
            u.has_authority = true;
            u.has_user = rv->has_user;
            u.user = rv->user;
            u.has_pass = rv->has_pass;
            u.pass = rv->pass;
            u.host = rv->host;
            u.has_port = rv->has_port;
            u.port = rv->port;
        }

        // the authority requires an absolute path
        // or an empty path
        if (it == end ||
            (u.has_authority &&
             (*it != '/' &&
              *it != '?' &&
              *it != '#')))
        {
            // path-empty
            return u;
        }

        // path-abempty
        // consume the whole path at once because
        // we're going to count number of segments
        // later after the replacements happen
        static constexpr auto segment_fmt_rule =
            pct_encoded_fmt_string_rule(path_chars);
        auto rp = grammar::parse(
            it, end, segment_fmt_rule);
        // path-abempty is allowed to be empty
        BOOST_ASSERT(rp);
        u.path = *rp;

        // [ "?" query ]
        {
            static constexpr auto query_fmt_rule =
                pct_encoded_fmt_string_rule(query_chars);
            auto rv = grammar::parse(
                it, end,
                grammar::tuple_rule(
                    grammar::squelch(
                        grammar::delim_rule('?')),
                    query_fmt_rule));
            // query is allowed to be empty but
            // delim rule is not
            if (rv)
            {
                u.has_query = true;
                u.query = *rv;
            }
        }

        // [ "#" fragment ]
        {
            static constexpr auto frag_fmt_rule =
                pct_encoded_fmt_string_rule(fragment_chars);
            auto rv = grammar::parse(
                it, end,
                grammar::tuple_rule(
                    grammar::squelch(
                        grammar::delim_rule('#')),
                    frag_fmt_rule));
            // frag is allowed to be empty but
            // delim rule is not
            if (rv)
            {
                u.has_frag = true;
                u.frag = *rv;
            }
        }

        return u;
    }
};

constexpr pattern_rule_t pattern_rule{};

result<pattern>
parse_pattern(
    string_view s)
{
    return grammar::parse(
        s, pattern_rule);
}

} // detail
} // urls
} // boost

#endif
