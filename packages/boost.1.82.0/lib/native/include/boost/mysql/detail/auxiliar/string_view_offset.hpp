//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_STRING_VIEW_OFFSET_HPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_STRING_VIEW_OFFSET_HPP

#include <boost/mysql/string_view.hpp>

#include <cstddef>
#include <cstdint>

namespace boost {
namespace mysql {
namespace detail {

// Represents a string_view using offsets into a buffer.
// Useful during deserialization, for buffers that may reallocate.
class string_view_offset
{
    std::size_t offset_;
    std::size_t size_;

public:
    constexpr string_view_offset() noexcept : offset_(0), size_(0) {}
    constexpr string_view_offset(std::size_t offset, std::size_t size) noexcept
        : offset_(offset), size_(size)
    {
    }
    constexpr std::size_t offset() const noexcept { return offset_; }
    constexpr std::size_t size() const noexcept { return size_; }
    constexpr bool operator==(string_view_offset rhs) const noexcept
    {
        return offset_ == rhs.offset_ && size_ == rhs.size_;
    }
    constexpr bool operator!=(string_view_offset rhs) const noexcept { return !(*this == rhs); }

    static string_view_offset from_sv(string_view from, const std::uint8_t* buffer_first) noexcept
    {
        return string_view_offset(
            from.data() - reinterpret_cast<const char*>(buffer_first),
            from.size()
        );
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif
