//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZE_BINARY_FIELD_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZE_BINARY_FIELD_HPP

#include <boost/mysql/field_view.hpp>
#include <boost/mysql/metadata.hpp>

#include <boost/mysql/detail/protocol/deserialization_context.hpp>
#include <boost/mysql/detail/protocol/deserialize_errc.hpp>

#include <cstdint>
#include <vector>

namespace boost {
namespace mysql {
namespace detail {

inline deserialize_errc deserialize_binary_field(
    deserialization_context& ctx,
    const metadata& meta,
    const std::uint8_t* buffer_first,
    field_view& output
);

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/protocol/impl/deserialize_binary_field.ipp>

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_PROTOCOL_BINARY_DESERIALIZATION_HPP_ */
