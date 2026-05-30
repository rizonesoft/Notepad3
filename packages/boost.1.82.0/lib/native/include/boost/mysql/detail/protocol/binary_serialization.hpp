//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_BINARY_SERIALIZATION_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_BINARY_SERIALIZATION_HPP

#include <boost/mysql/detail/protocol/serialization.hpp>
#include <boost/mysql/field_view.hpp>

namespace boost {
namespace mysql {
namespace detail {

template <>
struct serialization_traits<field_view, serialization_tag::none> : noop_deserialize<field_view>
{
    static inline std::size_t get_size_(
        const serialization_context& ctx,
        const field_view& input
    ) noexcept;

    static inline void serialize_(serialization_context& ctx, const field_view& input) noexcept;
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/protocol/impl/binary_serialization.ipp>

#endif
