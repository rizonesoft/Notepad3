//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_SERIALIZATION_CONTEXT_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_SERIALIZATION_CONTEXT_HPP

#include <boost/mysql/detail/protocol/capabilities.hpp>

#include <boost/asio/buffer.hpp>

#include <cstdint>
#include <cstring>

namespace boost {
namespace mysql {
namespace detail {

class serialization_context
{
    std::uint8_t* first_;
    capabilities capabilities_;

public:
    serialization_context(capabilities caps, std::uint8_t* first = nullptr) noexcept
        : first_(first), capabilities_(caps){};
    std::uint8_t* first() const noexcept { return first_; }
    void set_first(std::uint8_t* new_first) noexcept { first_ = new_first; }
    void set_first(boost::asio::mutable_buffer buff) noexcept
    {
        first_ = static_cast<std::uint8_t*>(buff.data());
    }
    void advance(std::size_t size) noexcept { first_ += size; }
    capabilities get_capabilities() const noexcept { return capabilities_; }
    void write(const void* buffer, std::size_t size) noexcept
    {
        memcpy(first_, buffer, size);
        advance(size);
    }
    void write(std::uint8_t elm) noexcept
    {
        *first_ = elm;
        ++first_;
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_PROTOCOL_SERIALIZATION_CONTEXT_HPP_ */
