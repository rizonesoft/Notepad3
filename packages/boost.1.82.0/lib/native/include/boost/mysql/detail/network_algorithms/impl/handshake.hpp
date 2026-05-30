//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_HANDSHAKE_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_HANDSHAKE_HPP

#pragma once

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/handshake_params.hpp>

#include <boost/mysql/detail/auth/auth_calculator.hpp>
#include <boost/mysql/detail/channel/channel.hpp>
#include <boost/mysql/detail/network_algorithms/handshake.hpp>
#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/db_flavor.hpp>
#include <boost/mysql/detail/protocol/handshake_messages.hpp>
#include <boost/mysql/detail/protocol/process_error_packet.hpp>
#include <boost/mysql/detail/protocol/serialization.hpp>

#include <boost/asio/buffer.hpp>

#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

inline std::uint8_t get_collation_first_byte(std::uint16_t value) { return value % 0xff; }

inline capabilities conditional_capability(bool condition, std::uint32_t cap)
{
    return capabilities(condition ? cap : 0);
}

inline db_flavor parse_db_version(string_view version_string) noexcept
{
    return version_string.find("MariaDB") != string_view::npos ? db_flavor::mariadb : db_flavor::mysql;
}

inline error_code deserialize_handshake(
    boost::asio::const_buffer buffer,
    handshake_packet& output,
    diagnostics& diag
)
{
    deserialization_context ctx(boost::asio::buffer(buffer), capabilities());
    std::uint8_t msg_type = 0;
    auto err = deserialize_message_part(ctx, msg_type);
    if (err)
        return err;
    if (msg_type == handshake_protocol_version_9)
    {
        return make_error_code(client_errc::server_unsupported);
    }
    else if (msg_type == error_packet_header)
    {
        // We don't know which DB is yet
        return process_error_packet(ctx, db_flavor::mysql, diag);
    }
    else if (msg_type != handshake_protocol_version_10)
    {
        return make_error_code(client_errc::protocol_value_error);
    }
    return deserialize_message(ctx, output);
}

inline error_code process_capabilities(
    const handshake_params& params,
    const handshake_packet& handshake,
    bool is_ssl_stream,
    capabilities& negotiated_caps
)
{
    auto ssl = params.ssl();
    capabilities server_caps(handshake.capability_falgs);
    capabilities required_caps = mandatory_capabilities |
                                 conditional_capability(!params.database().empty(), CLIENT_CONNECT_WITH_DB) |
                                 conditional_capability(
                                     ssl == ssl_mode::require && is_ssl_stream,
                                     CLIENT_SSL
                                 );
    if (required_caps.has(CLIENT_SSL) && !server_caps.has(CLIENT_SSL))
    {
        // This happens if the server doesn't have SSL configured. This special
        // error code helps users diagnosing their problem a lot (server_unsupported doesn't).
        return make_error_code(client_errc::server_doesnt_support_ssl);
    }
    else if (!server_caps.has_all(required_caps))
    {
        return make_error_code(client_errc::server_unsupported);
    }
    negotiated_caps = server_caps &
                      (required_caps | optional_capabilities |
                       conditional_capability(ssl == ssl_mode::enable && is_ssl_stream, CLIENT_SSL));
    return error_code();
}

// When receiving an auth response from the server, several things can happen:
//  - An OK packet. It means we are done with the auth phase. auth_result::complete.
//  - An auth switch response. It means we should change the auth plugin,
//    recalculate the auth response and send it back. auth_result::send_more_data.
//  - An auth more data. Same as auth switch response, but without changing
//    the authentication plugin. Also auth_result::send_more_data.
//  - An auth more data with a challenge equals to fast_auth_complete_challenge.
//    This means auth is complete and we should wait for an OK packet (auth_result::wait_for_ok).
//    I have no clue why the server sends this instead of just an OK packet. It
//    happens just for caching_sha2_password.
enum class auth_state
{
    complete,
    send_more_data,
    wait_for_ok,
    invalid
};

class handshake_processor
{
    handshake_params params_;
    diagnostics& diag_;
    channel_base& channel_;
    auth_calculator auth_calc_;
    auth_state auth_state_{auth_state::invalid};

public:
    handshake_processor(const handshake_params& params, diagnostics& diag, channel_base& channel)
        : params_(params), diag_(diag), channel_(channel){};
    const handshake_params& params() const noexcept { return params_; }
    channel_base& get_channel() noexcept { return channel_; }
    void clear_diagnostics() noexcept { diag_.clear(); }

