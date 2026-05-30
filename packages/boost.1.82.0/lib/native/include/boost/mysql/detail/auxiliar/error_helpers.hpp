//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_ERROR_HELPERS_HPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_ERROR_HELPERS_HPP

#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>

#include <boost/throw_exception.hpp>

namespace boost {
namespace mysql {
namespace detail {

inline void throw_on_error_loc(
    error_code err,
    const diagnostics& diag,
    const boost::source_location& loc

)
{
    if (err)
    {
        ::boost::throw_exception(error_with_diagnostics(err, diag), loc);
    }
}

inline void clear_errors(error_code& err, diagnostics& diag) noexcept
{
    err.clear();
    diag.clear();
}

// Helper to implement sync with exceptions functions
struct error_block
{
    error_code err;
    diagnostics diag;

    void check(const boost::source_location& loc) { throw_on_error_loc(err, diag, loc); }
};

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif
