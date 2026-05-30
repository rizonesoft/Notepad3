//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_CONSTANTS_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_CONSTANTS_HPP

#include <boost/mysql/detail/auxiliar/make_string_view.hpp>
#include <boost/mysql/detail/protocol/protocol_types.hpp>

#include <cstddef>

namespace boost {
namespace mysql {
namespace detail {

enum class protocol_field_type : std::uint8_t
{
    decimal = 0x00,      // Apparently not sent
    tiny = 0x01,         // TINYINT
    short_ = 0x02,       // SMALLINT
    long_ = 0x03,        // INT
    float_ = 0x04,       // FLOAT
    double_ = 0x05,      // DOUBLE
    null = 0x06,         // Apparently not sent
    timestamp = 0x07,    // TIMESTAMP
    longlong = 0x08,     // BIGINT
    int24 = 0x09,        // MEDIUMINT
    date = 0x0a,         // DATE
    time = 0x0b,         // TIME
    datetime = 0x0c,     // DATETIME
    year = 0x0d,         // YEAR
    varchar = 0x0f,      // Apparently not sent
    bit = 0x10,          // BIT
    json = 0xf5,         // JSON
    newdecimal = 0xf6,   // DECIMAL
    enum_ = 0xf7,        // Apparently not sent
    set = 0xf8,          // Apperently not sent
    tiny_blob = 0xf9,    // Apparently not sent
    medium_blob = 0xfa,  // Apparently not sent
    long_blob = 0xfb,    // Apparently not sent
    blob = 0xfc,         // Used for all TEXT and BLOB types
    var_string = 0xfd,   // Used for VARCHAR and VARBINARY
    string = 0xfe,       // Used for CHAR and BINARY, ENUM (enum flag set), SET (set flag set)
    geometry = 0xff      // GEOMETRY
};

constexpr std::size_t MAX_PACKET_SIZE = 0xffffff;
constexpr std::size_t HEADER_SIZE = 4;

// Server status flags
constexpr std::uint32_t SERVER_STATUS_IN_TRANS = 1;
constexpr std::uint32_t SERVER_STATUS_AUTOCOMMIT = 2;
constexpr std::uint32_t SERVER_MORE_RESULTS_EXISTS = 8;
constexpr std::uint32_t SERVER_QUERY_NO_GOOD_INDEX_USED = 16;
constexpr std::uint32_t SERVER_QUERY_NO_INDEX_USED = 32;
constexpr std::uint32_t SERVER_STATUS_CURSOR_EXISTS = 64;
constexpr std::uint32_t SERVER_STATUS_LAST_ROW_SENT = 128;
constexpr std::uint32_t SERVER_STATUS_DB_DROPPED = 256;
constexpr std::uint32_t SERVER_STATUS_NO_BACKSLASH_ESCAPES = 512;
constexpr std::uint32_t SERVER_STATUS_METADATA_CHANGED = 1024;
constexpr std::uint32_t SERVER_QUERY_WAS_SLOW = 2048;
constexpr std::uint32_t SERVER_PS_OUT_PARAMS = 4096;
constexpr std::uint32_t SERVER_STATUS_IN_TRANS_READONLY = 8192;
constexpr std::uint32_t SERVER_SESSION_STATE_CHANGED = (1UL << 14);

// Packet type constants
constexpr std::uint8_t handshake_protocol_version_9 = 9;
constexpr std::uint8_t handshake_protocol_version_10 = 10;
constexpr std::uint8_t error_packet_header = 0xff;
constexpr std::uint8_t ok_packet_header = 0x00;
constexpr std::uint8_t eof_packet_header = 0xfe;
constexpr std::uint8_t auth_switch_request_header = 0xfe;
constexpr std::uint8_t auth_more_data_header = 0x01;
constexpr string_view fast_auth_complete_challenge = make_string_view("\3");

// The binary collation number, used to distinguish blobs from strings
constexpr std::uint16_t binary_collation = 63;

// Column flags
namespace column_flags {

constexpr std::uint16_t not_null = 1;             // Field can't be NULL.
constexpr std::uint16_t pri_key = 2;              // Field is part of a primary key.
constexpr std::uint16_t unique_key = 4;           // Field is part of a unique key.
constexpr std::uint16_t multiple_key = 8;         // Field is part of a key.
constexpr std::uint16_t blob = 16;                // Field is a blob.
constexpr std::uint16_t unsigned_ = 32;           // Field is unsigned.
constexpr std::uint16_t zerofill = 64;            // Field is zerofill.
constexpr std::uint16_t binary = 128;             // Field is binary.
constexpr std::uint16_t enum_ = 256;              // field is an enum
constexpr std::uint16_t auto_increment = 512;     // field is a autoincrement field
constexpr std::uint16_t timestamp = 1024;         // Field is a timestamp.
constexpr std::uint16_t set = 2048;               // field is a set
constexpr std::uint16_t no_default_value = 4096;  // Field doesn't have default value.
constexpr std::uint16_t on_update_now = 8192;     // Field is set to NOW on UPDATE.
constexpr std::uint16_t part_key = 16384;         // Intern; Part of some key.
constexpr std::uint16_t num = 32768;              // Field is num (for clients)

}  // namespace column_flags

// Prepared statements
namespace cursor_types {

constexpr std::uint8_t no_cursor = 0;
constexpr std::uint8_t read_only = 1;
constexpr std::uint8_t for_update = 2;
constexpr std::uint8_t scrollable = 4;

}  // namespace cursor_types

// Text protocol deserialization constants
namespace textc {

constexpr unsigned max_decimals = 6u;

constexpr std::size_t year_sz = 4;
constexpr std::size_t month_sz = 2;
constexpr std::size_t day_sz = 2;
constexpr std::size_t hours_min_sz = 2;  // in TIME, it may be longer
constexpr std::size_t mins_sz = 2;
constexpr std::size_t secs_sz = 2;

constexpr std::size_t date_sz = year_sz + month_sz + day_sz + 2;           // delimiters
constexpr std::size_t time_min_sz = hours_min_sz + mins_sz + secs_sz + 2;  // delimiters
constexpr std::size_t time_max_sz = time_min_sz + max_decimals + 3;  // sign, period, hour extra character
constexpr std::size_t datetime_min_sz = date_sz + time_min_sz + 1;   // delimiter
constexpr std::size_t datetime_max_sz = datetime_min_sz + max_decimals + 1;  // period

constexpr unsigned time_max_hour = 838;

}  // namespace textc

// Binary protocol (de)serialization constants
namespace binc {

constexpr std::size_t length_sz = 1;  // length byte, for date, datetime and time
constexpr std::size_t year_sz = 2;
constexpr std::size_t month_sz = 1;
constexpr std::size_t date_day_sz = 1;
constexpr std::size_t time_days_sz = 4;
constexpr std::size_t hours_sz = 1;
constexpr std::size_t mins_sz = 1;
constexpr std::size_t secs_sz = 1;
constexpr std::size_t micros_sz = 4;
constexpr std::size_t time_sign_sz = 1;

constexpr std::size_t date_sz = year_sz + month_sz + date_day_sz;  // does not include length

constexpr std::size_t datetime_d_sz = date_sz;
constexpr std::size_t datetime_dhms_sz = datetime_d_sz + hours_sz + mins_sz + secs_sz;
constexpr std::size_t datetime_dhmsu_sz = datetime_dhms_sz + micros_sz;

constexpr std::size_t time_dhms_sz = time_sign_sz + time_days_sz + hours_sz + mins_sz + secs_sz;
constexpr std::size_t time_dhmsu_sz = time_dhms_sz + micros_sz;

constexpr std::size_t time_max_days = 34;  // equivalent to the 839 hours, in the broken format

}  // namespace binc

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif
