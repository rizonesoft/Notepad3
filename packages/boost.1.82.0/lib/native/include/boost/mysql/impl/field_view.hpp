//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_IMPL_FIELD_VIEW_HPP
#define BOOST_MYSQL_IMPL_FIELD_VIEW_HPP

#pragma once

#include <boost/mysql/bad_field_access.hpp>
#include <boost/mysql/blob.hpp>
#include <boost/mysql/blob_view.hpp>
#include <boost/mysql/field_kind.hpp>
#include <boost/mysql/field_view.hpp>

#include <boost/mysql/detail/auxiliar/access_fwd.hpp>

#include <cstdio>
#include <cstring>
#include <limits>
#include <ostream>

namespace boost {
namespace mysql {
namespace detail {

inline std::ostream& print_blob(std::ostream& os, blob_view value)
{
    if (value.empty())
        return os << "{}";

    char buffer[16]{};

    os << "{ ";
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        if (i != 0)
            os << ", ";
        unsigned byte = value[i];
        std::snprintf(buffer, sizeof(buffer), "0x%02x", byte);
        os << buffer;
    }
    os << " }";
    return os;
}

inline std::ostream& print_time(std::ostream& os, const time& value)
{
    // Worst-case output is 26 chars, extra space just in case
    char buffer[64]{};

    using namespace std::chrono;
    const char* sign = value < microseconds(0) ? "-" : "";
    auto num_micros = value % seconds(1);
    auto num_secs = duration_cast<seconds>(value % minutes(1) - num_micros);
    auto num_mins = duration_cast<minutes>(value % hours(1) - num_secs);
    auto num_hours = duration_cast<hours>(value - num_mins);

    snprintf(
        buffer,
        sizeof(buffer),
        "%s%02d:%02u:%02u.%06u",
        sign,
        static_cast<int>(std::abs(num_hours.count())),
        static_cast<unsigned>(std::abs(num_mins.count())),
        static_cast<unsigned>(std::abs(num_secs.count())),
        static_cast<unsigned>(std::abs(num_micros.count()))
    );

    os << buffer;
    return os;
}

inline bool blobs_equal(blob_view b1, blob_view b2)
{
    if (b1.size() != b2.size())
        return false;
    return b1.empty() || std::memcmp(b1.data(), b2.data(), b2.size()) == 0;
}

}  // namespace detail
}  // namespace mysql
}  // namespace boost

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(signed char v) noexcept
    : ikind_(internal_kind::int64), repr_(std::int64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(short v) noexcept
    : ikind_(internal_kind::int64), repr_(std::int64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(int v) noexcept
    : ikind_(internal_kind::int64), repr_(std::int64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(long v) noexcept
    : ikind_(internal_kind::int64), repr_(std::int64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(long long v) noexcept
    : ikind_(internal_kind::int64), repr_(std::int64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(unsigned char v) noexcept
    : ikind_(internal_kind::uint64), repr_(std::uint64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(unsigned short v) noexcept
    : ikind_(internal_kind::uint64), repr_(std::uint64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(unsigned int v) noexcept
    : ikind_(internal_kind::uint64), repr_(std::uint64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(unsigned long v) noexcept
    : ikind_(internal_kind::uint64), repr_(std::uint64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(unsigned long long v) noexcept
    : ikind_(internal_kind::uint64), repr_(std::uint64_t(v))
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(string_view v) noexcept
    : ikind_(internal_kind::string), repr_(v)
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(blob_view v) noexcept
    : ikind_(internal_kind::blob), repr_(v)
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(float v) noexcept
    : ikind_(internal_kind::float_), repr_(v)
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(double v) noexcept
    : ikind_(internal_kind::double_), repr_(v)
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(const date& v) noexcept
    : ikind_(internal_kind::date), repr_(v)
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(const datetime& v) noexcept
    : ikind_(internal_kind::datetime), repr_(v)
{
}

BOOST_CXX14_CONSTEXPR boost::mysql::field_view::field_view(const time& v) noexcept
    : ikind_(internal_kind::time), repr_(v)
{
}

BOOST_CXX14_CONSTEXPR inline boost::mysql::field_kind boost::mysql::field_view::kind() const noexcept
{
    switch (ikind_)
    {
    case internal_kind::null: return field_kind::null;
    case internal_kind::int64: return field_kind::int64;
    case internal_kind::uint64: return field_kind::uint64;
    case internal_kind::string: return field_kind::string;
    case internal_kind::blob: return field_kind::blob;
    case internal_kind::float_: return field_kind::float_;
    case internal_kind::double_: return field_kind::double_;
    case internal_kind::date: return field_kind::date;
    case internal_kind::datetime: return field_kind::datetime;
    case internal_kind::time: return field_kind::time;
    case internal_kind::field_ptr: return repr_.field_ptr->kind();
    // sv_offset values must be converted via offset_to_string_view before calling any other fn
    default: return field_kind::null;
    }
}

BOOST_CXX14_CONSTEXPR std::int64_t boost::mysql::field_view::as_int64() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<std::int64_t>();
    check_kind(internal_kind::int64);
    return repr_.int64;
}

BOOST_CXX14_CONSTEXPR std::uint64_t boost::mysql::field_view::as_uint64() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<std::uint64_t>();
    check_kind(internal_kind::uint64);
    return repr_.uint64;
}

BOOST_CXX14_CONSTEXPR boost::mysql::string_view boost::mysql::field_view::as_string() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<std::string>();
    check_kind(internal_kind::string);
    return repr_.string;
}

BOOST_CXX14_CONSTEXPR boost::mysql::blob_view boost::mysql::field_view::as_blob() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<blob>();
    check_kind(internal_kind::blob);
    return repr_.blob;
}

BOOST_CXX14_CONSTEXPR float boost::mysql::field_view::as_float() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<float>();
    check_kind(internal_kind::float_);
    return repr_.float_;
}

BOOST_CXX14_CONSTEXPR double boost::mysql::field_view::as_double() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<double>();
    check_kind(internal_kind::double_);
    return repr_.double_;
}

BOOST_CXX14_CONSTEXPR boost::mysql::date boost::mysql::field_view::as_date() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<date>();
    check_kind(internal_kind::date);
    return repr_.date_;
}

BOOST_CXX14_CONSTEXPR boost::mysql::datetime boost::mysql::field_view::as_datetime() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<datetime>();
    check_kind(internal_kind::datetime);
    return repr_.datetime_;
}

BOOST_CXX14_CONSTEXPR boost::mysql::time boost::mysql::field_view::as_time() const
{
    if (is_field_ptr())
        return repr_.field_ptr->as<time>();
    check_kind(internal_kind::time);
    return repr_.time_;
}

BOOST_CXX14_CONSTEXPR std::int64_t boost::mysql::field_view::get_int64() const noexcept
{
    return is_field_ptr() ? repr_.field_ptr->get<std::int64_t>() : repr_.int64;
}

BOOST_CXX14_CONSTEXPR std::uint64_t boost::mysql::field_view::get_uint64() const noexcept
{
    return is_field_ptr() ? repr_.field_ptr->get<std::uint64_t>() : repr_.uint64;
}

BOOST_CXX14_CONSTEXPR boost::mysql::string_view boost::mysql::field_view::get_string() const noexcept
{
    return is_field_ptr() ? string_view(repr_.field_ptr->get<std::string>()) : repr_.string;
}

BOOST_CXX14_CONSTEXPR boost::mysql::blob_view boost::mysql::field_view::get_blob() const noexcept
{
    return is_field_ptr() ? repr_.field_ptr->get<blob>() : repr_.blob;
}

BOOST_CXX14_CONSTEXPR float boost::mysql::field_view::get_float() const noexcept
{
    return is_field_ptr() ? repr_.field_ptr->get<float>() : repr_.float_;
}

BOOST_CXX14_CONSTEXPR double boost::mysql::field_view::get_double() const noexcept
{
    return is_field_ptr() ? repr_.field_ptr->get<double>() : repr_.double_;
}

BOOST_CXX14_CONSTEXPR boost::mysql::date boost::mysql::field_view::get_date() const noexcept
{
    return is_field_ptr() ? repr_.field_ptr->get<date>() : repr_.date_;
}

BOOST_CXX14_CONSTEXPR boost::mysql::datetime boost::mysql::field_view::get_datetime() const noexcept
{
    return is_field_ptr() ? repr_.field_ptr->get<datetime>() : repr_.datetime_;
}

BOOST_CXX14_CONSTEXPR boost::mysql::time boost::mysql::field_view::get_time() const noexcept
{
    return is_field_ptr() ? repr_.field_ptr->get<time>() : repr_.time_;
}

BOOST_CXX14_CONSTEXPR void boost::mysql::field_view::check_kind(internal_kind expected) const
{
    if (ikind_ != expected)
        throw bad_field_access();
}

BOOST_CXX14_CONSTEXPR bool boost::mysql::field_view::operator==(const field_view& rhs) const noexcept
{
    // Make operator== work for types not representable by field_kind
    if (ikind_ == internal_kind::sv_offset_string || ikind_ == internal_kind::sv_offset_blob)
    {
        return rhs.ikind_ == ikind_ && repr_.sv_offset_ == rhs.repr_.sv_offset_;
    }

    auto k = kind(), rhs_k = rhs.kind();
    switch (k)
    {
    case field_kind::null: return rhs_k == field_kind::null;
    case field_kind::int64:
        if (rhs_k == field_kind::int64)
            return get_int64() == rhs.get_int64();
        else if (rhs_k == field_kind::uint64)
        {
            std::int64_t this_val = get_int64();
            if (this_val < 0)
                return false;
            else
                return static_cast<std::uint64_t>(this_val) == rhs.get_uint64();
        }
        else
            return false;
    case field_kind::uint64:
        if (rhs_k == field_kind::uint64)
            return get_uint64() == rhs.get_uint64();
        else if (rhs_k == field_kind::int64)
        {
            std::int64_t rhs_val = rhs.get_int64();
            if (rhs_val < 0)
                return false;
            else
                return static_cast<std::uint64_t>(rhs_val) == get_uint64();
        }
        else
            return false;
    case field_kind::string: return rhs_k == field_kind::string && get_string() == rhs.get_string();
    case field_kind::blob:
        return rhs_k == field_kind::blob && detail::blobs_equal(get_blob(), rhs.get_blob());
    case field_kind::float_: return rhs_k == field_kind::float_ && get_float() == rhs.get_float();
    case field_kind::double_: return rhs_k == field_kind::double_ && get_double() == rhs.get_double();
    case field_kind::date: return rhs_k == field_kind::date && get_date() == rhs.get_date();
    case field_kind::datetime: return rhs_k == field_kind::datetime && get_datetime() == rhs.get_datetime();
    case field_kind::time: return rhs_k == field_kind::time && get_time() == rhs.get_time();
    default: assert(false); return false;
    }
}

inline std::ostream& boost::mysql::operator<<(std::ostream& os, const field_view& value)
{
    // Make operator<< work for detail::string_view_offset types
    if (value.ikind_ == field_view::internal_kind::sv_offset_string ||
        value.ikind_ == field_view::internal_kind::sv_offset_blob)
    {
        return os << "<sv_offset>";
    }

    switch (value.kind())
    {
    case field_kind::null: return os << "<NULL>";
    case field_kind::int64: return os << value.get_int64();
    case field_kind::uint64: return os << value.get_uint64();
    case field_kind::string: return os << value.get_string();
    case field_kind::blob: return detail::print_blob(os, value.get_blob());
    case field_kind::float_: return os << value.get_float();
    case field_kind::double_: return os << value.get_double();
    case field_kind::date: return os << value.get_date();
    case field_kind::datetime: return os << value.get_datetime();
    case field_kind::time: return detail::print_time(os, value.get_time());
    default: assert(false); return os;
    }
}

struct boost::mysql::detail::field_view_access
{
    static field_view construct(detail::string_view_offset v, bool is_blob) noexcept
    {
        return field_view(v, is_blob);
    }

    static void offset_to_string_view(field_view& fv, const std::uint8_t* buffer_first) noexcept
    {
        if (fv.ikind_ == field_view::internal_kind::sv_offset_string)
        {
            fv.ikind_ = field_view::internal_kind::string;
            fv.repr_.string = {
                reinterpret_cast<const char*>(buffer_first) + fv.repr_.sv_offset_.offset(),
                fv.repr_.sv_offset_.size()};
        }
        else if (fv.ikind_ == field_view::internal_kind::sv_offset_blob)
        {
            fv.ikind_ = field_view::internal_kind::blob;
            fv.repr_.blob = blob_view(
                buffer_first + fv.repr_.sv_offset_.offset(),
                fv.repr_.sv_offset_.size()
            );
        }
    }
};

#endif
