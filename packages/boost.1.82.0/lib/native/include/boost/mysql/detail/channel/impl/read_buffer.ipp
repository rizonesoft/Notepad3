//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_CHANNEL_IMPL_READ_BUFFER_IPP
#define BOOST_MYSQL_DETAIL_CHANNEL_IMPL_READ_BUFFER_IPP

#pragma once

#include <boost/mysql/detail/channel/read_buffer.hpp>

#include <cassert>
#include <cstring>

boost::mysql::detail::read_buffer::read_buffer(std::size_t size) : buffer_(size, std::uint8_t(0))
{
    buffer_.resize(buffer_.capacity());
}

void boost::mysql::detail::read_buffer::move_to_reserved(std::size_t length) noexcept
{
    assert(length <= current_message_size());
    current_message_offset_ += length;
}

void boost::mysql::detail::read_buffer::remove_current_message_last(std::size_t length) noexcept
{
    assert(length <= current_message_size());
    assert(length > 0);
    std::memmove(pending_first() - length, pending_first(), pending_size());
    pending_offset_ -= length;
    free_offset_ -= length;
}

void boost::mysql::detail::read_buffer::move_to_current_message(std::size_t n) noexcept
{
    assert(n <= pending_size());
    pending_offset_ += n;
}

void boost::mysql::detail::read_buffer::move_to_pending(std::size_t n) noexcept
{
    assert(n <= free_size());
    free_offset_ += n;
}

void boost::mysql::detail::read_buffer::remove_reserved() noexcept
{
    if (reserved_size() > 0)
    {
        std::size_t currmsg_size = current_message_size();
        std::size_t pend_size = pending_size();
        std::memmove(buffer_.data(), current_message_first(), currmsg_size + pend_size);
        current_message_offset_ = 0;
        pending_offset_ = currmsg_size;
        free_offset_ = currmsg_size + pend_size;
    }
}

void boost::mysql::detail::read_buffer::grow_to_fit(std::size_t n)
{
    if (free_size() < n)
    {
        buffer_.resize(buffer_.size() + n - free_size());
        buffer_.resize(buffer_.capacity());
    }
}

#endif
