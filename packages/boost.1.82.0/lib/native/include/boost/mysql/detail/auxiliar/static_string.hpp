//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_STATIC_STRING_HPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_STATIC_STRING_HPP

// A very simplified variable-length string with fixed max-size

#include <boost/mysql/string_view.hpp>

#include <array>
#include <cassert>
#include <cstring>
#include <ostream>

namespace boost {
namespace mysql {
namespace detail {

template <std::size_t max_size>
class static_string
{
    std::array<char, max_size> buffer_;
    std::size_t size_;

public:
    static_string() noexcept : size_(0) {}
    static_string(string_view value) noexcept : size_(value.size())
    {
        assert(value.size() <= max_size);
        size_ = value.size();
        std::memcpy(buffer_.data(), value.data(), value.size());
    }
    std::size_t size() const noexcept { return size_; }
    string_view value() const noexcept { return string_view(buffer_.data(), size_); }
    void append(const void* arr, std::size_t sz) noexcept
    {
        std::size_t new_size = size_ + sz;
        assert(new_size <= max_size);
        std::memcpy(buffer_.data() + size_, arr, sz);
        size_ = new_size;
    }
    void clear() noexcept { size_ = 0; }
    bool operator==(const static_string<max_size>& other) const noexcept { return value() == other.value(); }
    bool operator!=(const static_string<max_size>& other) const noexcept { return !(*this == other); }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_AUXILIAR_STATIC_STRING_HPP_ */
