// sktoolslib - common files for SK tools

// Copyright (C) 2014 - Stefan Kueng

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

//////////////////////////////////////////////////////////////////////
// required includes
//////////////////////////////////////////////////////////////////////

#ifndef _DEBUG
#ifndef __INTRIN_H_
#include <intrin.h>
#endif
#endif

#ifndef _PSAPI_H_
#include <psapi.h>
#endif

#ifndef _STRING_
#include <string>
#endif

#ifndef _VECTOR_
#include <vector>
#endif

#ifndef _FSTREAM_
#include <fstream>
#endif

#ifndef _TIME_
#include <time.h>
#endif

#pragma comment (lib, "psapi.lib")

/**
* Collects the profiling info for a given profiled block / line.
* Records execution count, min, max and accumulated execution time
* in CPU clock ticks.
*/

class CProfilingRecord
{
public:
    /// collected profiling info

    struct CSpent {
        unsigned __int64 sum;
        unsigned __int64 minValue;
        unsigned __int64 maxValue;

        void Add(unsigned __int64  value)
        {
            sum += value;

            if (value < minValue)
                minValue = value;
            if (value > maxValue)
                maxValue = value;
        }
        /// First value add
        void Init(unsigned __int64  value)
        {
            sum = value;
            minValue = value;
            maxValue = value;
        }
        void Init()
        {
            sum = 0;
            minValue = ULLONG_MAX; /* -1 */
            maxValue = 0;
        }
    };

    /// construction

    CProfilingRecord ( const char* name
                     , const char* file
                     , int line);

    /// record values

    void Add ( unsigned __int64 valueRdtsc
             , unsigned __int64 valueWall
             , unsigned __int64 valueUser
             , unsigned __int64 valueKernel
             );

    /// modification

    void Reset();

    /// data access

    const char* GetName() const {return name;}
    const char* GetFile() const {return file;}
    int GetLine() const {return line;}

    size_t GetCount() const {return count;}
    const CSpent & Get() const {return m_rdtsc; }
    const CSpent & GetU() const {return m_user; }
    const CSpent & GetK() const {return m_kernel; }
    const CSpent & GetW() const {return m_wall; }


private:

    /// identification

    const char* name;
    const char* file;
    int line;

    CSpent m_rdtsc, m_user, m_kernel, m_wall;
    size_t count;
};

/**
* RAII class that encapsulates a single execution of a profiled
* block / line. The result gets added to an existing profiling record.
*/

class CRecordProfileEvent
{
private:

    CProfilingRecord* record;

    /// the initial counter values
    unsigned __int64 m_rdtscStart;
    FILETIME m_kernelStart;
    FILETIME m_userStart;
    FILETIME m_wallStart;

public:

    /// construction: start clock

    CRecordProfileEvent (CProfilingRecord* aRecord);

    /// destruction: time interval to profiling record,
    /// if Stop() had not been called before

    ~CRecordProfileEvent();

    /// don't wait for destruction

    void Stop();
};

/// construction / destruction

inline CRecordProfileEvent::CRecordProfileEvent (CProfilingRecord* aRecord)
    : record (aRecord)
{
    // less precise first
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &m_wallStart);
    FILETIME ftTemp;
    GetProcessTimes(GetCurrentProcess(), &ftTemp, &ftTemp, &m_kernelStart, &m_userStart);

    m_rdtscStart = __rdtsc();
}

inline CRecordProfileEvent::~CRecordProfileEvent()
{
    Stop();
}

UINT64 inline DiffFiletime(FILETIME time1, FILETIME time2)
{
    return *(UINT64 *)&time1 - *(UINT64 *)&time2;
}

/// stop counting

inline void CRecordProfileEvent::Stop()
{
    if (record)
    {
        // more precise first
        unsigned __int64 nTake = __rdtsc() - m_rdtscStart;

        FILETIME kernelEnd, userEnd, ftTemp;
        GetProcessTimes(GetCurrentProcess(), &ftTemp, &ftTemp, &kernelEnd, &userEnd);

        SYSTEMTIME st;
        GetSystemTime(&st);
        FILETIME oTime;
        SystemTimeToFileTime(&st, &oTime);

        record->Add (nTake
                   , DiffFiletime(oTime, m_wallStart)
                   , DiffFiletime(userEnd, m_userStart)
                   , DiffFiletime(kernelEnd, m_kernelStart));
        record = NULL;
    }
}

/**
* Singleton class that acts as container for all profiling records.
* You may reset its content as well as write it to disk.
*/

class CProfilingInfo
{
private:

    typedef std::vector<CProfilingRecord*> TRecords;
    TRecords records;

    /// construction / destruction

    CProfilingInfo();
    ~CProfilingInfo(void);

    /// create report

    std::string GetReport() const;

public:

    /// access to default instance

    static CProfilingInfo* GetInstance();

    /// add a new record

    CProfilingRecord* Create ( const char* name
                             , const char* file
                             , int line);

    /// write the current results to disk

    void DumpReport();
};

/**
* Profiling macros
*/

#define PROFILE_CONCAT( a, b )   PROFILE_CONCAT3( a, b )
#define PROFILE_CONCAT3( a, b )  a##b

/// measures the time from the point of usage to the end of the respective block

#define PROFILE_BLOCK\
    static CProfilingRecord* PROFILE_CONCAT(record,__LINE__) \
        = CProfilingInfo::GetInstance()->Create(__FUNCTION__,__FILE__,__LINE__);\
    CRecordProfileEvent PROFILE_CONCAT(profileSection,__LINE__) (PROFILE_CONCAT(record,__LINE__));

/// measures the time taken to execute the respective code line

#define PROFILE_LINE(line)\
    static CProfilingRecord* PROFILE_CONCAT(record,__LINE__) \
        = CProfilingInfo::GetInstance()->Create(__FUNCTION__,__FILE__,__LINE__);\
    CRecordProfileEvent PROFILE_CONCAT(profileSection,__LINE__) (PROFILE_CONCAT(record,__LINE__));\
    line;\
    PROFILE_CONCAT(profileSection,__LINE__).Stop();


//////////////////////////////////////////////////////////////////////
// construction / destruction
//////////////////////////////////////////////////////////////////////

inline CProfilingRecord::CProfilingRecord ( const char* name
                                   , const char* file
                                   , int line)
    : name (name)
    , file (file)
    , line (line)
    , count (0)
{
    Reset();
}

//////////////////////////////////////////////////////////////////////
// record values
//////////////////////////////////////////////////////////////////////

void inline CProfilingRecord::Add (unsigned __int64 valueRdtsc
    , unsigned __int64 valueTime
    , unsigned __int64 valueUser
    , unsigned __int64 valueKernel
)
{
    if (!count++)
    {
        m_rdtsc.Init(valueRdtsc);
        m_user.Init(valueUser);
        m_kernel.Init(valueKernel);
        m_wall.Init(valueTime);
    }
    else
    {
        m_rdtsc.Add(valueRdtsc);
        m_user.Add(valueUser);
        m_kernel.Add(valueKernel);
        m_wall.Add(valueTime);
    }
}

//////////////////////////////////////////////////////////////////////
// modification
//////////////////////////////////////////////////////////////////////

inline void CProfilingRecord::Reset()
{
    count = 0;
    // we don't need to init other then count, to be save we do
    m_rdtsc.Init();
    m_user.Init();
    m_kernel.Init();
    m_wall.Init();
}

//////////////////////////////////////////////////////////////////////
// construction / destruction
//////////////////////////////////////////////////////////////////////

inline CProfilingInfo::CProfilingInfo()
{
}

inline CProfilingInfo::~CProfilingInfo(void)
{
    DumpReport();

    // free data

    for (size_t i = 0; i < records.size(); ++i)
        delete records[i];
}

//////////////////////////////////////////////////////////////////////
// access to default instance
//////////////////////////////////////////////////////////////////////

inline CProfilingInfo* CProfilingInfo::GetInstance()
{
    static CProfilingInfo instance;
    return &instance;
}

inline void CProfilingInfo::DumpReport()
{
    if (!records.empty())
    {
        // write profile to file

#ifdef _WIN32
        char buffer [MAX_PATH];
        if (GetModuleFileNameExA (GetCurrentProcess(), NULL, buffer, _countof(buffer)) > 0)
#else
        const char* buffer = "application";
#endif

        try
        {
            char datebuf[MAX_PATH];

            time_t rawtime;
            struct tm timeinfo;
            time ( &rawtime );
            localtime_s(&timeinfo, &rawtime);

            strftime(datebuf, MAX_PATH, "-%y-%m-%d-%H-%M", &timeinfo);
            std::string fileName (buffer);
            fileName += datebuf;
            fileName += ".profile";

            std::string report = GetInstance()->GetReport();

            std::ofstream file;
            file.open (fileName.c_str(), std::ios::binary | std::ios::out);
            file.write (report.c_str(), report.size());
        }
        catch (...)
        {
            // ignore all file errors etc.
        }
    }
}

