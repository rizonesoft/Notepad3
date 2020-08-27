// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Print.cpp                                                                   *
*   Scintilla Printing Functionality                                          *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
* Mostly taken from SciTE, (c) Neil Hodgson                                   *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2020   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/

#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif
#define VC_EXTRALEAN 1
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <windows.h>

#include <commctrl.h>
#include <commdlg.h>
#include <string_view>

extern "C" {
#include "Helpers.h"
#include "MuiLanguage.h"
#include "Dialogs.h"
#include "SciCall.h"
}

#include "resource.h"

extern "C" float Style_GetBaseFontSize();

// Stored objects...
static HGLOBAL hDevMode = nullptr;
static HGLOBAL hDevNames = nullptr;

static void EditPrintInit();


//=============================================================================
//
//  EditPrint() - Code from SciTE
//
void StatusUpdatePrintPage(int iPageNum)
{
  WCHAR tch[80] = { L'\0' };
  FormatLngStringW(tch,COUNTOF(tch),IDS_MUI_PRINTFILE,iPageNum);
  StatusSetText(Globals.hwndStatus,255,tch);
  //RedrawWindow(Globals.hwndStatus, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
  InvalidateRect(Globals.hwndStatus,NULL,TRUE);
  //UpdateWindow(Globals.hwndStatus);
}


extern "C" bool EditPrint(HWND hwnd,LPCWSTR pszDocTitle,LPCWSTR pszPageFormat)
{
  // Don't print empty documents
  if (SendMessage(hwnd,SCI_GETLENGTH,0,0) == 0) {
    InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_PRINT_EMPTY);
    return true;
  }

  PRINTDLG pdlg = { sizeof(PRINTDLG), nullptr, nullptr, nullptr, nullptr, 
    0, 0, 0, 0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

  pdlg.hwndOwner = GetParent(hwnd);
  pdlg.hInstance = Globals.hInstance;
  pdlg.Flags = PD_USEDEVMODECOPIES | PD_ALLPAGES | PD_RETURNDC;
  pdlg.nFromPage = 1;
  pdlg.nToPage = 1;
  pdlg.nMinPage = 1;
  pdlg.nMaxPage = 0xffffU;
  pdlg.nCopies = 1;
  pdlg.hDC = nullptr;
  pdlg.hDevMode = hDevMode;
  pdlg.hDevNames = hDevNames;

  DocPos const startPos = SciCall_GetSelectionStart();
  DocPos const endPos = SciCall_GetSelectionEnd();

  if (startPos == endPos) {
    pdlg.Flags |= PD_NOSELECTION;
  } else {
    pdlg.Flags |= PD_SELECTION;
  }

  // |= 0 - Don't display dialog box, just use the default printer and options
  pdlg.Flags |= (Flags.PrintFileAndLeave == 1) ? PD_RETURNDEFAULT : 0;

  if (!PrintDlg(&pdlg)) {
    return true; // False means error...
  }

  hDevMode = pdlg.hDevMode;
  hDevNames = pdlg.hDevNames;

  HDC const hdc = pdlg.hDC;

  // Get printer resolution
  POINT ptDpi;
  ptDpi.x = GetDeviceCaps(hdc, LOGPIXELSX);    // dpi in X direction
  ptDpi.y = GetDeviceCaps(hdc, LOGPIXELSY);    // dpi in Y direction

  // Start by getting the physical page size (in device units).
  POINT ptPage;
  ptPage.x = GetDeviceCaps(hdc, PHYSICALWIDTH);   // device units
  ptPage.y = GetDeviceCaps(hdc, PHYSICALHEIGHT);  // device units

  // Get the dimensions of the unprintable
  // part of the page (in device units).
  RECT rectPhysMargins;
  rectPhysMargins.left = GetDeviceCaps(hdc, PHYSICALOFFSETX);
  rectPhysMargins.top = GetDeviceCaps(hdc, PHYSICALOFFSETY);

  // To get the right and lower unprintable area,
  // we take the entire width and height of the paper and
  // subtract everything else.
  rectPhysMargins.right = ptPage.x            // total paper width
                          - GetDeviceCaps(hdc, HORZRES) // printable width
                          - rectPhysMargins.left;        // left unprintable margin

  rectPhysMargins.bottom = ptPage.y            // total paper height
                           - GetDeviceCaps(hdc, VERTRES)  // printable height
                           - rectPhysMargins.top;        // right unprintable margin

  // At this point, rectPhysMargins contains the widths of the
  // unprintable regions on all four sides of the page in device units.

  EditPrintInit();

  RECT rectMargins;
  // Take in account the page setup given by the user (if one value is not null)
  if (Settings.PrintMargin.left != 0 || Settings.PrintMargin.right != 0 ||
          Settings.PrintMargin.top != 0 || Settings.PrintMargin.bottom != 0) 
  {
    // Convert the hundredths of millimeters (HiMetric) or
    // thousandths of inches (HiEnglish) margin values
    // from the Page Setup dialog to device units.
    // (There are 2540 hundredths of a mm in an inch.)
    WCHAR localeInfo[3];
    GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IMEASURE, localeInfo, 3);

    RECT rectSetup;
    if (localeInfo[0] == L'0') {  // Metric system. L'1' is US System
      rectSetup.left = MulDiv (Settings.PrintMargin.left, ptDpi.x, 2540);
      rectSetup.top = MulDiv (Settings.PrintMargin.top, ptDpi.y, 2540);
      rectSetup.right  = MulDiv(Settings.PrintMargin.right, ptDpi.x, 2540);
      rectSetup.bottom  = MulDiv(Settings.PrintMargin.bottom, ptDpi.y, 2540);
    } else {
      rectSetup.left  = MulDiv(Settings.PrintMargin.left, ptDpi.x, 1000);
      rectSetup.top  = MulDiv(Settings.PrintMargin.top, ptDpi.y, 1000);
      rectSetup.right  = MulDiv(Settings.PrintMargin.right, ptDpi.x, 1000);
      rectSetup.bottom  = MulDiv(Settings.PrintMargin.bottom, ptDpi.y, 1000);
    }

    // Don't reduce margins below the minimum printable area
    rectMargins.left  = max_l(rectPhysMargins.left, rectSetup.left);
    rectMargins.top  = max_l(rectPhysMargins.top, rectSetup.top);
    rectMargins.right  = max_l(rectPhysMargins.right, rectSetup.right);
    rectMargins.bottom  = max_l(rectPhysMargins.bottom, rectSetup.bottom);
  } else {
    rectMargins.left  = rectPhysMargins.left;
    rectMargins.top  = rectPhysMargins.top;
    rectMargins.right  = rectPhysMargins.right;
    rectMargins.bottom  = rectPhysMargins.bottom;
  }

  // rectMargins now contains the values used to shrink the printable
  // area of the page.

  // Convert device coordinates into logical coordinates
  DPtoLP(hdc, (LPPOINT)&rectMargins, 2);
  DPtoLP(hdc, (LPPOINT)&rectPhysMargins, 2);

  // Convert page size to logical units and we're done!
  DPtoLP(hdc, (LPPOINT) &ptPage, 1);

  int headerLineHeight = MulDiv(8, ptDpi.y, 72);

  HFONT fontHeader = CreateFont(headerLineHeight,
                                0, 0, 0,
                                FW_BOLD,
                                0,
                                0,
                                0, 0, 0,
                                0, 0, 0,
                                L"Arial");

  SelectObject(hdc, fontHeader);

  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);
  headerLineHeight = tm.tmHeight + tm.tmExternalLeading;

  if (Settings.PrintHeader == 3)
    headerLineHeight = 0;

  int footerLineHeight = MulDiv(7,ptDpi.y, 72);
  HFONT fontFooter = CreateFont(footerLineHeight,
                                0, 0, 0,
                                FW_NORMAL,
                                0,
                                0,
                                0, 0, 0,
                                0, 0, 0,
                                L"Arial");

  SelectObject(hdc, fontFooter);
  GetTextMetrics(hdc, &tm);
  footerLineHeight = tm.tmHeight + tm.tmExternalLeading;

  if (Settings.PrintFooter == 1)
    footerLineHeight = 0;

  DOCINFO di = { sizeof(DOCINFO), nullptr, nullptr, nullptr, 0 };
  di.lpszDocName = pszDocTitle;
  di.lpszOutput = nullptr;
  di.lpszDatatype = nullptr;
  di.fwType = 0;
  if (StartDoc(hdc, &di) < 0) {
    DeleteDC(hdc);
    if (fontHeader)
      DeleteObject(fontHeader);
    if (fontFooter)
      DeleteObject(fontFooter);
    return false;
  }

  // Get current date...
  WCHAR dateString[MIDSZ_BUFFER] = { L'\0' };
  SYSTEMTIME st;
  GetLocalTime(&st);
  GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,nullptr,dateString,MIDSZ_BUFFER);

  // Get current time...
  if (Settings.PrintHeader == 0)
  {
    WCHAR timeString[SMALL_BUFFER] = { L'\0' };
    GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,nullptr,timeString,SMALL_BUFFER);
    StringCchCat(dateString,COUNTOF(dateString),L" ");
    StringCchCat(dateString,COUNTOF(dateString),timeString);
  }

  // Set print color mode
  int printColorModes[6] = {
    SC_PRINT_NORMAL,
    SC_PRINT_INVERTLIGHT,
    SC_PRINT_BLACKONWHITE,
    SC_PRINT_COLOURONWHITE,
    SC_PRINT_COLOURONWHITEDEFAULTBG,
    SC_PRINT_SCREENCOLOURS };

  SendMessage(hwnd,SCI_SETPRINTCOLOURMODE,printColorModes[Settings.PrintColorMode],0);
  //SendMessage(hwnd, SCI_SETPRINTWRAPMODE, SC_WRAP_WORD, 0); // default: SC_WRAP_WORD

  // Set print magnification...
  SendMessage(hwnd, SCI_SETPRINTMAGNIFICATION, (WPARAM)Settings.PrintZoom, 0);

  DocPos const lengthDocMax = SciCall_GetTextLength();
  DocPos lengthDoc = lengthDocMax;
  DocPos lengthPrinted = 0;

  // Requested to print selection
  if (pdlg.Flags & PD_SELECTION) {
    if (startPos > endPos) {
      lengthPrinted = endPos;
      lengthDoc = startPos;
    } else {
      lengthPrinted = startPos;
      lengthDoc = endPos;
    }

    if (lengthPrinted < 0)
      lengthPrinted = 0;
    if (lengthDoc > lengthDocMax)
      lengthDoc = lengthDocMax;
  }

  // We must substract the physical margins from the printable area
  struct Sci_RangeToFormat frPrint;
  frPrint.hdc = hdc;
  frPrint.hdcTarget = hdc;
  frPrint.rc.left = rectMargins.left - rectPhysMargins.left;
  frPrint.rc.top = rectMargins.top - rectPhysMargins.top;
  frPrint.rc.right = ptPage.x - rectMargins.right - rectPhysMargins.left;
  frPrint.rc.bottom = ptPage.y - rectMargins.bottom - rectPhysMargins.top;
  frPrint.rcPage.left = 0;
  frPrint.rcPage.top = 0;
  frPrint.rcPage.right = ptPage.x - rectPhysMargins.left - rectPhysMargins.right - 1;
  frPrint.rcPage.bottom = ptPage.y - rectPhysMargins.top - rectPhysMargins.bottom - 1;
  frPrint.rc.top += headerLineHeight + headerLineHeight / 2;
  frPrint.rc.bottom -= footerLineHeight + footerLineHeight / 2;
  
  // Print each page
  int pageNum = 1;
  WCHAR pageString[32] = { L'\0' };

  while (lengthPrinted < lengthDoc) 
  {
    bool printPage = (!(pdlg.Flags & PD_PAGENUMS) ||
      ((pageNum >= pdlg.nFromPage) && (pageNum <= pdlg.nToPage)));

    StringCchPrintf(pageString,COUNTOF(pageString),pszPageFormat,pageNum);

    if (printPage) {

      // Show wait cursor...
      SendMessage(Globals.hwndEdit, SCI_SETCURSOR, (WPARAM)SC_CURSORWAIT, 0);

      // Display current page number in Statusbar
      StatusUpdatePrintPage(pageNum);

      StartPage(hdc);

      SetTextColor(hdc, RGB(0,0,0));
      SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
      SelectObject(hdc, fontHeader);
      UINT ta = SetTextAlign(hdc, TA_BOTTOM);
      RECT rcw = {frPrint.rc.left, frPrint.rc.top - headerLineHeight - headerLineHeight / 2,
                  frPrint.rc.right, frPrint.rc.top - headerLineHeight / 2};
      rcw.bottom = rcw.top + headerLineHeight;

      if (Settings.PrintHeader < 3)
      {
        ExtTextOut(hdc, frPrint.rc.left + 5, frPrint.rc.top - headerLineHeight / 2,
                      /*ETO_OPAQUE*/0, &rcw, pszDocTitle,
                   (UINT)StringCchLenW(pszDocTitle,0), nullptr);
      }

      // Print date in header
      if (Settings.PrintHeader == 0 || Settings.PrintHeader == 1)
      {
        SIZE sizeInfo;
        SelectObject(hdc,fontFooter);
        GetTextExtentPoint32(hdc,dateString,(int)StringCchLenW(dateString,COUNTOF(dateString)),&sizeInfo);
        ExtTextOut(hdc, frPrint.rc.right - 5 - sizeInfo.cx, frPrint.rc.top - headerLineHeight / 2,
                      /*ETO_OPAQUE*/0, &rcw, dateString,
                      (UINT)StringCchLenW(dateString,COUNTOF(dateString)), nullptr);
      }

      if (Settings.PrintHeader < 3)
      {
        SetTextAlign(hdc, ta);
        HPEN pen = CreatePen(0, 1, RGB(0,0,0));
        HPEN const penOld = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, frPrint.rc.left, frPrint.rc.top - headerLineHeight / 4, nullptr);
        LineTo(hdc, frPrint.rc.right, frPrint.rc.top - headerLineHeight / 4);
        SelectObject(hdc, penOld);
        DeleteObject(pen);
      }
    }

    frPrint.chrg.cpMin = static_cast<DocPosCR>(lengthPrinted);
    frPrint.chrg.cpMax = static_cast<DocPosCR>(lengthDoc);

    lengthPrinted = (int)SendMessage(hwnd, SCI_FORMATRANGE, printPage, (LPARAM)&frPrint);

    if (printPage) {
      SetTextColor(hdc, RGB(0,0,0));
      SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
      SelectObject(hdc, fontFooter);
      UINT ta = SetTextAlign(hdc, TA_TOP);
      RECT rcw = {frPrint.rc.left, frPrint.rc.bottom + footerLineHeight / 2,
                  frPrint.rc.right, frPrint.rc.bottom + footerLineHeight + footerLineHeight / 2};

      if (Settings.PrintFooter == 0)
      {
        SIZE sizeFooter;
        GetTextExtentPoint32(hdc,pageString,(int)StringCchLenW(pageString,COUNTOF(pageString)),&sizeFooter);
        ExtTextOut(hdc, frPrint.rc.right - 5 - sizeFooter.cx, frPrint.rc.bottom + footerLineHeight / 2,
                      /*ETO_OPAQUE*/0, &rcw, pageString,
                      (UINT)StringCchLenW(pageString,COUNTOF(pageString)), nullptr);

        SetTextAlign(hdc, ta);
        HPEN pen = ::CreatePen(0, 1, RGB(0,0,0));
        HPEN const penOld = (HPEN)SelectObject(hdc, pen);
        SetBkColor(hdc, RGB(0,0,0));
        MoveToEx(hdc, frPrint.rc.left, frPrint.rc.bottom + footerLineHeight / 4, nullptr);
        LineTo(hdc, frPrint.rc.right, frPrint.rc.bottom + footerLineHeight / 4);
        SelectObject(hdc, penOld);
        DeleteObject(pen);
      }

      EndPage(hdc);
    }
    pageNum++;

    if ((pdlg.Flags & PD_PAGENUMS) && (pageNum > pdlg.nToPage))
      break;
  }

  SendMessage(hwnd,SCI_FORMATRANGE, false, 0);

  EndDoc(hdc);
  DeleteDC(hdc);
  if (fontHeader)
    DeleteObject(fontHeader);
  if (fontFooter)
    DeleteObject(fontFooter);

  // Reset Statusbar to default mode
  StatusSetSimple(Globals.hwndStatus,false);

  // Remove wait cursor...
  { POINT pt; SendMessage(Globals.hwndEdit, SCI_SETCURSOR, (WPARAM)SC_CURSORNORMAL, 0); GetCursorPos(&pt); SetCursorPos(pt.x, pt.y); }

  return true;
}


