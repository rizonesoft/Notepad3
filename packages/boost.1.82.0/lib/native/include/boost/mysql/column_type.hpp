//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_COLUMN_TYPE_HPP
#define BOOST_MYSQL_COLUMN_TYPE_HPP

#include <cstdint>
#include <ostream>

namespace boost {
namespace mysql {

/**
 * \brief Represents the database type of a MySQL column.
 * \details This represents a database type, as opposed to \ref field_kind, which represents a
 * C++ type.
 *\n
 * Unless otherwise noted, the names in this enumeration
 * directly correspond to the names of the types you would use in
 * a `CREATE TABLE` statement to create a column of this type
 * (e.g. `VARCHAR` corresponds to \ref column_type::varchar).
 */
enum class column_type
{
    tinyint,    ///< `TINYINT` (signed and unsigned).
    smallint,   ///< `SMALLINT` (signed and unsigned).
    mediumint,  ///< `MEDIUMINT` (signed and unsigned).
    int_,       ///< `INT` (signed and unsigned).
    bigint,     ///< `BIGINT` (signed and unsigned).
    float_,     ///< `FLOAT` (warning: FLOAT(p) where p >= 24 creates a DOUBLE column).
    double_,    ///< `DOUBLE`
    decimal,    ///< `DECIMAL`
    bit,        ///< `BIT`
    year,       ///< `YEAR`
    time,       ///< `TIME`
    date,       ///< `DATE`
    datetime,   ///< `DATETIME`
    timestamp,  ///< `TIMESTAMP`
    char_,      ///< `CHAR` (any length)
    varchar,    ///< `VARCHAR` (any length)
    binary,     ///< `BINARY` (any length)
    varbinary,  ///< `VARBINARY` (any length)
    text,       ///< `TEXT` types (`TINYTEXT`, `MEDIUMTEXT`, `TEXT` and `LONGTEXT`)
    blob,       ///< `BLOB` types (`TINYBLOB`, `MEDIUMBLOB`, `BLOB` and `LONGBLOB`)
    enum_,      ///< `ENUM`
    set,        ///< `SET`
    json,       ///< `JSON`
    geometry,   ///< `GEOMETRY`
    unknown,    ///< None of the known types; maybe a new MySQL type we have no knowledge of.
};

/**
 * \brief Streams a `column_type`.
 */
inline std::ostream& operator<<(std::ostream& os, column_type t)
{
    switch (t)
    {
    case column_type::tinyint: return os << "tinyint";
    case column_type::smallint: return os << "smallint";
    case column_type::mediumint: return os << "mediumint";
    case column_type::int_: return os << "int_";
    case column_type::bigint: return os << "bigint";
    case column_type::float_: return os << "float_";
    case column_type::double_: return os << "double_";
    case column_type::decimal: return os << "decimal";
    case column_type::bit: return os << "bit";
    case column_type::year: return os << "year";
    case column_type::time: return os << "time";
    case column_type::date: return os << "date";
    case column_type::datetime: return os << "datetime";
    case column_type::timestamp: return os << "timestamp";
    case column_type::char_: return os << "char_";
    case column_type::varchar: return os << "varchar";
    case column_type::binary: return os << "binary";
    case column_type::varbinary: return os << "varbinary";
    case column_type::text: return os << "text";
    case column_type::blob: return os << "blob";
    case column_type::enum_: return os << "enum_";
    case column_type::set: return os << "set";
    case column_type::json: return os << "json";
    case column_type::geometry: return os << "geometry";
    default: return os << "<unknown field type>";
    }
}

}  // namespace mysql
}  // namespace boost

#endif
