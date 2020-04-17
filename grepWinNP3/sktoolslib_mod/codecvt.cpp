#include "stdafx.h"
#include "codecvt.h"

using namespace std;

ucs2_conversion::result
ucs2_conversion::do_in(mbstate_t&,
                       const char*  from, const char* from_end, const char*& from_next,
                       wchar_t* to, wchar_t* to_limit, wchar_t*& to_next) const
{
    size_t max_input = (from_end - from) & ~1;
    size_t max_output = (to_limit - to);
    size_t count = min(max_input/2, max_output);

    result res = ok;

    from_next = from;
    to_next = to;
    for (;count--; from_next += 2, ++to_next) {
        unsigned char c1 = *from_next, c2 = *(from_next + 1);
        *to_next = c1 | c2 << 8;
    }
    if (to_next == to && from_next == from_end - 1) res = partial;
    return res;
}

ucs2_conversion::result
ucs2_conversion::do_out(mbstate_t&,
                  const wchar_t* from, const wchar_t* from_end, const wchar_t*& from_next,
                  char* to, char* to_limit, char*& to_next) const
{
    size_t max_input = (from_end - from);
    size_t max_output = (to_limit - to) & ~1;
    size_t count = min(max_input, max_output/2);

    from_next = from;
    to_next = to;
    for (;count--; ++from_next, to_next += 2) {
        *(to_next + 0) = (char)(*from_next & 0xFF);
        *(to_next + 1) = (char)(*from_next >> 8 & 0xFF);
    }
    return ok;
}

typedef unsigned char uchar;

inline unsigned char
take_6_bits(unsigned value, size_t right_position)
{
    return uchar((value >> right_position) & 63);
}



inline size_t
most_signifant_bit_position(unsigned value)
{
    size_t result(0);

    size_t half = 16;
    for(; half > 0; half >>= 1) {
        if (1u << (result + half) <= value ) result += half;
    }
    return result + 1;
    //return *lower_bound(range(0u, 31u), \x -> (1 << x <= value));
}



utf8_conversion::result
utf8_conversion::do_in(mbstate_t&,
                       const char*  from, const char* from_end, const char*& from_next,
                       wchar_t* to, wchar_t* to_limit, wchar_t*& to_next) const
{
    from_next = from;
    to_next = to;

    for(; to_next < to_limit && from_next < from_end; ++to_next) {

        if (uchar(*from_next) < 0x80) *to_next = uchar(*from_next++);
        else {

            // 111zxxxx : z = 0 xxxx are data bits
            size_t zero_bit_pos = most_signifant_bit_position(~*from_next);
            size_t extra_bytes  = 7 - zero_bit_pos;

            if (size_t(from_end - from_next) < extra_bytes + 1)
                return partial;

            *to_next = uchar(*from_next++) & (wchar_t)((1 << (zero_bit_pos - 1)) - 1);

            for (;extra_bytes--; ++from_next) {
                *to_next = *to_next << 6  |  uchar(*from_next) & 63;
            }
        }
    }

    return ok;
}


utf8_conversion::result
utf8_conversion::do_out(mbstate_t&,
                  const wchar_t* from, const wchar_t* from_end, const wchar_t*& from_next,
                  char* to, char* to_limit, char*& to_next) const
{
        from_next = from;
        to_next = to;

        for (;from_next < from_end; ++from_next) {

            unsigned symbol = *from_next;

            if (symbol < 0x7F) {
                if (to_next < to_limit)
                    *to_next++ = (unsigned char)symbol;
                else
                    return ok;
            } else {

                size_t msb_pos = most_signifant_bit_position(symbol);
                size_t extra_bytes = msb_pos / 6;

                if (size_t(to_limit - to_next) >= extra_bytes + 1) {

                    *to_next = uchar(0xFF80 >> extra_bytes);
                    *to_next++ |= take_6_bits(symbol, extra_bytes*6);

                    for(;extra_bytes--;)
                        *to_next++ = 0x80 | take_6_bits(symbol, extra_bytes*6);
                }
                else
                    return ok;
            }
        }
        return ok;
}