//=============================================================================
//
//  EditPrintSetup() - Code from SciTE
//
//  Custom controls: 30 Zoom
//                   31 Spin
//                   32 Header
//                   33 Footer
//                   34 Colors
//
extern "C" UINT_PTR CALLBACK PageSetupHook(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  UNUSED(lParam);

  switch (uiMsg)
  {
    case WM_INITDIALOG:
      {
        WCHAR tch[512];
        WCHAR *p1,*p2;

        SetDialogIconNP3(hwnd);

        UDACCEL const acc[1] = { { 0, 10 } };
        SendDlgItemMessage(hwnd,30,EM_LIMITTEXT,32,0);
        SendDlgItemMessage(hwnd,31,UDM_SETACCEL,1,(WPARAM)acc);
        SendDlgItemMessage(hwnd,31,UDM_SETRANGE32,SC_MIN_ZOOM_LEVEL,SC_MAX_ZOOM_LEVEL);
        SendDlgItemMessage(hwnd,31,UDM_SETPOS32,0,Settings.PrintZoom);

        // Set header options
        GetLngString(IDS_MUI_PRINT_HEADER,tch,COUNTOF(tch));
        StringCchCat(tch,COUNTOF(tch),L"|");
        p1 = tch;
        p2 = StrChr(p1, L'|');
        while (p2) {
          *p2++ = L'\0';
          if (*p1)
            SendDlgItemMessage(hwnd,32,CB_ADDSTRING,0,(LPARAM)p1);
          p1 = p2;
          p2 = StrChr(p1, L'|');  // next
        }
        SendDlgItemMessage(hwnd,32,CB_SETCURSEL,(WPARAM)Settings.PrintHeader,0);

        // Set footer options
        GetLngString(IDS_MUI_PRINT_FOOTER,tch,COUNTOF(tch));
        StringCchCat(tch,COUNTOF(tch),L"|");
        p1 = tch;
        p2 = StrChr(p1, L'|');
        while (p2) {
          *p2++ = L'\0';
          if (*p1)
            SendDlgItemMessage(hwnd,33,CB_ADDSTRING,0,(LPARAM)p1);
          p1 = p2;
          p2 = StrChr(p1, L'|');  // next
        }
        SendDlgItemMessage(hwnd,33,CB_SETCURSEL,(WPARAM)Settings.PrintFooter,0);

        // Set color options
        GetLngString(IDS_MUI_PRINT_COLOR,tch,COUNTOF(tch));
        StringCchCat(tch,COUNTOF(tch),L"|");
        p1 = tch;
        p2 = StrChr(p1, L'|');
        while (p2) {
          *p2++ = L'\0';
          if (*p1)
            SendDlgItemMessage(hwnd,34,CB_ADDSTRING,0,(LPARAM)p1);
          p1 = p2;
          p2 = StrChr(p1, L'|');  // next
        }
        SendDlgItemMessage(hwnd,34,CB_SETCURSEL,(LPARAM)Settings.PrintColorMode,0);

        // Make combos handier
        SendDlgItemMessage(hwnd,32,CB_SETEXTENDEDUI,true,0);
        SendDlgItemMessage(hwnd,33,CB_SETEXTENDEDUI,true,0);
        SendDlgItemMessage(hwnd,34,CB_SETEXTENDEDUI,true,0);
        SendDlgItemMessage(hwnd,1137,CB_SETEXTENDEDUI,true,0);
        SendDlgItemMessage(hwnd,1138,CB_SETEXTENDEDUI,true,0);
      }
      break;

    case WM_DPICHANGED:
      UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, NULL);
      break;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK)
      {
        BOOL bError = FALSE;
        int const iZoom = (int)SendDlgItemMessage(hwnd,31,UDM_GETPOS32,0,(LPARAM)&bError);
        Settings.PrintZoom = bError ? Defaults.PrintZoom : iZoom;
        /*int const iFontSize = (int)*/ SendDlgItemMessage(hwnd, 41, UDM_GETPOS32, 0, (LPARAM)&bError);
        Settings.PrintHeader = (int)SendDlgItemMessage(hwnd, 32, CB_GETCURSEL, 0, 0);
        Settings.PrintFooter = (int)SendDlgItemMessage(hwnd, 33, CB_GETCURSEL, 0, 0);
        Settings.PrintColorMode = (int)SendDlgItemMessage(hwnd, 34, CB_GETCURSEL, 0, 0);
      }
      else if (LOWORD(wParam) == IDC_PRINTER)
      {
        PostWMCommand(hwnd, 1026);
      }
      break;

    default:
      break;
  }
  return 0;
}


