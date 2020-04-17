// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2017 - Stefan Kueng

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

#include <map>
#include <string>

#ifndef stdstring
#   ifdef UNICODE
#       define stdstring std::wstring
#   else
#       define stdstring std::string
#   endif
#endif

/**
 *
 * A helper class for parsing command lines.
 * It provides methods to extract 'key' and 'value'
 * pairs of the form -keyname:value or /keyname:value.
 * Parameter examples:\n
 * \code
 * "/key1 /key2:myvalue -key3:anothervalue -key4:"this could be a path with spaces"
 * \endcode
 * /key is the same as -key\n
 * all keys and values are case-insensitive.
 * Please note that both keys and values are strings although the class
 * provides a method to get a long as a value.
 * Example:\n
 * \code
 * CCmdLineParser parser(::GetCommandLine());
 * if (parser.HasKey("r"))
 * {
 *  // the key -r or /r is there (could mean e.g. 'recurse')
 * }
 * //now assume the command line is /open:"c:\test.txt" /wait:30
 * CString file = parser.GetVal("open");
 * //file contains now c:\test.txt
 * long number = parser.GetLongVal("seconds");
 * //number has now the value 30
 * \endcode
 */
class CCmdLineParser
{
public:
    typedef std::map<stdstring, stdstring> CValsMap;
    typedef CValsMap::const_iterator ITERPOS;

    /**
     * Creates a CCmdLineParser object and parses the parameters in.
     * \param sCmdLine the command line
     */
    CCmdLineParser(LPCTSTR sCmdLine = nullptr);
    virtual ~CCmdLineParser();

    /**
     * returns the command line string this object was created on.
     * \return the command line
     */
    LPCTSTR getCmdLine() const { return m_sCmdLine.c_str(); }

    /**
     * Starts an iteration over all command line parameters.
     * \return the first position
     */
    ITERPOS begin() const;

    /**
     * Get the next key/value pair. If no more keys are available then
     * an empty key is returned.
     * \param the position from where to get. To get the first pair use the
     * begin() method. \a pos is incremented by 1 on return.
     * \param sKey returns the key
     * \param sValue returns the value
     * \return the next position
     */
    ITERPOS getNext(ITERPOS& pos, stdstring& sKey, stdstring& sValue) const;

    /**
     * Checks if the position is the last or if there are more key/value pairs in the command line.
     * \param pos the position to check
     * \return TRUE if no more key/value pairs are available
     */
    BOOL isLast(const ITERPOS& pos) const;

    /**
     * Checks if the given key is in the command line.
     * \param sKey the key to check for
     * \return TRUE if the key exists, FALSE if the key is not in command line
     */
    BOOL HasKey(LPCTSTR sKey) const;

    /**
     * Checks if a key also has a value or not.
     * \param sKey the key to check for a value
     * \return TRUE if the key has a value, FALSE if no value (or no key) was found
     */
    BOOL HasVal(LPCTSTR sKey) const;

    /**
     * Reads the value for a key. If the key has no value then nullptr is returned.
     * \param sKey the key to get the value from
     * \return the value string of the key
     */
    LPCTSTR GetVal(LPCTSTR sKey) const;

    /**
     * Reads the value for a key as a long. If the value is a string which can't be
     * converted to a number then 0 is returned.
     * \param the key to get the value from
     * \return the value converted to a long
     */
    LONG GetLongVal(LPCTSTR sKey) const;

    __int64 GetLongLongVal(LPCTSTR sKey) const;
private:
    BOOL Parse(LPCTSTR sCmdLine);
    CValsMap::const_iterator findKey(LPCTSTR sKey) const;
    const CValsMap& getVals() const { return m_valueMap; }

    stdstring   m_sCmdLine;
    CValsMap    m_valueMap;

    static const TCHAR m_sDelims[];
    static const TCHAR m_sValueSep[];
    static const TCHAR m_sQuotes[];
};
