//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_START_EXECUTION_GENERIC_HPP
#define BOOST_MYSQL_DETAIL_NETWORK_ALGORITHMS_IMPL_START_EXECUTION_GENERIC_HPP

#pragma once

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/execution_state.hpp>

#include <boost/mysql/detail/auxiliar/bytestring.hpp>
#include <boost/mysql/detail/channel/channel.hpp>
#include <boost/mysql/detail/network_algorithms/start_execution_generic.hpp>
#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/common_messages.hpp>
#include <boost/mysql/detail/protocol/process_error_packet.hpp>
#include <boost/mysql/detail/protocol/resultset_encoding.hpp>
#include <boost/mysql/detail/protocol/serialization.hpp>

#include <boost/asio/buffer.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>

namespace boost {
namespace mysql {
namespace detail {

class start_execution_processor
{
    channel_base& chan_;
    resultset_encoding encoding_;
    execution_state& st_;
    diagnostics& diag_;
    std::size_t num_fields_{};
    std::size_t remaining_fields_{};

public:
    start_execution_processor(
        channel_base& chan,
        resultset_encoding encoding,
        execution_state& st,
        diagnostics& diag
    ) noexcept
        : chan_(chan), encoding_(encoding), st_(st), diag_(diag){};

    void clear_diagnostics() noexcept { diag_.clear(); }

    template <BOOST_MYSQL_SERIALIZE_FN SerializeFn>
    void process_request(SerializeFn&& request)
    {
        execution_state_access::reset(st_, encoding_);
        std::forward<SerializeFn>(request)(chan_.current_capabilities(), chan_.shared_buffer());
    }

    void process_response(boost::asio::const_buffer msg, error_code& err)
    {
        auto
            response = deserialize_execute_response(msg, chan_.current_capabilities(), chan_.flavor(), diag_);
        switch (response.type)
        {
        case execute_response::type_t::error: err = response.data.err; break;
        case execute_response::type_t::ok_packet:
            execution_state_access::complete(st_, response.data.ok_pack);
            num_fields_ = 0;
            break;
        case execute_response::type_t::num_fields:
            num_fields_ = response.data.num_fields;
            remaining_fields_ = num_fields_;
            execution_state_access::prepare_meta(st_, num_fields_);
            break;
        }
    }

    error_code process_field_definition(boost::asio::const_buffer message)
    {
        column_definition_packet field_definition{};
        deserialization_context ctx(message, chan_.current_capabilities());
        auto err = deserialize_message(ctx, field_definition);
        if (err)
            return err;

        execution_state_access::add_meta(st_, field_definition, chan_.meta_mode());
        --remaining_fields_;
        return error_code();
    }

    std::uint8_t& sequence_number() noexcept { return execution_state_access::get_sequence_number(st_); }
    std::size_t num_fields() const noexcept { return num_fields_; }
    bool has_remaining_fields() const noexcept { return remaining_fields_ != 0; }
};

template <class Stream, BOOST_MYSQL_SERIALIZE_FN SerializeFn>
struct start_execution_generic_op : boost::asio::coroutine
{
    channel<Stream>& chan_;
    SerializeFn serialize_fn_;
    start_execution_processor processor_;

    template <class DeducedSerializeFn>
    start_execution_generic_op(
        channel<Stream>& chan,
        DeducedSerializeFn&& fn,
        const start_execution_processor& processor
    )
        : chan_(chan), serialize_fn_(std::forward<DeducedSerializeFn>(fn)), processor_(processor)
    {
    }

    template <class Self>
    void operator()(Self& self, error_code err = {}, boost::asio::const_buffer read_message = {})
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

            // Serialize the request
            processor_.process_request(std::move(serialize_fn_));

            // Send it
            BOOST_ASIO_CORO_YIELD chan_
                .async_write(chan_.shared_buffer(), processor_.sequence_number(), std::move(self));

            // Read the response
            BOOST_ASIO_CORO_YIELD chan_.async_read_one(processor_.sequence_number(), std::move(self));

            // Response may be: ok_packet, err_packet, local infile request
            // (not implemented), or response with fields
            processor_.process_response(read_message, err);
            if (err)
            {
                self.complete(err);
                BOOST_ASIO_CORO_YIELD break;
            }

