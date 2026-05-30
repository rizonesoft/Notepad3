//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_AUXILIAR_FIELD_TYPE_TRAITS_HPP
#define BOOST_MYSQL_DETAIL_AUXILIAR_FIELD_TYPE_TRAITS_HPP

#include <boost/mysql/field_view.hpp>

#include <boost/mysql/detail/auxiliar/void_t.hpp>
#include <boost/mysql/detail/config.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

// field_view_forward_iterator
template <typename T, typename = void>
struct is_field_view_forward_iterator : std::false_type
{
};

// clang-format off
template <typename T>
struct is_field_view_forward_iterator<T, void_t<
    typename std::enable_if<
        std::is_convertible<
            typename std::iterator_traits<T>::reference,
            ::boost::mysql::field_view
        >::value
    >::type,
    typename std::enable_if<
        std::is_base_of<
            std::forward_iterator_tag, 
            typename std::iterator_traits<T>::iterator_category
        >::value
    >::type
>> : std::true_type { };
// clang-format on

#ifdef BOOST_MYSQL_HAS_CONCEPTS

template <class T>
concept field_view_forward_iterator = is_field_view_forward_iterator<T>::value;

#define BOOST_MYSQL_FIELD_VIEW_FORWARD_ITERATOR ::boost::mysql::detail::field_view_forward_iterator

#else  // BOOST_MYSQL_HAS_CONCEPTS

#define BOOST_MYSQL_FIELD_VIEW_FORWARD_ITERATOR class

#endif  // BOOST_MYSQL_HAS_CONCEPTS

// field_like
template <class T>
using is_field_like = std::is_constructible<field_view, T>;

#ifdef BOOST_MYSQL_HAS_CONCEPTS

template <class T>
concept field_like = is_field_like<T>::value;

#define BOOST_MYSQL_FIELD_LIKE ::boost::mysql::detail::field_like

#else  // BOOST_MYSQL_HAS_CONCEPTS

#define BOOST_MYSQL_FIELD_LIKE class

#endif  // BOOST_MYSQL_HAS_CONCEPTS

// field_like_tuple
template <class... T>
struct is_field_like_tuple_impl : std::false_type
{
};

template <class... T>
struct is_field_like_tuple_impl<std::tuple<T...>>
    : mp11::mp_all_of<mp11::mp_list<T...>, is_field_like>
{
};

template <class Tuple>
struct is_field_like_tuple : is_field_like_tuple_impl<typename std::decay<Tuple>::type>
{
};

#ifdef BOOST_MYSQL_HAS_CONCEPTS

template <class T>
concept field_like_tuple = is_field_like_tuple<T>::value;

#define BOOST_MYSQL_FIELD_LIKE_TUPLE ::boost::mysql::detail::field_like_tuple

#else  // BOOST_MYSQL_HAS_CONCEPTS

#define BOOST_MYSQL_FIELD_LIKE_TUPLE class

#endif  // BOOST_MYSQL_HAS_CONCEPTS

// Helpers
template <typename T>
using enable_if_field_view_forward_iterator = typename std::enable_if<
    is_field_view_forward_iterator<T>::value>::type;

template <typename T>
using enable_if_field_like_tuple = typename std::enable_if<is_field_like_tuple<T>::value>::type;

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#endif