extern "C" void EditPrintSetup(HWND hwnd)
{
  DLGTEMPLATE* pDlgTemplate = LoadThemedDialogTemplate(MAKEINTRESOURCE(IDD_MUI_PAGESETUP),Globals.hLngResContainer);

  PAGESETUPDLG pdlg;
  ZeroMemory(&pdlg,sizeof(PAGESETUPDLG));
  pdlg.lStructSize = sizeof(PAGESETUPDLG);
  pdlg.Flags = PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGESETUPTEMPLATEHANDLE;
  pdlg.lpfnPageSetupHook = PageSetupHook;
  pdlg.hPageSetupTemplate = pDlgTemplate;
  pdlg.hwndOwner = GetParent(hwnd);
  pdlg.hInstance = Globals.hInstance;

  EditPrintInit();

  if (Settings.PrintMargin.left != 0 || Settings.PrintMargin.right != 0 ||
          Settings.PrintMargin.top != 0 || Settings.PrintMargin.bottom != 0) {
    pdlg.Flags |= PSD_MARGINS;

    pdlg.rtMargin.left = Settings.PrintMargin.left;
    pdlg.rtMargin.top = Settings.PrintMargin.top;
    pdlg.rtMargin.right = Settings.PrintMargin.right;
    pdlg.rtMargin.bottom = Settings.PrintMargin.bottom;
  }

  pdlg.hDevMode = hDevMode;
  pdlg.hDevNames = hDevNames;

  if (PageSetupDlg(&pdlg)) {

    Settings.PrintMargin.left = pdlg.rtMargin.left;
    Settings.PrintMargin.top = pdlg.rtMargin.top;
    Settings.PrintMargin.right = pdlg.rtMargin.right;
    Settings.PrintMargin.bottom = pdlg.rtMargin.bottom;

    hDevMode = pdlg.hDevMode;
    hDevNames = pdlg.hDevNames;
  }

  FreeMem(pDlgTemplate);
}


