#include "stdafx.h"
#include "codecvt.h"

using namespace std;

Ucs2Conversion::result
    Ucs2Conversion::do_in(mbstate_t&,
                           const char* from, const char* fromEnd, const char*& fromNext,
                           wchar_t* to, wchar_t* toLimit, wchar_t*& toNext) const
{
    size_t maxInput  = (fromEnd - from) & ~1;
    size_t maxOutput = (toLimit - to);
    size_t count     = min(maxInput / 2, maxOutput);

    result res = ok;

    fromNext = from;
    toNext   = to;
    for (; count--; fromNext += 2, ++toNext)
    {
        unsigned char c1 = *fromNext, c2 = *(fromNext + 1);
        *toNext = c1 | c2 << 8;
    }
    if (toNext == to && fromNext == fromEnd - 1)
        res = partial;
    return res;
}

Ucs2Conversion::result
    Ucs2Conversion::do_out(mbstate_t&,
                            const wchar_t* from, const wchar_t* fromEnd, const wchar_t*& fromNext,
                            char* to, char* toLimit, char*& toNext) const
{
    size_t maxInput  = (fromEnd - from);
    size_t maxOutput = (toLimit - to) & ~1;
    size_t count     = min(maxInput, maxOutput / 2);

    fromNext = from;
    toNext   = to;
    for (; count--; ++fromNext, toNext += 2)
    {
        *(toNext + 0) = static_cast<char>(*fromNext & 0xFF);
        *(toNext + 1) = static_cast<char>(*fromNext >> 8 & 0xFF);
    }
    return ok;
}

inline unsigned char
    take6Bits(unsigned value, size_t rightPosition)
{
    return static_cast<unsigned char>((value >> rightPosition) & 63);
}

inline size_t
    mostSignifantBitPosition(unsigned value)
{
    size_t result(0);

    size_t half = 16;
    for (; half > 0; half >>= 1)
    {
        if (1u << (result + half) <= value)
            result += half;
    }
    return result + 1;
    //return *lower_bound(range(0u, 31u), \x -> (1 << x <= value));
}

UTF8Conversion::result
    UTF8Conversion::do_in(mbstate_t&,
                           const char* from, const char* fromEnd, const char*& fromNext,
                           wchar_t* to, wchar_t* toLimit, wchar_t*& toNext) const
{
    fromNext = from;
    toNext   = to;

    for (; toNext < toLimit && fromNext < fromEnd; ++toNext)
    {
        if (static_cast<unsigned char>(*fromNext) < 0x80)
            *toNext = static_cast<unsigned char>(*fromNext++);
        else
        {
            // 111zxxxx : z = 0 xxxx are data bits
            size_t zeroBitPos = mostSignifantBitPosition(~*fromNext);
            size_t extraBytes = 7 - zeroBitPos;

            if (static_cast<size_t>(fromEnd - fromNext) < extraBytes + 1)
                return partial;

            *toNext = static_cast<unsigned char>(*fromNext++) & static_cast<wchar_t>((1 << (zeroBitPos - 1)) - 1);

            for (; extraBytes--; ++fromNext)
            {
                *toNext = *toNext << 6 | static_cast<unsigned char>(*fromNext) & 63;
            }
        }
    }

    return ok;
}

UTF8Conversion::result
    UTF8Conversion::do_out(mbstate_t&,
                            const wchar_t* from, const wchar_t* fromEnd, const wchar_t*& fromNext,
                            char* to, char* toLimit, char*& toNext) const
{
    fromNext = from;
    toNext   = to;

    for (; fromNext < fromEnd; ++fromNext)
    {
        unsigned symbol = *fromNext;

        if (symbol < 0x7F)
        {
            if (toNext < toLimit)
                *toNext++ = static_cast<unsigned char>(symbol);
            else
                return ok;
        }
        else
        {
            size_t msbPos     = mostSignifantBitPosition(symbol);
            size_t extraBytes = msbPos / 6;

            if (static_cast<size_t>(toLimit - toNext) >= extraBytes + 1)
            {
                *toNext = static_cast<unsigned char>(0xFF80 >> extraBytes);
                *toNext++ |= take6Bits(symbol, extraBytes * 6);

                for (; extraBytes--;)
                    *toNext++ = 0x80 | take6Bits(symbol, extraBytes * 6);
            }
            else
                return ok;
        }
    }
    return ok;
}
