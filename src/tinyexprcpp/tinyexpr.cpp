// SPDX-License-Identifier: Zlib
/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015-2020 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * TINYEXPR++ - Tiny recursive descent parser and evaluation engine in C++
 *
 * Copyright (c) 2020-2026 Blake Madden
 *
 * C++ version of the TinyExpr library.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "tinyexpr.h"

#pragma warning(push)
#pragma warning(disable : 4702) // unreachable code (benign in template-heavy visitor patterns)

// NOLINTBEGIN(readability-redundant-casting,readability-avoid-nested-conditional-operator,hicpp-named-parameter,readability-named-parameter)

// builtin functions
namespace te_builtins
    {
    [[nodiscard]]
    constexpr static te_type te_false_value() noexcept
        {
        return 0;
        }

    [[nodiscard]]
    constexpr static te_type te_true_value() noexcept
        {
        return 1;
        }

    [[nodiscard]]
    constexpr static te_type te_nan_value() noexcept
        {
        return te_parser::te_nan;
        }

    [[nodiscard]]
    static te_type te_max_integer() noexcept
        {
        return te_parser::get_max_integer();
        }

    [[nodiscard]]
    static te_type te_is_nan(te_type val)
        {
        return (!std::isfinite(val) ? 1 : 0);
        }

    [[nodiscard]]
    static te_type te_even(te_type val)
        {
        if (!std::isfinite(val))
            {
            return te_parser::te_nan;
            }
        int64_t rounded{ static_cast<int64_t>(std::ceil(std::abs(val))) };
        if ((rounded % 2) != 0)
            {
            ++rounded;
            }
        return (val < 0 ? -(static_cast<te_type>(rounded)) : static_cast<te_type>(rounded));
        }

    [[nodiscard]]
    static te_type te_odd(te_type val)
        {
        if (!std::isfinite(val))
            {
            return te_parser::te_nan;
            }
        int64_t rounded{ static_cast<int64_t>(std::ceil(std::abs(val))) };
        if ((rounded % 2) == 0)
            {
            ++rounded;
            }
        return (val < 0 ? -(static_cast<te_type>(rounded)) : static_cast<te_type>(rounded));
        }

    [[nodiscard]]
    static te_type te_is_even(te_type val)
        {
        if (!std::isfinite(val))
            {
            return te_parser::te_nan;
            }
        const int64_t floored{ static_cast<int64_t>(std::floor(val)) };
        return ((floored % 2) == 0 ? te_true_value() : te_false_value());
        }

    [[nodiscard]]
    static te_type te_is_odd(te_type val)
        {
        if (!std::isfinite(val))
            {
            return te_parser::te_nan;
            }
        const int64_t floored{ static_cast<int64_t>(std::floor(val)) };
        return ((floored % 2) != 0 ? te_true_value() : te_false_value());
        }

    [[nodiscard]]
    constexpr static te_type te_equal(te_type val1, te_type val2) noexcept
        {
        return (!std::isfinite(val1) || !std::isfinite(val2)) ?
                   te_parser::te_nan :
                   static_cast<te_type>((val1 == val2) ? 1 : 0);
        }

    [[nodiscard]]
    constexpr static te_type te_not_equal(te_type val1, te_type val2) noexcept
        {
        return (!std::isfinite(val1) || !std::isfinite(val2)) ?
                   te_parser::te_nan :
                   static_cast<te_type>((val1 != val2) ? 1 : 0);
        }

    [[nodiscard]]
    constexpr static te_type te_less_than(te_type val1, te_type val2) noexcept
        {
        return (!std::isfinite(val1) || !std::isfinite(val2)) ?
                   te_parser::te_nan :
                   static_cast<te_type>((val1 < val2) ? 1 : 0);
        }

    [[nodiscard]]
    constexpr static te_type te_less_than_equal_to(te_type val1, te_type val2) noexcept
        {
        return (!std::isfinite(val1) || !std::isfinite(val2)) ?
                   te_parser::te_nan :
                   static_cast<te_type>((val1 <= val2) ? 1 : 0);
        }

    [[nodiscard]]
    constexpr static te_type te_greater_than(te_type val1, te_type val2) noexcept
        {
        return (!std::isfinite(val1) || !std::isfinite(val2)) ?
                   te_parser::te_nan :
                   static_cast<te_type>((val1 > val2) ? 1 : 0);
        }

    [[nodiscard]]
    constexpr static te_type te_greater_than_equal_to(te_type val1, te_type val2) noexcept
        {
        return (!std::isfinite(val1) || !std::isfinite(val2)) ?
                   te_parser::te_nan :
                   static_cast<te_type>((val1 >= val2) ? 1 : 0);
        }

    [[nodiscard]]
    constexpr static te_type te_and(te_type val1, te_type val2)
        {
        // clang-format off
        return (!std::isfinite(val1) && !std::isfinite(val2)) ?
            te_parser::te_nan :
            static_cast<te_type>(
                (te_parser::number_to_bool(val1) && te_parser::number_to_bool(val2)) ? 1 : 0);
        // clang-format on
        }

    [[nodiscard]]
    constexpr static te_type te_or(te_type val1, te_type val2)
        {
        // clang-format off
        return (!std::isfinite(val1) && !std::isfinite(val2)) ?
            te_parser::te_nan :
            static_cast<te_type>(
                (te_parser::number_to_bool(val1) || te_parser::number_to_bool(val2)) ? 1 : 0);
        // clang-format on
        }

    [[nodiscard]]
    static te_type te_not(te_type val)
        {
        return std::isfinite(val) ? static_cast<te_type>(!te_parser::number_to_bool(val)) :
                                    te_parser::te_nan;
        }

    /// @warning This version of round emulates Excel's behavior of supporting
    ///     negative decimal places (e.g., ROUND(21.5, -1) = 20). Be aware
    ///     of that if using this function outside TinyExpr++.
    [[nodiscard]]
    static te_type te_round(te_type val, te_type decimalPlaces) // NOLINT
        {
        const bool useNegativeRound{ decimalPlaces < 0 };
        const size_t adjustedDecimalPlaces{ !std::isfinite(decimalPlaces) ?
                                                0 :
                                                static_cast<size_t>(std::abs(decimalPlaces)) };

        const auto decimalPosition = static_cast<te_type>(std::pow(10, adjustedDecimalPlaces));
        if (!std::isfinite(decimalPosition))
            {
            return te_parser::te_nan;
            }
        constexpr te_type ROUND_EPSILON{ 0.5 }; // NOLINT

        if (!useNegativeRound)
            {
            if (val < 0)
                {
                return (decimalPosition == 0) ?
                           std::ceil(val - ROUND_EPSILON) :
                           std::ceil(static_cast<te_type>(val * decimalPosition) - ROUND_EPSILON) /
                               decimalPosition;
                }
            return (decimalPosition == 0) ?
                       std::floor(val + ROUND_EPSILON) :
                       std::floor(static_cast<te_type>(val * decimalPosition) + ROUND_EPSILON) /
                           decimalPosition;
            }
        // ROUND(21.5, -1) = 20
        if (val < 0)
            {
            return std::ceil(static_cast<te_type>(val / decimalPosition) - ROUND_EPSILON) *
                   decimalPosition;
            }
        return std::floor(static_cast<te_type>(val / decimalPosition) + ROUND_EPSILON) *
               decimalPosition;
        }

    [[nodiscard]]
    static te_type te_nper(te_type rate, te_type pmt, te_type presentValue, te_type futureValue,
                           te_type type)
        {
        if (!std::isfinite(rate) || !std::isfinite(pmt) || !std::isfinite(presentValue))
            {
            return te_parser::te_nan;
            }

        if (!std::isfinite(futureValue))
            {
            futureValue = 0;
            }
        if (!std::isfinite(type))
            {
            type = 0;
            }

        type = (type != 0) ? 1 : 0;

        if (rate <= -1.0)
            {
            return te_parser::te_nan;
            }

        if (rate == 0.0)
            {
            if (pmt == 0.0)
                {
                return te_parser::te_nan;
                }
            return -(presentValue + futureValue) / pmt;
            }

        const te_type onePlusRate = 1 + rate;
        const te_type paymentTerm = pmt * (1 + rate * type);

        const te_type numerator = paymentTerm - futureValue * rate;
        const te_type denominator = presentValue * rate + paymentTerm;
        const te_type ratio = numerator / denominator;

        if (!std::isfinite(ratio) || ratio <= 0.0 || onePlusRate <= 0.0)
            {
            return te_parser::te_nan;
            }

        return std::log(ratio) / std::log(onePlusRate);
        }

    [[nodiscard]]
    static te_type te_fv(te_type rate, te_type nper, te_type pmt, te_type presentValue,
                         te_type type)
        {
        if (!std::isfinite(rate) || !std::isfinite(nper) || !std::isfinite(pmt))
            {
            return te_parser::te_nan;
            }

        if (!std::isfinite(presentValue))
            {
            presentValue = 0;
            }
        if (!std::isfinite(type))
            {
            type = 0;
            }

        // Excel: nper == 0 is valid
        if (nper == 0)
            {
            return -presentValue;
            }

        type = (type != 0) ? 1 : 0;

        if (rate == 0.0)
            {
            return -(presentValue + pmt * nper);
            }

        const te_type powVal = std::pow(1 + rate, nper);
        if (!std::isfinite(powVal))
            {
            return te_parser::te_nan;
            }

        return -(presentValue * powVal + (pmt * (1 + (rate * type)) * (powVal - 1) / rate));
        }

    [[nodiscard]]
    static te_type te_pmt(te_type rate, te_type nper, te_type presentValue, te_type futureValue,
                          te_type type)
        {
        if (!std::isfinite(rate) || !std::isfinite(nper) || !std::isfinite(presentValue))
            {
            return te_parser::te_nan;
            }
        if (!std::isfinite(futureValue))
            {
            futureValue = 0;
            }
        if (!std::isfinite(type))
            {
            type = 0;
            }
        if (nper == 0)
            {
            return te_parser::te_nan;
            }
        if (rate < -1.0)
            {
            return te_parser::te_nan;
            }
        if (rate == -1.0)
            {
            return 0.0;
            }

        // coerce type to 0 or 1
        type = (type != 0) ? 1 : 0;

        // zero-interest
        if (rate == 0.0)
            {
            return -(presentValue + futureValue) / nper;
            }

        const te_type powVal = std::pow(1 + rate, nper);
        if (!std::isfinite(powVal) || powVal == 0.0)
            {
            return te_parser::te_nan;
            }

        return -((presentValue * powVal) + futureValue) * rate /
               ((1 + (rate * type)) * (powVal - 1));
        }

    [[nodiscard]]
    static te_type te_ipmt(te_type rate, te_type period, te_type periods, te_type presentValue,
                           te_type futureValue, te_type type)
        {
        if (!std::isfinite(rate) || !std::isfinite(period) || !std::isfinite(periods) ||
            !std::isfinite(presentValue))
            {
            return te_parser::te_nan;
            }

        if (!std::isfinite(futureValue))
            {
            futureValue = 0;
            }
        if (!std::isfinite(type))
            {
            type = 0;
            }

        type = (type != 0) ? 1 : 0;

        if (periods <= 0.0)
            {
            return te_parser::te_nan;
            }
        if (period < 1.0 || period > periods)
            {
            return te_parser::te_nan;
            }
        if (rate == 0.0)
            {
            return 0.0;
            }
        if (rate <= -1.0)
            {
            // Excel: linear interest regime for IPMT
            if (type != 0 && period == 1.0)
                {
                return 0.0;
                }

            return -presentValue * rate;
            }

        const te_type payment = te_pmt(rate, periods, presentValue, futureValue, type);
        if (!std::isfinite(payment))
            {
            return te_parser::te_nan;
            }

        // Excel: type == 1 has zero interest in the first period
        if (type == 1 && period == 1.0)
            {
            return 0.0;
            }

        // compute FV at period - 1 (this must allow period - 1 == 0)
        const te_type num = period - 1.0;
        const te_type powVal = std::pow(1 + rate, num);
        if (!std::isfinite(powVal))
            {
            return te_parser::te_nan;
            }

        // FV(r, n, pmt, pv, type) (note: no FV argument; payment already encodes it)
        const te_type fvAtN =
            -(presentValue * powVal + payment * (1 + (rate * type)) * (powVal - 1) / rate);

        te_type interest = fvAtN * rate;

        // Excel adjustment for type == 1
        if (type == 1)
            {
            interest /= (1 + rate);
            }

        return interest;
        }

    [[nodiscard]]
    static te_type te_pv(te_type rate, te_type nper, te_type pmt, te_type futureValue, te_type type)
        {
        if (!std::isfinite(rate) || !std::isfinite(nper) || !std::isfinite(pmt))
            {
            return te_parser::te_nan;
            }

        if (!std::isfinite(futureValue))
            {
            futureValue = 0;
            }
        if (!std::isfinite(type))
            {
            type = 0;
            }

        if (nper == 0)
            {
            return 0.0;
            }

        // Excel: rate <= -1 -> #NUM!
        if (rate <= -1.0)
            {
            return te_parser::te_nan;
            }

        // coerce type to 0 or 1
        type = (type != 0) ? 1 : 0;

        if (rate == 0.0)
            {
            return -(futureValue + pmt * nper);
            }

        const te_type powVal = std::pow(1 + rate, nper);
        if (!std::isfinite(powVal) || powVal == 0.0)
            {
            return te_parser::te_nan;
            }

        const te_type annuity = (pmt * (1 + (rate * type)) * (powVal - 1)) / rate;
        return -(futureValue + annuity) / powVal;
        }

    [[nodiscard]]
    static te_type te_ppmt(te_type rate, te_type period, te_type periods, te_type presentValue,
                           te_type futureValue, te_type type)
        {
        if (!std::isfinite(rate) || !std::isfinite(period) || !std::isfinite(periods) ||
            !std::isfinite(presentValue))
            {
            return te_parser::te_nan;
            }

        if (!std::isfinite(futureValue))
            {
            futureValue = 0;
            }
        if (!std::isfinite(type))
            {
            type = 0;
            }

        if (period < 1.0 || period > periods || periods <= 0.0)
            {
            return te_parser::te_nan;
            }

        type = (type != 0) ? 1 : 0;

        const te_type payment = te_pmt(rate, periods, presentValue, futureValue, type);
        if (!std::isfinite(payment))
            {
            return te_parser::te_nan;
            }

        const te_type interest = te_ipmt(rate, period, periods, presentValue, futureValue, type);
        if (!std::isfinite(interest))
            {
            return te_parser::te_nan;
            }

        return payment - interest;
        }

    [[nodiscard]]
    static te_type te_cumipmt(te_type rate, te_type periods, te_type presentValue,
                              te_type startPeriod, te_type endPeriod, te_type type)
        {
        if (!std::isfinite(rate) || !std::isfinite(periods) || !std::isfinite(presentValue) ||
            !std::isfinite(startPeriod) || !std::isfinite(endPeriod) || !std::isfinite(type))
            {
            return te_parser::te_nan;
            }

        periods = std::trunc(periods);
        startPeriod = std::trunc(startPeriod);
        endPeriod = std::trunc(endPeriod);
        type = std::trunc(type);

        if (rate <= 0.0 || periods <= 0.0 || presentValue <= 0.0)
            {
            return te_parser::te_nan;
            }

        if (startPeriod < 1.0 || endPeriod < 1.0 || startPeriod > endPeriod || endPeriod > periods)
            {
            return te_parser::te_nan;
            }

        if (type != 0.0 && type != 1.0)
            {
            return te_parser::te_nan;
            }

        const auto fromPeriod = static_cast<int64_t>(startPeriod);
        const auto toPeriod = static_cast<int64_t>(endPeriod);

        te_type total{ 0 };

        for (int64_t period = fromPeriod; period <= toPeriod; ++period)
            {
            const te_type interestPayment =
                te_ipmt(rate, static_cast<te_type>(period), periods, presentValue, 0, type);
            if (!std::isfinite(interestPayment))
                {
                return te_parser::te_nan;
                }
            total += interestPayment;
            }

        return total;
        }

    [[nodiscard]]
    static te_type te_cumprinc(te_type rate, te_type periods, te_type presentValue,
                               te_type startPeriod, te_type endPeriod, te_type type)
        {
        if (!std::isfinite(rate) || !std::isfinite(periods) || !std::isfinite(presentValue) ||
            !std::isfinite(startPeriod) || !std::isfinite(endPeriod) || !std::isfinite(type))
            {
            return te_parser::te_nan;
            }

        periods = std::trunc(periods);
        startPeriod = std::trunc(startPeriod);
        endPeriod = std::trunc(endPeriod);
        type = std::trunc(type);

        if (rate <= 0.0 || periods <= 0.0 || presentValue <= 0.0)
            {
            return te_parser::te_nan;
            }

        if (startPeriod < 1.0 || endPeriod < 1.0 || startPeriod > endPeriod || endPeriod > periods)
            {
            return te_parser::te_nan;
            }

        if (type != 0.0 && type != 1.0)
            {
            return te_parser::te_nan;
            }

        te_type total{ 0 };

        for (auto period = static_cast<int64_t>(startPeriod);
             period <= static_cast<int64_t>(endPeriod); ++period)
            {
            const te_type principal =
                te_ppmt(rate, static_cast<te_type>(period), periods, presentValue, 0, type);

            if (!std::isfinite(principal))
                {
                return te_parser::te_nan;
                }

            total += principal;
            }

        return total;
        }

    [[nodiscard]]
    static te_type te_nominal(te_type effectiveRate, te_type periods)
        {
        if (periods < 1 || effectiveRate <= 0)
            {
            return te_parser::te_nan;
            }
        return periods * (std::pow(1 + effectiveRate, (1 / periods)) - 1);
        }

    [[nodiscard]]
    static te_type te_effect(te_type nominalRate, te_type periods)
        {
        if (periods < 1 || nominalRate <= 0)
            {
            return te_parser::te_nan;
            }
        return std::pow(1 + (nominalRate / periods), periods) - 1;
        }

    [[nodiscard]]
    static te_type te_asset_depreciation(te_type cost, te_type salvage, te_type life,
                                         te_type period, te_type month)
        {
        constexpr te_type NUM_OF_MONTHS{ 12 };
        constexpr te_type FRAC_TO_PERCENT{ 100 };
        // month in the first year of depreciation is optional and defaults to a full year
        if (!std::isfinite(month))
            {
            month = NUM_OF_MONTHS;
            }
        if (month < 1 || month > NUM_OF_MONTHS || life <= 0 || cost <= 0 || period < 1 ||
            period >= (life + 2))
            {
            return te_parser::te_nan;
            }

        te_type intPrefix{ 0 };
        te_type mantissa = std::modf(life, &intPrefix) * FRAC_TO_PERCENT;
        if (mantissa > 0)
            {
            return te_parser::te_nan;
            }
        mantissa = std::modf(period, &intPrefix) * FRAC_TO_PERCENT;
        if (mantissa > 0)
            {
            return te_parser::te_nan;
            }

        // month gets rounded down in spreadsheet programs
        month = std::floor(static_cast<te_type>(month));

        // we just verified that this are integral, but round down to fully ensure that
        life = std::floor(static_cast<te_type>(life));
        period = std::floor(static_cast<te_type>(period));

        // rate gets clipped to three-decimal precision according to Excel docs
        const auto rate = te_round(1 - (std::pow((salvage / cost), (1 / life))), 3);
        if (period == 1)
            {
            return cost * rate * (month / NUM_OF_MONTHS);
            }
        te_type priorDepreciation{ 0.0 };
        te_type costAfterDepreciation{ cost };
        for (uint64_t i = 1; i < static_cast<uint64_t>(period) - 1; ++i)
            {
            const auto depreciation = (costAfterDepreciation * rate);
            priorDepreciation += depreciation;
            costAfterDepreciation -= depreciation;
            }
        priorDepreciation += costAfterDepreciation * rate * (month / NUM_OF_MONTHS);
        return (period == life + 1) ?
                   ((cost - priorDepreciation) * rate * (NUM_OF_MONTHS - month)) / NUM_OF_MONTHS :
                   (cost - priorDepreciation) * rate;
        }

    [[nodiscard]]
    constexpr static te_type te_pi() noexcept
        {
        return static_cast<te_type>(3.14159265358979323846); // NOLINT
        }

    [[nodiscard]]
    constexpr static te_type te_e() noexcept
        {
        return static_cast<te_type>(2.71828182845904523536); // NOLINT
        }

    [[nodiscard]]
    static te_type te_fac(te_type val) noexcept
        { /* simplest version of factorial */
        if (!std::isfinite(val) || val < 0.0)
            {
            return te_parser::te_nan;
            }
        if (val > (std::numeric_limits<unsigned int>::max)())
            {
            return std::numeric_limits<te_type>::infinity();
            }
        const auto unsignedVal = static_cast<size_t>(val);
        uint32_t result{ 1 };
        for (uint32_t i = 1; i <= unsignedVal; i++)
            {
            if (i > (std::numeric_limits<uint32_t>::max)() / result)
                {
                return std::numeric_limits<te_type>::infinity();
                }
            result *= i;
            }
        return static_cast<te_type>(result);
        }

    [[nodiscard]]
    static te_type te_absolute_value(te_type val)
        {
        return std::fabs(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_log(te_type val)
        {
        return std::log(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_log10(te_type val)
        {
        return std::log10(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_pow(te_type val1, te_type val2)
        {
        return std::pow(static_cast<te_type>(val1), static_cast<te_type>(val2));
        }

    [[nodiscard]]
    static te_type te_tan(te_type val)
        {
        return std::tan(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_tanh(te_type val)
        {
        return std::tanh(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_trunc(te_type val)
        {
        return std::trunc(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_sin(te_type val)
        {
        return std::sin(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_sinh(te_type val)
        {
        return std::sinh(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_sqrt(te_type val)
        {
        if (val < 0)
            {
            throw std::runtime_error("Negative value passed to SQRT.");
            }
        return std::sqrt(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_floor(te_type val)
        {
        return std::floor(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_ceil(te_type val)
        {
        return std::ceil(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_exp(te_type val)
        {
        return std::exp(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_cos(te_type val)
        {
        return std::cos(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_cosh(te_type val)
        {
        return std::cosh(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_acos(te_type val)
        {
        return std::acos(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_asin(te_type val)
        {
        if (std::isfinite(val) && (val < -1.0 || val > 1.0))
            {
            throw std::runtime_error("Argument passed to ASIN must be between -1 and 1.");
            }
        return std::asin(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_atan(te_type val)
        {
        return std::atan(static_cast<te_type>(val));
        }

    [[nodiscard]]
    static te_type te_atan2(te_type val1, te_type val2)
        {
        return std::atan2(static_cast<te_type>(val1), (static_cast<te_type>(val2)));
        }

    [[nodiscard]]
    static te_type te_tgamma(te_type val)
        {
        return std::tgamma(val);
        }

    [[nodiscard]]
    static te_type te_random()
        {
#ifdef TE_RAND_SEED
        static std::mt19937 gen(static_cast<unsigned int>(TE_RAND_SEED));
#elif defined(TE_RAND_SEED_TIME)
        static std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
#else
        static std::mt19937 gen(std::random_device{}());
#endif

        static std::uniform_real_distribution<te_type> distribute(0, 1);
        return distribute(gen);
        }

    [[nodiscard]]
    constexpr static te_type te_divide(te_type val1, te_type val2)
        {
        if (val2 == 0)
            {
            throw std::runtime_error("Division by zero.");
            }
        return val1 / val2;
        }

    [[nodiscard]]
    static te_type te_modulus(te_type val1, te_type val2)
        {
        if (val2 == 0)
            {
            throw std::runtime_error("Modulus by zero.");
            }
        return std::fmod(val1, val2);
        }

    [[nodiscard]]
    static te_type te_sum(te_type val1, te_type val2, te_type val3, te_type val4, te_type val5,
                          te_type val6, te_type val7, te_type val8, te_type val9, te_type val10,
                          te_type val11, te_type val12, te_type val13, te_type val14, te_type val15,
                          te_type val16, te_type val17, te_type val18, te_type val19, te_type val20,
                          te_type val21, te_type val22, te_type val23, te_type val24)
        {
        const auto getSumMaybeNan = [](const auto val) { return (!std::isfinite(val) ? 0 : val); };

        return getSumMaybeNan(val1) + getSumMaybeNan(val2) + getSumMaybeNan(val3) +
               getSumMaybeNan(val4) + getSumMaybeNan(val5) + getSumMaybeNan(val6) +
               getSumMaybeNan(val7) + getSumMaybeNan(val8) + getSumMaybeNan(val9) +
               getSumMaybeNan(val10) + getSumMaybeNan(val11) + getSumMaybeNan(val12) +
               getSumMaybeNan(val13) + getSumMaybeNan(val14) + getSumMaybeNan(val15) +
               getSumMaybeNan(val16) + getSumMaybeNan(val17) + getSumMaybeNan(val18) +
               getSumMaybeNan(val19) + getSumMaybeNan(val20) + getSumMaybeNan(val21) +
               getSumMaybeNan(val22) + getSumMaybeNan(val23) + getSumMaybeNan(val24);
        }

    [[nodiscard]]
    static te_type te_average(te_type val1, te_type val2, te_type val3, te_type val4, te_type val5,
                              te_type val6, te_type val7, te_type val8, te_type val9, te_type val10,
                              te_type val11, te_type val12, te_type val13, te_type val14,
                              te_type val15, te_type val16, te_type val17, te_type val18,
                              te_type val19, te_type val20, te_type val21, te_type val22,
                              te_type val23, te_type val24)
        {
        const auto isValidMaybeNan = [](const auto val) { return (!std::isfinite(val) ? 0 : 1); };

        const auto validN =
            isValidMaybeNan(val1) + isValidMaybeNan(val2) + isValidMaybeNan(val3) +
            isValidMaybeNan(val4) + isValidMaybeNan(val5) + isValidMaybeNan(val6) +
            isValidMaybeNan(val7) + isValidMaybeNan(val8) + isValidMaybeNan(val9) +
            isValidMaybeNan(val10) + isValidMaybeNan(val11) + isValidMaybeNan(val12) +
            isValidMaybeNan(val13) + isValidMaybeNan(val14) + isValidMaybeNan(val15) +
            isValidMaybeNan(val16) + isValidMaybeNan(val17) + isValidMaybeNan(val18) +
            isValidMaybeNan(val19) + isValidMaybeNan(val20) + isValidMaybeNan(val21) +
            isValidMaybeNan(val22) + isValidMaybeNan(val23) + isValidMaybeNan(val24);
        const auto total =
            te_sum(val1, val2, val3, val4, val5, val6, val7, val8, val9, val10, val11, val12, val13,
                   val14, val15, val16, val17, val18, val19, val20, val21, val22, val23, val24);
        return te_divide(total, static_cast<te_type>(validN));
        }

    // Combinations (without repetition)
    [[nodiscard]]
    static te_type te_ncr(te_type val1, te_type val2) noexcept
        {
        if (!std::isfinite(val1) || !std::isfinite(val2) || val1 < 0.0 || val2 < 0.0 || val1 < val2)
            {
            return te_parser::te_nan;
            }
        if (val1 > (std::numeric_limits<unsigned int>::max)() ||
            val2 > (std::numeric_limits<unsigned int>::max)())
            {
            return std::numeric_limits<te_type>::infinity();
            }
        const uint32_t usignN{ static_cast<unsigned int>(val1) };
        uint32_t usignR{ static_cast<unsigned int>(val2) };
        uint32_t result{ 1 };
        if (usignR > usignN / 2)
            {
            usignR = usignN - usignR;
            }
        for (decltype(usignR) i = 1; i <= usignR; i++)
            {
            if (result > (std::numeric_limits<uint32_t>::max)() / (usignN - usignR + i))
                {
                return std::numeric_limits<te_type>::infinity();
                }
            result *= usignN - usignR + i;
            result /= i;
            }
        return static_cast<te_type>(result);
        }

    // Permutations (without repetition)
    [[nodiscard]]
    static te_type te_npr(te_type val1, te_type val2) noexcept
        {
        return te_ncr(val1, val2) * te_fac(val2);
        }

    [[nodiscard]]
    constexpr static te_type te_add(te_type val1, te_type val2) noexcept
        {
        return val1 + val2;
        }

    [[nodiscard]]
    constexpr static te_type te_sub(te_type val1, te_type val2) noexcept
        {
        return val1 - val2;
        }

    [[nodiscard]]
    constexpr static te_type te_mul(te_type val1, te_type val2) noexcept
        {
        return val1 * val2;
        }

#if __cplusplus >= 202002L && !defined(TE_FLOAT)
    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_right_rotate8(te_type val1, te_type val2)
        {
        constexpr int BITNESS{ 8 };
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise RIGHT ROTATE operation must use integers.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Bitwise RIGHT ROTATE value must be positive.");
            }
        if (val2 > BITNESS)
            {
            throw std::runtime_error("Rotation operation must be between 0-8.");
            }

        return static_cast<te_type>(std::rotr(static_cast<uint8_t>(val1), static_cast<int>(val2)));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_left_rotate8(te_type val1, te_type val2)
        {
        constexpr int BITNESS{ 8 };
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise LEFT ROTATE operation must use integers.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Bitwise LEFT ROTATE value must be positive.");
            }
        if (val2 > BITNESS)
            {
            throw std::runtime_error("Rotation operation must be between 0-8.");
            }

        return static_cast<te_type>(std::rotl(static_cast<uint8_t>(val1), static_cast<int>(val2)));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_right_rotate16(te_type val1, te_type val2)
        {
        constexpr int BITNESS{ 16 };
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise RIGHT ROTATE operation must use integers.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Bitwise RIGHT ROTATE value must be positive.");
            }
        if (val2 > BITNESS)
            {
            throw std::runtime_error("Rotation operation must be between 0-16.");
            }

        return static_cast<te_type>(std::rotr(static_cast<uint16_t>(val1), static_cast<int>(val2)));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_left_rotate16(te_type val1, te_type val2)
        {
        constexpr int BITNESS{ 16 };
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise LEFT ROTATE operation must use integers.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Bitwise LEFT ROTATE value must be positive.");
            }
        if (val2 > BITNESS)
            {
            throw std::runtime_error("Rotation operation must be between 0-16.");
            }

        return static_cast<te_type>(std::rotl(static_cast<uint16_t>(val1), static_cast<int>(val2)));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_right_rotate32(te_type val1, te_type val2)
        {
        constexpr int BITNESS{ 32 };
        if constexpr (!te_parser::supports_32bit())
            {
            throw std::runtime_error("32-bit bitwise operations are not supported.");
            }
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise RIGHT ROTATE operation must use integers.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Bitwise RIGHT ROTATE value must be positive.");
            }
        if (val2 > BITNESS)
            {
            throw std::runtime_error("Rotation operation must be between 0-32.");
            }

        return static_cast<te_type>(std::rotr(static_cast<uint32_t>(val1), static_cast<int>(val2)));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_left_rotate32(te_type val1, te_type val2)
        {
        constexpr int BITNESS{ 32 };
        if constexpr (!te_parser::supports_32bit())
            {
            throw std::runtime_error("32-bit bitwise operations are not supported.");
            }
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise LEFT ROTATE operation must use integers.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Bitwise LEFT ROTATE value must be positive.");
            }
        if (val2 > BITNESS)
            {
            throw std::runtime_error("Rotation operation must be between 0-32.");
            }

        return static_cast<te_type>(std::rotl(static_cast<uint32_t>(val1), static_cast<int>(val2)));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_right_rotate64(te_type val1, te_type val2)
        {
        constexpr int BITNESS{ 63 };
        if constexpr (!te_parser::supports_64bit())
            {
            throw std::runtime_error("64-bit bitwise operations are not supported.");
            }
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise RIGHT ROTATE operation must use integers.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Bitwise RIGHT ROTATE value must be positive.");
            }
        if (val2 > BITNESS)
            {
            throw std::runtime_error("Rotation operation must be between 0-63");
            }

        return static_cast<te_type>(std::rotr(static_cast<uint64_t>(val1), static_cast<int>(val2)));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_left_rotate64(te_type val1, te_type val2)
        {
        constexpr int BITNESS{ 63 };
        if constexpr (!te_parser::supports_64bit())
            {
            throw std::runtime_error("64-bit bitwise operations are not supported.");
            }
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise LEFT ROTATE operation must use integers.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Bitwise LEFT ROTATE value must be positive.");
            }
        if (val2 > BITNESS)
            {
            throw std::runtime_error("Rotation operation must be between 0-63");
            }

        return static_cast<te_type>(std::rotl(static_cast<uint64_t>(val1), static_cast<int>(val2)));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_right_rotate(te_type val1, te_type val2)
        {
        if constexpr (te_parser::supports_64bit())
            {
            return te_right_rotate64(val1, val2);
            }
        if constexpr (te_parser::supports_32bit())
            {
            return te_right_rotate32(val1, val2);
            }

        return te_right_rotate16(val1, val2);
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_left_rotate(te_type val1, te_type val2)
        {
        if constexpr (te_parser::supports_64bit())
            {
            return te_left_rotate64(val1, val2);
            }
        else if constexpr (te_parser::supports_32bit())
            {
            return te_left_rotate32(val1, val2);
            }
        else
            {
            return te_left_rotate16(val1, val2);
            }
        }
#endif
    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_bitwise_not8(te_type val)
        {
        if (std::floor(val) != val)
            {
            throw std::runtime_error("Bitwise NOT must use integers.");
            }
        if (val < 0)
            {
            throw std::runtime_error("Bitwise NOT value must be positive.");
            }
        if (val > std::numeric_limits<uint8_t>::max())
            {
            throw std::runtime_error("Value is too large for bitwise NOT.");
            }

        // force the bit manipulation to stay unsigned, like what Excel does
        const uint8_t intVal{ static_cast<uint8_t>(val) };
        const decltype(intVal) result{ std::bit_not<decltype(intVal)>{}(intVal) };
        return static_cast<te_type>(result);
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_bitwise_not16(te_type val)
        {
        if (std::floor(val) != val)
            {
            throw std::runtime_error("Bitwise NOT must use integers.");
            }
        if (val < 0)
            {
            throw std::runtime_error("Bitwise NOT value must be positive.");
            }
        if (val > std::numeric_limits<uint16_t>::max())
            {
            throw std::runtime_error("Value is too large for bitwise NOT.");
            }

        const uint16_t intVal{ static_cast<uint16_t>(val) };
        const decltype(intVal) result{ std::bit_not<decltype(intVal)>{}(intVal) };
        return static_cast<te_type>(result);
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_bitwise_not32(te_type val)
        {
        if constexpr (!te_parser::supports_32bit())
            {
            throw std::runtime_error("32-bit bitwise operations are not supported.");
            }
        if (std::floor(val) != val)
            {
            throw std::runtime_error("Bitwise NOT must use integers.");
            }
        if (val < 0)
            {
            throw std::runtime_error("Bitwise NOT value must be positive.");
            }
        if (val > std::numeric_limits<uint32_t>::max())
            {
            throw std::runtime_error("Value is too large for bitwise NOT.");
            }

        const uint32_t intVal{ static_cast<uint32_t>(val) };
        const decltype(intVal) result{ std::bit_not<decltype(intVal)>{}(intVal) };
        return static_cast<te_type>(result);
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_bitwise_not64(te_type val)
        {
        if constexpr (!te_parser::supports_64bit())
            {
            throw std::runtime_error("64-bit bitwise operations are not supported.");
            }
        if (std::floor(val) != val)
            {
            throw std::runtime_error("Bitwise NOT must use integers.");
            }
        if (val < 0)
            {
            throw std::runtime_error("Bitwise NOT value must be positive.");
            }
        if (val > static_cast<te_type>(std::numeric_limits<uint64_t>::max()))
            {
            throw std::runtime_error("Value is too large for bitwise NOT.");
            }

        const uint64_t intVal{ static_cast<uint64_t>(val) };
        const decltype(intVal) result{ std::bit_not<decltype(intVal)>{}(intVal) };
        return static_cast<te_type>(result);
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_bitwise_not(te_type val)
        {
        if constexpr (te_parser::supports_64bit())
            {
            return te_bitwise_not64(val);
            }
        if constexpr (te_parser::supports_32bit())
            {
            return te_bitwise_not32(val);
            }

        return te_bitwise_not16(val);
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_bitwise_or(te_type val1, te_type val2)
        {
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise OR operation must use integers.");
            }
        // negative technically should be allowed, but spreadsheet programs do
        // not allow them; hence, we won't either
        if (val1 < 0 || val2 < 0)
            {
            throw std::runtime_error("Bitwise OR operation must use positive integers.");
            }
        if (val1 > te_parser::MAX_BITOPS_VAL || val2 > te_parser::MAX_BITOPS_VAL)
            {
            throw std::runtime_error("Value is too large for bitwise operation.");
            }
        return static_cast<te_type>(static_cast<uint64_t>(val1) | static_cast<uint64_t>(val2));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_bitwise_xor(te_type val1, te_type val2)
        {
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise XOR operation must use integers.");
            }
        // negative technically should be allowed, but spreadsheet programs do
        // not allow them; hence, we won't either
        if (val1 < 0 || val2 < 0)
            {
            throw std::runtime_error("Bitwise XOR operation must use positive integers.");
            }
        if (val1 > te_parser::MAX_BITOPS_VAL || val2 > te_parser::MAX_BITOPS_VAL)
            {
            throw std::runtime_error("Value is too large for bitwise operation.");
            }
        return static_cast<te_type>(static_cast<uint64_t>(val1) ^ static_cast<uint64_t>(val2));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_bitwise_and(te_type val1, te_type val2)
        {
        if (std::floor(val1) != val1 || std::floor(val2) != val2)
            {
            throw std::runtime_error("Bitwise AND operation must use integers.");
            }
        // negative technically should be allowed, but spreadsheet programs do
        // not allow them; hence, we won't either
        if (val1 < 0 || val2 < 0)
            {
            throw std::runtime_error("Bitwise AND operation must use positive integers.");
            }
        if (val1 > te_parser::MAX_BITOPS_VAL || val2 > te_parser::MAX_BITOPS_VAL)
            {
            throw std::runtime_error("Value is too large for bitwise operation.");
            }
        return static_cast<te_type>(static_cast<uint64_t>(val1) & static_cast<uint64_t>(val2));
        }

    // Shift operators
    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_left_shift(te_type val1, te_type val2)
        {
        // For 64-bit, you can shift 63 bits.
        // If we are limited to something like 53 bits, then we can use that (same as Excel)
        constexpr static auto MAX_BITNESS_PARAM =
            (te_parser::supports_64bit() ? te_parser::get_max_integer_bitness() - 1 :
                                           te_parser::get_max_integer_bitness());
        if (std::floor(val1) != val1)
            {
            throw std::runtime_error("Left side of left shift (<<) operation must be an integer.");
            }
        if (std::floor(val2) != val2)
            {
            throw std::runtime_error(
                "Additive expression of left shift (<<) operation must be an integer.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Left side of left shift (<<) operation cannot be negative.");
            }
        if (val1 > te_parser::MAX_BITOPS_VAL)
            {
            throw std::runtime_error("Value is too large for bitwise operation.");
            }
        // bitness is limited to 53-bit or 64-bit, so ensure shift doesn't go beyond that
        // and cause undefined behavior
        if (val2 < 0 || val2 > MAX_BITNESS_PARAM)
            {
            throw std::runtime_error(
                "Additive expression of left shift (<<) operation must be between 0-" +
                std::to_string(MAX_BITNESS_PARAM));
            }

        const auto multiplier = (static_cast<uint64_t>(1) << static_cast<uint64_t>(val2));
        const auto maxBaseNumber = (std::numeric_limits<uint64_t>::max() / multiplier);
        if (static_cast<uint64_t>(val1) > maxBaseNumber)
            {
            throw std::runtime_error(
                "Overflow in left shift (<<) operation; base number is too large.");
            }
        return static_cast<te_type>(static_cast<uint64_t>(val1) << static_cast<uint64_t>(val2));
        }

    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_right_shift(te_type val1, te_type val2)
        {
        constexpr static auto MAX_BITNESS_PARAM =
            (te_parser::supports_64bit() ? te_parser::get_max_integer_bitness() - 1 :
                                           te_parser::get_max_integer_bitness());

        if (std::floor(val1) != val1)
            {
            throw std::runtime_error("Left side of right shift (>>) operation must be an integer.");
            }
        if (std::floor(val2) != val2)
            {
            throw std::runtime_error(
                "Additive expression of right shift (>>) operation must be an integer.");
            }
        if (val1 < 0)
            {
            throw std::runtime_error("Left side of right shift (<<) operation cannot be negative.");
            }
        if (val1 > te_parser::MAX_BITOPS_VAL)
            {
            throw std::runtime_error("Value is too large for bitwise operation.");
            }
        if (val2 < 0 || val2 > MAX_BITNESS_PARAM)
            {
            throw std::runtime_error(
                "Additive expression of right shift (>>) operation must be between 0-" +
                std::to_string(MAX_BITNESS_PARAM));
            }

        return static_cast<te_type>(static_cast<uint64_t>(val1) >> static_cast<uint64_t>(val2));
        }

    /// @warning This emulates Excel, where a negative shift amount acts as a right shift.\n
    ///     Be aware of this if using this function outside TinyExpr++.
    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_left_shift_or_right(te_type val1, te_type val2)
        {
        return (val2 >= 0) ? te_left_shift(val1, val2) : te_right_shift(val1, std::abs(val2));
        }

    /// @warning This emulates Excel, where a negative shift amount acts as a right shift.\n
    ///     Be aware of this if using this function outside TinyExpr++.
    //--------------------------------------------------
    [[nodiscard]]
    static te_type te_right_shift_or_left(te_type val1, te_type val2)
        {
        return (val2 >= 0) ? te_right_shift(val1, val2) : te_left_shift(val1, std::abs(val2));
        }

    [[nodiscard]]
    constexpr static te_type te_sqr(te_type val) noexcept
        {
        return val * val;
        }

    [[nodiscard]]
    static te_type te_max_maybe_nan(te_type val1, te_type val2MaybeNan) noexcept
        {
        return (std::max)(val1, !std::isfinite(val2MaybeNan) ? val1 : val2MaybeNan);
        }

    [[nodiscard]]
    static te_type te_max(te_type val1, te_type val2, te_type val3, te_type val4, te_type val5,
                          te_type val6, te_type val7, te_type val8, te_type val9, te_type val10,
                          te_type val11, te_type val12, te_type val13, te_type val14, te_type val15,
                          te_type val16, te_type val17, te_type val18, te_type val19, te_type val20,
                          te_type val21, te_type val22, te_type val23, te_type val24) noexcept
        {
        // assumes that at least val1 is a number, rest can be NaN
        // NOLINTBEGIN
        auto maxVal = te_max_maybe_nan(val1, val2);
        maxVal = te_max_maybe_nan(maxVal, val3);
        maxVal = te_max_maybe_nan(maxVal, val4);
        maxVal = te_max_maybe_nan(maxVal, val5);
        maxVal = te_max_maybe_nan(maxVal, val6);
        maxVal = te_max_maybe_nan(maxVal, val7);
        maxVal = te_max_maybe_nan(maxVal, val8);
        maxVal = te_max_maybe_nan(maxVal, val9);
        maxVal = te_max_maybe_nan(maxVal, val10);
        maxVal = te_max_maybe_nan(maxVal, val11);
        maxVal = te_max_maybe_nan(maxVal, val12);
        maxVal = te_max_maybe_nan(maxVal, val13);
        maxVal = te_max_maybe_nan(maxVal, val14);
        maxVal = te_max_maybe_nan(maxVal, val15);
        maxVal = te_max_maybe_nan(maxVal, val16);
        maxVal = te_max_maybe_nan(maxVal, val17);
        maxVal = te_max_maybe_nan(maxVal, val18);
        maxVal = te_max_maybe_nan(maxVal, val19);
        maxVal = te_max_maybe_nan(maxVal, val20);
        maxVal = te_max_maybe_nan(maxVal, val21);
        maxVal = te_max_maybe_nan(maxVal, val22);
        maxVal = te_max_maybe_nan(maxVal, val23);
        return te_max_maybe_nan(maxVal, val24);
        // NOLINTEND
        }

    [[nodiscard]]
    static te_type te_min_maybe_nan(te_type val1, te_type val2MaybeNan) noexcept
        {
        return (std::min)(val1, !std::isfinite(val2MaybeNan) ? val1 : val2MaybeNan);
        }

    [[nodiscard]]
    static te_type te_min(te_type val1, te_type val2, te_type val3, te_type val4, te_type val5,
                          te_type val6, te_type val7, te_type val8, te_type val9, te_type val10,
                          te_type val11, te_type val12, te_type val13, te_type val14, te_type val15,
                          te_type val16, te_type val17, te_type val18, te_type val19, te_type val20,
                          te_type val21, te_type val22, te_type val23, te_type val24) noexcept
        {
        // assumes that at least val1 is legit, rest can be NaN
        // NOLINTBEGIN
        auto minVal = te_min_maybe_nan(val1, val2);
        minVal = te_min_maybe_nan(minVal, val3);
        minVal = te_min_maybe_nan(minVal, val4);
        minVal = te_min_maybe_nan(minVal, val5);
        minVal = te_min_maybe_nan(minVal, val6);
        minVal = te_min_maybe_nan(minVal, val7);
        minVal = te_min_maybe_nan(minVal, val8);
        minVal = te_min_maybe_nan(minVal, val9);
        minVal = te_min_maybe_nan(minVal, val10);
        minVal = te_min_maybe_nan(minVal, val11);
        minVal = te_min_maybe_nan(minVal, val12);
        minVal = te_min_maybe_nan(minVal, val13);
        minVal = te_min_maybe_nan(minVal, val14);
        minVal = te_min_maybe_nan(minVal, val15);
        minVal = te_min_maybe_nan(minVal, val16);
        minVal = te_min_maybe_nan(minVal, val17);
        minVal = te_min_maybe_nan(minVal, val18);
        minVal = te_min_maybe_nan(minVal, val19);
        minVal = te_min_maybe_nan(minVal, val20);
        minVal = te_min_maybe_nan(minVal, val21);
        minVal = te_min_maybe_nan(minVal, val22);
        minVal = te_min_maybe_nan(minVal, val23);
        return te_min_maybe_nan(minVal, val24);
        // NOLINTEND
        }

    [[nodiscard]]
    static te_type te_and_maybe_nan(te_type val1, te_type val2MaybeNan)
        {
        return !std::isfinite(val2MaybeNan) ?
                   static_cast<te_type>(te_parser::number_to_bool(val1)) :
                   static_cast<te_type>(te_parser::number_to_bool(val1) &&
                                        te_parser::number_to_bool(val2MaybeNan));
        }

    [[nodiscard]]
    static te_type te_and_variadic(te_type val1, te_type val2, te_type val3, te_type val4,
                                   te_type val5, te_type val6, te_type val7, te_type val8,
                                   te_type val9, te_type val10, te_type val11, te_type val12,
                                   te_type val13, te_type val14, te_type val15, te_type val16,
                                   te_type val17, te_type val18, te_type val19, te_type val20,
                                   te_type val21, te_type val22, te_type val23, te_type val24)
        {
        // at least val1 must be legit, rest can be NaN
        if (!std::isfinite(val1))
            {
            return te_parser::te_nan;
            }
        // NOLINTBEGIN
        auto andVal = te_and_maybe_nan(val1, val2);
        andVal = te_and_maybe_nan(andVal, val3);
        andVal = te_and_maybe_nan(andVal, val4);
        andVal = te_and_maybe_nan(andVal, val5);
        andVal = te_and_maybe_nan(andVal, val6);
        andVal = te_and_maybe_nan(andVal, val7);
        andVal = te_and_maybe_nan(andVal, val8);
        andVal = te_and_maybe_nan(andVal, val9);
        andVal = te_and_maybe_nan(andVal, val10);
        andVal = te_and_maybe_nan(andVal, val11);
        andVal = te_and_maybe_nan(andVal, val12);
        andVal = te_and_maybe_nan(andVal, val13);
        andVal = te_and_maybe_nan(andVal, val14);
        andVal = te_and_maybe_nan(andVal, val15);
        andVal = te_and_maybe_nan(andVal, val16);
        andVal = te_and_maybe_nan(andVal, val17);
        andVal = te_and_maybe_nan(andVal, val18);
        andVal = te_and_maybe_nan(andVal, val19);
        andVal = te_and_maybe_nan(andVal, val20);
        andVal = te_and_maybe_nan(andVal, val21);
        andVal = te_and_maybe_nan(andVal, val22);
        andVal = te_and_maybe_nan(andVal, val23);
        return te_and_maybe_nan(andVal, val24);
        // NOLINTEND
        }

    [[nodiscard]]
    static te_type te_or_maybe_nan(te_type val1, te_type val2MaybeNan)
        {
        return !std::isfinite(val2MaybeNan) ?
                   static_cast<te_type>(te_parser::number_to_bool(val1)) :
                   static_cast<te_type>(te_parser::number_to_bool(val1) ||
                                        te_parser::number_to_bool(val2MaybeNan));
        }

    [[nodiscard]]
    static te_type te_or_variadic(te_type val1, te_type val2, te_type val3, te_type val4,
                                  te_type val5, te_type val6, te_type val7, te_type val8,
                                  te_type val9, te_type val10, te_type val11, te_type val12,
                                  te_type val13, te_type val14, te_type val15, te_type val16,
                                  te_type val17, te_type val18, te_type val19, te_type val20,
                                  te_type val21, te_type val22, te_type val23, te_type val24)
        {
        // at least val1 must be legit, rest can be NaN
        if (!std::isfinite(val1))
            {
            return te_parser::te_nan;
            }
        // NOLINTBEGIN
        auto orVal = te_or_maybe_nan(val1, val2);
        orVal = te_or_maybe_nan(orVal, val3);
        orVal = te_or_maybe_nan(orVal, val4);
        orVal = te_or_maybe_nan(orVal, val5);
        orVal = te_or_maybe_nan(orVal, val6);
        orVal = te_or_maybe_nan(orVal, val7);
        orVal = te_or_maybe_nan(orVal, val8);
        orVal = te_or_maybe_nan(orVal, val9);
        orVal = te_or_maybe_nan(orVal, val10);
        orVal = te_or_maybe_nan(orVal, val11);
        orVal = te_or_maybe_nan(orVal, val12);
        orVal = te_or_maybe_nan(orVal, val13);
        orVal = te_or_maybe_nan(orVal, val14);
        orVal = te_or_maybe_nan(orVal, val15);
        orVal = te_or_maybe_nan(orVal, val16);
        orVal = te_or_maybe_nan(orVal, val17);
        orVal = te_or_maybe_nan(orVal, val18);
        orVal = te_or_maybe_nan(orVal, val19);
        orVal = te_or_maybe_nan(orVal, val20);
        orVal = te_or_maybe_nan(orVal, val21);
        orVal = te_or_maybe_nan(orVal, val22);
        orVal = te_or_maybe_nan(orVal, val23);
        return te_or_maybe_nan(orVal, val24);
        // NOLINTEND
        }

    [[nodiscard]]
    static te_type te_if(te_type val1, te_type val2, te_type val3)
        {
        return te_parser::number_to_bool(val1) ? val2 : val3;
        }

    [[nodiscard]]
    static te_type te_ifs(te_type if1, te_type if1True, te_type if2, te_type if2True, te_type if3,
                          te_type if3True, te_type if4, te_type if4True, te_type if5,
                          te_type if5True, te_type if6, te_type if6True, te_type if7,
                          te_type if7True, te_type if8, te_type if8True, te_type if9,
                          te_type if9True, te_type if10, te_type if10True, te_type if11,
                          te_type if11True, te_type if12, te_type if12True)
        {
        return te_parser::number_to_bool(if1)  ? if1True :
               te_parser::number_to_bool(if2)  ? if2True :
               te_parser::number_to_bool(if3)  ? if3True :
               te_parser::number_to_bool(if4)  ? if4True :
               te_parser::number_to_bool(if5)  ? if5True :
               te_parser::number_to_bool(if6)  ? if6True :
               te_parser::number_to_bool(if7)  ? if7True :
               te_parser::number_to_bool(if8)  ? if8True :
               te_parser::number_to_bool(if9)  ? if9True :
               te_parser::number_to_bool(if10) ? if10True :
               te_parser::number_to_bool(if11) ? if11True :
               te_parser::number_to_bool(if12) ? if12True :
                                                 te_parser::te_nan;
        }

    [[nodiscard]]
    constexpr static te_type te_supports_32bit() noexcept
        {
        return te_parser::supports_32bit() ? te_true_value() : te_false_value();
        }

    [[nodiscard]]
    constexpr static te_type te_supports_64bit() noexcept
        {
        return te_parser::supports_64bit() ? te_true_value() : te_false_value();
        }

    // cotangent
    [[nodiscard]]
    static te_type te_cot(te_type val) noexcept
        {
        if (val == 0.0)
            {
            return te_parser::te_nan;
            }
        return 1 / static_cast<te_type>(std::tan(val));
        }

    [[nodiscard]]
    constexpr static te_type te_sign(te_type val) noexcept
        {
        return static_cast<te_type>((val < 0.0) ? -1 : (val > 0.0) ? 1 : 0);
        }

    [[nodiscard]]
    constexpr static te_type te_negate(te_type val) noexcept
        {
        return -val;
        }

    [[nodiscard]]
    constexpr static te_type te_comma([[maybe_unused]] te_type unusedVal, // NOLINT
                                      te_type val2) noexcept
        {
        return val2;
        }
    } // namespace te_builtins

//--------------------------------------------------
void te_parser::te_free_parameters(te_expr* texp)
    {
    if (texp == nullptr)
        {
        return;
        }
    if (is_closure(texp->m_value))
        {
        // last param is the context object, we don't manage that here
        for (auto param = texp->m_parameters.begin(); param != texp->m_parameters.end() - 1;
             ++param)
            {
            te_free(*param);
            *param = nullptr;
            }
        }
    else if (is_function(texp->m_value))
        {
        for (auto* param : texp->m_parameters)
            {
            te_free(param);
            param = nullptr;
            }
        }
    }

//--------------------------------------------------
const std::set<te_variable> te_parser::m_functions = { // NOLINT
    { "abs", static_cast<te_fun1>(te_builtins::te_absolute_value), TE_PURE },
    { "acos", static_cast<te_fun1>(te_builtins::te_acos), TE_PURE },
    // variadic, accepts 1-24 arguments
    { "and", static_cast<te_fun24>(te_builtins::te_and_variadic),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "asin", static_cast<te_fun1>(te_builtins::te_asin), TE_PURE },
    { "atan", static_cast<te_fun1>(te_builtins::te_atan), TE_PURE },
    { "atan2", static_cast<te_fun2>(te_builtins::te_atan2), TE_PURE },
    { "average", static_cast<te_fun24>(te_builtins::te_average),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
#ifndef TE_FLOAT
    { "bitand", static_cast<te_fun2>(te_builtins::te_bitwise_and), TE_PURE },
    { "bitor", static_cast<te_fun2>(te_builtins::te_bitwise_or), TE_PURE },
    #if __cplusplus >= 202002L
    { "bitlrotate8", static_cast<te_fun2>(te_builtins::te_left_rotate8), TE_PURE },
    { "bitrrotate8", static_cast<te_fun2>(te_builtins::te_right_rotate8), TE_PURE },
    { "bitlrotate16", static_cast<te_fun2>(te_builtins::te_left_rotate16), TE_PURE },
    { "bitrrotate16", static_cast<te_fun2>(te_builtins::te_right_rotate16), TE_PURE },
    { "bitlrotate32", static_cast<te_fun2>(te_builtins::te_left_rotate32), TE_PURE },
    { "bitrrotate32", static_cast<te_fun2>(te_builtins::te_right_rotate32), TE_PURE },
    { "bitlrotate64", static_cast<te_fun2>(te_builtins::te_left_rotate64), TE_PURE },
    { "bitrrotate64", static_cast<te_fun2>(te_builtins::te_right_rotate64), TE_PURE },
    { "bitlrotate", static_cast<te_fun2>(te_builtins::te_left_rotate), TE_PURE },
    { "bitrrotate", static_cast<te_fun2>(te_builtins::te_right_rotate), TE_PURE },
    #endif
    { "bitnot8", static_cast<te_fun1>(te_builtins::te_bitwise_not8), TE_PURE },
    { "bitnot16", static_cast<te_fun1>(te_builtins::te_bitwise_not16), TE_PURE },
    { "bitnot32", static_cast<te_fun1>(te_builtins::te_bitwise_not32), TE_PURE },
    { "bitnot64", static_cast<te_fun1>(te_builtins::te_bitwise_not64), TE_PURE },
    { "bitnot", static_cast<te_fun1>(te_builtins::te_bitwise_not), TE_PURE },
    { "bitlshift", static_cast<te_fun2>(te_builtins::te_left_shift_or_right), TE_PURE },
    { "bitrshift", static_cast<te_fun2>(te_builtins::te_right_shift_or_left), TE_PURE },
    { "bitxor", static_cast<te_fun2>(te_builtins::te_bitwise_xor), TE_PURE },
#endif
    { "ceil", static_cast<te_fun1>(te_builtins::te_ceil), TE_PURE },
    { "clamp",
      static_cast<te_fun3>(
          [](const te_type num, const te_type start, const te_type end) // NOLINT
          {
              return (start <= end) ? std::clamp<te_type>(num, start, end) :
                                      std::clamp<te_type>(num, end, start);
          }),
      TE_PURE },
    { "combin", static_cast<te_fun2>(te_builtins::te_ncr), TE_PURE },
    { "cos", static_cast<te_fun1>(te_builtins::te_cos), TE_PURE },
    { "cosh", static_cast<te_fun1>(te_builtins::te_cosh), TE_PURE },
    { "cot", static_cast<te_fun1>(te_builtins::te_cot), TE_PURE },
    { "cumipmt", static_cast<te_fun6>(te_builtins::te_cumipmt),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "cumprinc", static_cast<te_fun6>(te_builtins::te_cumprinc),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "db", static_cast<te_fun5>(te_builtins::te_asset_depreciation),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "e", static_cast<te_fun0>(te_builtins::te_e), TE_PURE },
    { "effect", static_cast<te_fun2>(te_builtins::te_effect), TE_PURE },
    { "even", static_cast<te_fun1>(te_builtins::te_even), TE_PURE },
    { "exp", static_cast<te_fun1>(te_builtins::te_exp), TE_PURE },
    { "fac", static_cast<te_fun1>(te_builtins::te_fac), TE_PURE },
    { "fact", static_cast<te_fun1>(te_builtins::te_fac), TE_PURE },
    { "false", static_cast<te_fun0>(te_builtins::te_false_value), TE_PURE },
    { "floor", static_cast<te_fun1>(te_builtins::te_floor), TE_PURE },
    { "fv", static_cast<te_fun5>(te_builtins::te_fv),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "iserr", static_cast<te_fun1>(te_builtins::te_is_nan), TE_PURE },
    { "iserror", static_cast<te_fun1>(te_builtins::te_is_nan), TE_PURE },
    { "iseven", static_cast<te_fun1>(te_builtins::te_is_even), TE_PURE },
    { "isna", static_cast<te_fun1>(te_builtins::te_is_nan), TE_PURE },
    { "isnan", static_cast<te_fun1>(te_builtins::te_is_nan), TE_PURE },
    { "isodd", static_cast<te_fun1>(te_builtins::te_is_odd), TE_PURE },
    { "if", static_cast<te_fun3>(te_builtins::te_if), TE_PURE },
    { "ifs", static_cast<te_fun24>(te_builtins::te_ifs),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "ipmt", static_cast<te_fun6>(te_builtins::te_ipmt),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "ln", static_cast<te_fun1>(te_builtins::te_log), TE_PURE },
    { "log10", static_cast<te_fun1>(te_builtins::te_log10), TE_PURE },
    { "max", static_cast<te_fun24>(te_builtins::te_max),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "maxint", static_cast<te_fun0>(te_builtins::te_max_integer), TE_PURE },
    { "min", static_cast<te_fun24>(te_builtins::te_min),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "mod", static_cast<te_fun2>(te_builtins::te_modulus), TE_PURE },
    { "na", static_cast<te_fun0>(te_builtins::te_nan_value), TE_PURE },
    { "nan", static_cast<te_fun0>(te_builtins::te_nan_value), TE_PURE },
    { "ncr", static_cast<te_fun2>(te_builtins::te_ncr), TE_PURE },
    { "nominal", static_cast<te_fun2>(te_builtins::te_nominal), TE_PURE },
    { "not", static_cast<te_fun1>(te_builtins::te_not), TE_PURE },
    { "nper", static_cast<te_fun5>(te_builtins::te_nper),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "npr", static_cast<te_fun2>(te_builtins::te_npr), TE_PURE },
    { "odd", static_cast<te_fun1>(te_builtins::te_odd), TE_PURE },
    { "or", static_cast<te_fun24>(te_builtins::te_or_variadic),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "permut", static_cast<te_fun2>(te_builtins::te_npr), TE_PURE },
    { "pi", static_cast<te_fun0>(te_builtins::te_pi), TE_PURE },
    { "pmt", static_cast<te_fun5>(te_builtins::te_pmt),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "pow", static_cast<te_fun2>(te_builtins::te_pow), TE_PURE },
    { "power", /* Excel alias*/ static_cast<te_fun2>(te_builtins::te_pow), TE_PURE },
    { "ppmt", static_cast<te_fun6>(te_builtins::te_ppmt),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "pv", static_cast<te_fun5>(te_builtins::te_pv),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "rand", static_cast<te_fun0>(te_builtins::te_random), TE_PURE },
    { "round", static_cast<te_fun2>(te_builtins::te_round),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "sign", static_cast<te_fun1>(te_builtins::te_sign), TE_PURE },
    { "sin", static_cast<te_fun1>(te_builtins::te_sin), TE_PURE },
    { "sinh", static_cast<te_fun1>(te_builtins::te_sinh), TE_PURE },
    { "sqr", static_cast<te_fun1>(te_builtins::te_sqr), TE_PURE },
    { "sqrt", static_cast<te_fun1>(te_builtins::te_sqrt), TE_PURE },
    { "sum", static_cast<te_fun24>(te_builtins::te_sum),
      static_cast<te_variable_flags>(TE_PURE | TE_VARIADIC) },
    { "supports32bit", static_cast<te_fun0>(te_builtins::te_supports_32bit), TE_PURE },
    { "supports64bit", static_cast<te_fun0>(te_builtins::te_supports_64bit), TE_PURE },
    { "tan", static_cast<te_fun1>(te_builtins::te_tan), TE_PURE },
    { "tanh", static_cast<te_fun1>(te_builtins::te_tanh), TE_PURE },
    { "tgamma", static_cast<te_fun1>(te_builtins::te_tgamma), TE_PURE },
    { "true", static_cast<te_fun0>(te_builtins::te_true_value), TE_PURE },
    { "trunc", static_cast<te_fun1>(te_builtins::te_trunc), TE_PURE }
};

//--------------------------------------------------
void te_parser::next_token(state* theState)
    {
    assert(theState);
    if (theState == nullptr)
        {
        return;
        }

    theState->m_type = state::token_type::TOK_NULL;

    do // NOLINT
        {
        if (*theState->m_next == 0)
            {
            theState->m_type = state::token_type::TOK_END;
            return;
            }

        /* Try reading a number. */
        if (((*theState->m_next >= '0') && (*theState->m_next <= '9')) ||
            (*theState->m_next == get_decimal_separator()))
            {
            char* nEnd{ nullptr };
#ifdef TE_FLOAT
            theState->m_value = static_cast<te_type>(std::strtof(theState->m_next, &nEnd));
#elif defined(TE_LONG_DOUBLE)
            theState->m_value = static_cast<te_type>(std::strtold(theState->m_next, &nEnd));
#else
            theState->m_value = static_cast<te_type>(std::strtod(theState->m_next, &nEnd));
#endif
            theState->m_next = nEnd;
            theState->m_type = state::token_type::TOK_NUMBER;
            }
        else
            {
            /* Look for a variable or builtin function call. */
            if (is_letter(*theState->m_next) || (*theState->m_next == '_'))
                {
                const char* start = theState->m_next;
                while (is_name_char_valid(*theState->m_next))
                    {
                    std::advance(theState->m_next, 1);
                    }

                m_varFound = false;
                const std::string_view currentVarToken{ start, static_cast<std::string::size_type>(
                                                                   theState->m_next - start) };
                m_currentVar = find_lookup(theState, currentVarToken);
                if (m_currentVar != theState->m_lookup.cend())
                    {
                    m_varFound = true;
                    }
                else
                    {
                    m_currentVar = find_builtin(currentVarToken);
                    if (m_currentVar != m_functions.cend())
                        {
                        m_varFound = true;
                        }
                    // if unknown symbol resolve is not a no-op, then try using it
                    // to see what this variable is
                    else if (m_unknownSymbolResolve.index() != 0)
                        {
                        try
                            {
                            // "te_type usr(string_view)" resolver
                            if (m_unknownSymbolResolve.index() == 1)
                                {
                                const auto retUsrVal =
                                    std::get<1>(m_unknownSymbolResolve)(currentVarToken);
                                if (std::isfinite(retUsrVal))
                                    {
                                    add_variable_or_function(
                                        { te_variable::name_type{ currentVarToken }, retUsrVal });
                                    m_currentVar = find_lookup(theState, currentVarToken);
                                    assert(
                                        m_currentVar != theState->m_lookup.cend() &&
                                        "Internal error in parser using unknown symbol resolver.");
                                    if (m_currentVar != theState->m_lookup.cend())
                                        {
                                        m_resolvedVariables.insert(
                                            te_variable::name_type{ currentVarToken });
                                        m_varFound = true;
                                        }
                                    }
                                }
                            // "te_type usr(string_view, string&)" resolver
                            else if (m_unknownSymbolResolve.index() == 2)
                                {
                                const auto retUsrVal = std::get<2>(m_unknownSymbolResolve)(
                                    currentVarToken, m_lastErrorMessage);
                                if (std::isfinite(retUsrVal))
                                    {
                                    add_variable_or_function(
                                        { te_variable::name_type{ currentVarToken }, retUsrVal });
                                    m_currentVar = find_lookup(theState, currentVarToken);
                                    assert(
                                        m_currentVar != theState->m_lookup.cend() &&
                                        "Internal error in parser using unknown symbol resolver.");
                                    if (m_currentVar != theState->m_lookup.cend())
                                        {
                                        m_resolvedVariables.insert(
                                            te_variable::name_type{ currentVarToken });
                                        m_varFound = true;
                                        }
                                    }
                                }
                            }
                        catch (const std::exception& exp)
                            {
                            m_lastErrorMessage = exp.what();
                            }
                        }
                    }

                if (!m_varFound)
                    {
                    theState->m_type = state::token_type::TOK_ERROR;
                    }
                else
                    {
#ifndef TE_NO_BOOKKEEPING
                    // keep track of what's been used in the formula
                    if (is_function(m_currentVar->m_value) || is_closure(m_currentVar->m_value))
                        {
                        m_usedFunctions.insert(m_currentVar->m_name);
                        }
                    else
                        {
                        m_usedVars.insert(m_currentVar->m_name);
                        }
#endif

                    if (is_constant(m_currentVar->m_value))
                        {
                        theState->m_type = state::token_type::TOK_NUMBER;
                        theState->m_value = m_currentVar->m_value;
                        }
                    else if (is_variable(m_currentVar->m_value))
                        {
                        theState->m_type = state::token_type::TOK_VARIABLE;
                        theState->m_value = m_currentVar->m_value;
                        }
                    else if (is_function(m_currentVar->m_value))
                        {
                        theState->m_type = state::token_type::TOK_FUNCTION;
                        theState->m_varType = m_currentVar->m_type;
                        theState->m_value = m_currentVar->m_value;
                        }
                    else if (is_closure(m_currentVar->m_value))
                        {
                        theState->context = m_currentVar->m_context;
                        theState->m_type = state::token_type::TOK_FUNCTION;
                        theState->m_varType = m_currentVar->m_type;
                        theState->m_value = m_currentVar->m_value;
                        }
                    }
                }
            else
                {
                /* Look for an operator or special character. */
                const auto tok = *theState->m_next;
                std::advance(theState->m_next, 1);
                if (tok == '+')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = te_builtins::te_add;
                    }
                else if (tok == '-')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = te_builtins::te_sub;
                    }
#ifndef TE_FLOAT
                else if (tok == '~')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = te_builtins::te_bitwise_not;
                    }
#else
                else if (tok == '~')
                    {
                    theState->m_type = state::token_type::TOK_ERROR;
                    }
#endif
                else if (tok == '*' && (*theState->m_next == '*'))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_pow);
                    std::advance(theState->m_next, 1);
                    }
                else if (tok == '*')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = te_builtins::te_mul;
                    }
                else if (tok == '/')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = te_builtins::te_divide;
                    }
#if defined(TE_BITWISE_OPERATORS) && !defined(TE_FLOAT)
                else if (tok == '^')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_bitwise_xor);
                    }
#else
                else if (tok == '^')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_pow);
                    }
#endif
                else if (tok == '%')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = te_builtins::te_modulus;
                    }
#ifdef TE_BRACKETS_AS_PARENS
                else if (tok == '(' || tok == '[')
#else
                else if (tok == '(')
#endif
                    {
                    theState->m_type = state::token_type::TOK_OPEN;
                    }
#ifdef TE_BRACKETS_AS_PARENS
                else if (tok == ')' || tok == ']')
#else
                else if (tok == ')')
#endif
                    {
                    theState->m_type = state::token_type::TOK_CLOSE;
                    }
                else if (tok == get_list_separator())
                    {
                    theState->m_type = state::token_type::TOK_SEP;
                    }
#if __cplusplus >= 202002L && !defined(TE_FLOAT)
                // rotate (circular shift) operators (uses the 64-bit integer version)
                else if (tok == '<' && (*theState->m_next == '<') &&
                         (*std::next(theState->m_next) == '<'))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_left_rotate);
                    std::advance(theState->m_next, 2);
                    }
                else if (tok == '>' && (*theState->m_next == '>') &&
                         (*std::next(theState->m_next) == '>'))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_right_rotate);
                    std::advance(theState->m_next, 2);
                    }
#else
                // if rotation is not available, error if used in formula
                else if ((tok == '<' && (*theState->m_next == '<') &&
                          (*std::next(theState->m_next) == '<')) ||
                         (tok == '>' && (*theState->m_next == '>') &&
                          (*std::next(theState->m_next) == '>')))
                    {
                    theState->m_type = state::token_type::TOK_ERROR;
                    }
#endif
#ifndef TE_FLOAT
                // shift operators
                else if (tok == '<' && (*theState->m_next == '<'))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_left_shift);
                    std::advance(theState->m_next, 1);
                    }
                else if (tok == '>' && (*theState->m_next == '>'))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_right_shift);
                    std::advance(theState->m_next, 1);
                    }
#else
                // shift operators that will be disabled for float types
                else if ((tok == '<' && (*theState->m_next == '<')) ||
                         (tok == '>' && (*theState->m_next == '>')))
                    {
                    theState->m_type = state::token_type::TOK_ERROR;
                    }
#endif
                // logical operators
                else if (tok == '=' && (*theState->m_next == '='))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_equal);
                    std::advance(theState->m_next, 1);
                    }
                else if (tok == '=')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_equal);
                    }
                else if (tok == '!' && (*theState->m_next == '=')) // NOLINT
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_not_equal);
                    std::advance(theState->m_next, 1);
                    }
                else if (tok == '<' && (*theState->m_next == '>'))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_not_equal);
                    std::advance(theState->m_next, 1);
                    }
                else if (tok == '<' && (*theState->m_next == '='))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_less_than_equal_to);
                    std::advance(theState->m_next, 1);
                    }
                else if (tok == '<')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_less_than);
                    }
                else if (tok == '>' && (*theState->m_next == '='))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_greater_than_equal_to);
                    std::advance(theState->m_next, 1);
                    }
                else if (tok == '>')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_greater_than);
                    }
                else if (tok == '&' && (*theState->m_next == '&'))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_and);
                    std::advance(theState->m_next, 1);
                    }
#if defined(TE_BITWISE_OPERATORS) && !defined(TE_FLOAT)
                else if (tok == '&')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_bitwise_and);
                    }
#else
                else if (tok == '&')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_and);
                    }
#endif
                else if (tok == '|' && (*theState->m_next == '|'))
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_or);
                    std::advance(theState->m_next, 1);
                    }
#if defined(TE_BITWISE_OPERATORS) && !defined(TE_FLOAT)
                else if (tok == '|')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_bitwise_or);
                    }
#else
                else if (tok == '|')
                    {
                    theState->m_type = state::token_type::TOK_INFIX;
                    theState->m_value = static_cast<te_fun2>(te_builtins::te_or);
                    }
#endif
                else if (tok == ' ' || tok == '\t' || tok == '\n' || tok == '\r')
                    { /*noop*/
                    }
                else
                    {
                    theState->m_type = state::token_type::TOK_ERROR;
                    }
                }
            }
        } while (theState->m_type == state::token_type::TOK_NULL);
    }

//--------------------------------------------------
te_expr* te_parser::base(state* theState)
    {
    /* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> |
                        <function-X> "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
    te_expr* ret{ nullptr };

    if (theState->m_type == state::token_type::TOK_OPEN)
        {
        next_token(theState);
        ret = list(theState);
        if (theState->m_type != state::token_type::TOK_CLOSE)
            {
            theState->m_type = state::token_type::TOK_ERROR;
            }
        else
            {
            next_token(theState);
            }
        }
    else if (theState->m_type == state::token_type::TOK_NUMBER)
        {
        ret = new_expr(TE_DEFAULT, theState->m_value);
        next_token(theState);
        }
    else if (theState->m_type == state::token_type::TOK_VARIABLE)
        {
        ret = new_expr(TE_DEFAULT, theState->m_value);
        next_token(theState);
        }
    else if (theState->m_type == state::token_type::TOK_NULL ||
             theState->m_type == state::token_type::TOK_ERROR ||
             theState->m_type == state::token_type::TOK_END ||
             theState->m_type == state::token_type::TOK_SEP ||
             theState->m_type == state::token_type::TOK_CLOSE ||
             theState->m_type == state::token_type::TOK_INFIX)
        {
        ret = new_expr(TE_DEFAULT, te_variant_type{ te_nan });
        theState->m_type = state::token_type::TOK_ERROR;
        }
    else if (is_function0(theState->m_value) || is_closure0(theState->m_value))
        {
        ret = new_expr(theState->m_varType, theState->m_value, {});
        if (is_closure(theState->m_value))
            {
            ret->m_parameters[0] = theState->context;
            }
        next_token(theState);
        if (theState->m_type == state::token_type::TOK_OPEN)
            {
            next_token(theState);
            if (theState->m_type != state::token_type::TOK_CLOSE)
                {
                theState->m_type = state::token_type::TOK_ERROR;
                }
            else
                {
                next_token(theState);
                }
            }
        }
    else if (is_function1(theState->m_value) || is_closure1(theState->m_value))
        {
        ret = new_expr(theState->m_varType, theState->m_value);
        if (is_closure(theState->m_value))
            {
            ret->m_parameters[1] = theState->context;
            }
        next_token(theState);
        ret->m_parameters[0] = power(theState);
        }
    else if (is_function(theState->m_value) || is_closure(theState->m_value))
        {
        const auto arity = get_arity(theState->m_value);

        ret = new_expr(theState->m_varType, theState->m_value);
        if (is_closure(theState->m_value))
            {
            ret->m_parameters[arity] = theState->context;
            }
        next_token(theState);

        if (theState->m_type != state::token_type::TOK_OPEN)
            {
            theState->m_type = state::token_type::TOK_ERROR;
            }
        else
            {
            // If there are vars or other functions in the parameters, keep track of the original
            // opening function; that is what we will do our variadic check on.
            const bool varValid{ m_varFound };
            const std::set<te_variable>::const_iterator openingVar = m_currentVar;
            // load any parameters
            std::decay<decltype(arity)>::type i{ 0 }; // NOLINT
            for (i = 0; i < arity; i++)
                {
                next_token(theState);
                ret->m_parameters[i] = expr_level1(theState);
                if (theState->m_type != state::token_type::TOK_SEP)
                    {
                    break;
                    }
                }
            if (theState->m_type == state::token_type::TOK_CLOSE && (i != arity - 1) && varValid &&
                is_variadic(openingVar->m_type))
                {
                next_token(theState);
                }
            else if (theState->m_type != state::token_type::TOK_CLOSE || (i != arity - 1))
                {
                theState->m_type = state::token_type::TOK_ERROR;
                }
            else
                {
                next_token(theState);
                }
            }
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::list(state* theState)
    {
    /* <list>      =    <expr> {"," <expr>} */
    te_expr* ret = expr_level1(theState);

    while (theState->m_type == state::token_type::TOK_SEP)
        {
        next_token(theState);
        ret = new_expr(TE_PURE, te_variant_type(te_builtins::te_comma),
                       { ret, expr_level1(theState) });
        }

    return ret;
    }

// Operator precedence, lowest to highest:
//--------------------------------------------------
te_expr* te_parser::expr_level1(state* theState)
    {
    /* <expr>      =    <term> {(logic operations) <term>} */
    // These are the lowest of operator precedence
    // (once we have split tokens into arguments)
    te_expr* ret = expr_level2(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           get_function2(theState->m_value) == te_builtins::te_or)
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, expr_level2(theState) });
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::expr_level2(state* theState)
    {
    /* <expr>      =    <term> {(logic operations) <term>} */
    // next to lowest in precedence...
    te_expr* ret = expr_level3(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           get_function2(theState->m_value) == te_builtins::te_and)
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, expr_level3(theState) });
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::expr_level3(state* theState)
    {
    /* <expr>      =    <term> {(logic operations) <term>} */
    // next to lowest in precedence...
    te_expr* ret = expr_level4(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           get_function2(theState->m_value) == te_builtins::te_bitwise_or)
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, expr_level4(theState) });
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::expr_level4(state* theState)
    {
    /* <expr>      =    <term> {(logic operations) <term>} */
    // next to lowest in precedence...
    te_expr* ret = expr_level5(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           get_function2(theState->m_value) == te_builtins::te_bitwise_xor)
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, expr_level5(theState) });
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::expr_level5(state* theState)
    {
    /* <expr>      =    <term> {(logic operations) <term>} */
    // next to lowest in precedence...
    te_expr* ret = expr_level6(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           get_function2(theState->m_value) == te_builtins::te_bitwise_and)
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, expr_level6(theState) });
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::expr_level6(state* theState)
    {
    /* <expr>      =    <term> {(logic operations) <term>} */
    // next to lowest in precedence...
    te_expr* ret = expr_level7(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           (get_function2(theState->m_value) == te_builtins::te_equal ||
            get_function2(theState->m_value) == te_builtins::te_not_equal))
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, expr_level7(theState) });
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::expr_level7(state* theState)
    {
    /* <expr>      =    <term> {(comparison operators) <term>} */
    te_expr* ret = expr_level8(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           (get_function2(theState->m_value) == te_builtins::te_less_than ||
            get_function2(theState->m_value) == te_builtins::te_less_than_equal_to ||
            get_function2(theState->m_value) == te_builtins::te_greater_than ||
            get_function2(theState->m_value) == te_builtins::te_greater_than_equal_to))
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, expr_level8(theState) });
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::expr_level8(state* theState)
    {
    /* <expr>      =    <term> {("<<" | ">>") <term>} */
    te_expr* ret = expr_level9(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           (get_function2(theState->m_value) == te_builtins::te_left_shift ||
            get_function2(theState->m_value) == te_builtins::te_right_shift
#if __cplusplus >= 202002L && !defined(TE_FLOAT)
            || get_function2(theState->m_value) == te_builtins::te_left_rotate ||
            get_function2(theState->m_value) == te_builtins::te_right_rotate ||
            get_function2(theState->m_value) == te_builtins::te_left_rotate32 ||
            get_function2(theState->m_value) == te_builtins::te_right_rotate32 ||
            get_function2(theState->m_value) == te_builtins::te_left_rotate16 ||
            get_function2(theState->m_value) == te_builtins::te_right_rotate16 ||
            get_function2(theState->m_value) == te_builtins::te_left_rotate8 ||
            get_function2(theState->m_value) == te_builtins::te_right_rotate8
#endif
            ))
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, expr_level9(theState) });
        }

    return ret;
    }

//--------------------------------------------------
te_expr* te_parser::expr_level9(state* theState)
    {
    /* <expr>      =    <term> {("+" | "-") <term>} */
    te_expr* ret = term(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           (get_function2(theState->m_value) == te_builtins::te_add ||
            get_function2(theState->m_value) == te_builtins::te_sub))
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, term(theState) });
        }

    return ret;
    }

// Higher levels of operator precedence:
//--------------------------------------------------
te_expr* te_parser::term(state* theState)
    {
    /* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
    // third from the highest level of operator precedence
    te_expr* ret = factor(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           (get_function2(theState->m_value) == te_builtins::te_mul ||
            get_function2(theState->m_value) == te_builtins::te_divide ||
            get_function2(theState->m_value) == te_builtins::te_modulus))
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, factor(theState) });
        }

    return ret;
    }

//--------------------------------------------------
#ifdef TE_POW_FROM_RIGHT
te_expr* te_parser::factor(te_parser::state* theState)
    {
    /* <factor>    =    <power> {"^" <power>} */
    // second from the highest level of operator precedence
    te_expr* ret = power(theState);

    int neg{ 0 };

    if (ret->m_type == TE_PURE && is_function1(ret->m_value) &&
        get_function1(ret->m_value) == te_builtins::te_negate)
        {
        te_expr* se = ret->m_parameters[0];
        delete ret;
        ret = se;
        neg = 1;
        }

    te_expr* insertion{ nullptr };
    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           (get_function2(theState->m_value) == static_cast<te_fun2>(te_builtins::te_pow)))
        {
        const te_fun2 t = get_function2(theState->m_value);
        next_token(theState);

        if (insertion)
            {
            /* Make exponentiation go right-to-left. */
            te_expr* insert = new_expr(TE_PURE, t, { insertion->m_parameters[1], power(theState) });
            insertion->m_parameters[1] = insert;
            insertion = insert;
            }
        else
            {
            ret = new_expr(TE_PURE, t, { ret, power(theState) });
            insertion = ret;
            }
        }

    if (neg)
        {
        ret = new_expr(TE_PURE, te_variant_type(te_builtins::te_negate), { ret });
        }

    return ret;
    }
#else
te_expr* te_parser::factor(state* theState)
    {
    /* <factor>    =    <power> {"^" <power>} */
    // second from the highest level of operator precedence
    te_expr* ret = power(theState);

    while (theState->m_type == state::token_type::TOK_INFIX && is_function2(theState->m_value) &&
           (get_function2(theState->m_value) == static_cast<te_fun2>(te_builtins::te_pow)))
        {
        const te_fun2 func = get_function2(theState->m_value);
        next_token(theState);
        ret = new_expr(TE_PURE, func, { ret, power(theState) });
        }

    return ret;
    }
#endif

//--------------------------------------------------
te_expr* te_parser::power(state* theState)
    {
    /* <power>     =    {("-" | "+")} <base> */
    // highest level of operator precedence
    int theSign{ 1 };
    bool bitwiseNot{ false };
    while (theState->m_type == state::token_type::TOK_INFIX &&
           ((is_function2(theState->m_value) &&
             (get_function2(theState->m_value) == te_builtins::te_add ||
              get_function2(theState->m_value) == te_builtins::te_sub))
#ifndef TE_FLOAT
            || (is_function1(theState->m_value) &&
                (get_function1(theState->m_value) == te_builtins::te_bitwise_not))
#endif
                ))
        {
        if (is_function2(theState->m_value) &&
            get_function2(theState->m_value) == te_builtins::te_sub)
            {
            theSign = -theSign;
            }
        else if (is_function1(theState->m_value) &&
                 get_function1(theState->m_value) == te_builtins::te_bitwise_not)
            {
            bitwiseNot = true;
            }
        next_token(theState);
        }

    te_expr* ret{ nullptr };

    if (bitwiseNot)
        {
        ret = new_expr(TE_PURE, te_variant_type(te_builtins::te_bitwise_not), { base(theState) });
        }
    else if (theSign == -1)
        {
        ret = new_expr(TE_PURE, te_variant_type(te_builtins::te_negate), { base(theState) });
        }
    else
        {
        ret = base(theState);
        }

    return ret;
    }

