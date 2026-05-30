//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

#ifndef BOOST_URL_DETAIL_IMPL_PCT_FORMAT_IPP
#define BOOST_URL_DETAIL_IMPL_PCT_FORMAT_IPP

#include <boost/url/detail/pct_format.hpp>
#include <boost/url/grammar/unsigned_rule.hpp>

namespace boost {
namespace urls {
namespace detail {

std::size_t
pct_vmeasure(
    grammar::lut_chars const& cs,
    format_parse_context& pctx,
    measure_context& mctx)
{
    auto it0 = pctx.begin();
    auto end = pctx.end();
    while( it0 != end )
    {
        // look for replacement id
        char const* it1 = it0;
        while(
            it1 != end &&
            *it1 != '{' )
        {
            ++it1;
        }

        // output literal prefix
        if( it0 != it1 )
        {
            for (char const* i = it0; i != it1; ++i)
                mctx.advance_to( mctx.out() + measure_one(*i, cs));
        }

        // over
        if( it1 == end )
        {
            break;
        }

        // enter replacement id
        ++it1;
        BOOST_ASSERT(it1 != end);

        // handle escaped replacement (second '{')
        // there's no "{{" in URL templates because
        // '{'s are not allowed in URLs
        BOOST_ASSERT(*it1 != '{');
        /*
        if( *it1 == '{' )
        {
            mctx.advance_to( mctx.out() + measure_one('{', cs));
            ++it1;
            // this was not a real replacement,
            // so we just keep moving
            continue;
        }
        */


        // parse {id} or {id:specs}
        char const* id_start = it1;
        while (it1 != end &&
               *it1 != ':' &&
               *it1 != '}')
        {
            ++it1;
        }
        string_view id(id_start, it1);

        // move to specs start
        if (it1 != end &&
            *it1 == ':')
            ++it1;
        pctx.advance_to( it1 );

        // get format_arg to use
        auto idv = grammar::parse(
            id, grammar::unsigned_rule<std::size_t>{});
        if (idv)
        {
            mctx.arg( *idv ).measure( pctx, mctx, cs );
        }
        else if (!id.empty())
        {
            mctx.arg( id ).measure( pctx, mctx, cs );
        }
        else
        {
            std::size_t arg_id = pctx.next_arg_id();
            mctx.arg( arg_id ).measure( pctx, mctx, cs );
        }


        it1 = pctx.begin();
        BOOST_ASSERT(*it1 == '}');
        it0 = it1 + 1;
    }

    return mctx.out();
}

char*
pct_vformat(
    grammar::lut_chars const& cs,
    format_parse_context& pctx,
    format_context& fctx)
{
    auto it0 = pctx.begin();
    auto end = pctx.end();
    while( it0 != end )
    {
        // look for replacement id
        char const* it1 = it0;
        while(
            it1 != end &&
            *it1 != '{' )
        {
            ++it1;
        }

        // output literal prefix
        if( it0 != it1 )
        {
            for (char const* i = it0; i != it1; ++i)
            {
                char* o = fctx.out();
                encode_one(o, *i, cs);
                fctx.advance_to(o);
            }
        }

        // over
        if( it1 == end )
        {
            break;
        }

        // enter replacement id
        ++it1;
        BOOST_ASSERT(it1 != end);

        // handle escaped replacement (second '{')
        // there's no "{{" in URL templates because
        // '{'s are not allowed in URLs
        BOOST_ASSERT(*it1 != '{');
        /*
        if( *it1 == '{' )
        {
            char* o = fctx.out();
            encode_one(o, '{', cs);
            fctx.advance_to(o);
            ++it1;
            // this was not a real replacement,
            // so we just keep moving
            continue;
        }
        */

        // parse {id} or {id:specs}
        char const* id_start = it1;
        while (it1 != end &&
               *it1 != ':' &&
               *it1 != '}')
        {
            ++it1;
        }
        string_view id(id_start, it1);

        // move to specs part
        if (it1 != end &&
            *it1 == ':')
            ++it1;
        pctx.advance_to( it1 );

        // get format_arg to use
        auto idv = grammar::parse(
            id, grammar::unsigned_rule<std::size_t>{});
        if (idv)
        {
            fctx.arg( *idv ).format( pctx, fctx, cs );
        }
        else if (!id.empty())
        {
            fctx.arg( id ).format( pctx, fctx, cs );
        }
        else
        {
            std::size_t arg_id = pctx.next_arg_id();
            fctx.arg( arg_id ).format( pctx, fctx, cs );
        }

        it1 = pctx.begin();
        BOOST_ASSERT(*it1 == '}');
        it0 = it1 + 1;
    }

    return fctx.out();
}

} // detail
} // urls
} // boost

#endif
