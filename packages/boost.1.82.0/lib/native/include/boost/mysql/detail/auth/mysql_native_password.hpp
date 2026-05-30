//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUTH_MYSQL_NATIVE_PASSWORD_HPP
#define BOOST_MYSQL_DETAIL_AUTH_MYSQL_NATIVE_PASSWORD_HPP

#include <boost/mysql/error_code.hpp>
#include <boost/mysql/string_view.hpp>

#include <boost/mysql/detail/auxiliar/bytestring.hpp>

#include <cstdint>

namespace boost {
namespace mysql {
namespace detail {
namespace mysql_native_password {

// Authorization for this plugin is always challenge (nonce) -> response
// (hashed password).
inline error_code compute_response(
    string_view password,
    string_view challenge,
    bool use_ssl,
    bytestring& output
);

}  // namespace mysql_native_password
}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/auth/impl/mysql_native_password.ipp>

#endif
