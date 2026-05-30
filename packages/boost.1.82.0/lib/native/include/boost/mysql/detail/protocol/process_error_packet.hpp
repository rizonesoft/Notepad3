//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_PROCESS_ERROR_PACKET_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_PROCESS_ERROR_PACKET_HPP

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>

#include <boost/mysql/detail/protocol/db_flavor.hpp>
#include <boost/mysql/detail/protocol/deserialization_context.hpp>

namespace boost {
namespace mysql {
namespace detail {

inline error_code process_error_packet(deserialization_context& ctx, db_flavor flavor, diagnostics& diag);

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/protocol/impl/process_error_packet.ipp>

#endif
