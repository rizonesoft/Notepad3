/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* Print.cpp                                                                   *
*   Scintilla Printing Functionality                                          *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*	Mostly taken from SciTE, (c) Neil Hodgson                                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2016   *
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

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <string.h>
#include "platform.h"
#include "scintilla.h"
#include "scilexer.h"
#include "resource.h"
extern "C" {
#include "dialogs.h"
#include "helpers.h"
}


extern "C" HINSTANCE g_hInstance;
extern "C" HWND hwndEdit;

// Global settings...
extern "C" int iPrintHeader;
extern "C" int iPrintFooter;
extern "C" int iPrintColor;
extern "C" int iPrintZoom;
extern "C" RECT pagesetupMargin;
extern "C" int flagPrintFileAndLeave;


// Stored objects...
HGLOBAL hDevMode = nullptr;
HGLOBAL hDevNames = nullptr;


//=============================================================================
//
//  EditPrint() - Code from SciTE
//
extern "C" HWND hwndStatus;

void StatusUpdatePrintPage(int iPageNum)
{
  WCHAR tch[32] = { L'\0' };

  FormatString(tch,COUNTOF(tch),IDS_PRINTFILE,iPageNum);

  StatusSetText(hwndStatus,255,tch);
  StatusSetSimple(hwndStatus,TRUE);

  InvalidateRect(hwndStatus,nullptr,TRUE);
  UpdateWindow(hwndStatus);
}


