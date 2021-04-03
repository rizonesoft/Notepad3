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
#include "Monitor.h"
#include <vector>
#include <algorithm>

static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC /*hdc*/, LPRECT lprcMonitor, LPARAM pData)
{
    MONITORINFOEX miEx{};
    miEx.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMon, &miEx);

    if (miEx.dwFlags == DISPLAY_DEVICE_MIRRORING_DRIVER)
    {
        return TRUE;
    }

    std::vector<RECT>* pMonRects = reinterpret_cast<std::vector<RECT>*>(pData);
    pMonRects->push_back(*lprcMonitor);
    return TRUE;
}

std::wstring GetMonitorSetupHash()
{
    std::vector<RECT> monRects;
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnum, reinterpret_cast<LPARAM>(&monRects));
    std::sort(monRects.begin(), monRects.end(),
              [](const RECT& a, const RECT& b) -> bool {
                  if (a.left == b.left)
                      return a.top < b.top;
                  return a.left < b.left;
              });
    return GetHashText(monRects.data(), monRects.size() * sizeof(RECT), HashType::HashMd5);
}