//=============================================================================
//
//  EditPrintInit() - Setup default page margin if no values from registry
//
static void EditPrintInit()
{
  if (Settings.PrintMargin.left  <= -1 || Settings.PrintMargin.top    <= -1 ||
      Settings.PrintMargin.right <= -1 || Settings.PrintMargin.bottom <= -1) {
    WCHAR localeInfo[3];
    GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IMEASURE, localeInfo, 3);
    LONG const _margin = (localeInfo[0] == L'0') ? 2000L : 1000L;  // Metric system. L'1' is US System
    Settings.PrintMargin = { _margin, _margin, _margin, _margin };
  }
}

#if 0
#include <fcntl.h>
#include <io.h>
//#include <iostream>
//#include <fstream>

void RedirectIOToConsole()
{
  int hConHandle;
  intptr_t lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;

  // allocate a console for this app
  AllocConsole();
  // set the screen buffer to be big enough to let us scroll text
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  coninfo.dwSize.Y = 500;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

  // redirect unbuffered STDOUT to the console
  lStdHandle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "w");
  *stdout = *fp;
  setvbuf(stdout, NULL, _IONBF, 0);

  // redirect unbuffered STDIN to the console
  lStdHandle = (intptr_t)GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "r");
  *stdin = *fp;
  setvbuf(stdin, NULL, _IONBF, 0);

  // redirect unbuffered STDERR to the console
  lStdHandle = (intptr_t)GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen(hConHandle, "w");
  *stderr = *fp;
  setvbuf(stderr, NULL, _IONBF, 0);

  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
  std::ios::sync_with_stdio();
}
#endif

// End of Print.cpp
