//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DATETIME_HPP
#define BOOST_MYSQL_DATETIME_HPP

#include <boost/mysql/days.hpp>

#include <boost/config.hpp>

#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <ratio>

namespace boost {
namespace mysql {

/**
 * \brief Type representing MySQL `DATETIME` and `TIMESTAMP` data types.
 * \details Represents a broken datetime by its year, month, day, hour, minute, second and
 * microsecond components. This type is close to the protocol and should not be used as a vocabulary
 * type. Instead, cast it to a `std::chrono::time_point` by calling \ref as_time_point or \ref
 * get_time_point.
 * \n
 * As opposed to `time_point`, this type allows representing invalid and zero datetimes.
 */
class datetime
{
public:
    /**
     * \brief A `std::chrono::time_point` that can represent any valid datetime.
     * \details Represents microseconds since the UNIX epoch, with the same precision for all architectures.
     */
    using time_point = std::chrono::
        time_point<std::chrono::system_clock, std::chrono::duration<std::int64_t, std::micro>>;

    /**
     * \brief Constructs a zero datetime.
     * \details Results in a datetime with all of its components set to zero.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr datetime() noexcept = default;

    /**
     * \brief Constructs a datetime from its individual components.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr datetime(
        std::uint16_t year,
        std::uint8_t month,
        std::uint8_t day,
        std::uint8_t hour = 0,
        std::uint8_t minute = 0,
        std::uint8_t second = 0,
        std::uint32_t microsecond = 0
    ) noexcept
        : year_(year),
          month_(month),
          day_(day),
          hour_(hour),
          minute_(minute),
          second_(second),
          microsecond_(microsecond)
    {
    }

    /**
     * \brief Constructs a datetime from a `time_point`.
     * \par Exception safety
     * Strong guarantee. Throws on invalid input.
     * \throws std::out_of_range If the resulting `datetime` object would be
     * out of the [\ref min_datetime, \ref max_datetime] range.
     */
    BOOST_CXX14_CONSTEXPR inline explicit datetime(time_point tp);

    /**
     * \brief Retrieves the year component.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr std::uint16_t year() const noexcept { return year_; }

    /**
     * \brief Retrieves the month component.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr std::uint8_t month() const noexcept { return month_; }

    /**
     * \brief Retrieves the day component.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr std::uint8_t day() const noexcept { return day_; }

    /**
     * \brief Retrieves the hour component.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr std::uint8_t hour() const noexcept { return hour_; }

    /**
     * \brief Retrieves the minute component.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr std::uint8_t minute() const noexcept { return minute_; }

    /**
     * \brief Retrieves the second component.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr std::uint8_t second() const noexcept { return second_; }

    /**
     * \brief Retrieves the microsecond component.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr std::uint32_t microsecond() const noexcept { return microsecond_; }

    /**
     * \brief Returns `true` if `*this` represents a valid `time_point`.
     * \details If any of the individual components is out of range, the datetime
     * doesn't represent an actual `time_point` (e.g. `datetime(2020, 2, 30)`) or
     * the datetime is not in the [\ref min_date, \ref max_date] validity range,
     * returns `false`. Otherwise, returns `true`.
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr bool valid() const noexcept;

    /**
     * \brief Converts `*this` into a `time_point` (unchecked access).
     * \par Preconditions
     * `this->valid() == true` (if violated, results in undefined behavior).
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    BOOST_CXX14_CONSTEXPR inline time_point get_time_point() const noexcept;

    /**
     * \brief Converts `*this` into a `time_point` (checked access).
     * \par Exception safety
     * Strong guarantee.
     * \throws std::invalid_argument If `!this->valid()`.
     */
    BOOST_CXX14_CONSTEXPR inline time_point as_time_point() const;

    /**
     * \brief Tests for equality.
     * \details Two datetimes are considered equal if all of its individual components
     * are equal. This function works for invalid datetimes, too.
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr bool operator==(const datetime& rhs) const noexcept;

    /**
     * \brief Tests for inequality.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr bool operator!=(const datetime& rhs) const noexcept { return !(*this == rhs); }

    /**
     * \brief Returns the current system time as a datetime object.
     * \par Exception safety
     * Strong guarantee. Only throws if obtaining the current time throws.
     */
    static datetime now()
    {
        auto now = time_point::clock::now();
        return datetime(std::chrono::time_point_cast<time_point::duration>(now));
    }

private:
    std::uint16_t year_{};
    std::uint8_t month_{};
    std::uint8_t day_{};
    std::uint8_t hour_{};
    std::uint8_t minute_{};
    std::uint8_t second_{};
    std::uint32_t microsecond_{};

    BOOST_CXX14_CONSTEXPR inline time_point unch_get_time_point() const noexcept;
};

/**
 * \relates datetime
 * \brief Streams a datetime.
 * \details This function works for invalid datetimes, too.
 */
inline std::ostream& operator<<(std::ostream& os, const datetime& v);

/// The minimum allowed value for \ref datetime.
constexpr datetime min_datetime(0u, 1u, 1u);

/// The maximum allowed value for \ref datetime.
constexpr datetime max_datetime(9999u, 12u, 31u, 23u, 59u, 59u, 999999u);

}  // namespace mysql
}  // namespace boost

#include <boost/mysql/impl/datetime.hpp>

#endif
