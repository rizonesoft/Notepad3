// sktoolslib - common files for SK tools

// Copyright (C) 2014, 2017 - Stefan Kueng

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
#include "DownloadFile.h"

#pragma comment(lib, "wininet.lib")

CDownloadFile::CDownloadFile(LPCWSTR useragent, CProgressDlg * pProgress)
    : m_pProgress(pProgress)
{
    hOpenHandle = InternetOpen(useragent, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
}

CDownloadFile::~CDownloadFile(void)
{
    if (hOpenHandle)
        InternetCloseHandle(hOpenHandle);
}

bool CDownloadFile::DownloadFile(const std::wstring& url, const std::wstring& dest) const
{
    wchar_t hostname[INTERNET_MAX_HOST_NAME_LENGTH] = { 0 };
    wchar_t urlpath[INTERNET_MAX_PATH_LENGTH] = { 0 };
    URL_COMPONENTS urlComponents = {0};
    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.lpszHostName = hostname;
    urlComponents.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;
    urlComponents.lpszUrlPath = urlpath;
    urlComponents.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
    if (!InternetCrackUrl(url.c_str(), (DWORD)url.size(), 0, &urlComponents))
        return GetLastError()==0;

    DeleteUrlCacheEntry(url.c_str());

    bool isHttps = urlComponents.nScheme == INTERNET_SCHEME_HTTPS;
    HINTERNET hConnectHandle = InternetConnect(hOpenHandle, hostname, urlComponents.nPort, nullptr, nullptr, isHttps ? INTERNET_SCHEME_HTTP : urlComponents.nScheme, 0, 0);
    if (!hConnectHandle)
    {
        DWORD err = GetLastError();
        CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L": Download of %s failed on InternetConnect: %d\n", url.c_str(), err);
        return err == 0;
    }
    HINTERNET hResourceHandle = HttpOpenRequest(hConnectHandle, nullptr, urlpath, nullptr, nullptr, nullptr, INTERNET_FLAG_KEEP_CONNECTION | (isHttps ? INTERNET_FLAG_SECURE : 0), 0);
    if (!hResourceHandle)
    {
        DWORD err = GetLastError();
        CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L": Download of %s failed on HttpOpenRequest: %d\n", url.c_str(), err);
        InternetCloseHandle(hConnectHandle);
        return err == 0;
    }

    {
        DWORD dwError = 0;
        bool httpsendrequest = false;
        do
        {
            httpsendrequest = !!HttpSendRequest(hResourceHandle, nullptr, 0, nullptr, 0);
            dwError = InternetErrorDlg(nullptr, hResourceHandle, ERROR_SUCCESS, FLAGS_ERROR_UI_FILTER_FOR_ERRORS | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS | FLAGS_ERROR_UI_FLAGS_GENERATE_DATA, nullptr);
        } while (dwError == ERROR_INTERNET_FORCE_RETRY);

        if (!httpsendrequest)
        {
            DWORD err = GetLastError();
            CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L": Download of %s failed: %d, %d\n", url.c_str(), httpsendrequest, err);
            InternetCloseHandle(hResourceHandle);
            InternetCloseHandle(hConnectHandle);
            return err == 0;
        }
    }

    DWORD contentLength = 0;
    {
        DWORD length = sizeof(contentLength);
        HttpQueryInfo(hResourceHandle, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&contentLength, &length, nullptr);
    }
    {
        DWORD statusCode = 0;
        DWORD length = sizeof(statusCode);
        if (!HttpQueryInfo(hResourceHandle, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&statusCode, &length, nullptr) || statusCode != 200)
        {
            CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L": Download of %s returned %d\n", url.c_str(), statusCode);
            InternetCloseHandle(hResourceHandle);
            InternetCloseHandle(hConnectHandle);
            if (statusCode == 404)
                return false;
            else if (statusCode == 403)
                return false;
            return false;// INET_E_DOWNLOAD_FAILURE;
        }
    }

    HANDLE hDestFile = CreateFile(dest.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hDestFile == INVALID_HANDLE_VALUE)
    {
        InternetCloseHandle(hResourceHandle);
        InternetCloseHandle(hConnectHandle);
        return false;
    }
    DWORD downloadedSum = 0; // sum of bytes downloaded so far
    do
    {
        DWORD size; // size of the data available
        if (!InternetQueryDataAvailable(hResourceHandle, &size, 0, 0))
        {
            DWORD err = GetLastError();
            CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L": Download of %s failed on InternetQueryDataAvailable: %d\n", url.c_str(), err);
            InternetCloseHandle(hResourceHandle);
            InternetCloseHandle(hConnectHandle);
            return err == 0;
        }

        DWORD downloaded; // size of the downloaded data
        auto Data = std::make_unique<TCHAR[]>(size + 1);
        if (!InternetReadFile(hResourceHandle, (LPVOID)Data.get(), size, &downloaded))
        {
            DWORD err = GetLastError();
            CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L": Download of %s failed on InternetReadFile: %d\n", url.c_str(), err);
            InternetCloseHandle(hResourceHandle);
            InternetCloseHandle(hConnectHandle);
            return err == 0;
        }

        if (downloaded == 0)
        {
            break;
        }

        Data[downloaded] = '\0';
        DWORD dwWritten = 0;
        WriteFile(hDestFile, Data.get(), downloaded, &dwWritten, nullptr);

        downloadedSum += downloaded;

        if (contentLength == 0) // got no content-length from web server
        {
            if (m_pProgress)
                m_pProgress->SetProgress(0, 0);
        }
        else
        {
            if (downloadedSum > contentLength)
                downloadedSum = contentLength - 1;
            if (m_pProgress)
            {
                m_pProgress->SetProgress(downloadedSum, contentLength + 1);
                if (m_pProgress->HasUserCancelled())
                {
                    downloadedSum = 0;
                    break;
                }
            }
        }
    }
    while (true);
    CloseHandle(hDestFile);
    InternetCloseHandle(hResourceHandle);
    InternetCloseHandle(hConnectHandle);
    if (downloadedSum == 0)
    {
        CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) L": Download size of %s was zero or user canceled.\n", url.c_str());
        return false;// INET_E_DOWNLOAD_FAILURE;
    }
    return true;
}
