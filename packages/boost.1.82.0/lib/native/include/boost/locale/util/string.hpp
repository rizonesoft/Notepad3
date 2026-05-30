//
// Copyright (c) 2022-2023 Alexander Grund
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_LOCALE_UTIL_STRING_HPP
#define BOOST_LOCALE_UTIL_STRING_HPP

#include <boost/locale/config.hpp>

namespace boost { namespace locale { namespace util {
    /// Return the end of a C-string, i.e. the pointer to the trailing NULL byte
    template<typename Char>
    Char* str_end(Char* str)
    {
        while(*str)
            ++str;
        return str;
    }

    inline bool is_upper_ascii(const char c)
    {
        return 'A' <= c && c <= 'Z';
    }

    inline bool is_lower_ascii(const char c)
    {
        return 'a' <= c && c <= 'z';
    }

    inline bool is_numeric_ascii(const char c)
    {
        return '0' <= c && c <= '9';
    }

}}} // namespace boost::locale::util

#endif