//////////////////////////////////////////////////////////////////////
// create a report
//////////////////////////////////////////////////////////////////////

inline static std::string IntToStr (unsigned __int64 value)
{
    char buffer[100];
    _ui64toa_s (value, buffer, _countof(buffer), 10);

    std::string result = buffer;
    for (size_t i = 3; i < result.length(); i += 4)
        result.insert (result.length() - i, 1, ',');

    return result;
};

inline std::string CProfilingInfo::GetReport() const
{
    enum { LINE_LENGTH = 600 };

    char lineBuffer [LINE_LENGTH];
    std::string result;
    result.reserve (LINE_LENGTH * records.size());

    const char * const format ="%15s%17s%17s%17s%17s\n";

    for ( TRecords::const_iterator iter = records.begin(), end = records.end()
        ; iter != end
        ; ++iter)
    {
        size_t nCount = (*iter)->GetCount();
        sprintf_s ( lineBuffer, "%7sx %s\n%s:%s\n"
                  , IntToStr (nCount).c_str()
                  , (*iter)->GetName()
                  , (*iter)->GetFile()
                  , IntToStr ((*iter)->GetLine()).c_str());
        result += lineBuffer;
        if (nCount==0)
            continue;

        sprintf_s ( lineBuffer, format
                  , "type", "sum", "avg", "min", "max");
        result += lineBuffer;

        sprintf_s ( lineBuffer, format
                  , "CPU Ticks"
                  , IntToStr ((*iter)->Get().sum).c_str()
                  , IntToStr ((*iter)->Get().sum/nCount).c_str()
                  , IntToStr ((*iter)->Get().minValue).c_str()
                  , IntToStr ((*iter)->Get().maxValue).c_str());
        result += lineBuffer;

        sprintf_s ( lineBuffer, format
                  , "UserMode[us]"
                  , IntToStr ((*iter)->GetU().sum/10).c_str()
                  , IntToStr ((*iter)->GetU().sum/10/nCount).c_str()
                  , IntToStr ((*iter)->GetU().minValue/10).c_str()
                  , IntToStr ((*iter)->GetU().maxValue/10).c_str());
        result += lineBuffer;

        sprintf_s ( lineBuffer, format
                  , "KernelMode[us]"
                  , IntToStr ((*iter)->GetK().sum/10).c_str()
                  , IntToStr ((*iter)->GetK().sum/10/nCount).c_str()
                  , IntToStr ((*iter)->GetK().minValue/10).c_str()
                  , IntToStr ((*iter)->GetK().maxValue/10).c_str());
        result += lineBuffer;

        sprintf_s ( lineBuffer, format
                  , "WallTime[us]"
                  , IntToStr ((*iter)->GetW().sum/10).c_str()
                  , IntToStr ((*iter)->GetW().sum/10/nCount).c_str()
                  , IntToStr ((*iter)->GetW().minValue/10).c_str()
                  , IntToStr ((*iter)->GetW().maxValue/10).c_str());
        result += lineBuffer;

        result += "\n";
    }

    // now print the processor speed read from the registry: the user may want to
    // convert the processor ticks to seconds
    HKEY hKey;
    // open the key where the proc speed is hidden:
    long lError = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                                0,
                                KEY_READ,
                                &hKey);

    if(lError == ERROR_SUCCESS)
    {
        // query the key:
        DWORD BufSize = _MAX_PATH;
        DWORD dwMHz = 0;
        RegQueryValueExA(hKey, "~MHz", NULL, NULL, (LPBYTE) &dwMHz, &BufSize);
        RegCloseKey(hKey);

        sprintf_s ( lineBuffer, "processor speed is %ld MHz\n", dwMHz);
        result += lineBuffer;
    }


    return result;
}

//////////////////////////////////////////////////////////////////////
// add a new record
//////////////////////////////////////////////////////////////////////

inline CProfilingRecord* CProfilingInfo::Create ( const char* name
                                         , const char* file
                                         , int line)
{
    CProfilingRecord* record = new CProfilingRecord (name, file, line);
    records.push_back (record);

    return record;
}

