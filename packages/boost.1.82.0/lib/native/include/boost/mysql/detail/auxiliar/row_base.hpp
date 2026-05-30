//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_ROW_BASE_HPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_ROW_BASE_HPP

#include <boost/mysql/field_view.hpp>

#include <cstddef>
#include <vector>

namespace boost {
namespace mysql {
namespace detail {

// Base class providing implementation helpers for row and rows.
// Models a field_view vector with strings pointing into a
// single character buffer.
class row_base
{
public:
    row_base() = default;
    inline row_base(const field_view* fields, std::size_t size);
    inline row_base(const row_base&);
    row_base(row_base&&) = default;
    inline row_base& operator=(const row_base&);
    row_base& operator=(row_base&&) = default;
    ~row_base() = default;

    inline void assign(const field_view* fields, std::size_t size);
    inline void copy_strings();
    inline void clear() noexcept
    {
        fields_.clear();
        string_buffer_.clear();
    }

protected:
    std::vector<field_view> fields_;

private:
    std::vector<unsigned char> string_buffer_;
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/auxiliar/impl/row_base.ipp>

#endif
