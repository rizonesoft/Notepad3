//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUTH_IMPL_CACHING_SHA2_PASSWORD_IPP
#define BOOST_MYSQL_DETAIL_AUTH_IMPL_CACHING_SHA2_PASSWORD_IPP

#pragma once

#include <boost/mysql/client_errc.hpp>

#include <boost/mysql/detail/auth/caching_sha2_password.hpp>
#include <boost/mysql/detail/auxiliar/bytestring.hpp>
#include <boost/mysql/detail/auxiliar/make_string_view.hpp>

#include <cstring>
#include <openssl/sha.h>

namespace boost {
namespace mysql {
namespace detail {
namespace caching_sha2_password {

constexpr std::size_t challenge_length = 20;
constexpr std::size_t response_length = 32;
constexpr string_view perform_full_auth = make_string_view("\4");

// challenge must point to challenge_length bytes of data
// output must point to response_length bytes of data
inline void compute_auth_string(string_view password, const void* challenge, void* output)
{
    static_assert(response_length == SHA256_DIGEST_LENGTH, "Buffer size mismatch");

    // SHA(SHA(password_sha) concat challenge) XOR password_sha
    // hash1 = SHA(pass)
    using sha_buffer = std::uint8_t[response_length];
    sha_buffer password_sha;
    SHA256(reinterpret_cast<const unsigned char*>(password.data()), password.size(), password_sha);

    // SHA(password_sha) concat challenge = buffer
    std::uint8_t buffer[response_length + challenge_length];
    SHA256(password_sha, response_length, buffer);
    std::memcpy(buffer + response_length, challenge, challenge_length);

    // SHA(SHA(password_sha) concat challenge) = SHA(buffer) = salted_password
    sha_buffer salted_password;
    SHA256(buffer, sizeof(buffer), salted_password);

    // salted_password XOR password_sha
    for (unsigned i = 0; i < response_length; ++i)
    {
        static_cast<std::uint8_t*>(output)[i] = salted_password[i] ^ password_sha[i];
    }
}

}  // namespace caching_sha2_password
}  // namespace detail
}  // namespace mysql
}  // namespace boost

inline boost::mysql::error_code boost::mysql::detail::caching_sha2_password::compute_response(
    string_view password,
    string_view challenge,
    bool use_ssl,
    bytestring& output
)
{
    if (challenge == perform_full_auth)
    {
        if (!use_ssl)
        {
            return make_error_code(client_errc::auth_plugin_requires_ssl);
        }
        output.assign(password.begin(), password.end());
        output.push_back(0);
        return error_code();
    }
    else
    {
        // Check challenge size
        if (challenge.size() != challenge_length)
        {
            return make_error_code(client_errc::protocol_value_error);
        }

        // Do the calculation
        output.resize(response_length);
        compute_auth_string(password, challenge.data(), output.data());
        return error_code();
    }
}

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_AUTH_IMPL_CACHING_SHA2_PASSWORD_IPP_ */
