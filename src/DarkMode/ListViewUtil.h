#pragma once

struct SubclassInfo
{
	COLORREF headerTextColor;
};

void InitListView(HWND hListView)
{
	HWND hHeader = ListView_GetHeader(hListView);

	if (g_darkModeSupported)
	{
		SetWindowSubclass(hListView, [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR /*uIdSubclass*/, DWORD_PTR dwRefData) -> LRESULT {
			switch (uMsg)
			{
			case WM_NOTIFY:
			{
				if (reinterpret_cast<LPNMHDR>(lParam)->code == NM_CUSTOMDRAW)
				{
					LPNMCUSTOMDRAW nmcd = reinterpret_cast<LPNMCUSTOMDRAW>(lParam);
					switch (nmcd->dwDrawStage)
					{
					case CDDS_PREPAINT:
						return CDRF_NOTIFYITEMDRAW;
					case CDDS_ITEMPREPAINT:
					{
						auto info = reinterpret_cast<SubclassInfo*>(dwRefData);
						SetTextColor(nmcd->hdc, info->headerTextColor);
						return CDRF_DODEFAULT;
					}
					}
				}
			}
			break;
			case WM_THEMECHANGED:
			{
				if (g_darkModeSupported)
				{
					HWND hHeader = ListView_GetHeader(hWnd);

					AllowDarkModeForWindow(hWnd, g_darkModeEnabled);
					AllowDarkModeForWindow(hHeader, g_darkModeEnabled);

					HTHEME hTheme = OpenThemeData(nullptr, L"ItemsView");
					if (hTheme)
					{
						COLORREF color;
						if (SUCCEEDED(GetThemeColor(hTheme, 0, 0, TMT_TEXTCOLOR, &color)))
						{
							ListView_SetTextColor(hWnd, color);
						}
						if (SUCCEEDED(GetThemeColor(hTheme, 0, 0, TMT_FILLCOLOR, &color)))
						{
							ListView_SetTextBkColor(hWnd, color);
							ListView_SetBkColor(hWnd, color);
						}
						CloseThemeData(hTheme);
					}

					hTheme = OpenThemeData(hHeader, L"Header");
					if (hTheme)
					{
						auto info = reinterpret_cast<SubclassInfo*>(dwRefData);
						GetThemeColor(hTheme, HP_HEADERITEM, 0, TMT_TEXTCOLOR, &(info->headerTextColor));
						CloseThemeData(hTheme);
					}

					SendMessageW(hHeader, WM_THEMECHANGED, wParam, lParam);

					RedrawWindow(hWnd, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE);
				}
			}
			break;
			case WM_DESTROY:
			{
				auto info = reinterpret_cast<SubclassInfo*>(dwRefData);
				delete info;
			}
			break;
			}
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}, 0, reinterpret_cast<DWORD_PTR>(new SubclassInfo{}));
	}

	ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP);

	// Hide focus dots
	SendMessage(hListView, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);

	SetWindowTheme(hHeader, L"ItemsView", nullptr); // DarkMode
	SetWindowTheme(hListView, L"ItemsView", nullptr); // DarkMode
}
