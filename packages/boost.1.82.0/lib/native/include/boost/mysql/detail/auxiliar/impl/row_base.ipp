//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_IMPL_ROW_BASE_IPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_IMPL_ROW_BASE_IPP

#pragma once

#include <boost/mysql/detail/auxiliar/row_base.hpp>

#include <functional>

namespace boost {
namespace mysql {
namespace detail {

inline bool overlaps(const void* first1, const void* first2, std::size_t size) noexcept
{
    std::greater_equal<const void*> gte;
    std::less<const void*> lt;
    const void* last1 = static_cast<const unsigned char*>(first1) + size;
    const void* last2 = static_cast<const unsigned char*>(first2) + size;
    return (gte(first1, first2) && lt(first1, last2)) || (gte(last1, first2) && lt(last1, last2));
}

inline std::size_t get_string_size(field_view f) noexcept
{
    switch (f.kind())
    {
    case field_kind::string: return f.get_string().size();
    case field_kind::blob: return f.get_blob().size();
    default: return 0;
    }
}

inline unsigned char* copy_string(unsigned char* buffer_it, field_view& f) noexcept
{
    auto str = f.get_string();
    if (!str.empty())
    {
        assert(!overlaps(buffer_it, str.data(), str.size()));
        std::memcpy(buffer_it, str.data(), str.size());
        f = field_view(string_view(reinterpret_cast<const char*>(buffer_it), str.size()));
        buffer_it += str.size();
    }
    return buffer_it;
}

inline unsigned char* copy_blob(unsigned char* buffer_it, field_view& f) noexcept
{
    auto b = f.get_blob();
    if (!b.empty())
    {
        assert(!overlaps(buffer_it, b.data(), b.size()));
        std::memcpy(buffer_it, b.data(), b.size());
        f = field_view(blob_view(buffer_it, b.size()));
        buffer_it += b.size();
    }
    return buffer_it;
}

}  // namespace detail
}  // namespace mysql
}  // namespace boost

boost::mysql::detail::row_base::row_base(const field_view* fields, std::size_t size)
    : fields_(fields, fields + size)
{
    copy_strings();
}

boost::mysql::detail::row_base::row_base(const row_base& rhs) : fields_(rhs.fields_) { copy_strings(); }

boost::mysql::detail::row_base& boost::mysql::detail::row_base::operator=(const row_base& rhs)
{
    if (this != &rhs)
    {
        fields_ = rhs.fields_;
        copy_strings();
    }
    return *this;
}

void boost::mysql::detail::row_base::assign(const field_view* fields, std::size_t size)
{
    fields_.assign(fields, fields + size);
    copy_strings();
}

inline void boost::mysql::detail::row_base::copy_strings()
{
    // Calculate the required size for the new strings
    std::size_t size = 0;
    for (auto f : fields_)
    {
        size += get_string_size(f);
    }

    // Make space
    string_buffer_.resize(size);

    // Copy strings and blobs
    unsigned char* buffer_it = string_buffer_.data();
    for (auto& f : fields_)
    {
        switch (f.kind())
        {
        case field_kind::string: buffer_it = copy_string(buffer_it, f); break;
        case field_kind::blob: buffer_it = copy_blob(buffer_it, f); break;
        default: break;
        }
    }
    assert(buffer_it == string_buffer_.data() + string_buffer_.size());
}

#endif
