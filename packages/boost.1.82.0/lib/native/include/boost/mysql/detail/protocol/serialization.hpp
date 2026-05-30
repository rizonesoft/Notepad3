//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_MYSQL_DETAIL_PROTOCOL_SERIALIZATION_HPP
#define BOOST_MYSQL_DETAIL_PROTOCOL_SERIALIZATION_HPP

#include <boost/mysql/detail/auxiliar/bytestring.hpp>
#include <boost/mysql/detail/protocol/deserialization_context.hpp>
#include <boost/mysql/detail/protocol/deserialize_errc.hpp>
#include <boost/mysql/detail/protocol/protocol_types.hpp>
#include <boost/mysql/detail/protocol/serialization_context.hpp>

#include <boost/endian/conversion.hpp>

#include <algorithm>
#include <type_traits>

namespace boost {
namespace mysql {
namespace detail {

enum class serialization_tag
{
    none,
    plain_int,
    enumeration,
    struct_with_fields
};

template <class T>
constexpr serialization_tag get_serialization_tag();

template <class T, serialization_tag = get_serialization_tag<T>()>
struct serialization_traits;

template <class... Types>
deserialize_errc deserialize(deserialization_context& ctx, Types&... output) noexcept;

template <class... Types>
void serialize(serialization_context& ctx, const Types&... input) noexcept;

template <class... Types>
std::size_t get_size(const serialization_context& ctx, const Types&... input) noexcept;

// Plain integers
template <class T>
struct serialization_traits<T, serialization_tag::plain_int>
{
    static deserialize_errc deserialize_(deserialization_context& ctx, T& output) noexcept;
    static void serialize_(serialization_context& ctx, T input) noexcept;
    static constexpr std::size_t get_size_(const serialization_context&, T) noexcept
    {
        return sizeof(T);
    }
};

// int3
template <>
struct serialization_traits<int3, serialization_tag::none>
{
    static inline deserialize_errc deserialize_(deserialization_context& ctx, int3& output) noexcept
    {
        if (!ctx.enough_size(3))
            return deserialize_errc::incomplete_message;
        output.value = boost::endian::load_little_u24(ctx.first());
        ctx.advance(3);
        return deserialize_errc::ok;
    }
    static inline void serialize_(serialization_context& ctx, int3 input) noexcept
    {
        boost::endian::store_little_u24(ctx.first(), input.value);
        ctx.advance(3);
    }
    static inline std::size_t get_size_(const serialization_context&, int3) noexcept { return 3; }
};

// int_lenenc
template <>
struct serialization_traits<int_lenenc, serialization_tag::none>
{
    static inline deserialize_errc deserialize_(
        deserialization_context& ctx,
        int_lenenc& output
    ) noexcept;
    static inline void serialize_(serialization_context& ctx, int_lenenc input) noexcept;
    static inline std::size_t get_size_(const serialization_context&, int_lenenc input) noexcept;
};

// string_fixed
template <std::size_t N>
struct serialization_traits<string_fixed<N>, serialization_tag::none>
{
    static deserialize_errc deserialize_(
        deserialization_context& ctx,
        string_fixed<N>& output
    ) noexcept
    {
        if (!ctx.enough_size(N))
            return deserialize_errc::incomplete_message;
        memcpy(output.data(), ctx.first(), N);
        ctx.advance(N);
        return deserialize_errc::ok;
    }
    static void serialize_(serialization_context& ctx, const string_fixed<N>& input) noexcept
    {
        ctx.write(input.data(), N);
    }
    static constexpr std::size_t get_size_(const serialization_context&, const string_fixed<N>&) noexcept
    {
        return N;
    }
};

// string_null
template <>
struct serialization_traits<string_null, serialization_tag::none>
{
    static inline deserialize_errc deserialize_(
        deserialization_context& ctx,
        string_null& output
    ) noexcept;
    static inline void serialize_(serialization_context& ctx, string_null input) noexcept
    {
        ctx.write(input.value.data(), input.value.size());
        ctx.write(0);  // null terminator
    }
    static inline std::size_t get_size_(const serialization_context&, string_null input) noexcept
    {
        return input.value.size() + 1;
    }
};

// string_eof
template <>
struct serialization_traits<string_eof, serialization_tag::none>
{
    static inline deserialize_errc deserialize_(
        deserialization_context& ctx,
        string_eof& output
    ) noexcept;
    static inline void serialize_(serialization_context& ctx, string_eof input) noexcept
    {
        ctx.write(input.value.data(), input.value.size());
    }
    static inline std::size_t get_size_(const serialization_context&, string_eof input) noexcept
    {
        return input.value.size();
    }
};

// string_lenenc
template <>
struct serialization_traits<string_lenenc, serialization_tag::none>
{
    static inline deserialize_errc deserialize_(
        deserialization_context& ctx,
        string_lenenc& output
    ) noexcept;
    static inline void serialize_(serialization_context& ctx, string_lenenc input) noexcept
    {
        serialize(ctx, int_lenenc(input.value.size()));
        ctx.write(input.value.data(), input.value.size());
    }
    static inline std::size_t get_size_(
        const serialization_context& ctx,
        string_lenenc input
    ) noexcept
    {
        return get_size(ctx, int_lenenc(input.value.size())) + input.value.size();
    }
};

// Enums
template <class T>
struct serialization_traits<T, serialization_tag::enumeration>
{
    using underlying_type = typename std::underlying_type<T>::type;

