//
// experimental/detail/channel_payload.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_EXPERIMENTAL_DETAIL_CHANNEL_PAYLOAD_HPP
#define BOOST_ASIO_EXPERIMENTAL_DETAIL_CHANNEL_PAYLOAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>
#include <boost/asio/detail/type_traits.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/experimental/detail/channel_message.hpp>

#if defined(BOOST_ASIO_HAS_STD_VARIANT)
# include <variant>
#endif // defined(BOOST_ASIO_HAS_STD_VARIANT)

#include <boost/asio/detail/push_options.hpp>

namespace boost {
namespace asio {
namespace experimental {
namespace detail {

template <typename... Signatures>
class channel_payload;

template <typename R>
class channel_payload<R()>
{
public:
  explicit channel_payload(channel_message<R()>)
  {
  }

  template <typename Handler>
  void receive(Handler& handler)
  {
    BOOST_ASIO_MOVE_OR_LVALUE(Handler)(handler)();
  }
};

template <typename Signature>
class channel_payload<Signature>
{
public:
  channel_payload(BOOST_ASIO_MOVE_ARG(channel_message<Signature>) m)
    : message_(BOOST_ASIO_MOVE_CAST(channel_message<Signature>)(m))
  {
  }

  template <typename Handler>
  void receive(Handler& handler)
  {
    message_.receive(handler);
  }

private:
  channel_message<Signature> message_;
};

#if defined(BOOST_ASIO_HAS_STD_VARIANT)

template <typename... Signatures>
class channel_payload
{
public:
  template <typename Signature>
  channel_payload(BOOST_ASIO_MOVE_ARG(channel_message<Signature>) m)
    : message_(BOOST_ASIO_MOVE_CAST(channel_message<Signature>)(m))
  {
  }

  template <typename Handler>
  void receive(Handler& handler)
  {
    std::visit(
        [&](auto& message)
        {
          message.receive(handler);
        }, message_);
  }

private:
  std::variant<channel_message<Signatures>...> message_;
};

#else // defined(BOOST_ASIO_HAS_STD_VARIANT)

template <typename R1, typename R2>
class channel_payload<R1(), R2(boost::system::error_code)>
{
public:
  typedef channel_message<R1()> void_message_type;
  typedef channel_message<R2(boost::system::error_code)> error_message_type;

  channel_payload(BOOST_ASIO_MOVE_ARG(void_message_type))
    : message_(0, boost::system::error_code()),
      empty_(true)
  {
  }

  channel_payload(BOOST_ASIO_MOVE_ARG(error_message_type) m)
    : message_(BOOST_ASIO_MOVE_CAST(error_message_type)(m)),
      empty_(false)
  {
  }

  template <typename Handler>
  void receive(Handler& handler)
  {
    if (empty_)
      channel_message<R1()>(0).receive(handler);
    else
      message_.receive(handler);
  }

private:
  error_message_type message_;
  bool empty_;
};

#endif // defined(BOOST_ASIO_HAS_STD_VARIANT)

} // namespace detail
} // namespace experimental
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_EXPERIMENTAL_DETAIL_CHANNEL_PAYLOAD_HPP
