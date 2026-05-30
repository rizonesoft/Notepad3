//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZATION_CONTEXT_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZATION_CONTEXT_HPP

#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/deserialize_errc.hpp>

#include <boost/asio/buffer.hpp>

#include <cassert>
#include <cstdint>
#include <cstring>

namespace boost {
namespace mysql {
namespace detail {

class deserialization_context
{
    const std::uint8_t* first_;
    const std::uint8_t* last_;
    capabilities capabilities_;

public:
    deserialization_context(
        const std::uint8_t* first,
        const std::uint8_t* last,
        capabilities caps
    ) noexcept
        : first_(first), last_(last), capabilities_(caps)
    {
        assert(last_ >= first_);
    };
    deserialization_context(boost::asio::const_buffer buff, capabilities caps) noexcept
        : deserialization_context(
              static_cast<const std::uint8_t*>(buff.data()),
              static_cast<const std::uint8_t*>(buff.data()) + buff.size(),
              caps
          ){};
    const std::uint8_t* first() const noexcept { return first_; }
    const std::uint8_t* last() const noexcept { return last_; }
    void set_first(const std::uint8_t* new_first) noexcept
    {
        first_ = new_first;
        assert(last_ >= first_);
    }
    void advance(std::size_t sz) noexcept
    {
        first_ += sz;
        assert(last_ >= first_);
    }
    void rewind(std::size_t sz) noexcept { first_ -= sz; }
    std::size_t size() const noexcept { return last_ - first_; }
    bool empty() const noexcept { return last_ == first_; }
    bool enough_size(std::size_t required_size) const noexcept { return size() >= required_size; }
    capabilities get_capabilities() const noexcept { return capabilities_; }
    deserialize_errc copy(void* to, std::size_t sz) noexcept
    {
        if (!enough_size(sz))
            return deserialize_errc::incomplete_message;
        memcpy(to, first_, sz);
        advance(sz);
        return deserialize_errc::ok;
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZATION_CONTEXT_HPP_ */
