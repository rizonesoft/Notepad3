//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_CHANNEL_MESSAGE_WRITER_PROCESSOR_HPP
#define BOOST_MYSQL_DETAIL_CHANNEL_MESSAGE_WRITER_PROCESSOR_HPP

#include <boost/mysql/detail/protocol/common_messages.hpp>
#include <boost/mysql/detail/protocol/constants.hpp>

#include <boost/asio/buffer.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace boost {
namespace mysql {
namespace detail {

class message_writer_processor
{
    boost::asio::const_buffer buffer_to_write_;
    std::uint8_t* seqnum_{nullptr};
    std::size_t bytes_written_{0};
    std::array<std::uint8_t, HEADER_SIZE> header_buffer_{};
    bool send_empty_frame_{false};  // Should we send a last, empty frame? (for empty messages or
                                    // messages of max_frame_size_)
    std::size_t max_frame_size_;

    inline std::uint32_t compute_size_to_write() const
    {
        return static_cast<std::uint32_t>((std::min
        )(max_frame_size_, buffer_to_write_.size() - bytes_written_));
    }

    inline void process_header_write(std::uint32_t size_to_write, std::uint8_t seqnum)
    {
        packet_header header;
        header.packet_size.value = size_to_write;
        header.sequence_number = seqnum;
        serialization_context ctx(
            capabilities(0),
            header_buffer_.data()
        );  // capabilities not relevant here
        serialize(ctx, header);
    }

public:
    // We write two buffers at once: the header and the body part
    using buffers_type = std::array<boost::asio::const_buffer, 2>;

    message_writer_processor(std::size_t max_frame_size = MAX_PACKET_SIZE) noexcept
        : max_frame_size_(max_frame_size)
    {
    }

    void reset(boost::asio::const_buffer buffer, std::uint8_t& seqnum)
    {
        buffer_to_write_ = buffer;
        seqnum_ = &seqnum;
        bytes_written_ = 0;
        send_empty_frame_ = (buffer.size() == 0);  // If the packet is empty, we should just send
                                                   // the header
    }

    buffers_type prepare_next_chunk() noexcept
    {
        auto first = static_cast<const std::uint8_t*>(buffer_to_write_.data());
        auto size_to_write = compute_size_to_write();
        process_header_write(size_to_write, (*seqnum_)++);
        return {
            {boost::asio::buffer(header_buffer_),
             boost::asio::buffer(first + bytes_written_, size_to_write)}
        };
    }

    void on_bytes_written() noexcept
    {
        std::size_t new_bytes = compute_size_to_write();
        bytes_written_ += new_bytes;
        // If we sent all the bytes, but the last frame was just max_frame_size_ bytes, the protocol
        // requires us to send a last, empty frame
        send_empty_frame_ =
            (new_bytes == max_frame_size_ && bytes_written_ == buffer_to_write_.size());
    }

    bool is_complete() const noexcept
    {
        return !send_empty_frame_ && bytes_written_ >= buffer_to_write_.size();
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif
