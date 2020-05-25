// encoding: UTF-8
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.

#ifndef WINVER                  // Minimum platform is Windows 7
#define WINVER 0x0601
#endif

#ifndef _WIN32_WINNT            // Minimum platform is Windows 7
#define _WIN32_WINNT 0x0601
#endif

#ifndef _WIN32_WINDOWS          // Minimum platform is Windows 7
#define _WIN32_WINDOWS 0x0601
#endif


// C RunTime Header Files
#include <algorithm>
#include <vector>
#include <string>

// Windows header files
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <dwrite.h>
#include <strsafe.h>


/// ##############################################################################
/// https://docs.microsoft.com/en-us/windows/win32/directwrite/introducing-directwrite
/// ##############################################################################

extern IDWriteFactory* g_dwrite;


// Ignore unreferenced parameters, since they are very common
// when implementing callbacks.
#pragma warning(disable : 4100)


////////////////////////////////////////
// COM inheritance helpers.

// Releases a COM object and nullptrifies pointer.
template <typename InterfaceType>
inline void SafeRelease(InterfaceType** currentObject)
{
    if (*currentObject != nullptr)
    {
        (*currentObject)->Release();
        *currentObject = nullptr;
    }
}


// Acquires an additional reference, if non-nullptr.
template <typename InterfaceType>
inline InterfaceType* SafeAcquire(InterfaceType* newObject)
{
    if (newObject != nullptr)
        newObject->AddRef();

    return newObject;
}


// Sets a new COM object, releasing the old one.
template <typename InterfaceType>
inline void SafeSet(InterfaceType** currentObject, InterfaceType* newObject)
{
    SafeAcquire(newObject);
    SafeRelease(currentObject);
    *currentObject = newObject;
}


// Releases a COM object and nullptrifies pointer.
template <typename InterfaceType>
inline InterfaceType* SafeDetach(InterfaceType** currentObject)
{
    InterfaceType* oldObject = *currentObject;
    *currentObject = nullptr;
    return oldObject;
}


// Sets a new COM object, acquiring the reference.
template <typename InterfaceType>
inline void SafeAttach(InterfaceType** currentObject, InterfaceType* newObject)
{
    SafeRelease(currentObject);
    *currentObject = newObject;
}


// Maps exceptions to equivalent HRESULTs,
inline HRESULT ExceptionToHResult() noexcept
{
    try
    {
        throw;  // Rethrow previous exception.
    }
    catch(std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
    catch (...)
    {
        return E_FAIL;
    }
}