//--------------------------------------------------
// tuple-list-maker
template<typename F, size_t... Indices>
auto make_closure_arg_list(const F& func, te_expr* ctx, std::index_sequence<Indices...>)
    {
    return std::make_tuple(ctx, func(Indices)...);
    }

template<typename F, size_t... Indices>
auto make_function_arg_list(const F& func, std::index_sequence<Indices...>)
    {
    return std::make_tuple(func(Indices)...);
    }

te_type te_parser::te_eval(const te_expr* texp)
    {
    if (texp == nullptr)
        {
        return te_nan;
        }

    // NOLINTBEGIN
    const auto M = [&texp = std::as_const(texp)](const size_t e)
    { return (e < texp->m_parameters.size()) ? te_eval(texp->m_parameters[e]) : te_nan; };

    return std::visit(
        [&, texp]<typename T0>(const T0& var) -> te_type
        {
            using T = std::decay_t<T0>;
            if constexpr (te_is_constant_v<T>)
                {
                return var;
                }
            if constexpr (te_is_variable_v<T>)
                {
                return *var;
                }
            if constexpr (std::is_same_v<T, te_fun0>)
                {
                return var();
                }
            if constexpr (std::is_same_v<T, te_confun0>)
                {
                return var(texp->m_parameters[0]);
                }
            if constexpr (te_is_closure_v<T>)
                {
                constexpr size_t n_args = te_function_arity<T>;
                static_assert(n_args > 0);
                return std::apply(var,
                                  make_closure_arg_list(M, texp->m_parameters[n_args - 1],
                                                        std::make_index_sequence<n_args - 1>{}));
                }
            if constexpr (te_is_function_v<T>)
                {
                constexpr size_t n_args = te_function_arity<T>;
                return std::apply(var,
                                  make_function_arg_list(M, std::make_index_sequence<n_args>{}));
                }
            return te_nan;
        },
        texp->m_value);
    // NOLINTEND
    }

