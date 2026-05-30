//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_STATEMENT_HPP
#define BOOST_MYSQL_STATEMENT_HPP

#include <boost/mysql/detail/auxiliar/access_fwd.hpp>

#include <cassert>
#include <cstdint>

namespace boost {
namespace mysql {

/**
 * \brief Represents a server-side prepared statement.
 * \details
 * This is a lightweight class, holding a handle to a server-side prepared statement.
 * \n
 * Note that statement's destructor doesn't deallocate the statement from the
 * server, as this implies a network transfer that may fail.
 *
 * \par Thread safety
 * Distinct objects: safe. \n
 * Shared objects: unsafe. \n
 */
class statement
{
public:
    /**
     * \brief Default constructor.
     * \details Default constructed statements have `this->valid() == false`.
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    statement() = default;

    /**
     * \brief Returns `true` if the object represents an actual server statement.
     * \details Calling any function other than assignment on a statement for which
     * this function returns `false` results in undefined behavior.
     * \n
     * Returns `false` for default-constructed statements.
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    bool valid() const noexcept { return valid_; }

    /**
     * \brief Returns a server-side identifier for the statement (unique in a per-connection basis).
     * \details Note that, once a statement is closed, the server may recycle its ID.
     *
     * \par Preconditions
     * `this->valid() == true`
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    std::uint32_t id() const noexcept
    {
        assert(valid());
        return id_;
    }

    /**
     * \brief Returns the number of parameters that should be provided when executing the statement.
     * \par Preconditions
     * `this->valid() == true`
     *
     * \par Exception safety
     * No-throw guarantee.
     */
    unsigned num_params() const noexcept
    {
        assert(valid());
        return num_params_;
    }

private:
    bool valid_{false};
    std::uint32_t id_{0};
    std::uint16_t num_params_{0};

#ifndef BOOST_MYSQL_DOXYGEN
    friend struct detail::statement_access;
#endif
};

}  // namespace mysql
}  // namespace boost

#include <boost/mysql/impl/statement.hpp>

#endif
