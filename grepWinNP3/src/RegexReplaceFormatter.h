// grepWin - regex search and replace for Windows

// Copyright (C) 2011-2012, 2014-2015, 2021 - Stefan Kueng

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
#pragma once
#include <string>
#include <stdio.h>
#include <algorithm>
#include <map>
#pragma warning(push)
#pragma warning(disable : 4996) // warning STL4010: Various members of std::allocator are deprecated in C++17
#include <boost/regex.hpp>
#include <boost/spirit/include/classic_file_iterator.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#pragma warning(pop)

class NumberReplacer
{
public:
    NumberReplacer()
        : leadZero(false)
        , padding(0)
        , start(1)
        , increment(1)
    {
    }

    bool         leadZero;
    int          padding;
    int          start;
    int          increment;
    std::wstring expression;
};

class NumberReplacerA
{
public:
    NumberReplacerA()
        : leadZero(false)
        , padding(0)
        , start(1)
        , increment(1)
    {
    }

    bool        leadZero;
    int         padding;
    int         start;
    int         increment;
    std::string expression;
};

extern std::vector<NumberReplacer>  g_incVec;
extern std::vector<NumberReplacerA> g_incVecA;

class RegexReplaceFormatter
{
public:
    RegexReplaceFormatter(const std::wstring& sReplace)
        : m_sReplace(sReplace)
    {
        g_incVec.clear();
        // parse for ${count0L}, ${count0L(n)}, ${count0L(n,m)}, where
        // ${count}
        // is replaced later with numbers starting from 1, incremented by 1
        // ${count(n)}
        // is replaced with numbers starting from n, incremented by 1
        // ${count(n,m)}
        // is replaced with numbers starting from n, incremented by m
        // 0 and L are optional and specify the size of the right-aligned
        // number string. If 0 is specified, zeros are used for padding, otherwise spaces.
        //boost::wregex expression = boost::wregex(L"(?<!\\\\)\\$\\{count(?<leadzero>0)?(?<length>\\d+)?(\\((?<startval>[-0-9]+)\\)||\\((?<startval>[-0-9]+),(?<increment>[-0-9]+)\\))?\\}", boost::regex::normal);
        boost::wregex                                      expression = boost::wregex(L"\\$\\{count(?<leadzero>0)?(?<length>\\d+)?(\\((?<startval>[-0-9]+)\\)||\\((?<startval>[-0-9]+),(?<increment>[-0-9]+)\\))?\\}", boost::regex::normal);
        boost::match_results<std::wstring::const_iterator> whatc;
        std::wstring::const_iterator                       start = m_sReplace.begin();
        std::wstring::const_iterator                       end   = m_sReplace.end();
        boost::match_flag_type                             flags = boost::match_default | boost::format_all;
        while (boost::regex_search(start, end, whatc, expression, flags))
        {
            if (whatc[0].matched)
            {
                NumberReplacer nr;
                nr.leadZero    = (static_cast<std::wstring>(whatc[L"leadzero"]) == L"0");
                nr.padding     = _wtoi(static_cast<std::wstring>(whatc[L"length"]).c_str());
                std::wstring s = static_cast<std::wstring>(whatc[L"startval"]);
                if (!s.empty())
                    nr.start = _wtoi(s.c_str());
                s = static_cast<std::wstring>(whatc[L"increment"]);
                if (!s.empty())
                    nr.increment = _wtoi(s.c_str());
                if (nr.increment == 0)
                    nr.increment = 1;
                nr.expression = static_cast<std::wstring>(whatc[0]);
                g_incVec.push_back(nr);
            }
            // update search position:
            if (start == whatc[0].second)
            {
                if (start == end)
                    break;
                ++start;
            }
            else
                start = whatc[0].second;
            // update flags:
            flags |= boost::match_prev_avail;
            flags |= boost::match_not_bob;
        }
    }

    void SetReplacePair(const std::wstring& s1, const std::wstring& s2)
    {
        m_replaceMap[s1] = s2;
    }

    std::wstring operator()(boost::match_results<std::wstring::const_iterator> what) const
    {
        std::wstring sReplace = what.format(m_sReplace);
        if (!m_replaceMap.empty())
        {
            for (auto it = m_replaceMap.cbegin(); it != m_replaceMap.cend(); ++it)
            {
                auto itBegin = std::search(sReplace.begin(), sReplace.end(), it->first.begin(), it->first.end());
                while (itBegin != sReplace.end())
                {
                    if ((itBegin == sReplace.begin()) || ((*(itBegin - 1)) != '\\'))
                    {
                        auto itEnd = itBegin + it->first.size();
                        sReplace.replace(itBegin, itEnd, it->second);
                    }
                    else if ((*(itBegin - 1)) == '\\')
                    {
                        sReplace.erase(itBegin - 1);
                    };
                    itBegin = std::search(sReplace.begin(), sReplace.end(), it->first.begin(), it->first.end());
                }
            }
        }
        if (!g_incVec.empty())
        {
            for (auto it = g_incVec.begin(); it != g_incVec.end(); ++it)
            {
                auto itBegin = std::search(sReplace.begin(), sReplace.end(), it->expression.begin(), it->expression.end());
                if (itBegin != sReplace.end())
                {
                    if ((itBegin == sReplace.begin()) || ((*(itBegin - 1)) != '\\'))
                    {
                        auto    itEnd      = itBegin + it->expression.size();
                        wchar_t format[10] = {0};
                        if (it->padding)
                        {
                            if (it->leadZero)
                                swprintf_s(format, _countof(format), L"%%0%dd", it->padding);
                            else
                                swprintf_s(format, _countof(format), L"%%%dd", it->padding);
                        }
                        else
                            wcscpy_s(format, L"%d");
                        wchar_t buf[50] = {0};
                        swprintf_s(buf, _countof(buf), format, it->start);
                        sReplace.replace(itBegin, itEnd, buf);
                        it->start += it->increment;
                    }
                    else if ((*(itBegin - 1)) == '\\')
                    {
                        sReplace.erase(itBegin - 1);
                    };
                }
            }
        }

        //sReplace = boost::regex_replace(what[0].str(), sReplace, boost::match_default);

        return sReplace;
    }

private:
    std::wstring                         m_sReplace;
    std::map<std::wstring, std::wstring> m_replaceMap;
};

