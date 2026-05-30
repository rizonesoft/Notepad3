//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZE_ROW_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_DESERIALIZE_ROW_HPP

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/execution_state.hpp>
#include <boost/mysql/field_view.hpp>

#include <boost/mysql/detail/auxiliar/access_fwd.hpp>
#include <boost/mysql/detail/protocol/capabilities.hpp>
#include <boost/mysql/detail/protocol/db_flavor.hpp>
#include <boost/mysql/detail/protocol/resultset_encoding.hpp>
#include <boost/mysql/detail/protocol/serialization_context.hpp>

#include <boost/asio/buffer.hpp>

#include <vector>

namespace boost {
namespace mysql {
namespace detail {

// Exposed here for the sake of testing
inline void deserialize_row(
    resultset_encoding encoding,
    deserialization_context& ctx,
    const std::vector<metadata>& meta,
    const std::uint8_t* buffer_first,
    std::vector<field_view>& output,
    error_code& err
);

inline void deserialize_row(
    boost::asio::const_buffer read_message,
    capabilities current_capabilities,
    db_flavor flavor,
    const std::uint8_t* buffer_first,  // to store strings as offsets and allow buffer reallocation
    execution_state& st,               // should be !complete()
    std::vector<field_view>& output,
    error_code& err,
    diagnostics& diag
);

inline void offsets_to_string_views(
    std::vector<field_view>& fields,
    const std::uint8_t* buffer_first
) noexcept
{
    for (auto& f : fields)
        field_view_access::offset_to_string_view(f, buffer_first);
}

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/protocol/impl/deserialize_row.ipp>

#endif
