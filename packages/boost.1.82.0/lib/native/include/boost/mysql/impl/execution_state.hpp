//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_IMPL_EXECUTION_STATE_HPP
#define BOOST_MYSQL_IMPL_EXECUTION_STATE_HPP

#pragma once

#include <boost/mysql/execution_state.hpp>
#include <boost/mysql/metadata.hpp>
#include <boost/mysql/metadata_mode.hpp>

#include <boost/mysql/detail/auxiliar/access_fwd.hpp>

struct boost::mysql::detail::execution_state_access
{
    static void reset(execution_state& st, detail::resultset_encoding encoding) noexcept
    {
        st.seqnum_ = 0;
        st.encoding_ = encoding;
        st.meta_.clear();
        st.eof_received_ = false;
    }

    static void complete(execution_state& st, const detail::ok_packet& pack)
    {
        st.affected_rows_ = pack.affected_rows.value;
        st.last_insert_id_ = pack.last_insert_id.value;
        st.warnings_ = pack.warnings;
        st.info_.assign(pack.info.value.begin(), pack.info.value.end());
        st.eof_received_ = true;
    }

    static void prepare_meta(execution_state& st, std::size_t num_fields) { st.meta_.reserve(num_fields); }

    static void add_meta(
        execution_state& st,
        const detail::column_definition_packet& pack,
        metadata_mode mode
    )
    {
        st.meta_.push_back(metadata_access::construct(pack, mode == metadata_mode::full));
    }

    static resultset_encoding get_encoding(const execution_state& st) noexcept { return st.encoding_; }

    static std::uint8_t& get_sequence_number(execution_state& st) noexcept { return st.seqnum_; }

    static std::vector<metadata>& get_metadata(execution_state& st) noexcept { return st.meta_; }
};

#endif
