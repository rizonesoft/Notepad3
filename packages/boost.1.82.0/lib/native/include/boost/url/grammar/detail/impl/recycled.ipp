//
// Copyright (c) 2022 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/url
//

#ifndef BOOST_URL_GRAMMAR_DETAIL_IMPL_RECYCLED_IPP
#define BOOST_URL_GRAMMAR_DETAIL_IMPL_RECYCLED_IPP

#include <cstdlib>
#include <utility>
#include <atomic>

#ifdef BOOST_URL_REPORT
# ifdef _MSC_VER
#  include <intrin.h>
# endif
#endif

namespace boost {
namespace urls {
namespace grammar {
namespace detail {

struct all_reports
{
    // current count
    std::atomic<std::size_t> count = {0};

    // current bytes
    std::atomic<std::size_t> bytes = {0};

    // highest total ptr count
    std::atomic<std::size_t> count_max = {0};

    // highest total bytes
    std::atomic<std::size_t> bytes_max = {0};

    // largest single allocation
    std::atomic<std::size_t> alloc_max = {0};

    ~all_reports()
    {
        // breakpoint here to view report
#ifdef BOOST_URL_REPORT
# ifdef _MSC_VER
        if(count_max > 0)
            ::__debugbreak();
# endif
#endif
    }
};

static all_reports all_reports_;

void
recycled_add_impl(
    std::size_t n) noexcept
{
    auto& a = all_reports_;

    std::size_t new_count = ++a.count;
    std::size_t old_count_max = a.count_max;
    while (
        old_count_max < new_count &&
        !a.count_max.compare_exchange_weak(
            old_count_max, new_count))
    {}

    std::size_t new_bytes = a.bytes.fetch_add(n) + n;
    std::size_t old_bytes_max = a.bytes_max;
    while (
        old_bytes_max < new_bytes &&
        !a.bytes_max.compare_exchange_weak(
            old_bytes_max, new_bytes))
    {}

    std::size_t old_alloc_max = a.alloc_max;
    while (
        old_alloc_max < n &&
        !a.alloc_max.compare_exchange_weak(
            old_alloc_max, n))
    {}
}

void
recycled_remove_impl(
    std::size_t n) noexcept
{
    all_reports_.count--;
    all_reports_.bytes-=n;
}

} // detail
} // grammar
} // urls
} // boost

#endif
