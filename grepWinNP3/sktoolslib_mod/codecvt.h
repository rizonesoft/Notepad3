
/** Several code conversions facets.

    E.g. this is how to convert UCS-2 to UTF-8

    @code
    wifstream ifs("input", ios_base::binary);
    wofstream ofs("output", ios_base::binary);

    ifs.rdbuf()->pubimbue(locale(locale(), new ucs2_conversion()));
    ofs.rbbuf()->pubimbue(locale(locale(), new utf8_conversion()));
    ofs << ifs.rdbuf();
    @endcode

    @author Vladimir Prus <ghost@cs.msu.su>

    @file
*/

#pragma once
#include <locale>
#pragma warning(push)
#pragma warning(disable : 4511)
#pragma warning(disable : 4512)

/** Conversion facet that allows to use Unicode files in UCS-2 encoding */
class Ucs2Conversion
    : public std::codecvt<wchar_t, char, std::mbstate_t>
{
protected:
    result do_in(std::mbstate_t& state,
                 const char* from, const char* fromEnd, const char*& fromNext,
                 wchar_t* to, wchar_t* toLimit, wchar_t*& toNext) const override;

    result do_out(std::mbstate_t& state,
                  const wchar_t* from, const wchar_t* fromEnd, const wchar_t*& fromNext,
                  char* to, char* toLimit, char*& toNext) const override;

    bool do_always_noconv() const throw() override { return false; }
    int  do_encoding() const throw() override { return 2; }
};

/** Conversion facet that allows to read Unicode files in UTF-8 encoding */
class UTF8Conversion
    : public std::codecvt<wchar_t, char, std::mbstate_t>
{
protected:
    result do_in(std::mbstate_t& state,
                 const char* from, const char* fromEnd, const char*& fromNext,
                 wchar_t* to, wchar_t* toLimit, wchar_t*& toNext) const override;

    result do_out(std::mbstate_t& state,
                  const wchar_t* from, const wchar_t* fromEnd, const wchar_t*& fromNext,
                  char* to, char* toLimit, char*& toNext) const override;

    bool do_always_noconv() const throw() override { return false; }
    int  do_encoding() const throw() override { return 2; }
};
#pragma warning(pop)