extern "C" BOOL EditPrint(HWND hwnd,LPCWSTR pszDocTitle,LPCWSTR pszPageFormat)
{

  // Don't print empty documents
  if (SendMessage(hwnd,SCI_GETLENGTH,0,0) == 0) {
    MsgBox(MBWARN,IDS_PRINT_EMPTY);
    return TRUE;
  }

  int startPos;
  int endPos;

  HDC hdc;

  RECT rectMargins;
  RECT rectPhysMargins;
  RECT rectSetup;
  POINT ptPage;
  POINT ptDpi;

  //RECT rectSetup;

  TEXTMETRIC tm;

  int headerLineHeight;
  HFONT fontHeader;

  int footerLineHeight;
  HFONT fontFooter;

  WCHAR dateString[MIDSZ_BUFFER] = { L'\0' };

  DOCINFO di = {sizeof(DOCINFO), nullptr, nullptr, nullptr, 0};

  int lengthDoc;
  int lengthDocMax;
  int lengthPrinted;

  struct Sci_RangeToFormat frPrint;

  int pageNum;
  BOOL printPage;

  WCHAR pageString[32] = { L'\0' };

  HPEN pen;
  HPEN penOld;

  PRINTDLG pdlg = { sizeof(PRINTDLG), nullptr, nullptr, nullptr, nullptr, 
    0, 0, 0, 0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
  pdlg.hwndOwner = GetParent(hwnd);
  pdlg.hInstance = g_hInstance;
  pdlg.Flags = PD_USEDEVMODECOPIES | PD_ALLPAGES | PD_RETURNDC;
  pdlg.nFromPage = 1;
  pdlg.nToPage = 1;
  pdlg.nMinPage = 1;
  pdlg.nMaxPage = 0xffffU;
  pdlg.nCopies = 1;
  pdlg.hDC = nullptr;
  pdlg.hDevMode = hDevMode;
  pdlg.hDevNames = hDevNames;

  startPos = (int)SendMessage(hwnd,SCI_GETSELECTIONSTART,0,0);
  endPos = (int)SendMessage(hwnd,SCI_GETSELECTIONEND,0,0);

  if (startPos == endPos) {
    pdlg.Flags |= PD_NOSELECTION;
  } else {
    pdlg.Flags |= PD_SELECTION;
  }

  // |= 0 - Don't display dialog box, just use the default printer and options
  pdlg.Flags |= (flagPrintFileAndLeave == 1) ? PD_RETURNDEFAULT : 0;

  if (!PrintDlg(&pdlg)) {
    return TRUE; // False means error...
  }

  hDevMode = pdlg.hDevMode;
  hDevNames = pdlg.hDevNames;

  hdc = pdlg.hDC;

  // Get printer resolution
  ptDpi.x = GetDeviceCaps(hdc, LOGPIXELSX);    // dpi in X direction
  ptDpi.y = GetDeviceCaps(hdc, LOGPIXELSY);    // dpi in Y direction

  // Start by getting the physical page size (in device units).
  ptPage.x = GetDeviceCaps(hdc, PHYSICALWIDTH);   // device units
  ptPage.y = GetDeviceCaps(hdc, PHYSICALHEIGHT);  // device units

  // Get the dimensions of the unprintable
  // part of the page (in device units).
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

  // Take in account the page setup given by the user (if one value is not null)
  if (pagesetupMargin.left != 0 || pagesetupMargin.right != 0 ||
          pagesetupMargin.top != 0 || pagesetupMargin.bottom != 0) {

    // Convert the hundredths of millimeters (HiMetric) or
    // thousandths of inches (HiEnglish) margin values
    // from the Page Setup dialog to device units.
    // (There are 2540 hundredths of a mm in an inch.)

    WCHAR localeInfo[3];
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, localeInfo, 3);

    if (localeInfo[0] == L'0') {  // Metric system. L'1' is US System
      rectSetup.left = MulDiv (pagesetupMargin.left, ptDpi.x, 2540);
      rectSetup.top = MulDiv (pagesetupMargin.top, ptDpi.y, 2540);
      rectSetup.right  = MulDiv(pagesetupMargin.right, ptDpi.x, 2540);
      rectSetup.bottom  = MulDiv(pagesetupMargin.bottom, ptDpi.y, 2540);
    } else {
      rectSetup.left  = MulDiv(pagesetupMargin.left, ptDpi.x, 1000);
      rectSetup.top  = MulDiv(pagesetupMargin.top, ptDpi.y, 1000);
      rectSetup.right  = MulDiv(pagesetupMargin.right, ptDpi.x, 1000);
      rectSetup.bottom  = MulDiv(pagesetupMargin.bottom, ptDpi.y, 1000);
    }

    // Dont reduce margins below the minimum printable area
    rectMargins.left  = max(rectPhysMargins.left, rectSetup.left);
    rectMargins.top  = max(rectPhysMargins.top, rectSetup.top);
    rectMargins.right  = max(rectPhysMargins.right, rectSetup.right);
    rectMargins.bottom  = max(rectPhysMargins.bottom, rectSetup.bottom);
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

  headerLineHeight = MulDiv(8,ptDpi.y, 72);
  fontHeader = CreateFont(headerLineHeight,
                          0, 0, 0,
                          FW_BOLD,
                          0,
                          0,
                          0, 0, 0,
                          0, 0, 0,
                          L"Arial");
  SelectObject(hdc, fontHeader);
  GetTextMetrics(hdc, &tm);
  headerLineHeight = tm.tmHeight + tm.tmExternalLeading;

  if (iPrintHeader == 3)
    headerLineHeight = 0;

  footerLineHeight = MulDiv(7,ptDpi.y, 72);
  fontFooter = CreateFont(footerLineHeight,
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

  if (iPrintFooter == 1)
    footerLineHeight = 0;

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
    return FALSE;
  }

  // Get current date...
  SYSTEMTIME st;
  GetLocalTime(&st);
  GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,nullptr,dateString,MIDSZ_BUFFER);

  // Get current time...
  if (iPrintHeader == 0)
  {
    WCHAR timeString[SMALL_BUFFER] = { L'\0' };
    GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,nullptr,timeString,SMALL_BUFFER);
    StringCchCat(dateString,COUNTOF(dateString),L" ");
    StringCchCat(dateString,COUNTOF(dateString),timeString);
  }

  // Set print color mode
  int printColorModes[5] = {
    SC_PRINT_NORMAL,
    SC_PRINT_INVERTLIGHT,
    SC_PRINT_BLACKONWHITE,
    SC_PRINT_COLOURONWHITE,
    SC_PRINT_COLOURONWHITEDEFAULTBG };
  SendMessage(hwnd,SCI_SETPRINTCOLOURMODE,printColorModes[iPrintColor],0);

  // Set print zoom...
  SendMessage(hwnd,SCI_SETPRINTMAGNIFICATION,(WPARAM)iPrintZoom,0);

  lengthDoc = (int)SendMessage(hwnd,SCI_GETLENGTH,0,0);
  lengthDocMax = lengthDoc;
  lengthPrinted = 0;

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
  pageNum = 1;

  while (lengthPrinted < lengthDoc) {
    printPage = (!(pdlg.Flags & PD_PAGENUMS) ||
                 (pageNum >= pdlg.nFromPage) && (pageNum <= pdlg.nToPage));

    StringCchPrintf(pageString,COUNTOF(pageString),pszPageFormat,pageNum);

    if (printPage) {

      // Show wait cursor...
      SendMessage(hwndEdit, SCI_SETCURSOR, (WPARAM)SC_CURSORWAIT, 0);

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

      if (iPrintHeader < 3)
      {
        ExtTextOut(hdc, frPrint.rc.left + 5, frPrint.rc.top - headerLineHeight / 2,
                      /*ETO_OPAQUE*/0, &rcw, pszDocTitle,
                      lstrlen(pszDocTitle), nullptr);
      }

      // Print date in header
      if (iPrintHeader == 0 || iPrintHeader == 1)
      {
        SIZE sizeInfo;
        SelectObject(hdc,fontFooter);
        GetTextExtentPoint32(hdc,dateString,StringCchLenW(dateString,COUNTOF(dateString)),&sizeInfo);
        ExtTextOut(hdc, frPrint.rc.right - 5 - sizeInfo.cx, frPrint.rc.top - headerLineHeight / 2,
                      /*ETO_OPAQUE*/0, &rcw, dateString,
                      StringCchLenW(dateString,COUNTOF(dateString)), nullptr);
      }

      if (iPrintHeader < 3)
      {
        SetTextAlign(hdc, ta);
        pen = CreatePen(0, 1, RGB(0,0,0));
        penOld = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, frPrint.rc.left, frPrint.rc.top - headerLineHeight / 4, nullptr);
        LineTo(hdc, frPrint.rc.right, frPrint.rc.top - headerLineHeight / 4);
        SelectObject(hdc, penOld);
        DeleteObject(pen);
      }
    }

    frPrint.chrg.cpMin = lengthPrinted;
    frPrint.chrg.cpMax = lengthDoc;

    lengthPrinted = (int)SendMessage(hwnd, SCI_FORMATRANGE, printPage, (LPARAM)&frPrint);

    if (printPage) {
      SetTextColor(hdc, RGB(0,0,0));
      SetBkColor(hdc, RGB(0xFF, 0xFF, 0xFF));
      SelectObject(hdc, fontFooter);
      UINT ta = SetTextAlign(hdc, TA_TOP);
      RECT rcw = {frPrint.rc.left, frPrint.rc.bottom + footerLineHeight / 2,
                  frPrint.rc.right, frPrint.rc.bottom + footerLineHeight + footerLineHeight / 2};

      if (iPrintFooter == 0)
      {
        SIZE sizeFooter;
        GetTextExtentPoint32(hdc,pageString,StringCchLenW(pageString,COUNTOF(pageString)),&sizeFooter);
        ExtTextOut(hdc, frPrint.rc.right - 5 - sizeFooter.cx, frPrint.rc.bottom + footerLineHeight / 2,
                      /*ETO_OPAQUE*/0, &rcw, pageString,
                      StringCchLenW(pageString,COUNTOF(pageString)), nullptr);

        SetTextAlign(hdc, ta);
        pen = ::CreatePen(0, 1, RGB(0,0,0));
        penOld = (HPEN)SelectObject(hdc, pen);
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

  SendMessage(hwnd,SCI_FORMATRANGE, FALSE, 0);

  EndDoc(hdc);
  DeleteDC(hdc);
  if (fontHeader)
    DeleteObject(fontHeader);
  if (fontFooter)
    DeleteObject(fontFooter);

  // Reset Statusbar to default mode
  StatusSetSimple(hwndStatus,FALSE);

  // Remove wait cursor...
  { POINT pt; SendMessage(hwndEdit, SCI_SETCURSOR, (WPARAM)SC_CURSORNORMAL, 0); GetCursorPos(&pt); SetCursorPos(pt.x, pt.y); }

  return TRUE;
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
  switch (uiMsg)
  {
    case WM_INITDIALOG:
      {
        WCHAR tch[512];
        WCHAR *p1,*p2;

        SendDlgItemMessage(hwnd,30,EM_LIMITTEXT,32,0);

        SendDlgItemMessage(hwnd,31,UDM_SETRANGE,0,MAKELONG((short)20,(short)-10));
        SendDlgItemMessage(hwnd,31,UDM_SETPOS,0,MAKELONG((short)iPrintZoom,0));

        // Set header options
        GetString(IDS_PRINT_HEADER,tch,COUNTOF(tch));
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
        SendDlgItemMessage(hwnd,32,CB_SETCURSEL,(WPARAM)iPrintHeader,0);

        // Set footer options
        GetString(IDS_PRINT_FOOTER,tch,COUNTOF(tch));
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
        SendDlgItemMessage(hwnd,33,CB_SETCURSEL,(WPARAM)iPrintFooter,0);

        // Set color options
        GetString(IDS_PRINT_COLOR,tch,COUNTOF(tch));
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
        SendDlgItemMessage(hwnd,34,CB_SETCURSEL,(WPARAM)iPrintColor,0);

        // Make combos handier
        SendDlgItemMessage(hwnd,32,CB_SETEXTENDEDUI,TRUE,0);
        SendDlgItemMessage(hwnd,33,CB_SETEXTENDEDUI,TRUE,0);
        SendDlgItemMessage(hwnd,34,CB_SETEXTENDEDUI,TRUE,0);
        SendDlgItemMessage(hwnd,1137,CB_SETEXTENDEDUI,TRUE,0);
        SendDlgItemMessage(hwnd,1138,CB_SETEXTENDEDUI,TRUE,0);
      }
      break;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK)
      {
        int iPos = (int)SendDlgItemMessage(hwnd,31,UDM_GETPOS,0,0);
        if (HIWORD(iPos) == 0)
          iPrintZoom = (int)(short)LOWORD(iPos);
        else
          iPrintZoom = 0;

        iPrintHeader = (int)SendDlgItemMessage(hwnd, 32, CB_GETCURSEL, 0, 0);
        iPrintFooter = (int)SendDlgItemMessage(hwnd, 33, CB_GETCURSEL, 0, 0);
        iPrintColor = (int)SendDlgItemMessage(hwnd, 34, CB_GETCURSEL, 0, 0);
      }
      break;

    default:
      break;
  }

  UNUSED(lParam);

  return(0);
}


extern "C" void EditPrintSetup(HWND hwnd)
{
  DLGTEMPLATE* pDlgTemplate =
    LoadThemedDialogTemplate(MAKEINTRESOURCE(IDD_PAGESETUP),g_hInstance);

  PAGESETUPDLG pdlg;
  ZeroMemory(&pdlg,sizeof(PAGESETUPDLG));
  pdlg.lStructSize = sizeof(PAGESETUPDLG);
  pdlg.Flags = PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGESETUPTEMPLATEHANDLE;
  pdlg.lpfnPageSetupHook = PageSetupHook;
  pdlg.hPageSetupTemplate = pDlgTemplate;
  pdlg.hwndOwner = GetParent(hwnd);
  pdlg.hInstance = g_hInstance;

  if (pagesetupMargin.left != 0 || pagesetupMargin.right != 0 ||
          pagesetupMargin.top != 0 || pagesetupMargin.bottom != 0) {
    pdlg.Flags |= PSD_MARGINS;

    pdlg.rtMargin.left = pagesetupMargin.left;
    pdlg.rtMargin.top = pagesetupMargin.top;
    pdlg.rtMargin.right = pagesetupMargin.right;
    pdlg.rtMargin.bottom = pagesetupMargin.bottom;
  }

  pdlg.hDevMode = hDevMode;
  pdlg.hDevNames = hDevNames;

  if (PageSetupDlg(&pdlg)) {

    pagesetupMargin.left = pdlg.rtMargin.left;
    pagesetupMargin.top = pdlg.rtMargin.top;
    pagesetupMargin.right = pdlg.rtMargin.right;
    pagesetupMargin.bottom = pdlg.rtMargin.bottom;

    hDevMode = pdlg.hDevMode;
    hDevNames = pdlg.hDevNames;
  }

  LocalFree(pDlgTemplate);
}


//=============================================================================
//
//  EditPrintInit() - Setup default page margin if no values from registry
//
extern "C" void EditPrintInit()
{
  if (pagesetupMargin.left == -1 || pagesetupMargin.top == -1 ||
      pagesetupMargin.right == -1 || pagesetupMargin.bottom == -1)
  {
    WCHAR localeInfo[3];
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, localeInfo, 3);

    if (localeInfo[0] == L'0') {  // Metric system. L'1' is US System
      pagesetupMargin.left = 2000;
      pagesetupMargin.top = 2000;
      pagesetupMargin.right = 2000;
      pagesetupMargin.bottom = 2000; }

    else {
      pagesetupMargin.left = 1000;
      pagesetupMargin.top = 1000;
      pagesetupMargin.right = 1000;
      pagesetupMargin.bottom = 1000; }
  }
}


// End of Print.cpp
