//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUTH_IMPL_AUTH_CALCULATOR_IPP
#define BOOST_MYSQL_DETAIL_AUTH_IMPL_AUTH_CALCULATOR_IPP

#pragma once

#include <boost/mysql/client_errc.hpp>

#include <boost/mysql/detail/auth/auth_calculator.hpp>
#include <boost/mysql/detail/auth/caching_sha2_password.hpp>
#include <boost/mysql/detail/auth/mysql_native_password.hpp>
#include <boost/mysql/detail/auxiliar/make_string_view.hpp>

namespace boost {
namespace mysql {
namespace detail {

constexpr authentication_plugin mysql_native_password_plugin{
    make_string_view("mysql_native_password"),
    &mysql_native_password::compute_response};

constexpr authentication_plugin caching_sha2_password_plugin{
    make_string_view("caching_sha2_password"),
    &caching_sha2_password::compute_response};

constexpr const authentication_plugin* all_authentication_plugins[] = {
    &mysql_native_password_plugin,
    &caching_sha2_password_plugin};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

inline const boost::mysql::detail::authentication_plugin* boost::mysql::detail::auth_calculator::
    find_plugin(string_view name)
{
    auto it = std::find_if(
        std::begin(all_authentication_plugins),
        std::end(all_authentication_plugins),
        [name](const authentication_plugin* plugin) { return plugin->name == name; }
    );
    return it == std::end(all_authentication_plugins) ? nullptr : *it;
}

inline boost::mysql::error_code boost::mysql::detail::auth_calculator::calculate(
    string_view plugin_name,
    string_view password,
    string_view challenge,
    bool use_ssl
)
{
    plugin_ = find_plugin(plugin_name);
    if (plugin_)
    {
        // Blank password: we should just return an empty auth string
        if (password.empty())
        {
            response_.clear();
            return error_code();
        }
        else
        {
            return plugin_->calculator(password, challenge, use_ssl, response_);
        }
    }
    else
    {
        return make_error_code(client_errc::unknown_auth_plugin);
    }
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_AUTH_IMPL_AUTH_CALCULATOR_IPP_ */
