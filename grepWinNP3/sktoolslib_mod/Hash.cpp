// sktoolslib - common files for SK tools

// Copyright (C) 2020-2021 - Stefan Kueng

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
#include "Hash.h"
#include <vector>
#include <Wincrypt.h>

std::wstring GetHashText(const void* data, size_t dataSize, HashType hashType)
{
    HCRYPTPROV hProv = NULL;

    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        return {};
    }

    BOOL       hashOk = FALSE;
    HCRYPTPROV hHash  = NULL;
    switch (hashType)
    {
        case HashType::HashSha1:
            hashOk = CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash);
            break;
        case HashType::HashMd5:
            hashOk = CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash);
            break;
        case HashType::HashSha256:
            hashOk = CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);
            break;
    }

    if (!hashOk)
    {
        CryptReleaseContext(hProv, 0);
        return {};
    }

    if (!CryptHashData(hHash, static_cast<const BYTE*>(data), static_cast<DWORD>(dataSize), 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return {};
    }

    DWORD cbHashSize = 0, dwCount = sizeof(DWORD);
    if (!CryptGetHashParam(hHash, HP_HASHSIZE, reinterpret_cast<BYTE*>(&cbHashSize), &dwCount, 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return {};
    }

    std::vector<BYTE> buffer(cbHashSize);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(&buffer[0]), &cbHashSize, 0))
    {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return {};
    }

    std::wostringstream oss;
    oss.fill('0');
    oss.width(2);

    for (const auto& b : buffer)
    {
        oss << std::hex << static_cast<const int>(b);
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    return oss.str();
}