            // Read all of the field definitions
            while (processor_.has_remaining_fields())
            {
                // Read from the stream if we need it
                if (!chan_.has_read_messages())
                {
                    BOOST_ASIO_CORO_YIELD chan_.async_read_some(std::move(self));
                }

                // Read the field definition packet
                read_message = chan_.next_read_message(processor_.sequence_number(), err);
                if (err)
                {
                    self.complete(err);
                    BOOST_ASIO_CORO_YIELD break;
                }

                // Process the message
                err = processor_.process_field_definition(read_message);
                if (err)
                {
                    self.complete(err);
                    BOOST_ASIO_CORO_YIELD break;
                }
            }

            // No EOF packet is expected here, as we require deprecate EOF capabilities
            self.complete(error_code());
        }
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

inline boost::mysql::detail::execute_response boost::mysql::detail::deserialize_execute_response(
    boost::asio::const_buffer msg,
    capabilities caps,
    db_flavor flavor,
    diagnostics& diag
) noexcept
{
    // Response may be: ok_packet, err_packet, local infile request (not implemented)
    // If it is none of this, then the message type itself is the beginning of
    // a length-encoded int containing the field count
    deserialization_context ctx(msg, caps);
    std::uint8_t msg_type = 0;
    auto err = deserialize_message_part(ctx, msg_type);
    if (err)
        return err;

    if (msg_type == ok_packet_header)
    {
        ok_packet ok_pack;
        err = deserialize_message(ctx, ok_pack);
        if (err)
            return err;
        return ok_pack;
    }
    else if (msg_type == error_packet_header)
    {
        return process_error_packet(ctx, flavor, diag);
    }
    else
    {
        // Resultset with metadata. First packet is an int_lenenc with
        // the number of field definitions to expect. Message type is part
        // of this packet, so we must rewind the context
        ctx.rewind(1);
        int_lenenc num_fields;
        err = deserialize_message(ctx, num_fields);
        if (err)
            return err;

        // We should have at least one field.
        // The max number of fields is some value around 1024. For simplicity/extensibility,
        // we accept anything less than 0xffff
        if (num_fields.value == 0 || num_fields.value > 0xffffu)
        {
            return make_error_code(client_errc::protocol_value_error);
        }

        return static_cast<std::size_t>(num_fields.value);
    }
}

template <class Stream, BOOST_MYSQL_SERIALIZE_FN SerializeFn>
void boost::mysql::detail::start_execution_generic(
    resultset_encoding encoding,
    channel<Stream>& channel,
    const SerializeFn& fn,
    execution_state& st,
    error_code& err,
    diagnostics& diag
)
{
    // Clear info
    diag.clear();

    // Serialize the request
    start_execution_processor processor(channel, encoding, st, diag);
    processor.process_request(fn);

    // Send it
    channel.write(channel.shared_buffer(), processor.sequence_number(), err);
    if (err)
        return;

    // Read the response
    auto read_buffer = channel.read_one(processor.sequence_number(), err);
    if (err)
        return;

    // Response may be: ok_packet, err_packet, local infile request (not implemented), or response
    // with fields
    processor.process_response(read_buffer, err);
    if (err)
        return;

    // Read all of the field definitions (zero if empty resultset)
    while (processor.has_remaining_fields())
    {
        // Read from the stream if required
        if (!channel.has_read_messages())
        {
            channel.read_some(err);
            if (err)
                return;
        }

        // Read the field definition packet
        read_buffer = channel.next_read_message(processor.sequence_number(), err);
        if (err)
            return;

        // Process the message
        err = processor.process_field_definition(read_buffer);
        if (err)
            return;
    }
}

template <
    class Stream,
    BOOST_MYSQL_SERIALIZE_FN SerializeFn,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(::boost::mysql::error_code)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(boost::mysql::error_code))
boost::mysql::detail::async_start_execution_generic(
    resultset_encoding encoding,
    channel<Stream>& channel,
    SerializeFn&& fn,
    execution_state& st,
    diagnostics& diag,
    CompletionToken&& token
)
{
    return boost::asio::async_compose<CompletionToken, void(error_code)>(
        start_execution_generic_op<Stream, typename std::decay<SerializeFn>::type>(
            channel,
            std::forward<SerializeFn>(fn),
            start_execution_processor(channel, encoding, st, diag)
        ),
        token,
        channel
    );
}

#endif