class RegexReplaceFormatterA
{
public:
    RegexReplaceFormatterA(const std::string& sReplace)
        : m_sReplace(sReplace)
    {
        g_incVec.clear();
        // parse for ${count0L}, ${count0L(n)}, ${count0L(n,m)}, where
        // ${count}
        // is replaced later with numbers starting from 1, incremented by 1
        // ${count(n)}
        // is replaced with numbers starting from n, incremented by 1
        // ${count(n,m)}
        // is replaced with numbers starting from n, incremented by m
        // 0 and L are optional and specify the size of the right-aligned
        // number string. If 0 is specified, zeros are used for padding, otherwise spaces.
        //boost::wregex expression = boost::wregex(L"(?<!\\\\)\\$\\{count(?<leadzero>0)?(?<length>\\d+)?(\\((?<startval>[-0-9]+)\\)||\\((?<startval>[-0-9]+),(?<increment>[-0-9]+)\\))?\\}", boost::regex::normal);
        boost::regex                                      expression = boost::regex("\\$\\{count(?<leadzero>0)?(?<length>\\d+)?(\\((?<startval>[-0-9]+)\\)||\\((?<startval>[-0-9]+),(?<increment>[-0-9]+)\\))?\\}", boost::regex::normal);
        boost::match_results<std::string::const_iterator> whatc;
        std::string::const_iterator                       start = m_sReplace.begin();
        std::string::const_iterator                       end   = m_sReplace.end();
        boost::match_flag_type                            flags = boost::match_default | boost::format_all;
        while (boost::regex_search(start, end, whatc, expression, flags))
        {
            if (whatc[0].matched)
            {
                NumberReplacerA nr;
                nr.leadZero   = (static_cast<std::string>(whatc["leadzero"]) == "0");
                nr.padding    = atoi(static_cast<std::string>(whatc["length"]).c_str());
                std::string s = static_cast<std::string>(whatc["startval"]);
                if (!s.empty())
                    nr.start = atoi(s.c_str());
                s = static_cast<std::string>(whatc["increment"]);
                if (!s.empty())
                    nr.increment = atoi(s.c_str());
                if (nr.increment == 0)
                    nr.increment = 1;
                nr.expression = static_cast<std::string>(whatc[0]);
                g_incVecA.push_back(nr);
            }
            // update search position:
            if (start == whatc[0].second)
            {
                if (start == end)
                    break;
                ++start;
            }
            else
                start = whatc[0].second;
            // update flags:
            flags |= boost::match_prev_avail;
            flags |= boost::match_not_bob;
        }
    }

    void SetReplacePair(const std::string& s1, const std::string& s2)
    {
        m_replaceMap[s1] = s2;
    }

    template <typename It>
    std::string operator()(boost::match_results<It> what) const
    {
        std::string sReplace = what.format(m_sReplace);
        if (!m_replaceMap.empty())
        {
            for (auto it = m_replaceMap.cbegin(); it != m_replaceMap.cend(); ++it)
            {
                auto itBegin = std::search(sReplace.begin(), sReplace.end(), it->first.begin(), it->first.end());
                while (itBegin != sReplace.end())
                {
                    if ((itBegin == sReplace.begin()) || ((*(itBegin - 1)) != '\\'))
                    {
                        auto itEnd = itBegin + it->first.size();
                        sReplace.replace(itBegin, itEnd, it->second);
                    }
                    else if ((*(itBegin - 1)) == '\\')
                    {
                        sReplace.erase(itBegin - 1);
                    };
                    itBegin = std::search(sReplace.begin(), sReplace.end(), it->first.begin(), it->first.end());
                }
            }
        }
        if (!g_incVec.empty())
        {
            for (auto it = g_incVec.begin(); it != g_incVec.end(); ++it)
            {
                auto itBegin = std::search(sReplace.begin(), sReplace.end(), it->expression.begin(), it->expression.end());
                if (itBegin != sReplace.end())
                {
                    if ((itBegin == sReplace.begin()) || ((*(itBegin - 1)) != '\\'))
                    {
                        auto itEnd      = itBegin + it->expression.size();
                        char format[10] = {0};
                        if (it->padding)
                        {
                            if (it->leadZero)
                                sprintf_s(format, _countof(format), "%%0%dd", it->padding);
                            else
                                sprintf_s(format, _countof(format), "%%%dd", it->padding);
                        }
                        else
                            strcpy_s(format, "%d");
                        char buf[50] = {0};
                        sprintf_s(buf, _countof(buf), format, it->start);
                        sReplace.replace(itBegin, itEnd, buf);
                        it->start += it->increment;
                    }
                    else if ((*(itBegin - 1)) == '\\')
                    {
                        sReplace.erase(itBegin - 1);
                    };
                }
            }
        }

        return sReplace;
    }

private:
    std::string                        m_sReplace;
    std::map<std::string, std::string> m_replaceMap;
};
