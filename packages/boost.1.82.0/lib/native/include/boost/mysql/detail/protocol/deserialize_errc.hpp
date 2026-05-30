//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZE_ERRC_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZE_ERRC_HPP

#include <boost/mysql/client_errc.hpp>
#include <boost/mysql/error_code.hpp>

#include <cassert>

namespace boost {
namespace mysql {
namespace detail {

enum class deserialize_errc
{
    ok = 0,
    incomplete_message = 1,
    protocol_value_error,
    server_unsupported
};

inline error_code to_error_code(deserialize_errc v) noexcept
{
    switch (v)
    {
    case deserialize_errc::ok: return error_code();
    case deserialize_errc::incomplete_message: return error_code(client_errc::incomplete_message);
    case deserialize_errc::protocol_value_error:
        return error_code(client_errc::protocol_value_error);
    case deserialize_errc::server_unsupported: return error_code(client_errc::server_unsupported);
    default: assert(false); return error_code();  // avoid warnings
    }
}

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif
