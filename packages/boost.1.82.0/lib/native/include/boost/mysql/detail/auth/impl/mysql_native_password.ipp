//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUTH_IMPL_MYSQL_NATIVE_PASSWORD_IPP
#define BOOST_MYSQL_DETAIL_AUTH_IMPL_MYSQL_NATIVE_PASSWORD_IPP

#pragma once

#include <boost/mysql/client_errc.hpp>

#include <boost/mysql/detail/auth/mysql_native_password.hpp>

#include <cstring>
#include <openssl/sha.h>

namespace boost {
namespace mysql {
namespace detail {
namespace mysql_native_password {

constexpr std::size_t challenge_length = 20;
constexpr std::size_t response_length = 20;

// challenge must point to challenge_length bytes of data
// output must point to response_length bytes of data
// SHA1( password ) XOR SHA1( "20-bytes random data from server" <concat> SHA1( SHA1( password ) ) )
inline void compute_auth_string(string_view password, const void* challenge, void* output)
{
    // SHA1 (password)
    using sha1_buffer = unsigned char[SHA_DIGEST_LENGTH];
    sha1_buffer password_sha1;
    SHA1(reinterpret_cast<const unsigned char*>(password.data()), password.size(), password_sha1);

    // Add server challenge (salt)
    unsigned char salted_buffer[challenge_length + SHA_DIGEST_LENGTH];
    memcpy(salted_buffer, challenge, challenge_length);
    SHA1(password_sha1, sizeof(password_sha1), salted_buffer + 20);
    sha1_buffer salted_sha1;
    SHA1(salted_buffer, sizeof(salted_buffer), salted_sha1);

    // XOR
    static_assert(response_length == SHA_DIGEST_LENGTH, "Buffer size mismatch");
    for (std::size_t i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        static_cast<std::uint8_t*>(output)[i] = password_sha1[i] ^ salted_sha1[i];
    }
}

}  // namespace mysql_native_password
}  // namespace detail
}  // namespace mysql
}  // namespace boost

inline boost::mysql::error_code boost::mysql::detail::mysql_native_password::compute_response(
    string_view password,
    string_view challenge,
    bool,  // use_ssl
    bytestring& output
)
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

#endif