#pragma warning(pop)

//--------------------------------------------------
void te_parser::optimize(te_expr* texp)
    {
    if (texp == nullptr)
        {
        return;
        }
    /* Evaluates as much as possible. */
    if (is_constant(texp->m_value) || is_variable(texp->m_value))
        {
        return;
        }

    /* Only optimize out functions flagged as pure. */
    if (is_pure(texp->m_type))
        {
        const auto arity = get_arity(texp->m_value);
        bool known{ true };
        for (std::decay_t<decltype(arity)> i = 0; i < arity; ++i)
            {
            if (texp->m_parameters[i] == nullptr)
                {
                break;
                }
            optimize(texp->m_parameters[i]);
            if (!is_constant(texp->m_parameters[i]->m_value))
                {
                known = false;
                }
            }
        if (known)
            {
            const auto value = te_eval(texp);
            te_free_parameters(texp);
            texp->m_type = TE_DEFAULT;
            texp->m_value = value;
            }
        }
    }

//--------------------------------------------------
te_expr* te_parser::te_compile(const std::string_view expression, std::set<te_variable>& variables)
    {
    state theState(expression.data(), TE_DEFAULT, variables);

    next_token(&theState);
    te_expr* root = list(&theState);

    if (theState.m_type != state::token_type::TOK_END)
        {
        // if a parse error, clean up
        te_free(root);
        m_errorPos = (theState.m_next - theState.m_start);
        if (m_errorPos > 0)
            {
            --m_errorPos;
            }
        return nullptr;
        }

    try
        {
        optimize(root);
        }
    catch ([[maybe_unused]]
           const std::exception& exp)
        {
        // parsed OK, but there was an evaluation error;
        // clean up and throw the message back up to compile()
        te_free(root);
        throw;
        }
    m_errorPos = te_parser::npos;
    return root;
    }

