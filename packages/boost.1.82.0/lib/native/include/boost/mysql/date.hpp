//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DATE_HPP
#define BOOST_MYSQL_DATE_HPP

#include <boost/mysql/days.hpp>

#include <boost/mysql/detail/auxiliar/datetime.hpp>

#include <boost/config.hpp>

#include <chrono>
#include <cstdint>
#include <iosfwd>

namespace boost {
namespace mysql {

/**
 * \brief Type representing MySQL `DATE` data type.
 * \details Represents a broken date by its year, month and day components.
 * This type is close to the protocol and should not be used as a vocabulary type.
 * Instead, cast it to a `std::chrono::time_point` by calling \ref as_time_point
 * or \ref get_time_point.
 * \n
 * As opposed to `time_point`, this type allows representing invalid and zero dates.
 */
class date
{
public:
    /// A `std::chrono::time_point` that can represent any valid `date`.
    using time_point = std::chrono::time_point<std::chrono::system_clock, days>;

    /**
     * \brief Constructs a zero date.
     * \details
     * Results in a date with all of its components set to zero.
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr date() noexcept = default;

    /**
     * \brief Constructs a date from its year, month and date components.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr date(std::uint16_t year, std::uint8_t month, std::uint8_t day) noexcept
        : year_(year), month_(month), day_(day)
    {
    }

    /**
     * \brief Constructs a date from a `time_point`.
     * \par Exception safety
     * Strong guarantee. Throws on invalid input.
     * \throws std::out_of_range If the resulting `date` would be
     * out of the [\ref min_date, \ref max_date] range.
     */
    BOOST_CXX14_CONSTEXPR inline explicit date(time_point tp);

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
     * \brief Returns `true` if `*this` represents a valid `time_point`.
     * \details If any of the individual components is out of range, the date
     * doesn't represent an actual `time_point` (e.g. `date(2020, 2, 30)`) or
     * the date is not in the [\ref min_date, \ref max_date] validity range,
     * returns `false`. Otherwise, returns `true`.
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr bool valid() const noexcept { return detail::is_valid(year_, month_, day_); }

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
     * \details Two dates are considered equal if all of its individual components
     * are equal. This function works for invalid dates, too.
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr bool operator==(const date& rhs) const noexcept;

    /**
     * \brief Tests for inequality.
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    constexpr bool operator!=(const date& rhs) const noexcept { return !(rhs == *this); }

    /**
     * \brief Returns the current system time as a date object.
     * \par Exception safety
     * Strong guarantee. Only throws if obtaining the current time throws.
     */
    static date now()
    {
        auto now = time_point::clock::now();
        return date(std::chrono::time_point_cast<time_point::duration>(now));
    }

private:
    std::uint16_t year_{};
    std::uint8_t month_{};
    std::uint8_t day_{};

    BOOST_CXX14_CONSTEXPR inline time_point unch_get_time_point() const noexcept;
};

/**
 * \relates date
 * \brief Streams a date.
 * \details This function works for invalid dates, too.
 */
inline std::ostream& operator<<(std::ostream& os, const date& v);

/// The minimum allowed value for \ref date.
constexpr date min_date{0u, 1u, 1u};

/// The maximum allowed value for \ref date.
constexpr date max_date{9999u, 12u, 31u};

}  // namespace mysql
}  // namespace boost

#include <boost/mysql/impl/date.hpp>

#endif