    // Once the handshake is processed, the capabilities are stored in the channel
    bool use_ssl() const noexcept { return channel_.current_capabilities().has(CLIENT_SSL); }

    // Initial greeting processing
    error_code process_handshake(boost::asio::const_buffer buffer, bool is_ssl_stream)
    {
        // Deserialize server greeting
        handshake_packet handshake;
        auto err = deserialize_handshake(buffer, handshake, diag_);
        if (err)
            return err;

        // Check capabilities
        capabilities negotiated_caps;
        err = process_capabilities(params_, handshake, is_ssl_stream, negotiated_caps);
        if (err)
            return err;
        channel_.set_current_capabilities(negotiated_caps);

        // Check whether it's MySQL or MariaDB
        auto flavor = parse_db_version(handshake.server_version.value);
        channel_.set_flavor(flavor);

        // Compute auth response
        return auth_calc_.calculate(
            handshake.auth_plugin_name.value,
            params_.password(),
            handshake.auth_plugin_data.value(),
            use_ssl()
        );
    }

    // Response to that initial greeting
    void compose_ssl_request(bytestring& buffer)
    {
        ssl_request sslreq{
            channel_.current_capabilities().get(),
            static_cast<std::uint32_t>(MAX_PACKET_SIZE),
            get_collation_first_byte(params_.connection_collation()),
            {}};

        // Serialize and send
        serialize_message(sslreq, channel_.current_capabilities(), buffer);
    }

    void compose_handshake_response(bytestring& buffer)
    {
        // Compose response
        handshake_response_packet response{
            channel_.current_capabilities().get(),
            static_cast<std::uint32_t>(MAX_PACKET_SIZE),
            get_collation_first_byte(params_.connection_collation()),
            string_null(params_.username()),
            string_lenenc(auth_calc_.response()),
            string_null(params_.database()),
            string_null(auth_calc_.plugin_name())};

        // Serialize
        serialize_message(response, channel_.current_capabilities(), buffer);
    }

    // Server handshake response
    error_code process_handshake_server_response(
        boost::asio::const_buffer server_response,
        bytestring& write_buffer
    )
    {
        deserialization_context ctx(server_response, channel_.current_capabilities());
        std::uint8_t msg_type = 0;
        auto err = deserialize_message_part(ctx, msg_type);
        if (err)
            return err;
        if (msg_type == ok_packet_header)
        {
            // Auth success via fast auth path
            auth_state_ = auth_state::complete;
            return error_code();
        }
        else if (msg_type == error_packet_header)
        {
            return process_error_packet(ctx, channel_.flavor(), diag_);
        }
        else if (msg_type == auth_switch_request_header)
        {
            // We have received an auth switch request. Deserialize it
            auth_switch_request_packet auth_sw;
            auto err = deserialize_message(ctx, auth_sw);
            if (err)
                return err;

            // Compute response
            err = auth_calc_.calculate(
                auth_sw.plugin_name.value,
                params_.password(),
                auth_sw.auth_plugin_data.value,
                use_ssl()
            );
            if (err)
                return err;

            // Serialize
            serialize_message(
                auth_switch_response_packet{string_eof(auth_calc_.response())},
                channel_.current_capabilities(),
                write_buffer
            );

            auth_state_ = auth_state::send_more_data;
            return error_code();
        }
        else if (msg_type == auth_more_data_header)
        {
            // We have received an auth more data request. Deserialize it
            auth_more_data_packet more_data;
            auto err = deserialize_message(ctx, more_data);
            if (err)
                return err;

            string_view challenge = more_data.auth_plugin_data.value;
            if (challenge == fast_auth_complete_challenge)
            {
                auth_state_ = auth_state::wait_for_ok;
                return error_code();
            }

            // Compute response
            err = auth_calc_.calculate(auth_calc_.plugin_name(), params_.password(), challenge, use_ssl());
            if (err)
                return err;

            serialize_message(
                auth_switch_response_packet{string_eof(auth_calc_.response())},
                channel_.current_capabilities(),
                write_buffer
            );

            auth_state_ = auth_state::send_more_data;
            return error_code();
        }
        else
        {
            return make_error_code(client_errc::protocol_value_error);
        }
    }

    bool should_send_auth_switch_response() const noexcept
    {
        return auth_state_ == auth_state::send_more_data;
    }

    bool auth_complete() const noexcept { return auth_state_ == auth_state::complete; }
};