//--------------------------------------------------
bool te_parser::compile(const std::string_view expression)
    {
    reset_state();
    if (get_list_separator() == get_decimal_separator())
        {
        throw std::runtime_error("List and decimal separators cannot be the same.");
        }
    if (expression.empty())
        {
        m_expression.clear();
        m_errorPos = 0;
        return false;
        }
    m_expression.assign(expression);

    // In case the expression was a spreadsheet formula like "=SUM(...)",
    // remove the '=' in front.
    if (!m_expression.empty() && m_expression.front() == '=')
        {
        m_expression.erase(0, 1);
        }

    size_t commentStart{ 0 };
    while (commentStart != std::string::npos)
        {
        commentStart = m_expression.find('/', commentStart);
        if (commentStart == std::string::npos || commentStart == m_expression.length() - 1)
            {
            break;
            }
        // remove multi-line comments
        if (m_expression[commentStart + 1] == '*')
            {
            const auto commentEnd = m_expression.find("*/", commentStart);
            if (commentEnd == std::string::npos)
                {
                m_errorPos = static_cast<decltype(m_errorPos)>(commentStart);
                m_parseSuccess = false;
                m_result = te_nan;
                return false;
                }
            m_expression.erase(commentStart, (commentEnd + 2) - commentStart);
            }
        // remove single-line comments
        else if (m_expression[commentStart + 1] == '/')
            {
            const auto commentEnd = m_expression.find_first_of("\n\r", commentStart);
            if (commentEnd == std::string::npos)
                {
                m_expression.erase(commentStart);
                break;
                }
            m_expression.erase(commentStart, commentEnd - commentStart);
            }
        else
            {
            ++commentStart;
            }
        }

    try
        {
        m_compiledExpression = te_compile(m_expression, get_variables_and_functions());
        m_parseSuccess = (m_compiledExpression != nullptr);
        }
    catch (const std::exception& expt)
        {
        m_parseSuccess = false;
        m_result = te_nan;
        m_lastErrorMessage = expt.what();
        // not a syntax error in the expression, something threw a math error
        m_errorPos = te_parser::npos;
        }

    reset_usr_resolved_if_necessary();

    return m_parseSuccess;
    }

//--------------------------------------------------
te_type te_parser::evaluate()
    {
    try
        {
        if (m_expression.empty())
            {
            m_parseSuccess = false;
            m_errorPos = 0;
            m_lastErrorMessage = "Expression is empty.";
            }
        m_result = (m_compiledExpression != nullptr) ? te_eval(m_compiledExpression) : te_nan;
        }
    catch (const std::exception& exp)
        {
        m_parseSuccess = false;
        m_result = te_nan;
        m_lastErrorMessage = exp.what();
        }

    reset_usr_resolved_if_necessary();

    return m_result;
    }

//--------------------------------------------------
te_type
te_parser::evaluate(const std::string_view expression) // NOLINT(-readability-identifier-naming)
    {
    if (compile(expression))
        {
        return evaluate();
        }

    if (expression.empty())
        {
        m_parseSuccess = false;
        m_errorPos = 0;
        m_lastErrorMessage = "Expression is empty.";
        }

    return te_nan;
    }

//--------------------------------------------------
// cppcheck-suppress unusedFunction
std::string te_parser::list_available_functions_and_variables()
    {
    std::string report = "Built-in Functions:\n";
    for (const auto& func : m_functions)
        {
        report.append(func.m_name).append("\n");
        }
    report.append("\nCustom Functions & Variables:\n");
    for (const auto& func : get_variables_and_functions())
        {
        report.append(func.m_name).append("\n");
        }
    return report;
    }

//--------------------------------------------------
// cppcheck-suppress unusedFunction
std::string te_parser::info()
    {
    std::string sysInfo{ "TinyExpr++ system info:\n=======================\n" };
#ifdef TE_FLOAT
    sysInfo += "Data type:                float\n";
#elif defined(TE_LONG_DOUBLE)
    sysInfo += "Data type:                long double\n";
#else
    sysInfo += "Data type:                double\n";
#endif
    if constexpr (supports_32bit())
        {
        sysInfo += "Supports 32-bit integers: yes\n";
        }
    else
        {
        sysInfo += "Supports 32-bit integers: no\n";
        }
    if constexpr (supports_64bit())
        {
        sysInfo += "Supports 64-bit integers: yes\n";
        }
    else
        {
        sysInfo += "Supports 64-bit integers: no\n";
        }
    sysInfo +=
        "Max supported integer:    " + std::to_string(static_cast<uint64_t>(get_max_integer())) +
        "\n";
#ifdef TE_BITWISE_OPERATORS
    sysInfo += "^, &, | operators:        bitwise XOR, bitwise AND, and bitwise OR\n";
#else
    sysInfo += "^, &, | operators:        exponentiation, logical AND, and logical OR\n";
#endif
#ifdef TE_BRACKETS_AS_PARENS
    sysInfo += "[] are treated as ():     yes\n";
#else
    sysInfo += "[] are treated as ():     no\n";
#endif
#ifdef TE_POW_FROM_RIGHT
    sysInfo += "Exponentiation:           performed right-to-left\n";
#else
    sysInfo += "Exponentiation:           performed left-to-right\n";
#endif
#ifdef TE_NO_BOOKKEEPING
    sysInfo += "Function-use tracking:    disabled\n";
#else
    sysInfo += "Function-use tracking:    enabled\n";
#endif
    return sysInfo;
    }

// NOLINTEND(readability-redundant-casting,readability-avoid-nested-conditional-operator,hicpp-named-parameter,readability-named-parameter)
