//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_IMPL_ROW_VIEW_HPP
#define BOOST_MYSQL_IMPL_ROW_VIEW_HPP

#pragma once

#include <boost/mysql/row_view.hpp>

#include <boost/mysql/detail/auxiliar/access_fwd.hpp>

#include <cstddef>
#include <ostream>
#include <stdexcept>

boost::mysql::field_view boost::mysql::row_view::at(std::size_t i) const
{
    if (i >= size_)
        throw std::out_of_range("row_view::at");
    return fields_[i];
}

inline bool boost::mysql::operator==(const row_view& lhs, const row_view& rhs) noexcept
{
    if (lhs.size() != rhs.size())
        return false;
    for (std::size_t i = 0; i < lhs.size(); ++i)
    {
        if (lhs[i] != rhs[i])
            return false;
    }
    return true;
}

struct boost::mysql::detail::row_view_access
{
    static row_view construct(const field_view* f, std::size_t size) noexcept { return row_view(f, size); }
};

#endif
