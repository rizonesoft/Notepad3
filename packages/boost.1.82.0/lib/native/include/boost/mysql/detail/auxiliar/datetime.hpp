//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_DATETIME_HPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_DATETIME_HPP

// All these algorithms have been taken from:
// http://howardhinnant.github.io/date_algorithms.html

#include <boost/config.hpp>

#include <cstdint>

namespace boost {
namespace mysql {
namespace detail {

constexpr inline bool is_valid(std::uint16_t years, std::uint8_t month, std::uint8_t day) noexcept;

BOOST_CXX14_CONSTEXPR inline int ymd_to_days(
    std::uint16_t years,
    std::uint8_t month,
    std::uint8_t day
) noexcept;

BOOST_CXX14_CONSTEXPR inline bool days_to_ymd(
    int num_days,
    std::uint16_t& years,
    std::uint8_t& month,
    std::uint8_t& day
) noexcept;

constexpr std::uint16_t max_year = 9999;
constexpr std::uint8_t max_month = 12;
constexpr std::uint8_t max_day = 31;
constexpr std::uint8_t max_hour = 23;
constexpr std::uint8_t max_min = 59;
constexpr std::uint8_t max_sec = 59;
constexpr std::uint32_t max_micro = 999999;

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/auxiliar/impl/datetime.hpp>

#endif
