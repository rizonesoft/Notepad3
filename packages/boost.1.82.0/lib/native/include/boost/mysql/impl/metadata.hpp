//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_IMPL_METADATA_HPP
#define BOOST_MYSQL_IMPL_METADATA_HPP

#pragma once

#include <boost/mysql/metadata.hpp>

#include <boost/mysql/detail/auxiliar/access_fwd.hpp>
#include <boost/mysql/detail/protocol/constants.hpp>

namespace boost {
namespace mysql {
namespace detail {

inline column_type compute_field_type_string(std::uint32_t flags, std::uint16_t collation)
{
    if (flags & column_flags::set)
        return column_type::set;
    else if (flags & column_flags::enum_)
        return column_type::enum_;
    else if (collation == binary_collation)
        return column_type::binary;
    else
        return column_type::char_;
}

inline column_type compute_field_type_var_string(std::uint16_t collation)
{
    return collation == binary_collation ? column_type::varbinary : column_type::varchar;
}

inline column_type compute_field_type_blob(std::uint16_t collation)
{
    return collation == binary_collation ? column_type::blob : column_type::text;
}

inline column_type compute_field_type(
    protocol_field_type protocol_type,
    std::uint32_t flags,
    std::uint16_t collation
)
{
    switch (protocol_type)
    {
    case protocol_field_type::decimal:
    case protocol_field_type::newdecimal: return column_type::decimal;
    case protocol_field_type::geometry: return column_type::geometry;
    case protocol_field_type::tiny: return column_type::tinyint;
    case protocol_field_type::short_: return column_type::smallint;
    case protocol_field_type::int24: return column_type::mediumint;
    case protocol_field_type::long_: return column_type::int_;
    case protocol_field_type::longlong: return column_type::bigint;
    case protocol_field_type::float_: return column_type::float_;
    case protocol_field_type::double_: return column_type::double_;
    case protocol_field_type::bit: return column_type::bit;
    case protocol_field_type::date: return column_type::date;
    case protocol_field_type::datetime: return column_type::datetime;
    case protocol_field_type::timestamp: return column_type::timestamp;
    case protocol_field_type::time: return column_type::time;
    case protocol_field_type::year: return column_type::year;
    case protocol_field_type::json: return column_type::json;
    case protocol_field_type::string: return compute_field_type_string(flags, collation);
    case protocol_field_type::var_string: return compute_field_type_var_string(collation);
    case protocol_field_type::blob: return compute_field_type_blob(collation);
    default: return column_type::unknown;
    }
}

}  // namespace detail
}  // namespace mysql
}  // namespace boost

boost::mysql::metadata::metadata(const detail::column_definition_packet& msg, bool copy_strings)
    : schema_(copy_strings ? msg.schema.value : string_view()),
      table_(copy_strings ? msg.table.value : string_view()),
      org_table_(copy_strings ? msg.org_table.value : string_view()),
      name_(copy_strings ? msg.name.value : string_view()),
      org_name_(copy_strings ? msg.org_name.value : string_view()),
      character_set_(msg.character_set),
      column_length_(msg.column_length),
      type_(detail::compute_field_type(msg.type, msg.flags, msg.character_set)),
      flags_(msg.flags),
      decimals_(msg.decimals)
{
}

struct boost::mysql::detail::metadata_access
{
    static metadata construct(const detail::column_definition_packet& msg, bool copy_strings)
    {
        return metadata(msg, copy_strings);
    }
};

#endif
