// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2017, 2020-2021 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "stdafx.h"
#include "CmdLineParser.h"
#include <locale>
#include <algorithm>

const wchar_t CCmdLineParser::m_sDelims[]   = L"-/";
const wchar_t CCmdLineParser::m_sQuotes[]   = L"\"";
const wchar_t CCmdLineParser::m_sValueSep[] = L" :"; // don't forget space!!

static void SearchReplace(std::wstring& str, const std::wstring& toreplace, const std::wstring& replacewith)
{
    std::wstring            result;
    std::wstring::size_type pos = 0;
    for (;;)
    {
        std::wstring::size_type next = str.find(toreplace, pos);
        result.append(str, pos, next - pos);
        if (next != std::wstring::npos)
        {
            result.append(replacewith);
            pos = next + toreplace.size();
        }
        else
        {
            break; // exit loop
        }
    }
    str = std::move(result);
}

CCmdLineParser::CCmdLineParser(LPCWSTR sCmdLine)
{
    if (sCmdLine)
    {
        Parse(sCmdLine);
    }
}

CCmdLineParser::~CCmdLineParser()
{
    m_valueMap.clear();
}

BOOL CCmdLineParser::Parse(LPCWSTR sCmdLine)
{
    const std::wstring sEmpty = L""; // use this as a value if no actual value is given in commandline

    if (!sCmdLine)
        return false;

    m_valueMap.clear();
    m_sCmdLine = sCmdLine;

    LPCWSTR sCurrent = sCmdLine;

    for (;;)
    {
        //format is  -Key:"arg"

        if (sCurrent[0] == '\0')
            break; // no more data, leave loop

        LPCWSTR sArg = wcspbrk(sCurrent, m_sDelims);
        if (!sArg)
            break; // no (more) delimiters found
        sArg = sArg + 1;

        if (sArg[0] == '\0')
            break; // ends with delim

        LPCWSTR sVal = wcspbrk(sArg, m_sValueSep);
        if (sVal == nullptr)
        {
            std::wstring key(sArg);
            std::transform(key.begin(), key.end(), key.begin(), ::towlower);
            m_valueMap.insert(CValsMap::value_type(key, sEmpty));
            break;
        }
        else if (sVal[0] == L' ' || wcslen(sVal) == 1)
        {
            // cmdline ends with /Key: or a key with no value
            std::wstring key(sArg, static_cast<int>(sVal - sArg));
            if (!key.empty())
            {
                std::transform(key.begin(), key.end(), key.begin(), ::towlower);
                m_valueMap.insert(CValsMap::value_type(key, sEmpty));
            }
            sCurrent = sVal + 1;
        }
        else
        {
            // key has value
            std::wstring key(sArg, static_cast<int>(sVal - sArg));
            std::transform(key.begin(), key.end(), key.begin(), ::towlower);

            sVal = sVal + 1;

            LPCWSTR sQuote           = wcspbrk(sVal, m_sQuotes), sEndQuote(nullptr);
            bool    hasEscapedQuotes = false;
            if (sQuote == sVal)
            {
                // string with quotes (defined in m_sQuotes, e.g. '")
                sQuote    = sVal + 1;
                sEndQuote = wcspbrk(sQuote, m_sQuotes);
                while (sEndQuote && sEndQuote[-1] == '\\')
                {
                    hasEscapedQuotes = true;
                    sEndQuote        = sEndQuote + 1;
                    sEndQuote        = wcspbrk(sEndQuote, m_sQuotes);
                }
            }
            else
            {
                sQuote    = sVal;
                sEndQuote = wcschr(sQuote, L' ');
                while (sEndQuote && sEndQuote[-1] == '\\')
                {
                    hasEscapedQuotes = true;
                    sEndQuote        = sEndQuote + 1;
                    sEndQuote        = wcspbrk(sEndQuote, m_sQuotes);
                }
            }

            if (sEndQuote == nullptr)
            {
                // no end quotes or terminating space, take the rest of the string to its end
                std::wstring csVal(sQuote);
                if (hasEscapedQuotes)
                    SearchReplace(csVal, L"\\\"", L"\"");
                if (!key.empty())
                {
                    m_valueMap.insert(CValsMap::value_type(key, csVal));
                }
                break;
            }
            else
            {
                // end quote
                if (!key.empty())
                {
                    std::wstring csVal(sQuote, static_cast<int>(sEndQuote - sQuote));
                    if (hasEscapedQuotes)
                        SearchReplace(csVal, L"\\\"", L"\"");
                    m_valueMap.insert(CValsMap::value_type(key, csVal));
                }
                sCurrent = sEndQuote + 1;
                continue;
            }
        }
    }

    return TRUE;
}

CCmdLineParser::CValsMap::const_iterator CCmdLineParser::findKey(LPCWSTR sKey) const
{
    std::wstring s(sKey);
    std::transform(s.begin(), s.end(), s.begin(), ::towlower);
    return m_valueMap.find(s);
}

BOOL CCmdLineParser::HasKey(LPCWSTR sKey) const
{
    CValsMap::const_iterator it = findKey(sKey);
    if (it == m_valueMap.end())
        return false;
    return true;
}

BOOL CCmdLineParser::HasVal(LPCWSTR sKey) const
{
    CValsMap::const_iterator it = findKey(sKey);
    if (it == m_valueMap.end())
        return false;
    if (it->second.empty())
        return false;
    return true;
}

LPCWSTR CCmdLineParser::GetVal(LPCWSTR sKey) const
{
    CValsMap::const_iterator it = findKey(sKey);
    if (it == m_valueMap.end())
        return nullptr;
    return it->second.c_str();
}

LONG CCmdLineParser::GetLongVal(LPCWSTR sKey) const
{
    CValsMap::const_iterator it = findKey(sKey);
    if (it == m_valueMap.end())
        return 0;
    return _wtol(it->second.c_str());
}

__int64 CCmdLineParser::GetLongLongVal(LPCWSTR sKey) const
{
    CValsMap::const_iterator it = findKey(sKey);
    if (it == m_valueMap.end())
        return 0;
    return _wtoi64(it->second.c_str());
}

CCmdLineParser::ITERPOS CCmdLineParser::begin() const
{
    return m_valueMap.begin();
}

CCmdLineParser::ITERPOS CCmdLineParser::getNext(ITERPOS& pos, std::wstring& sKey, std::wstring& sValue) const
{
    if (m_valueMap.end() == pos)
    {
        sKey.clear();
        return pos;
    }
    else
    {
        sKey   = pos->first;
        sValue = pos->second;
        ++pos;
        return pos;
    }
}

BOOL CCmdLineParser::isLast(const ITERPOS& pos) const
{
    return (pos == m_valueMap.end());
}