template <class Stream>
struct handshake_op : boost::asio::coroutine
{
    handshake_processor processor_;

    handshake_op(const handshake_params& params, diagnostics& diag, channel<Stream>& channel)
        : processor_(params, diag, channel)
    {
    }

    channel<Stream>& get_channel() noexcept
    {
        return static_cast<channel<Stream>&>(processor_.get_channel());
    }

    template <class Self>
    void operator()(Self& self, error_code err = {}, boost::asio::const_buffer read_msg = {})
    {
        // Error checking
        if (err)
        {
            self.complete(err);
            return;
        }

        // Non-error path
        BOOST_ASIO_CORO_REENTER(*this)
        {
            processor_.clear_diagnostics();

            // Setup the channel
            get_channel().reset();

            // Read server greeting
            BOOST_ASIO_CORO_YIELD get_channel().async_read_one(
                get_channel().shared_sequence_number(),
                std::move(self)
            );

            // Process server greeting
            err = processor_.process_handshake(read_msg, is_ssl_stream<Stream>::value);
            if (err)
            {
                self.complete(err);
                BOOST_ASIO_CORO_YIELD break;
            }

            // SSL
            if (processor_.use_ssl())
            {
                // Send SSL request
                processor_.compose_ssl_request(get_channel().shared_buffer());
                BOOST_ASIO_CORO_YIELD get_channel().async_write(
                    get_channel().shared_buffer(),
                    get_channel().shared_sequence_number(),
                    std::move(self)
                );

                // SSL handshake
                BOOST_ASIO_CORO_YIELD get_channel().stream().async_handshake(std::move(self));
            }

            // Compose and send handshake response
            processor_.compose_handshake_response(get_channel().shared_buffer());
            BOOST_ASIO_CORO_YIELD get_channel().async_write(
                get_channel().shared_buffer(),
                get_channel().shared_sequence_number(),
                std::move(self)
            );

            while (!processor_.auth_complete())
            {
                // Receive response
                BOOST_ASIO_CORO_YIELD get_channel().async_read_one(
                    get_channel().shared_sequence_number(),
                    std::move(self)
                );

                // Process it
                err = processor_.process_handshake_server_response(read_msg, get_channel().shared_buffer());
                if (err)
                {
                    self.complete(err);
                    BOOST_ASIO_CORO_YIELD break;
                }

                // We received an auth switch response and we have the response ready to be sent
                if (processor_.should_send_auth_switch_response())
                {
                    BOOST_ASIO_CORO_YIELD get_channel().async_write(
                        get_channel().shared_buffer(),
                        get_channel().shared_sequence_number(),
                        std::move(self)
                    );
                }
            }

            self.complete(error_code());
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

template <class Stream>
void boost::mysql::detail::handshake(
    channel<Stream>& channel,
    const handshake_params& params,
    error_code& err,
    diagnostics& diag
)
{
    channel.reset();

    // Set up processor
    handshake_processor processor(params, diag, channel);

    // Read server greeting
    auto b = channel.read_one(channel.shared_sequence_number(), err);
    if (err)
        return;

    // Process server greeting (handshake)
    err = processor.process_handshake(b, is_ssl_stream<Stream>::value);
    if (err)
        return;

    // SSL
    if (processor.use_ssl())
    {
        // Send SSL request
        processor.compose_ssl_request(channel.shared_buffer());
        channel.write(channel.shared_buffer(), channel.shared_sequence_number(), err);
        if (err)
            return;

        // SSL handshake
        channel.stream().handshake(err);
        if (err)
            return;
    }

    // Handshake response
    processor.compose_handshake_response(channel.shared_buffer());
    channel.write(channel.shared_buffer(), channel.shared_sequence_number(), err);
    if (err)
        return;

    while (!processor.auth_complete())
    {
        // Receive response
        b = channel.read_one(channel.shared_sequence_number(), err);
        if (err)
            return;

        // Process it
        err = processor.process_handshake_server_response(b, channel.shared_buffer());
        if (err)
            return;

        if (processor.should_send_auth_switch_response())
        {
            // We received an auth switch request and we have the response ready to be sent
            channel.write(channel.shared_buffer(), channel.shared_sequence_number(), err);
            if (err)
                return;
        }
    };
}

template <class Stream, BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_handshake(
    channel<Stream>& chan,
    const handshake_params& params,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code)>(
        handshake_op<Stream>(params, diag, chan),
        token,
        chan
    );
}

#endif
