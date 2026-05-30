//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_COMMON_MESSAGES_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_COMMON_MESSAGES_HPP

#include <boost/mysql/detail/protocol/constants.hpp>
#include <boost/mysql/detail/protocol/serialization.hpp>

namespace boost {
namespace mysql {
namespace detail {

// header
struct packet_header
{
    int3 packet_size;
    std::uint8_t sequence_number;

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        std::forward<Callable>(cb)(self.packet_size, self.sequence_number);
    }
};

// ok packet
struct ok_packet
{
    // header: int<1>     header     0x00 or 0xFE the OK packet header
    int_lenenc affected_rows;
    int_lenenc last_insert_id;
    std::uint16_t status_flags;  // server_status_flags
    std::uint16_t warnings;
    // CLIENT_SESSION_TRACK: not implemented
    string_lenenc info;

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        // clang-format off
        std::forward<Callable>(cb)(
            self.affected_rows,
            self.last_insert_id,
            self.status_flags,
            self.warnings,
            self.info
        );
        // clang-format on
    }
};

template <>
struct serialization_traits<ok_packet, serialization_tag::struct_with_fields> : noop_serialize<ok_packet>
{
    static inline deserialize_errc deserialize_(deserialization_context& ctx, ok_packet& output) noexcept;
};

// err packet
struct err_packet
{
    // int<1>     header     0xFF ERR packet header
    std::uint16_t error_code;
    string_fixed<1> sql_state_marker;
    string_fixed<5> sql_state;
    string_eof error_message;

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        // clang-format off
        std::forward<Callable>(cb)(
            self.error_code,
            self.sql_state_marker,
            self.sql_state,
            self.error_message
        );
        // clang-format on
    }
};

// col def
struct column_definition_packet
{
    string_lenenc catalog;        // always "def"
    string_lenenc schema;         // database
    string_lenenc table;          // virtual table
    string_lenenc org_table;      // physical table
    string_lenenc name;           // virtual column name
    string_lenenc org_name;       // physical column name
    std::uint16_t character_set;  // collation id, somehow named character_set in the protocol docs
    std::uint32_t column_length;  // maximum length of the field
    protocol_field_type type;     // type of the column as defined in enum_field_types
    std::uint16_t flags;          // Flags as defined in Column Definition Flags
    std::uint8_t decimals;        // max shown decimal digits. 0x00 for int/static strings; 0x1f for
                                  // dynamic strings, double, float

    template <class Self, class Callable>
    static void apply(Self& self, Callable&& cb)
    {
        // clang-format off
        std::forward<Callable>(cb)(
            self.catalog,
            self.schema,
            self.table,
            self.org_table,
            self.name,
            self.org_name,
            self.character_set,
            self.column_length,
            self.type,
            self.flags,
            self.decimals
        );
        // clang-format on
    }
};

template <>
struct serialization_traits<column_definition_packet, serialization_tag::struct_with_fields>
    : noop_serialize<column_definition_packet>
{
    static inline deserialize_errc deserialize_(
        deserialization_context& ctx,
        column_definition_packet& output
    ) noexcept;
};

// connection quit
struct quit_packet
{
    static constexpr std::uint8_t command_id = 0x01;

    template <class Self, class Callable>
    static void apply(Self&, Callable&&) noexcept
    {
    }
};

// connection ping
struct ping_packet
{
    static constexpr std::uint8_t command_id = 0x0e;

    template <class Self, class Callable>
    static void apply(Self&, Callable&&) noexcept
    {
    }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/protocol/impl/common_messages.ipp>

#endif /* INCLUDE_BOOST_MYSQL_DETAIL_PROTOCOL_COMMON_MESSAGES_HPP_ */