    static deserialize_errc deserialize_(deserialization_context& ctx, T& output) noexcept
    {
        underlying_type value = 0;
        auto err = deserialize(ctx, value);
        output = static_cast<T>(value);
        return err;
    }
    static void serialize_(serialization_context& ctx, T input) noexcept
    {
        serialize(ctx, static_cast<underlying_type>(input));
    }
    static std::size_t get_size_(const serialization_context&, T) noexcept
    {
        return sizeof(underlying_type);
    }
};

// Structs and commands
// To allow a limited way of reflection, structs should
// provide a static member function with signature:
// void apply(Self&, Callable&&), which will invoke Callable
// with each field in the struct as argument (similar to std::apply).
// Making the member function static with a Self template parameter
// allows calling apply with const and non-const objects as self.
// Types fulfilling this requirement will have the below function
// return true
template <class T>
constexpr bool is_struct_with_fields();

template <class T>
struct serialization_traits<T, serialization_tag::struct_with_fields>
{
    static deserialize_errc deserialize_(deserialization_context& ctx, T& output) noexcept;
    static void serialize_(serialization_context& ctx, const T& input) noexcept;
    static std::size_t get_size_(const serialization_context& ctx, const T& input) noexcept;
};

// Use these to make all messages implement all methods, leaving the not required
// ones with a default implementation. While this is not strictly required,
// it simplifies the testing infrastructure
template <class T>
struct noop_serialize
{
    static inline std::size_t get_size_(const serialization_context&, const T&) noexcept
    {
        return 0;
    }
    static inline void serialize_(serialization_context&, const T&) noexcept {}
};

template <class T>
struct noop_deserialize
{
    static inline deserialize_errc deserialize_(deserialization_context&, T&) noexcept
    {
        return deserialize_errc::ok;
    }
};

// Helper to serialize top-level messages
template <class Serializable, class Allocator>
void serialize_message(
    const Serializable& input,
    capabilities caps,
    basic_bytestring<Allocator>& buffer
);

template <class Deserializable>
error_code deserialize_message(deserialization_context& ctx, Deserializable& output);

template <class Deserializable>
error_code deserialize_message_part(deserialization_context& ctx, Deserializable& output);

// Helpers for (de) serializing a set of fields
template <class... Types>
void serialize_fields(serialization_context& ctx, const Types&... fields) noexcept;

}  // namespace detail
}  // namespace mysql
}  // namespace boost

#include <boost/mysql/detail/protocol/impl/serialization.hpp>

#include <boost/mysql/detail/protocol/impl/serialization.ipp>

#endif
