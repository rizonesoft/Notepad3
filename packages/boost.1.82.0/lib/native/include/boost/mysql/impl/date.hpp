//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_IMPL_DATE_HPP
#define BOOST_MYSQL_IMPL_DATE_HPP

#pragma once

#include <boost/mysql/date.hpp>

#include <cstdio>
#include <ostream>
#include <stdexcept>

BOOST_CXX14_CONSTEXPR boost::mysql::date::date(time_point tp)
{
    bool ok = detail::days_to_ymd(tp.time_since_epoch().count(), year_, month_, day_);
    if (!ok)
        throw std::out_of_range("date::date: time_point was out of range");
}

BOOST_CXX14_CONSTEXPR boost::mysql::date::time_point boost::mysql::date::get_time_point() const noexcept
{
    assert(valid());
    return unch_get_time_point();
}

BOOST_CXX14_CONSTEXPR boost::mysql::date::time_point boost::mysql::date::as_time_point() const
{
    if (!valid())
        throw std::invalid_argument("date::as_time_point: invalid date");
    return unch_get_time_point();
}

constexpr bool boost::mysql::date::operator==(const date& rhs) const noexcept
{
    return year_ == rhs.year_ && month_ == rhs.month_ && day_ == rhs.day_;
}

BOOST_CXX14_CONSTEXPR boost::mysql::date::time_point boost::mysql::date::unch_get_time_point() const noexcept
{
    return time_point(days(detail::ymd_to_days(year_, month_, day_)));
}

std::ostream& boost::mysql::operator<<(std::ostream& os, const date& value)
{
    // Worst-case output is 14 chars, extra space just in case
    char buffer[32]{};
    snprintf(
        buffer,
        sizeof(buffer),
        "%04u-%02u-%02u",
        static_cast<unsigned>(value.year()),
        static_cast<unsigned>(value.month()),
        static_cast<unsigned>(value.day())
    );
    os << buffer;
    return os;
}

#endif
