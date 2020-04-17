
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

#include <locale>
#pragma warning(push)
#pragma warning(disable: 4511)
#pragma warning(disable: 4512)

/** Conversion facet that allows to use Unicode files in UCS-2 encoding */
class ucs2_conversion
: public std::codecvt<wchar_t, char, std::mbstate_t>
{
protected:

    result do_in(std::mbstate_t& state,
                 const char* from, const char* from_end, const char*& from_next,
                 wchar_t* to, wchar_t* to_limit, wchar_t*& to_next) const;

    result do_out(std::mbstate_t& state,
                  const wchar_t* from, const wchar_t* from_end, const wchar_t*& from_next,
                  char* to, char* to_limit, char*& to_next) const;

    bool do_always_noconv() const throw() { return false; }
    int  do_encoding() const throw() { return 2; }
};


/** Conversion facet that allows to read Unicode files in UTF-8 encoding */
class utf8_conversion
: public std::codecvt<wchar_t, char, std::mbstate_t>
{
protected:

    result do_in(std::mbstate_t& state,
                 const char* from, const char* from_end, const char*& from_next,
                 wchar_t* to, wchar_t* to_limit, wchar_t*& to_next) const;

    result do_out(std::mbstate_t& state,
                  const wchar_t* from, const wchar_t* from_end, const wchar_t*& from_next,
                  char* to, char* to_limit, char*& to_next) const;

    bool do_always_noconv() const throw() { return false; }
    int  do_encoding() const throw() { return 2; }
};
#pragma warning(pop)
