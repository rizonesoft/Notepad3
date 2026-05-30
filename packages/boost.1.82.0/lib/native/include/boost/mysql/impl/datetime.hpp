//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_IMPL_DATETIME_HPP
#define BOOST_MYSQL_IMPL_DATETIME_HPP

#pragma once

#include <boost/mysql/datetime.hpp>
#include <boost/mysql/days.hpp>

#include <boost/mysql/detail/auxiliar/datetime.hpp>

#include <cassert>
#include <cstdio>
#include <ostream>
#include <stdexcept>

BOOST_CXX14_CONSTEXPR boost::mysql::datetime::datetime(time_point tp)
{
    using std::chrono::duration_cast;
    using std::chrono::hours;
    using std::chrono::microseconds;
    using std::chrono::minutes;
    using std::chrono::seconds;

    // Avoiding using -= for durations as it's not constexpr until C++17
    auto input_dur = tp.time_since_epoch();
    auto rem = input_dur % days(1);
    if (rem.count() < 0)
        rem = rem + days(1);
    auto num_days = duration_cast<days>(input_dur - rem);
    auto num_hours = duration_cast<hours>(rem);
    rem = rem - num_hours;
    auto num_minutes = duration_cast<minutes>(rem);
    rem = rem - num_minutes;
    auto num_seconds = duration_cast<seconds>(rem);
    rem = rem - num_seconds;
    auto num_microseconds = duration_cast<microseconds>(rem);

    assert(num_hours.count() >= 0 && num_hours.count() <= detail::max_hour);
    assert(num_minutes.count() >= 0 && num_minutes.count() <= detail::max_min);
    assert(num_seconds.count() >= 0 && num_seconds.count() <= detail::max_sec);
    assert(num_microseconds.count() >= 0 && num_microseconds.count() <= detail::max_micro);

    bool ok = detail::days_to_ymd(num_days.count(), year_, month_, day_);
    if (!ok)
        throw std::out_of_range("datetime::datetime: time_point was out of range");

    microsecond_ = static_cast<std::uint32_t>(num_microseconds.count());
    second_ = static_cast<std::uint8_t>(num_seconds.count());
    minute_ = static_cast<std::uint8_t>(num_minutes.count());
    hour_ = static_cast<std::uint8_t>(num_hours.count());
}

constexpr bool boost::mysql::datetime::valid() const noexcept
{
    return detail::is_valid(year_, month_, day_) && hour_ <= detail::max_hour && minute_ <= detail::max_min &&
           second_ <= detail::max_sec && microsecond_ <= detail::max_micro;
}

BOOST_CXX14_CONSTEXPR boost::mysql::datetime::time_point boost::mysql::datetime::get_time_point(
) const noexcept
{
    assert(valid());
    return unch_get_time_point();
}

BOOST_CXX14_CONSTEXPR boost::mysql::datetime::time_point boost::mysql::datetime::as_time_point() const
{
    if (!valid())
        throw std::invalid_argument("datetime::as_time_point: invalid datetime");
    return unch_get_time_point();
}

constexpr bool boost::mysql::datetime::operator==(const datetime& rhs) const noexcept
{
    return year_ == rhs.year_ && month_ == rhs.month_ && day_ == rhs.day_ && hour_ == rhs.hour_ &&
           minute_ == rhs.minute_ && second_ == rhs.second_ && microsecond_ == rhs.microsecond_;
}

BOOST_CXX14_CONSTEXPR boost::mysql::datetime::time_point boost::mysql::datetime::unch_get_time_point(
) const noexcept
{
    // Doing time of day independently to prevent overflow
    days d(detail::ymd_to_days(year_, month_, day_));
    auto time_of_day = std::chrono::hours(hour_) + std::chrono::minutes(minute_) +
                       std::chrono::seconds(second_) + std::chrono::microseconds(microsecond_);
    return time_point(d) + time_of_day;
}

std::ostream& boost::mysql::operator<<(std::ostream& os, const datetime& value)
{
    // Worst-case output is 37 chars, extra space just in case
    char buffer[64]{};
    snprintf(
        buffer,
        sizeof(buffer),
        "%04u-%02u-%02u %02d:%02u:%02u.%06u",
        static_cast<unsigned>(value.year()),
        static_cast<unsigned>(value.month()),
        static_cast<unsigned>(value.day()),
        static_cast<unsigned>(value.hour()),
        static_cast<unsigned>(value.minute()),
        static_cast<unsigned>(value.second()),
        static_cast<unsigned>(value.microsecond())
    );
    os << buffer;
    return os;
}

#endif
