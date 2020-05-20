// encoding: UTF-8
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include <shlwapi.h>

#include "ChooseFont.h"
#include "FontEnumeration.h"
#include "GdiTextRenderer.h"
#include "Scintilla.h"

extern "C" {
#include "../resource.h"
#include "TypeDefs.h"
}

//----------------------------------------------------------------------------

IDWriteFactory* g_dwrite = nullptr;

static HINSTANCE g_hInstanceNP3;

extern "C" void CenterDlgInParent(HWND hDlg);


//=============================================================================
//
//  LoadLngString()
//
static int LoadLngStringW(UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
  const int nLen = LoadStringW(Globals.hLngResContainer, uID, lpBuffer, nBufferMax);
  return (nLen != 0) ? nLen : LoadStringW(g_hInstanceNP3, uID, lpBuffer, nBufferMax);
}


/******************************************************************
*                                                                 *
* The ChooseFontDialog class controls all the UI associated with  *
* the dialog and uses the support routines in FontEnumeration.cpp *
* to enumerate fonts and get various information about each one.  *
*                                                                 *
******************************************************************/

class ChooseFontDialog
{
public:

  ChooseFontDialog(HWND hParent, const WCHAR* localeName, DPI_T dpi, LPCHOOSEFONT lpCFGDI);
  ~ChooseFontDialog();
  ChooseFontDialog() = delete;

  HRESULT GetTextFormat(IDWriteTextFormat** textFormat);
  HRESULT GetTextFormat(IDWriteTextFormat* textFormatIn, IDWriteTextFormat** textFormatOut);
  void    GetFontStyle(LPWSTR fontStyle, size_t cchMax);

private:

  HWND                    m_parent;
  HWND                    m_dialog;
  WCHAR                   m_localeName[LOCALE_NAME_MAX_LENGTH];
  DPI_T                   m_currentDPI;
  LPCHOOSEFONT            m_chooseFontStruct;
  IDWriteFontCollection*  m_fontCollection;
  IDWriteTextFormat*      m_currentTextFormat;
  IDWriteTextFormat*      m_renderTextFormat;
  WCHAR                   m_fontStyle[LF_FULLFACESIZE];

  HRESULT OnFontFamilySelect();
  HRESULT OnFontFaceSelect();
  HRESULT OnFontSizeSelect();

  HRESULT OnFontFamilyNameEdit(HWND hwndFontFamilies);
  HRESULT OnFontFaceNameEdit(HWND hwnd);
  HRESULT OnFontSizeNameEdit(HWND hwnd);

  HRESULT DrawSampleText(HDC sampleDC);

  static INT_PTR CALLBACK CFDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  BOOL OnInitDialog(HWND dialog, HWND hwndFocus, LPARAM lParam);
  void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
  void OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem);
};


void   ChooseFontDialog::GetFontStyle(LPWSTR fontStyle, size_t cchMax)
{
  StringCchCopy(fontStyle, cchMax, m_fontStyle);
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::ChooseFontDialog                              *
*                                                                 *
* Prepare to display the dialog                                   *
*                                                                 *
******************************************************************/

ChooseFontDialog::ChooseFontDialog(HWND hParent, const WCHAR* localeName, const DPI_T dpi, LPCHOOSEFONT lpCFGDI)
  : m_parent(hParent)
  , m_dialog(nullptr)
  , m_currentDPI(dpi)
  , m_chooseFontStruct(lpCFGDI)
  , m_fontCollection(nullptr)
  , m_currentTextFormat(nullptr)
  , m_renderTextFormat(nullptr)
{
  if (localeName != nullptr) {
    StringCchCopy(m_localeName, _ARRAYSIZE(m_localeName), localeName);
  }
  else {
    // Default to the users' locale
    //GetUserDefaultLocaleName(&m_localeName[0], COUNTOF(m_localeName));
    GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, &m_localeName[0], _ARRAYSIZE(m_localeName));
  }
  StringCchCopy(m_fontStyle, _ARRAYSIZE(m_fontStyle), L"");
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::~ChooseFontDialog                             *
*                                                                 *
* Clean up resources                                              *
*                                                                 *
******************************************************************/

ChooseFontDialog::~ChooseFontDialog()
{
  SafeRelease(&m_fontCollection);
  SafeRelease(&m_currentTextFormat);
  SafeRelease(&m_renderTextFormat);
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::GetTextFormat                                 *
*                                                                 *
* Create and display the dialog initialized to default attributes *
*                                                                 *
******************************************************************/

static DWRITE_FONT_WEIGHT GetFontWeightValue(LONG fontWeight)
{
  if (fontWeight < 150) {
    return DWRITE_FONT_WEIGHT_THIN;
  }
  else if (fontWeight < 250) {
    return DWRITE_FONT_WEIGHT_EXTRA_LIGHT; // == DWRITE_FONT_WEIGHT_ULTRA_LIGHT
  }
  else if (fontWeight < 325) {
    return DWRITE_FONT_WEIGHT_LIGHT;
  }
  else if (fontWeight < 375) {
    return DWRITE_FONT_WEIGHT_SEMI_LIGHT;
  }
  else if (fontWeight < 450) {
    return DWRITE_FONT_WEIGHT_NORMAL; // == DWRITE_FONT_WEIGHT_REGULAR
  }
  else if (fontWeight < 550) {
    return DWRITE_FONT_WEIGHT_MEDIUM;
  }
  else if (fontWeight < 650) {
    return DWRITE_FONT_WEIGHT_SEMI_BOLD; // == DWRITE_FONT_WEIGHT_DEMI_BOLD
  }
  else if (fontWeight < 750) {
    return DWRITE_FONT_WEIGHT_BOLD;
  }
  else if (fontWeight < 850) {
    return DWRITE_FONT_WEIGHT_EXTRA_BOLD; // == DWRITE_FONT_WEIGHT_ULTRA_BOLD
  }
  else if (fontWeight < 950) {
    return DWRITE_FONT_WEIGHT_HEAVY;  // == DWRITE_FONT_WEIGHT_BLACK
  }
  return DWRITE_FONT_WEIGHT_ULTRA_BLACK; // == DWRITE_FONT_WEIGHT_EXTRA_BLACK
}


static DWRITE_FONT_STYLE GetFontStyleValue(LPCWSTR const fontStyle, bool bItalic)
{
  if (StrStrI(fontStyle, L"oblique")) {
    return DWRITE_FONT_STYLE_OBLIQUE;
  }
  else if (StrStrI(fontStyle, L"italic") || bItalic) {
    return DWRITE_FONT_STYLE_ITALIC;
  }
  return DWRITE_FONT_STYLE_NORMAL;
}


static DWRITE_FONT_STRETCH GetFontStrechValue(LPCWSTR const fontStyle)
{
  if (StrStrI(fontStyle, L"condensed")) {
    return DWRITE_FONT_STRETCH_CONDENSED;
  }
  else if (StrStrI(fontStyle, L"extended")) {
    return DWRITE_FONT_STRETCH_EXPANDED;
  }
  else if (StrStrI(fontStyle, L"expanded")) {
    return DWRITE_FONT_STRETCH_EXPANDED;
  }
  return DWRITE_FONT_STRETCH_NORMAL;
}


HRESULT ChooseFontDialog::GetTextFormat(IDWriteTextFormat** textFormat)
{
  *textFormat = nullptr;

  // Default to the system font collection
  SafeRelease(&m_fontCollection);
  HRESULT hr = g_dwrite->GetSystemFontCollection(&m_fontCollection);

  // Create a default text format
  if (SUCCEEDED(hr)) {
    SafeRelease(&m_currentTextFormat);

    const WCHAR* const fontFamilyName = m_chooseFontStruct->lpLogFont->lfFaceName;
    const WCHAR* const fontStyleStrg = m_chooseFontStruct->lpszStyle;
    float const pointSize = static_cast<float>(m_chooseFontStruct->iPointSize) / 10.0f;

    DWRITE_FONT_WEIGHT  const fontWeight  = GetFontWeightValue(m_chooseFontStruct->lpLogFont->lfWeight);
    DWRITE_FONT_STYLE   const fontStyle   = GetFontStyleValue(fontStyleStrg, m_chooseFontStruct->lpLogFont->lfItalic);
    DWRITE_FONT_STRETCH const fontStretch = GetFontStrechValue(fontStyleStrg);

    hr = g_dwrite->CreateTextFormat(
      fontFamilyName,
      m_fontCollection,
      fontWeight,
      fontStyle,
      fontStretch,
      pointSize,
      m_localeName,
      &m_currentTextFormat);
  }

  // Open the dialog
  if (SUCCEEDED(hr)) {
    if (Globals.hLngResContainer) {
      hr = static_cast<HRESULT>(DialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_CHOOSEFONT), m_parent, CFDialogProc, reinterpret_cast<LPARAM>(this)));
    }
    else {
      hr = static_cast<HRESULT>(DialogBoxParam(g_hInstanceNP3, MAKEINTRESOURCE(IDD_MUI_CHOOSEFONT), m_parent, CFDialogProc, reinterpret_cast<LPARAM>(this)));
    }
  }

  // If all went well, and the user didn't cancel, return the new format.
  if (hr == S_OK) {
    *textFormat = SafeDetach(&m_currentTextFormat);
  }
  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::ChooseFontDialog                              *
*                                                                 *
* Create and display the dialog initialized to attributes in an   *
* already existing text format.                                   *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::GetTextFormat(IDWriteTextFormat* textFormatIn, IDWriteTextFormat** textFormatOut)
{
  *textFormatOut = nullptr;

  SafeSet(&m_currentTextFormat, textFormatIn);

  // Pull out the input font attributes
  SafeRelease(&m_fontCollection);
  HRESULT hr = m_currentTextFormat->GetFontCollection(&m_fontCollection);

  if (SUCCEEDED(hr)) {
    hr = m_currentTextFormat->GetLocaleName(&m_localeName[0], _ARRAYSIZE(m_localeName));
  }

  // Open the dialog
  if (SUCCEEDED(hr)) {
    if (Globals.hLngResContainer) {
      hr = static_cast<HRESULT>(DialogBoxParam(Globals.hLngResContainer, MAKEINTRESOURCE(IDD_MUI_CHOOSEFONT), m_parent, CFDialogProc, reinterpret_cast<LPARAM>(this)));
    }
    else {
      hr = static_cast<HRESULT>(DialogBoxParam(g_hInstanceNP3, MAKEINTRESOURCE(IDD_MUI_CHOOSEFONT), m_parent, CFDialogProc, reinterpret_cast<LPARAM>(this)));
    }
  }

  // If all went well, and the user didn't cancel, return the new format.
  if (hr == S_OK) {
    *textFormatOut = SafeDetach(&m_currentTextFormat);
  }
  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontFamilySelect                            *
*                                                                 *
* Update the font face list to match the newly select font family *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontFamilySelect()
{
  HRESULT hr = S_OK;

  HWND hwndFontFamilyNames = GetDlgItem(m_dialog, IDC_FONT_FAMILY_NAMES);
  HWND hwndFontFaceNames = GetDlgItem(m_dialog, IDC_FONT_FACE_NAMES);
  int currentSelection = ComboBox_GetCurSel(hwndFontFamilyNames);

  // Get the font family name
  WCHAR fontFamilyName[128];

  UINT32 fontFamilyNameLength = ComboBox_GetLBTextLen(hwndFontFamilyNames, currentSelection) + 1;
  if (fontFamilyNameLength > _ARRAYSIZE(fontFamilyName))
    hr = E_NOT_SUFFICIENT_BUFFER;

  if (SUCCEEDED(hr)) {
    ComboBox_GetLBText(hwndFontFamilyNames, currentSelection, &fontFamilyName[0]);
  }

  // Get the face names for the new font family
  IDWriteFontFamily*          fontFamily = nullptr;
  std::vector<IDWriteFont*>   fonts;

  // Get the font variants for this family
  if (currentSelection != CB_ERR) {
    hr = GetFonts(m_fontCollection, fontFamilyName, fonts);
  }
  // Initialize the face name list
  std::vector<FontFaceInfo> fontFaceInfo;
  if (SUCCEEDED(hr)) {
    ComboBox_ResetContent(hwndFontFaceNames);
    GetFontFaceInfo(fonts, m_localeName, fontFaceInfo);
  }

  if (SUCCEEDED(hr)) {
    for (size_t i = 0; i != fontFaceInfo.size(); ++i) {
      int fontFaceIndex = ComboBox_AddString(hwndFontFaceNames, fontFaceInfo[i].fontFaceName);
      ComboBox_SetItemData(hwndFontFaceNames, fontFaceIndex, fontFaceInfo[i].PackedFontAttributes());
    }
  }

  // Select the best fit font face for the current attributes
  if (SUCCEEDED(hr)) {
    FontFaceInfo desiredAttributes(
      L"",
      m_currentTextFormat->GetFontWeight(),
      m_currentTextFormat->GetFontStyle(),
      m_currentTextFormat->GetFontStretch());

    int selectedFontFaceName = 0;
    ULONG bestFitAttributes = GetBestFontAttributes(m_fontCollection, fontFamilyName, desiredAttributes);

    int fontFaceCount = ComboBox_GetCount(hwndFontFaceNames);

    for (int i = 0; i != fontFaceCount; ++i) {
      if (static_cast<ULONG>(ComboBox_GetItemData(hwndFontFaceNames, i)) == bestFitAttributes) {
        selectedFontFaceName = i;
        break;
      }
    }

    ComboBox_SetCurSel(hwndFontFaceNames, selectedFontFaceName);
    OnFontFaceSelect();
  }

  // Release the held font list.
  for (auto& font : fonts) {
    SafeRelease(&font);
  }
  SafeRelease(&fontFamily);

  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontFaceSelect                              *
*                                                                 *
* Record the new font face selection and redraw the sample text.  *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontFaceSelect()
{
  HRESULT hr = S_OK;

  // Signal the sample text window to redraw itself.
  InvalidateRect(GetDlgItem(m_dialog, IDC_SAMPLE_BOX), nullptr, false);

  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontSizeSelect                              *
*                                                                 *
* Record the new font size and redraw the sample text.            *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontSizeSelect()
{
  HRESULT hr = S_OK;

  // Signal the sample text window to redraw itself.
  InvalidateRect(GetDlgItem(m_dialog, IDC_SAMPLE_BOX), nullptr, false);

  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontFamilyNameEdit                          *
*                                                                 *
* Watch what is typed into the edit portion of the font family    *
* combo and automatically select a name if a match is found.  As  *
* an added feature, also match against localized forms of the     *
* family name.  For example the user can type "Meiryo" on a       *
* Japanese system and it will be found even though it's displayed *
* using a localized variant of the name.                          *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontFamilyNameEdit(HWND hwndFontFamilies)
{
  HRESULT hr = S_OK;

  // Save the state of the edit box selection
  DWORD editSelection = ComboBox_GetEditSel(hwndFontFamilies);
  int   editSelectionBegin = LOWORD(editSelection);
  int   editSelectionEnd = HIWORD(editSelection);

  // Get the text in the edit portion of the combo
  WCHAR fontFullName[128];
  ComboBox_GetText(hwndFontFamilies, &fontFullName[0], _ARRAYSIZE(fontFullName));

  // Try to find an exact match (case-insensitive)
  WCHAR fontFamilyName[128];
  StringCchCopyW(fontFamilyName, ARRAYSIZE(fontFamilyName), fontFullName);

  int matchingFontFamily = CB_ERR;
  PTSTR pSpc = NULL;
  do {
    //matchingFontFamily = ComboBox_FindStringExact(hwndFontFamilies, -1, fontFullName);
    matchingFontFamily = ComboBox_FindString(hwndFontFamilies, -1, fontFamilyName);
    if (matchingFontFamily == CB_ERR) { 
      pSpc = StrRChrIW(fontFamilyName, NULL, L' ');
      if (pSpc != NULL) { *pSpc = L'\0'; }
    }
  } while ((matchingFontFamily == CB_ERR) && (pSpc != NULL));
  
  bool usedAltMatch = false;

  if (matchingFontFamily == CB_ERR) {
    // If a match isn't found, scan all for alternate forms in the font
    // collection.
    IDWriteFontFamily* fontFamily = nullptr;
    hr = GetFontFamily(m_fontCollection, fontFullName, &fontFamily);

    if (SUCCEEDED(hr)) {
      // If a match is found, get the family name localized to the locale
      // we're using in the combo box and match against that.
      usedAltMatch = true;

      std::wstring localFontFamilyName;
      hr = GetFontFamilyName(fontFamily, m_localeName, localFontFamilyName);

      if (SUCCEEDED(hr)) {
        matchingFontFamily = ComboBox_FindStringExact(hwndFontFamilies, -1, localFontFamilyName.c_str());
      }
    }
    else if (hr == DWRITE_E_NOFONT) {
      // Ignore DWRITE_E_NOFONT errors
      hr = S_OK;
    }

    SafeRelease(&fontFamily);
  }

  // Process the match, if any
  if (SUCCEEDED(hr) && matchingFontFamily != CB_ERR)
  {
    ComboBox_SetCurSel(hwndFontFamilies, matchingFontFamily);

    // SetCurSel will update the edit text to match the text of the 
    // selected item.  If we matched against an alternate name put that
    // name back.
    if (usedAltMatch) {
      ComboBox_SetText(hwndFontFamilies, fontFullName);
    }
    // Reset the edit selection to what is was before SetCurSel.
    ComboBox_SetEditSel(hwndFontFamilies, editSelectionBegin, editSelectionEnd);

    hr = OnFontFamilySelect();
  }

  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontFaceNameEdit                            *
*                                                                 *
* Watch what is typed into the edit portion of the font face      *
* combo and automatically select a name if a match is found.      *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontFaceNameEdit(HWND hwnd)
{
  HRESULT hr = S_OK;

  // Save the state of the edit box selection
  DWORD editSelection = ComboBox_GetEditSel(hwnd);
  int   editSelectionBegin = LOWORD(editSelection);
  int   editSelectionEnd = HIWORD(editSelection);

  // Try to find the currently typed text
  WCHAR text[100];
  ComboBox_GetText(hwnd, &text[0], _ARRAYSIZE(text));

  int selectedItem = ComboBox_FindStringExact(hwnd, -1, text);
  if (selectedItem != CB_ERR) {
    // If text is found, select the corresponding list item, put the
    // selection state back to what it was originally, and redraw the
    // sample text
    ComboBox_SetCurSel(hwnd, selectedItem);
    ComboBox_SetEditSel(hwnd, editSelectionBegin, editSelectionEnd);
    hr = OnFontFaceSelect();
  }

  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnFontSizeNameEdit                            *
*                                                                 *
* Watch what is typed into the edit portion of the font size      *
* combo and automatically select a name if a match is found.      *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::OnFontSizeNameEdit(HWND hwnd)
{
  HRESULT hr = S_OK;

  // Save the state of the edit box selection
  DWORD editSelection = ComboBox_GetEditSel(hwnd);
  int   editSelectionBegin = LOWORD(editSelection);
  int   editSelectionEnd = HIWORD(editSelection);

  // Try to find the currently typed text
  WCHAR text[100];
  ComboBox_GetText(hwnd, &text[0], _ARRAYSIZE(text));

  int selectedItem = ComboBox_FindStringExact(hwnd, -1, text);
  if (selectedItem != CB_ERR) {
    // If text is found, select the corresponding list item, put the
    // selection state back to what it was originally, and redraw the
    // sample text
    ComboBox_SetCurSel(hwnd, selectedItem);
    ComboBox_SetEditSel(hwnd, editSelectionBegin, editSelectionEnd);
  }
  hr = OnFontSizeSelect();

  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::DrawSampleText                                *
*                                                                 *
******************************************************************/

HRESULT ChooseFontDialog::DrawSampleText(HDC sampleDC)
{
  static WCHAR sampleText[256] = { L'\0' };
  if (sampleText[0] == L'\0') {
    LoadLngStringW(IDS_MUI_EXAMPLE_TEXT, sampleText, _ARRAYSIZE(sampleText));
  }

  HRESULT hr = S_OK;

  HWND hwndFontFamilies = GetDlgItem(m_dialog, IDC_FONT_FAMILY_NAMES);
  HWND hwndFontFaces = GetDlgItem(m_dialog, IDC_FONT_FACE_NAMES);
  HWND hwndFontSizes = GetDlgItem(m_dialog, IDC_FONT_SIZE);
  HWND hwndSampleBox = GetDlgItem(m_dialog, IDC_SAMPLE_BOX);

  // Get the currently selected font family.  If there isn't one, then just
  // don't update the text format and continue to use whatever we had before
  int  selectedFontFamily = ComboBox_GetCurSel(hwndFontFamilies);

  if (selectedFontFamily != CB_ERR) {
    // Get the font family name
    WCHAR fontFamilyName[100];
    GetWindowText(hwndFontFamilies, &fontFamilyName[0], _ARRAYSIZE(fontFamilyName));

    // Get the font face attributes
    int selectedFontFace = ComboBox_GetCurSel(hwndFontFaces);
    auto packedAttributes = static_cast<ULONG>(ComboBox_GetItemData(hwndFontFaces, selectedFontFace));

    // Get the full font style
    ComboBox_GetText(hwndFontFaces, m_fontStyle, _ARRAYSIZE(m_fontStyle));

    // Get the font size
    WCHAR fontSizeText[100];
    GetWindowText(hwndFontSizes, &fontSizeText[0], _ARRAYSIZE(fontSizeText));

    auto pointSize = static_cast<float>(wcstod(fontSizeText, nullptr));
    if (pointSize <= 0.0f) { pointSize = 10.0f; }

    FontFaceInfo const fontFaceInfo(fontFamilyName, packedAttributes);

    // Recreate current text format object
    SafeRelease(&m_currentTextFormat);
    hr = g_dwrite->CreateTextFormat(
      fontFamilyName,
      m_fontCollection,
      fontFaceInfo.fontWeight,
      fontFaceInfo.fontStyle,
      fontFaceInfo.fontStretch,
      pointSize,
      m_localeName,
      &m_currentTextFormat);

    if (SUCCEEDED(hr)) {
      // Create the rendering text format object
      float dipSize = (pointSize * m_currentDPI.y) / 72.0f;
      SafeRelease(&m_renderTextFormat);
      hr = g_dwrite->CreateTextFormat(
        fontFamilyName,
        m_fontCollection,
        fontFaceInfo.fontWeight,
        fontFaceInfo.fontStyle,
        fontFaceInfo.fontStretch,
        dipSize,
        m_localeName,
        &m_renderTextFormat);
    }
  }
  // Get the size of the sample box
  RECT sampleBounds = {};
  GetClientRect(hwndSampleBox, &sampleBounds);

  UINT width = sampleBounds.right - sampleBounds.left;
  UINT height = sampleBounds.bottom - sampleBounds.top;

  // Layout the sample text using the text format and UI bounds (converted to DIPs)
  IDWriteTextLayout* textLayout = nullptr;
  if (SUCCEEDED(hr)) {
    hr = g_dwrite->CreateTextLayout(
      sampleText,
      _ARRAYSIZE(sampleText) - 1,
      m_renderTextFormat,
      static_cast<float>((width  * m_currentDPI.x) / GetDeviceCaps(sampleDC, LOGPIXELSX)),
      static_cast<float>((height * m_currentDPI.y) / GetDeviceCaps(sampleDC, LOGPIXELSY)),
      &textLayout);
  }

  // Create a DWrite surface to render to
  GdiTextRenderer* textRenderer = nullptr;
  if (SUCCEEDED(hr)) {
    textRenderer = SafeAcquire(new(std::nothrow) GdiTextRenderer());
    if (textRenderer != nullptr)
      hr = textRenderer->Initialize(m_dialog, sampleDC, width, height);
    else
      hr = E_FAIL;
  }

  if (SUCCEEDED(hr)) {
    // Fill the DWrite surface with the background color
    HDC dwriteDC = textRenderer->GetDC();
    SetDCBrushColor(dwriteDC, GetSysColor(COLOR_BTNFACE));
    FillRect(dwriteDC, &sampleBounds, GetStockBrush(DC_BRUSH));

    // Draw the text onto the DWrite surface
    hr = textLayout->Draw(nullptr, textRenderer, 0, 0);

    // Copy the DWrite surface to the sample on screen
    BitBlt(
      sampleDC,
      0,
      0,
      width,
      height,
      dwriteDC,
      0,
      0,
      SRCCOPY | NOMIRRORBITMAP);
  }

  SafeRelease(&textRenderer);
  SafeRelease(&textLayout);

  return hr;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::WndProc                                       *
*                                                                 *
* Dispatch window message to the appropriate hander               *
*                                                                 *
******************************************************************/

INT_PTR CALLBACK ChooseFontDialog::CFDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == WM_INITDIALOG) {
    SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);
  }
  auto this_ = reinterpret_cast<ChooseFontDialog*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  if (this_ != nullptr) {
    LPCFHOOKPROC hookFct = this_->m_chooseFontStruct->lpfnHook;
    if (hookFct) {
      (*hookFct)(hWnd, message, wParam, reinterpret_cast<LPARAM>(this_->m_chooseFontStruct));
    }
    switch (message) {
      HANDLE_MSG(hWnd, WM_INITDIALOG, this_->OnInitDialog);
      HANDLE_MSG(hWnd, WM_COMMAND, this_->OnCommand);
      HANDLE_MSG(hWnd, WM_DRAWITEM, this_->OnDrawItem);
    }
  }
  return FALSE;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnInitDialog                                  *
*                                                                 *
* Initialize the dialog by enumerating the font families and      *
* setting up a hardcoded list of standard font sizes.             *
*                                                                 *
******************************************************************/

BOOL ChooseFontDialog::OnInitDialog(HWND dialog, HWND hwndFocus, LPARAM lParam)
{
  m_dialog = dialog;

  SET_NP3_DLG_ICON_SMALL(dialog);

  HWND hwndFamilyNames = GetDlgItem(dialog, IDC_FONT_FAMILY_NAMES);
  HWND hwndSizes = GetDlgItem(dialog, IDC_FONT_SIZE);

  // Fill in the font family name list.

  std::vector<std::wstring> fontFamilyNames;
  if (FAILED(GetFontFamilyNames(m_fontCollection, m_localeName, fontFamilyNames)))
    return FALSE;

  for (size_t i = 0; i != fontFamilyNames.size(); ++i)
    ComboBox_AddString(hwndFamilyNames, fontFamilyNames[i].c_str());

  // Fill in the hardcoded font sizes

  static const float FontSizes[] = {
      1.5, 2.5, 3.5, 4.5, 5, 5.5, 6, 6.5, 7, 7.5,
      8, 8.5, 9, 9.5, 10, 10.5, 11, 11.5, 12, 12.5,
      13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
      26, 28, 30, 36, 48, 60, 72, 84, 96
  };

  WCHAR sizeName[100];
  sizeName[0] = '\0';

  for (int i = 0; i != _ARRAYSIZE(FontSizes); ++i) {
    StringCchPrintf(sizeName, _ARRAYSIZE(sizeName), L"%.3G", FontSizes[i]);
    ComboBox_AddString(hwndSizes, sizeName);
  }

  // Select the current size
  float fCurFontSize = roundf(m_currentTextFormat->GetFontSize() * 10.0f) / 10.0f;
  StringCchPrintf(sizeName, _ARRAYSIZE(sizeName), L"%.3G", fCurFontSize);

  SetWindowText(hwndSizes, sizeName);
  if (CB_ERR == ComboBox_SelectString(hwndSizes, -1, sizeName))
    SetWindowText(hwndSizes, sizeName);

  // Select the font family specified in the input text format.

  int selectedFontFamily = CB_ERR;
  std::wstring fontFamilyName;

  if (SUCCEEDED(GetFontFamilyNameFromFormat(m_currentTextFormat, fontFamilyName))) {
    selectedFontFamily = ComboBox_SelectString(hwndFamilyNames, -1, fontFamilyName.c_str());
  }

  if (selectedFontFamily == CB_ERR) {
    SetWindowText(hwndFamilyNames, fontFamilyName.c_str());
    OnFontFamilyNameEdit(hwndFamilyNames);
  }
  OnFontFamilySelect();

  CenterDlgInParent(m_dialog);

  return TRUE;
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnCommand                                     *
*                                                                 *
* Dispatch button clicks, changing listbox selections, etc.       *
*                                                                 *
******************************************************************/

void ChooseFontDialog::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
  if (id == IDCANCEL && codeNotify == BN_CLICKED)
    EndDialog(hwnd, S_FALSE);

  else if (id == IDOK && codeNotify == BN_CLICKED)
    EndDialog(hwnd, S_OK);

  else if (id == IDC_FONT_FAMILY_NAMES && codeNotify == CBN_SELCHANGE)
    OnFontFamilySelect();

  else if (id == IDC_FONT_FAMILY_NAMES && codeNotify == CBN_EDITCHANGE)
    OnFontFamilyNameEdit(hwndCtl);

  else if (id == IDC_FONT_FACE_NAMES && codeNotify == CBN_SELCHANGE)
    OnFontFaceSelect();

  else if (id == IDC_FONT_FACE_NAMES && codeNotify == CBN_EDITCHANGE)
    OnFontFaceNameEdit(hwndCtl);

  else if (id == IDC_FONT_SIZE && codeNotify == CBN_SELCHANGE)
    OnFontSizeSelect();

  else if (id == IDC_FONT_SIZE && codeNotify == CBN_EDITCHANGE)
    OnFontSizeNameEdit(hwndCtl);
}


/******************************************************************
*                                                                 *
* ChooseFontDialog::OnDrawItem                                    *
*                                                                 *
* Redraw the sample text whenever it's window needs updating      *
*                                                                 *
******************************************************************/

void ChooseFontDialog::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT* lpDrawItem)
{
  DrawSampleText(lpDrawItem->hDC);
}

// ############################################################################
// ############################################################################


static void  SetChosenFontFromTextFormat(
  IDWriteTextFormat* textFormat,
  LPCWSTR fontStyleStrg,
  LPCHOOSEFONT lpCF, const DPI_T dpi)
{
  if (textFormat != nullptr) {
    WCHAR fontFamilyName[100];
    HDC hdc = GetDC(lpCF->hwndOwner);

    textFormat->GetFontFamilyName(&fontFamilyName[0], _ARRAYSIZE(fontFamilyName));
    float const fFontSize = textFormat->GetFontSize();
    DWRITE_FONT_WEIGHT const fontWeight = textFormat->GetFontWeight();
    DWRITE_FONT_STYLE const fontStyle = textFormat->GetFontStyle();
    //DWRITE_FONT_STRETCH const fontStretch = textFormat->GetFontStretch();

    // copy font family name here, will be corrected on post processing step
    StringCchCopy(lpCF->lpLogFont->lfFaceName, LF_FACESIZE, fontFamilyName); // family name only here

    lpCF->iPointSize = static_cast<INT>(lroundf(fFontSize * 10.0f));
    lpCF->lpLogFont->lfHeight = -MulDiv(static_cast<int>(lround(fFontSize * SC_FONT_SIZE_MULTIPLIER)), 
                                        GetDeviceCaps(hdc, LOGPIXELSY), 72 * SC_FONT_SIZE_MULTIPLIER);
    lpCF->lpLogFont->lfWeight = fontWeight;
    lpCF->lpLogFont->lfItalic = static_cast<BYTE>((fontStyle != DWRITE_FONT_STYLE_NORMAL) ? TRUE : FALSE);
    //~lpCF->lpLogFont->lfQuality = static_cast<BYTE>(CLEARTYPE_QUALITY);

    StringCchCopy(lpCF->lpszStyle, LF_FULLFACESIZE, fontStyleStrg ? fontStyleStrg : L"");

    ReleaseDC(lpCF->hwndOwner, hdc);
  }
}
// ============================================================================


extern "C" bool ChooseFontDirectWrite(HWND hwnd, const WCHAR* localeName, DPI_T dpi, LPCHOOSEFONT lpCFGDI)
{
  if (!lpCFGDI) { return false; }

  // The Microsoft Security Development Lifecycle recommends that all
  // applications include the following call to ensure that heap corruptions
  // do not go unnoticed and therefore do not introduce opportunities
  // for security exploits.
  HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

  if (dpi.x == 0) { dpi.x = dpi.y = USER_DEFAULT_SCREEN_DPI; }

  g_hInstanceNP3 = lpCFGDI->hInstance;

  DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&g_dwrite);

  IDWriteTextFormat* textFormatOut = nullptr;
  ChooseFontDialog chooseFont(hwnd, localeName, dpi, lpCFGDI);
  chooseFont.GetTextFormat(&textFormatOut);
  WCHAR fontStyle[LF_FULLFACESIZE] = { L'\0' };
  chooseFont.GetFontStyle(fontStyle, _ARRAYSIZE(fontStyle));

  SetChosenFontFromTextFormat(textFormatOut, fontStyle, lpCFGDI, dpi);

  SafeRelease(&textFormatOut);
  SafeRelease(&g_dwrite);
  return true;
}

// ############################################################################
