// encoding: UTF-8
/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* FindReplace.c                                                               *
*   Find/Replace and occurrence matching                                      *
*   Based on code from Notepad2, (c) Florian Balmer 1996-2011                 *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2026   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/


#include "Helpers.h"

#include <assert.h>
#include <shlwapi.h>
#include <commctrl.h>

#include "Dialogs.h"
#include "Edit.h"
#include "Encoding.h"
#include "FindReplace.h"
#include "MuiLanguage.h"
#include "Notepad3.h"
#include "Styles.h"
#include "Config/Config.h"
#include "DarkMode/DarkMode.h"
#include "uthash/utlist.h"

//=============================================================================

static bool s_bSwitchedFindReplace = false;

static int s_xFindReplaceDlgSave;
static int s_yFindReplaceDlgSave;

static HSTRINGW s_hstrCurrentFindPattern = NULL;
static EDITFINDREPLACE s_FindReplaceData = INIT_EFR_DATA;

static void DuplicateEFR(LPEDITFINDREPLACE dst, CLPCEDITFINDREPLACE src);
static void EditMarkAll(LPCWSTR wchFind, int sFlags, DocPos rangeStart, DocPos rangeEnd, bool bMultiSel);
static void _TransformReplaceBackslashes(LPSTR pszInput, bool bRegEx, int* iReplaceMsg);
void FindReplaceInit(void)
{
    s_hstrCurrentFindPattern = StrgCreate(NULL);
}

void FindReplaceRelease(void)
{
    StrgDestroy(s_hstrCurrentFindPattern);
    ReleaseEFR(&s_FindReplaceData);
}

bool IsFindPatternEmpty(void)
{
    return StrgIsEmpty(s_hstrCurrentFindPattern);
}

//=============================================================================
//
//  GetFindPattern()
//
LPCWSTR GetFindPattern(void)
{
    return StrgGet(s_hstrCurrentFindPattern);
}

//=============================================================================
//
//  SetFindPattern()
//
void SetFindPattern(LPCWSTR wchFindPattern)
{
    StrgReset(s_hstrCurrentFindPattern, StrIsNotEmpty(wchFindPattern) ? wchFindPattern : L"");
}

//=============================================================================
//
//  SetFindPatternMB()
//
static void SetFindPatternMB(LPCSTR chFindPattern)
{
    StrgResetFromUTF8(s_hstrCurrentFindPattern, !StrIsEmptyA(chFindPattern) ? chFindPattern : "");
}

//=============================================================================
//
//  LengthOfFindPattern()
//
size_t LengthOfFindPattern(void)
{
    return StrgGetLength(s_hstrCurrentFindPattern);
}

//=============================================================================
//
// FindReplacePrepare()
//
LPEDITFINDREPLACE FindReplacePrepare(void)
{
    // init find pattern with current
    if (!IsFindPatternEmpty()) {
        StrgReset(Settings.EFR_Data.chFindPattern, GetFindPattern());
    }
    // copy settings to working data
    DuplicateEFR(&s_FindReplaceData, &(Settings.EFR_Data));

    if (g_flagMatchText) { // cmd line
        if (g_flagMatchText & 4) {
            s_FindReplaceData.fuFlags = (SCFIND_REGEXP | SCFIND_POSIX);
        } else {
            // /m without R flag: force text mode (clear regex flags) - fixes #5060
            s_FindReplaceData.fuFlags &= ~(SCFIND_REGEXP | SCFIND_POSIX);
        }
        if (g_flagMatchText & 8) {
            s_FindReplaceData.fuFlags |= SCFIND_MATCHCASE;
        }
        if (g_flagMatchText & 16) {
            s_FindReplaceData.fuFlags |= SCFIND_DOT_MATCH_ALL;
        }
        if (g_flagMatchText & 32) {
            s_FindReplaceData.bTransformBS = true;
        }
        s_FindReplaceData.bRegExprSearch = (s_FindReplaceData.fuFlags & SCFIND_REGEXP);
        s_FindReplaceData.bWildcardSearch = false;
        s_FindReplaceData.bReplaceClose = false;
    }
    return &s_FindReplaceData;
}


//=============================================================================
//
// FindReplaceUseSelection()
//
LPEDITFINDREPLACE FindReplaceUseSelection(void)
{
    if (SciCall_IsSelectionEmpty()) {
        EditSelectWordAtPos(SciCall_GetCurrentPos(), true);
    }

    size_t const cchSelection = SciCall_GetSelText(NULL);

    if (0 < cchSelection) {
        char* const szSelection = AllocMem((cchSelection + 1), HEAP_ZERO_MEMORY);
        if (szSelection) {
            SciCall_GetSelText(szSelection);

            SetFindPatternMB(szSelection);
            LPEDITFINDREPLACE const lpefr = FindReplacePrepare();
            StrgReset(lpefr->chFindPattern, GetFindPattern());

            FreeMem(szSelection);
            return lpefr;
        }
    }
    return NULL;
}


static void DuplicateEFR(LPEDITFINDREPLACE dst, CLPCEDITFINDREPLACE src)
{
    dst->fuFlags = src->fuFlags;
    dst->bTransformBS = src->bTransformBS;
    dst->bFindClose = src->bFindClose;
    dst->bReplaceClose = src->bReplaceClose;
    dst->bNoFindWrap = src->bNoFindWrap;
    dst->bRegExprSearch = src->bRegExprSearch;
    dst->bWildcardSearch = src->bWildcardSearch;
    dst->bMarkOccurences = src->bMarkOccurences;
    dst->bInstantIncrementalSearch = src->bInstantIncrementalSearch;
    dst->bHideNonMatchedLines = src->bHideNonMatchedLines;
    dst->bStateChanged = src->bStateChanged;
    dst->hwnd = src->hwnd;
    if (!(dst->chFindPattern)) {
        dst->chFindPattern = StrgCreate(StrgGet(src->chFindPattern));
    }
    else {
        StrgReset(dst->chFindPattern, StrgGet(src->chFindPattern));
    }
    if (!(dst->chReplaceTemplate)) {
        dst->chReplaceTemplate = StrgCreate(StrgGet(src->chReplaceTemplate));
    }
    else {
        StrgReset(dst->chReplaceTemplate, StrgGet(src->chReplaceTemplate));
    }
}


void ReleaseEFR(LPEDITFINDREPLACE efr)
{
    StrgDestroy(efr->chFindPattern);
    efr->chFindPattern = NULL;
    StrgDestroy(efr->chReplaceTemplate);
    efr->chReplaceTemplate = NULL;
}


//=============================================================================
//
//  Delay Message Queue Handling  (TODO: MultiThreading)
//

static CmdMessageQueue_t* MessageQueue = NULL;

// ----------------------------------------------------------------------------

static int msgcmp(void* mqc1, void* mqc2)
{
    const CmdMessageQueue_t* const pMQC1 = (CmdMessageQueue_t*)mqc1;
    const CmdMessageQueue_t* const pMQC2 = (CmdMessageQueue_t*)mqc2;

    if ((pMQC1->cmd == pMQC2->cmd)
        && (pMQC1->hwnd == pMQC2->hwnd)
        && (pMQC1->wparam == pMQC2->wparam)
        && (pMQC1->lparam == pMQC2->lparam)
       ) {
        return 0;
    }
    return 1;
}

static LONG64 sortcmp(void *mqc1, void *mqc2) {
    const CmdMessageQueue_t *const pMQC1 = (CmdMessageQueue_t *)mqc1;
    const CmdMessageQueue_t *const pMQC2 = (CmdMessageQueue_t *)mqc2;
    return (pMQC1->delay - pMQC2->delay);
}
// ----------------------------------------------------------------------------



#define _MQ_TIMER_CYCLE (USER_TIMER_MINIMUM << 1)
#define _MQ_STD  (_MQ_TIMER_CYCLE << 2)
#define _MQ_ms2cycl(T) (((T) + USER_TIMER_MINIMUM) / _MQ_TIMER_CYCLE)
#define _MQ_ASAP 0

static void  _MQ_AppendCmd(CmdMessageQueue_t* const pMsgQCmd, int cycles)
{
    if (!pMsgQCmd) { return; }

    cycles = clampi(cycles, 0, _MQ_ms2cycl(60000));

    CmdMessageQueue_t* pmqc = NULL;
    DL_SEARCH(MessageQueue, pmqc, pMsgQCmd, msgcmp);

    if (!pmqc) { // NOT found, create new one
        pmqc = AllocMem(sizeof(CmdMessageQueue_t), HEAP_ZERO_MEMORY);
        if (pmqc) {
            *pmqc = *pMsgQCmd;
            pmqc->delay = cycles;
            DL_APPEND(MessageQueue, pmqc);
        }
    } else {
        if ((pmqc->delay > 0) && (cycles > 0)) {
            pmqc->delay = (pmqc->delay + cycles) >> 1; // median delay
        } else {
            pmqc->delay = cycles;
        }
    }
    if (0 == cycles) {
        PostMessage(pMsgQCmd->hwnd, pMsgQCmd->cmd, pMsgQCmd->wparam, pMsgQCmd->lparam);
    }
    //~DL_SORT(MessageQueue, sortcmp); // next scheduled first
}
// ----------------------------------------------------------------------------

/* not used yet
static void _MQ_DropAll() {
    CmdMessageQueue_t *pmqc = NULL;
    DL_FOREACH(MessageQueue, pmqc) {
        pmqc->delay = -1;
    }
}
*/
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//
// called by MarkAll Timer
//
// TIMERPROC
static void CALLBACK MQ_ExecuteNext(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    UNREFERENCED_PARAMETER(hwnd);    // must be main wnd
    UNREFERENCED_PARAMETER(uMsg);    // must be WM_TIMER
    UNREFERENCED_PARAMETER(idEvent); // must be pTimerIdentifier
    UNREFERENCED_PARAMETER(dwTime);  // This is the value returned by the GetTickCoun(t)) function

    CmdMessageQueue_t *pmqc;
    DL_FOREACH(MessageQueue, pmqc) {
        if (pmqc->delay >= 0) {
            --(pmqc->delay);
        }
        if (pmqc->delay == 0) {
            if (IsWindow(pmqc->hwnd)) {
                PostMessage(pmqc->hwnd, pmqc->cmd, pmqc->wparam, pmqc->lparam);
            }
        }
    }
}



//=============================================================================
//
//  _SyncBoolOpt() / _SyncFlagOpt() - take a dialog check-box state over into the F/R data
//
static void _SyncBoolOpt(HWND hwnd, int idButton, bool* const pbOpt, bool* const pbChanged)
{
    bool const bChecked = IsButtonChecked(hwnd, idButton);
    if (bChecked != *pbOpt) {
        *pbOpt = bChecked;
        *pbChanged = true;
    }
}

static void _SyncFlagOpt(HWND hwnd, int idButton, UINT* const puFlags, const UINT flag, bool* const pbChanged)
{
    bool const bChecked = IsButtonChecked(hwnd, idButton);
    if (bChecked != ((*puFlags & flag) != 0)) {
        *puFlags = bChecked ? (*puFlags | flag) : (*puFlags & ~flag);
        *pbChanged = true;
    }
}


//=============================================================================
//
//  _SetSearchFlags()
//
static void  _SetSearchFlags(HWND hwnd, LPEDITFINDREPLACE lpefr)
{
    if (hwnd && lpefr) {

        bool const bIsFindDlg = (GetDlgItem(hwnd, IDC_REPLACETEXT) == NULL);

        HSTRINGW hstr = StrgCreate(NULL);

        ComboBox_GetTextHW(hwnd, IDC_FINDTEXT, hstr);
        if (StringCchCompareX(StrgGet(hstr), StrgGet(lpefr->chFindPattern)) != 0) {
            StrgReset(lpefr->chFindPattern, StrgGet(hstr));
            lpefr->bStateChanged = true;
        }

        if (!bIsFindDlg) {
            StrgReset(hstr, L"");
            ComboBox_GetTextHW(hwnd, IDC_REPLACETEXT, hstr);
            if (StringCchCompareX(StrgGet(hstr), StrgGet(lpefr->chReplaceTemplate)) != 0) {
                StrgReset(lpefr->chReplaceTemplate, StrgGet(hstr));
                lpefr->bStateChanged = true;
            }
        }

        StrgDestroy(hstr);

        _SyncFlagOpt(hwnd, IDC_FINDCASE, &lpefr->fuFlags, SCFIND_MATCHCASE, &lpefr->bStateChanged);
        _SyncFlagOpt(hwnd, IDC_FINDWORD, &lpefr->fuFlags, SCFIND_WHOLEWORD, &lpefr->bStateChanged);

        if (IsDialogControlEnabled(hwnd, IDC_FINDTRANSFORMBS)) {
            _SyncBoolOpt(hwnd, IDC_FINDTRANSFORMBS, &lpefr->bTransformBS, &lpefr->bStateChanged);
        }

        _SyncFlagOpt(hwnd, IDC_FINDSTART, &lpefr->fuFlags, SCFIND_WORDSTART, &lpefr->bStateChanged);

        // regular expression option and its search flag change together
        bool const bRegExChecked = IsButtonChecked(hwnd, IDC_FINDREGEXP);
        if (bRegExChecked != lpefr->bRegExprSearch) {
            lpefr->bRegExprSearch = bRegExChecked;
            lpefr->fuFlags = bRegExChecked ? (lpefr->fuFlags | SCFIND_REGEXP) : (lpefr->fuFlags & ~(UINT)SCFIND_REGEXP);
            lpefr->bStateChanged = true;
        }

        if (IsDialogControlEnabled(hwnd, IDC_DOT_MATCH_ALL)) {
            _SyncFlagOpt(hwnd, IDC_DOT_MATCH_ALL, &lpefr->fuFlags, SCFIND_DOT_MATCH_ALL, &lpefr->bStateChanged);
        }

        // force consistency
        if (lpefr->bRegExprSearch) {
            CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED);
        }
        bool const bWildcardChecked = IsButtonChecked(hwnd, IDC_WILDCARDSEARCH);
        if (bWildcardChecked != lpefr->bWildcardSearch) {
            lpefr->bWildcardSearch = bWildcardChecked;
            lpefr->bStateChanged = true;
        }
        if (bWildcardChecked) {
            if (!(lpefr->fuFlags & SCFIND_REGEXP)) {
                lpefr->fuFlags |= SCFIND_REGEXP; // wild-card search based on RegExpr
                lpefr->bStateChanged = true;
            }
        } else if ((lpefr->fuFlags & SCFIND_REGEXP) && !lpefr->bRegExprSearch) {
            lpefr->fuFlags &= ~(UINT)SCFIND_REGEXP;
            lpefr->bStateChanged = true;
        }

        _SyncBoolOpt(hwnd, IDC_NOWRAP, &lpefr->bNoFindWrap, &lpefr->bStateChanged);
        _SyncBoolOpt(hwnd, IDC_ALL_OCCURRENCES, &lpefr->bMarkOccurences, &lpefr->bStateChanged);
        _SyncBoolOpt(hwnd, IDC_INST_INCR_SEARCH, &lpefr->bInstantIncrementalSearch, &lpefr->bStateChanged);

        // close dialog after find resp. replace
        _SyncBoolOpt(hwnd, IDC_FINDCLOSE, bIsFindDlg ? &lpefr->bFindClose : &lpefr->bReplaceClose, &lpefr->bStateChanged);
    }
}


// Wildcard search uses the regexp engine to perform a simple search with * ? as wildcards
// instead of more advanced and user-unfriendly RegExpr syntax
// for speed, we only need POSIX syntax here
static void  _EscapeWildcards(wchar_t* wchFind, size_t cch, LPEDITFINDREPLACE lpefr)
{
    wchar_t* const wchWildcardEscaped = AllocMem(((cch<<1) + 1) * sizeof(wchar_t), HEAP_ZERO_MEMORY);
    if (wchWildcardEscaped) {

        size_t iSource = 0;
        size_t iDest = 0;

        lpefr->fuFlags |= SCFIND_REGEXP;

        bool bEsc = false;
        while ((iSource < cch) && (wchFind[iSource] != L'\0')) {
            wchar_t c = wchFind[iSource];
            if (c == L'*' && !bEsc) {
                wchWildcardEscaped[iDest++] = L'.';
            } else if (c == L'?' && !bEsc) {
                c = L'.';
            } else if (c == L'\\' && !bEsc) {
                bEsc = true;
            }
            else if (
                (c == L'^' ||
                 c == L'$' ||
                 c == L'(' ||
                 c == L')' ||
                 c == L'[' ||
                 c == L']' ||
                 c == L'{' ||
                 c == L'}' ||
                 //~c == L'.' ||
                 c == L'+' ||
                 c == L'|'
                ) ) {
                if (!bEsc) {
                    wchWildcardEscaped[iDest++] = L'\\';
                } else {
                    bEsc = false;
                }
            } else if (bEsc) {
                //~wchWildcardEscaped[iDest++] = L'\\'; ~ don't esc' backslash here
                bEsc = false;
            }
            wchWildcardEscaped[iDest++] = c;
            ++iSource;
        }

        StringCchCopyN(wchFind, cch, wchWildcardEscaped, (cch << 1) + 1);
        FreeMem(wchWildcardEscaped);
    }
}


//=============================================================================
//
//  _EditGetFindStrg()
//
static LPCWSTR _EditGetFindStrg(HWND hwnd, const LPEDITFINDREPLACE lpefr, bool bFillEmpty)
{
    UNREFERENCED_PARAMETER(hwnd);

    if (!lpefr) {
        return NULL;
    }

    static HSTRINGW hfind = NULL;
    // TODO: StrgDestroy(hfind);

    if (!hfind) {
        hfind = StrgCreate(NULL);
    }

    // 1st: try last used pattern
    if (StrgIsEmpty(lpefr->chFindPattern) && bFillEmpty) {
        StrgReset(hfind, GetFindPattern());
    } else {
        StrgReset(hfind, StrgGet(lpefr->chFindPattern));
    }

    // 2nd: try get clipboard content
    if (StrgIsEmpty(hfind) && bFillEmpty && Settings.SearchByClipboardIfEmpty) {
        LPWSTR const buf = StrgWriteAccessBuf(hfind, FNDRPL_BUFFER);
        EditGetClipboardW(buf, StrgGetAllocLength(hfind));
        StrgSanitize(hfind);
    }

    // 3nd: try get most recently used find pattern
    if (StrgIsEmpty(hfind) && bFillEmpty) {
        LPWSTR const buf = StrgWriteAccessBuf(hfind, FNDRPL_BUFFER);
        MRU_Enum(Globals.pMRUfind, 0, buf, (int)StrgGetAllocLength(hfind));
        StrgSanitize(hfind);
    }

    // 4th: return if still empty
    if (StrgIsEmpty(hfind)) {
        //~StrgDestroy(hfind); - static for speed
        return NULL;
    }

    // ensure to F/R-dialog data structure consistency
    StrgReset(lpefr->chFindPattern, StrgGet(hfind));
    SetFindPattern(StrgGet(hfind));

    if (lpefr->bWildcardSearch) {
        // _EscapeWildcards can expand each char to two (e.g. '|' -> '\|', '*' -> '.*'),
        // so ensure the buffer holds at least 2× the current content length.
        size_t const expandCap = (StrgGetLength(hfind) * 2) + 2;
        LPWSTR const buf = StrgWriteAccessBuf(hfind, max(expandCap, (size_t)FNDRPL_BUFFER));
        _EscapeWildcards(buf, StrgGetAllocLength(hfind), lpefr);
        StrgSanitize(hfind);
    }

    bool const bIsRegEx = (lpefr->fuFlags & SCFIND_REGEXP);
    if (lpefr->bTransformBS || bIsRegEx) {
        LPWSTR const buf = StrgWriteAccessBuf(hfind, FNDRPL_BUFFER);
        if (!bIsRegEx) { UnSlashW(buf, CP_UTF8); }
        StrgSanitize(hfind);
    }

    return StrgGet(hfind);
}


//=============================================================================
//
//  _GetReplaceString()
//
static char* _GetReplaceString(HWND hwnd, CLPCEDITFINDREPLACE lpefr, int* iReplaceMsg)
{
    char* pszReplace = NULL; // replace text of arbitrary size
    if (Settings.ReplaceByClipboardTag && (StrCmpW(StrgGet(lpefr->chReplaceTemplate), L"^c")) == 0)
    {
        *iReplaceMsg = SciCall_GetChangeHistory() ? SCI_REPLACETARGETMINIMAL : SCI_REPLACETARGET;
        pszReplace = EditGetClipboardText(hwnd, true, NULL, NULL);
    }
    else {
        int const cch = max_i(StrgLengthAsUTF8(lpefr->chReplaceTemplate), 255) + 1;
        pszReplace = (char*)AllocMem(cch, HEAP_ZERO_MEMORY);
        if (pszReplace) {
            StrgGetAsUTF8(lpefr->chReplaceTemplate, pszReplace, cch);
            bool const bIsRegEx = (lpefr->fuFlags & SCFIND_REGEXP);
            if (lpefr->bTransformBS || bIsRegEx) {
                _TransformReplaceBackslashes(pszReplace, bIsRegEx, iReplaceMsg);
            }
        }
    }
    return pszReplace; // transfer ownership
}


//=============================================================================
//
//  _FindInTarget()
//
// ONIG_MISMATCH
#define NOT_FOUND ((DocPos)(-1LL))

typedef enum { FRMOD_IGNORE = 0, FRMOD_NORM, FRMOD_WRAPED } FR_UPD_MODES;

static DocPos  _FindInTarget(LPCWSTR wchFind, int sFlags,
                             DocPos* begin, DocPos* end, bool bForceNext, FR_UPD_MODES fMode)
{
    static char chFind[8192] = { '\0' }; // max find buffer

    // mark-all and replace-all call this once per match with the same pattern: convert it only when it changed
    static WCHAR s_wchLastFind[COUNTOF(chFind)] = { L'\0' };
    static UINT  s_cpLastFind = 0; // code page of the pattern in chFind, 0 if chFind is not backed by s_wchLastFind

    DocPos iPos = NOT_FOUND;

    if (StrIsEmpty(wchFind)) {
        return iPos;
    }

    // remember original target range
    DocPos const saveTargetBeg = SciCall_GetTargetStart();
    DocPos const saveTargetEnd = SciCall_GetTargetEnd();

    DocPos const start = *begin;
    DocPos const stop  = *end;
    bool const bFindNext = (start <= stop); // else find previous

    DocPos len = 0;
    if ((s_cpLastFind == (UINT)Encoding_SciCP) && (wcscmp(wchFind, s_wchLastFind) == 0)) { // exact (ordinal) comparison
        len = (DocPos)strlen(chFind);
    } else {
        len = (DocPos)(WideCharToMultiByte(Encoding_SciCP, 0, wchFind, -1, chFind, COUNTOF(chFind), NULL, NULL) - 1);
        bool const bRemember = (len >= 0) && SUCCEEDED(StringCchCopyW(s_wchLastFind, COUNTOF(s_wchLastFind), wchFind));
        s_cpLastFind = bRemember ? (UINT)Encoding_SciCP : 0;
    }

    // Clear any lingering regex-warning status so the post-call state reflects
    // this search only. Without this, a stale SC_STATUS_WARN_REGEX from an
    // earlier _FindInTarget call could leak into the next caller's Sci_ConsumeRegexWarnStatus.
    SciCall_SetStatus(SC_STATUS_OK);

    // Scintilla treats a nonpositive pattern length as a match at the target start.
    if (len > 0) {
        SciCall_SetSearchFlags(sFlags);
        SciCall_SetTargetRange(start, stop);
        iPos = SciCall_SearchInTarget(len, chFind);

        // Leave the regex warning raised for the caller to consume.
        if ((sFlags & SCFIND_REGEXP) && SciCall_GetStatus() == SC_STATUS_WARN_REGEX) {
            SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd);
            return NOT_FOUND;
        }

        // Advance past a zero-length regex match or an invalid position.
        bool const bZeroLenMatch = ((iPos == start) && (start == SciCall_GetTargetEnd()));
        bool       bValidPos = !(bForceNext && bZeroLenMatch) && Sci_IsPosValid(iPos);
        while (!bValidPos) {
            DocPos const newStart = (bFindNext ? SciCall_PositionAfter(iPos) : SciCall_PositionBefore(iPos));
            bool const   bProceed = (bFindNext ? (newStart < stop) : (newStart > stop)) && (newStart != iPos);
            if (bProceed) {
                SciCall_SetTargetRange(newStart, stop);
                iPos = SciCall_SearchInTarget(len, chFind);
            }
            else {
                iPos = NOT_FOUND;
            }
            bValidPos = Sci_IsPosValid(iPos); // NOT_FOUND is a valid pos
        }
    }

    if (iPos >= 0) {
        if (fMode != FRMOD_IGNORE) {
            Globals.FindReplaceMatchFoundState = bFindNext ?
                                                 ((fMode == FRMOD_WRAPED) ? NXT_WRP_FND : NXT_FND) :
                                                 ((fMode == FRMOD_WRAPED) ? PRV_WRP_FND : PRV_FND);
        }
        // found in range, set begin and end of finding
        *begin = SciCall_GetTargetStart();
        *end = SciCall_GetTargetEnd();
    } else {
        if (fMode != FRMOD_IGNORE) {
            Globals.FindReplaceMatchFoundState = (fMode != FRMOD_WRAPED) ? (bFindNext ? NXT_NOT_FND : PRV_NOT_FND) : FND_NOP;
        }
    }

    SciCall_SetTargetRange(saveTargetBeg, saveTargetEnd); // restore original target range

    return iPos;
}

DocPos EditFindInTarget(LPCWSTR wchFind, int sFlags, DocPos* begin, DocPos* end, bool bForceNext)
{
    return _FindInTarget(wchFind, sFlags, begin, end, bForceNext, FRMOD_IGNORE);
}


//=============================================================================
//
//  _FindHasMatch()
//
typedef enum { MATCH = 0, NO_MATCH = 1, INVALID = 2 } RegExResult_t;

static RegExResult_t _FindHasMatch(HWND hwnd, const LPEDITFINDREPLACE lpefr, DocPos iStartPos, bool bMarkAll)
{
    DocPos const iTextEnd = Sci_GetDocEndPosition();
    DocPos start = iStartPos;
    DocPos end = iTextEnd;

    int const sFlags = (int)(lpefr->fuFlags);
    LPCWSTR      wchFind = _EditGetFindStrg(hwnd, lpefr, false);
    DocPos const iPos = _FindInTarget(wchFind, sFlags, &start, &end, false, FRMOD_IGNORE);
    bool const   bRegexInvalid = StrIsNotEmpty(wchFind) && (sFlags & SCFIND_REGEXP) && Sci_ConsumeRegexWarnStatus();

    if (bMarkAll) {
        EditClearAllOccurrenceMarkers(hwnd);
        if (iPos >= 0) {
            EditMarkAll(wchFind, sFlags, 0, iTextEnd, false);
            if (FocusedView.HideNonMatchedLines) {
                // Lines hidden by the previous match set may now be headers.
                SciCall_FoldAll(EXPAND);
                EditFoldMarkedLineRange(lpefr->hwnd, true);
            }
        } else if (FocusedView.HideNonMatchedLines) {
            EditToggleView(lpefr->hwnd);
        }
    }
    return ((iPos >= 0) ? MATCH : (bRegexInvalid ? INVALID : NO_MATCH));
}


//=============================================================================
//
//  _DelayMarkAll()
//
//
static void  _DelayMarkAll(int delay)
{
    CmdMessageQueue_t mqc = MQ_WM_CMD_INIT(Globals.hwndDlgFindReplace, IDT_TIMER_CALLBACK_MRKALL, 0LL);
    _MQ_AppendCmd(&mqc, _MQ_ms2cycl(delay));
}

//=============================================================================

static bool s_SaveMarkOccurrences = false;
static bool s_SaveMarkMatchVisible = false;

//=============================================================================
//
//  EditBoxForPasteFixes()
//  Single-line EDIT's default WM_PASTE truncates at the first line break;
//  EM_REPLACESEL inserts multi-line clipboard text unchanged.
//
static LRESULT CALLBACK EditBoxForPasteFixes(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                             UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    UNREFERENCED_PARAMETER(dwRefData);

    switch (uMsg) {
    case WM_PASTE: {
        // copy first: EM_REPLACESEL synchronously triggers the incremental search,
        // which must not run while the clipboard is held open
        WCHAR wchBuffer[FNDRPL_BUFFER] = { L'\0' };
        EditGetClipboardW(wchBuffer, COUNTOF(wchBuffer));
        if (StrIsNotEmpty(wchBuffer)) {
            SendMessage(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)wchBuffer);
            return 0;
        }
    } break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, EditBoxForPasteFixes, uIdSubclass);
        break;

    default:
        break;
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}


//=============================================================================
//
//  _ShowZeroLengthCallTip()
//
static void _ShowZeroLengthCallTip(DocPos iPosition)
{
    char chZeroLenCT[80] = { '\0' };
    GetLngStringW2MB(IDS_MUI_ZERO_LEN_MATCH, chZeroLenCT, COUNTOF(chZeroLenCT));
    SciCall_CallTipShow(iPosition, chZeroLenCT);
}


//=============================================================================
//
//  _EnableFRDlgCtrls()
//
static bool _EnableFRDlgCtrls(HWND hwnd) {

    bool const bEmptyFnd = ComboBox_GetTextLengthEx(hwnd, IDC_FINDTEXT) == 0 /*|| ComboBox_GetCurSelEx(hwnd, IDC_FINDTEXT) != CB_ERR)*/;
    bool const bEmptyRpl = ComboBox_GetTextLengthEx(hwnd, IDC_REPLACETEXT) == 0 /*|| ComboBox_GetCurSelEx(hwnd, IDC_REPLACETEXT) != CB_ERR)*/;
    bool const bEmptySel = (SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection());

    DialogEnableControl(hwnd, IDOK, !bEmptyFnd);
    DialogEnableControl(hwnd, IDC_FINDPREV, !bEmptyFnd);
    DialogEnableControl(hwnd, IDC_REPLACE, !bEmptyFnd);
    DialogEnableControl(hwnd, IDC_REPLACEALL, !bEmptyFnd);
    DialogEnableControl(hwnd, IDC_REPLACEINSEL, !bEmptyFnd && !bEmptySel);
    DialogEnableControl(hwnd, IDC_SWAPSTRG, !bEmptyFnd || !bEmptyRpl);

    return !bEmptyFnd;
}


//=============================================================================
//
//  _AddPatternToComboMRU()
//  Insert pszItem at the top of a CBS_DROPDOWN combo's list without touching
//  its edit field (no CBN_EDITCHANGE side-effects).
//
static void _AddPatternToComboMRU(HWND hwnd, int nIDDlgItem, LPCWSTR pszItem)
{
    if (StrIsEmpty(pszItem)) { return; }
    HWND const hwndCtl = GetDlgItem(hwnd, nIDDlgItem);
    int const  idx     = (int)SendMessage(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)(-1), (LPARAM)pszItem);
    if (idx != CB_ERR) {
        SendMessage(hwndCtl, CB_DELETESTRING, (WPARAM)idx, 0);
    }
    SendMessage(hwndCtl, CB_INSERTSTRING, 0, (LPARAM)pszItem);
}


//=============================================================================
//
//  EditFindReplaceDlgProc()
//
typedef struct {
    LPEDITFINDREPLACE pEfrData;
    const FindReplaceDlgRestore* pRestore;
} FindReplaceDlgInit;

static void _RestoreFindReplaceDlgPosition(HWND hwnd, const RECT* pPreviousRect)
{
    MONITORINFO mi = { sizeof(mi) };
    if (IsRectEmpty(pPreviousRect) ||
            !GetMonitorInfo(MonitorFromRect(pPreviousRect, MONITOR_DEFAULTTONEAREST), &mi)) {
        return;
    }

    // The first move can resize the dialog via WM_DPICHANGED. Fit it again using the new size.
    for (int i = 0; i < 2; ++i) {
        RECT rc;
        if (!GetWindowRect(hwnd, &rc)) {
            return;
        }
        LONG const x = max_l(mi.rcWork.left, min_l(pPreviousRect->left, mi.rcWork.right - (rc.right - rc.left)));
        LONG const y = max_l(mi.rcWork.top, min_l(pPreviousRect->top, mi.rcWork.bottom - (rc.bottom - rc.top)));
        if (rc.left == x && rc.top == y) {
            break;
        }
        SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}


// TODO: §§§ get rid of this buffer
static WCHAR s_wchBufIn[FNDRPL_BUFFER] = { L'\0' }; // tmp working buffer
static WCHAR s_wchBufOut[FNDRPL_BUFFER] = { L'\0' }; // tmp working buffer


static INT_PTR CALLBACK EditFindReplaceDlgProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
{
    static LPEDITFINDREPLACE s_pEfrData = NULL;

    static bool s_bIsReplaceDlg = false;
    static bool s_bHasReplacementDraft = false;
    static bool s_bRestoreFocusedView = false;

    static UINT_PTR pTimerIdentifier = 0;

    static DocPos s_InitialSearchStart = 0;
    static DocPos s_InitialAnchorPos = 0;
    static DocPos s_InitialCaretPos = 0;
    static DocLn s_InitialTopLine = -1;

#define SET_INITIAL_ANCHORS() {\
        s_InitialTopLine = -1;\
        s_InitialCaretPos = SciCall_GetCurrentPos();\
        s_InitialAnchorPos = SciCall_GetAnchor();\
        s_InitialSearchStart = SciCall_GetSelectionStart(); \
    }

    static RegExResult_t s_anyMatch = NO_MATCH;

    static HBRUSH hBrushRed;
    static HBRUSH hBrushGreen;
    static HBRUSH hBrushBlue;

    switch (umsg) {
    case WM_INITDIALOG: {

        const FindReplaceDlgInit* const pInit = (const FindReplaceDlgInit*)lParam;
        s_pEfrData = pInit->pEfrData;
        s_bRestoreFocusedView = pInit->pRestore && pInit->pRestore->bFocusedView;

        Globals.hwndDlgFindReplace = hwnd;
        s_bIsReplaceDlg = (GetDlgItem(hwnd, IDC_REPLACETEXT) != NULL);

        // clear cmd line stuff
        g_flagMatchText = 0;

        SetDialogIconNP3(hwnd);
        InitWindowCommon(hwnd, true);

#ifdef D_NP3_WIN10_DARK_MODE
        if (UseDarkMode()) {
            int const ctlx[] = { IDOK, IDCANCEL, IDC_FINDPREV, IDC_REPLACE, IDC_REPLACEALL,
                                 IDC_REPLACEINSEL, IDC_SWAPSTRG, IDC_TOGGLE_VISIBILITY
                               };
            for (int i = 0; i < COUNTOF(ctlx); ++i) {
                SetExplorerTheme(GetDlgItem(hwnd, ctlx[i]));
            }
            int const ctl[] = { IDC_FINDCASE, IDC_FINDWORD, IDC_FINDSTART, IDC_FINDTRANSFORMBS, IDC_FINDESCCTRLCHR, IDC_REPLESCCTRLCHR,
                                IDC_FINDREGEXP, IDC_DOT_MATCH_ALL, IDC_NOWRAP, IDC_FINDCLOSE,
                                IDC_ALL_OCCURRENCES, IDC_INST_INCR_SEARCH, IDC_WILDCARDSEARCH, IDC_TRANSPARENT, IDC_STATIC, IDC_STATIC2
                              };
            for (int i = 0; i < COUNTOF(ctl); ++i) {
                SetWindowTheme(GetDlgItem(hwnd, ctl[i]), L"", L""); // remove theme for BS_AUTORADIOBUTTON
            }
            //SetExplorerTheme(GetDlgItem(hwnd, IDC_RESIZEGRIP));
        }
#endif

        pTimerIdentifier = SetTimer(hwnd, 0, _MQ_TIMER_CYCLE, MQ_ExecuteNext);

        SET_INITIAL_ANCHORS()
        s_InitialTopLine = SciCall_GetFirstVisibleLine();

        EditSetCaretToSelectionStart(); // avoid search text selection jumps to next match (before ResizeDlg_InitX())

        Globals.iReplacedOccurrences = 0;
        Globals.FindReplaceMatchFoundState = FND_NOP;

        s_SaveMarkOccurrences = s_bSwitchedFindReplace ? s_SaveMarkOccurrences : Settings.MarkOccurrences;
        s_SaveMarkMatchVisible = s_bSwitchedFindReplace ? s_SaveMarkMatchVisible : Settings.MarkOccurrencesMatchVisible;
        // switch off normal mark occurrences
        Settings.MarkOccurrences = false;
        Settings.MarkOccurrencesMatchVisible = false;
        EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_MARKOCCUR_ONOFF, false);

        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_LIMITTEXT, FNDRPL_BUFFER, 0);
        SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_SETEXTENDEDUI, true, 0);

        // Load MRUs
        for (int i = 0; i < MRU_Count(Globals.pMRUfind); i++) {
            MRU_Enum(Globals.pMRUfind, i, s_wchBufOut, COUNTOF(s_wchBufOut));
            SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_ADDSTRING, 0, (LPARAM)s_wchBufOut);
        }

        COMBOBOXINFO cbInfoF = { sizeof(COMBOBOXINFO) };
        GetComboBoxInfo(GetDlgItem(hwnd, IDC_FINDTEXT), &cbInfoF);
        if (cbInfoF.hwndItem) {
            SetWindowSubclass(cbInfoF.hwndItem, EditBoxForPasteFixes, 0, 0);
            SHAutoComplete(cbInfoF.hwndItem, SHACF_FILESYS_ONLY | SHACF_AUTOAPPEND_FORCE_OFF | SHACF_AUTOSUGGEST_FORCE_OFF);
        }

        if (s_bIsReplaceDlg) {

            SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_LIMITTEXT, FNDRPL_BUFFER, 0);
            SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_SETEXTENDEDUI, true, 0);

            // Load MRUs
            for (int i = 0; i < MRU_Count(Globals.pMRUreplace); i++) {
                MRU_Enum(Globals.pMRUreplace, i, s_wchBufOut, COUNTOF(s_wchBufOut));
                SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_ADDSTRING, 0, (LPARAM)s_wchBufOut);
            }

            COMBOBOXINFO cbInfoR = { sizeof(COMBOBOXINFO) };
            GetComboBoxInfo(GetDlgItem(hwnd, IDC_REPLACETEXT), &cbInfoR);
            if (cbInfoR.hwndItem) {
                SetWindowSubclass(cbInfoR.hwndItem, EditBoxForPasteFixes, 0, 0);
                SHAutoComplete(cbInfoR.hwndItem, SHACF_FILESYS_ONLY | SHACF_AUTOAPPEND_FORCE_OFF | SHACF_AUTOSUGGEST_FORCE_OFF);
            }
        }

        // Initialize all checkboxes BEFORE setting combobox text.
        // Setting combobox text triggers CBN_EDITCHANGE synchronously,
        // which calls _SetSearchFlags() that reads checkbox states.
        // Checkboxes must reflect s_pEfrData values at that point.
        CheckDlgButton(hwnd, IDC_FINDREGEXP, SetBtn(s_pEfrData->bRegExprSearch));

        bool const bDotMatchAll = (s_pEfrData->fuFlags & SCFIND_DOT_MATCH_ALL) != 0;
        CheckDlgButton(hwnd, IDC_DOT_MATCH_ALL, SetBtn(s_pEfrData->bRegExprSearch && bDotMatchAll));
        DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, s_pEfrData->bRegExprSearch);

        if (s_pEfrData->bRegExprSearch) {
            s_pEfrData->bWildcardSearch = false;
        }
        if (s_pEfrData->bWildcardSearch) {
            CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_CHECKED);
            CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
        }

        // transform BS handled by regex (wild-card search based on):
        CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_pEfrData->bTransformBS || s_pEfrData->bRegExprSearch || s_pEfrData->bWildcardSearch));
        DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, !(s_pEfrData->bRegExprSearch || s_pEfrData->bWildcardSearch));

        CheckDlgButton(hwnd, IDC_ALL_OCCURRENCES, SetBtn(s_pEfrData->bMarkOccurences));
        if (!s_pEfrData->bMarkOccurences) {
            EditClearAllOccurrenceMarkers(s_pEfrData->hwnd);
        }

        CheckDlgButton(hwnd, IDC_INST_INCR_SEARCH, SetBtn(s_pEfrData->bInstantIncrementalSearch));

        CheckDlgButton(hwnd, IDC_FINDCASE, SetBtn(s_pEfrData->fuFlags & SCFIND_MATCHCASE));
        CheckDlgButton(hwnd, IDC_FINDWORD, SetBtn(s_pEfrData->fuFlags & SCFIND_WHOLEWORD));
        CheckDlgButton(hwnd, IDC_FINDSTART, SetBtn(s_pEfrData->fuFlags & SCFIND_WORDSTART));
        CheckDlgButton(hwnd, IDC_NOWRAP, SetBtn(s_pEfrData->bNoFindWrap));

        if (s_bIsReplaceDlg) {
            CheckDlgButton(hwnd, IDC_FINDCLOSE, SetBtn(s_pEfrData->bReplaceClose));
        } else {
            CheckDlgButton(hwnd, IDC_FINDCLOSE, SetBtn(s_pEfrData->bFindClose));
        }

        CheckDlgButton(hwnd, IDC_TRANSPARENT, SetBtn(Settings.FindReplaceTransparentMode));

        if (s_bIsReplaceDlg) {
            // Populate replacement text before the find combo can sync both fields.
            s_pEfrData = NULL;
            if (pInit->pRestore || s_bHasReplacementDraft || StrgIsNotEmpty(pInit->pEfrData->chReplaceTemplate)) {
                ComboBox_SetTextHW(hwnd, IDC_REPLACETEXT, pInit->pEfrData->chReplaceTemplate);
            }
            else if (MRU_Count(Globals.pMRUreplace)) {
                MRU_Enum(Globals.pMRUreplace, 0, s_wchBufOut, COUNTOF(s_wchBufOut));
                ComboBox_SetTextW(hwnd, IDC_REPLACETEXT, s_wchBufOut);
            }
            else {
                ComboBox_SetTextW(hwnd, IDC_REPLACETEXT, NULL);
            }
            s_pEfrData = pInit->pEfrData;
        }

        if (pInit->pRestore) {
            s_pEfrData = NULL;
            ComboBox_SetTextHW(hwnd, IDC_FINDTEXT, pInit->pEfrData->chFindPattern);
            s_pEfrData = pInit->pEfrData;
        } else if (!GetWindowTextLengthW(GetDlgItem(hwnd, IDC_FINDTEXT))) {
            if (StrgIsNotEmpty(s_pEfrData->chFindPattern)) {
                ComboBox_SetTextHW(hwnd, IDC_FINDTEXT, s_pEfrData->chFindPattern);
            }
            else if (MRU_Count(Globals.pMRUfind)) {
                MRU_Enum(Globals.pMRUfind, 0, s_wchBufOut, COUNTOF(s_wchBufOut));
                ComboBox_SetTextW(hwnd, IDC_FINDTEXT, s_wchBufOut);
            }
            else {
                ComboBox_SetTextW(hwnd, IDC_FINDTEXT, NULL);
            }
        }

        if (s_bSwitchedFindReplace) {
            // restore from prev Dlg
            SetDlgPos(hwnd, s_xFindReplaceDlgSave, s_yFindReplaceDlgSave);
            s_bSwitchedFindReplace = false;
        }
        else if (pInit->pRestore) {
            _RestoreFindReplaceDlgPosition(hwnd, &pInit->pRestore->windowRect);
        }
        else {
            if (Settings.FindReplaceDlgPosX == CW_USEDEFAULT || Settings.FindReplaceDlgPosY == CW_USEDEFAULT) {
                CenterDlgInParent(hwnd, false);
            }
            else {
                SetDlgPos(hwnd, Settings.FindReplaceDlgPosX, Settings.FindReplaceDlgPosY);
            }
        }

        WCHAR wchMenuBuf[80] = {L'\0'};
        HMENU hmenu = GetSystemMenu(hwnd, FALSE);

        GetLngString(IDS_MUI_SAVEPOS, wchMenuBuf, COUNTOF(wchMenuBuf));
        InsertMenu(hmenu, 0, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_SAVEPOS, wchMenuBuf);
        GetLngString(IDS_MUI_RESETPOS, wchMenuBuf, COUNTOF(wchMenuBuf));
        InsertMenu(hmenu, 1, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_RESETPOS, wchMenuBuf);
        InsertMenu(hmenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        GetLngString(IDS_MUI_CLEAR_FIND_HISTORY, wchMenuBuf, COUNTOF(wchMenuBuf));
        InsertMenu(hmenu, 3, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_CLEAR_FIND_HISTORY, wchMenuBuf);
        GetLngString(IDS_MUI_CLEAR_REPL_HISTORY, wchMenuBuf, COUNTOF(wchMenuBuf));
        InsertMenu(hmenu, 4, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_CLEAR_REPL_HISTORY, wchMenuBuf);
        InsertMenu(hmenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        GetLngString(IDS_MUI_SEARCHCLIPIFEMPTY, wchMenuBuf, COUNTOF(wchMenuBuf));
        InsertMenu(hmenu, 6, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_SEARCHCLIPIFEMPTY, wchMenuBuf);
        GetLngString(IDS_MUI_REPLCLIPTAG, wchMenuBuf, COUNTOF(wchMenuBuf));
        InsertMenu(hmenu, 7, MF_BYPOSITION | MF_STRING | MF_ENABLED, IDS_MUI_REPLCLIPTAG, wchMenuBuf);
        InsertMenu(hmenu, 8, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

        CheckCmd(hmenu, IDS_MUI_SEARCHCLIPIFEMPTY, Settings.SearchByClipboardIfEmpty);
        CheckCmd(hmenu, IDS_MUI_REPLCLIPTAG, Settings.ReplaceByClipboardTag);


        hBrushRed = CreateSolidBrush(rgbRedColorRef);
        hBrushGreen = CreateSolidBrush(rgbGreenColorRef);
        hBrushBlue = CreateSolidBrush(rgbBlueColorRef);

        s_anyMatch = NO_MATCH;

        _SetSearchFlags(hwnd, s_pEfrData); // sync
        s_pEfrData->bStateChanged = true;  // force update

        DialogEnableControl(hwnd, IDC_TOGGLE_VISIBILITY, s_pEfrData->bMarkOccurences);

        _DelayMarkAll(_MQ_STD);

        PostMessage(hwnd, WM_THEMECHANGED, 0, 0);
    }
    return TRUE; // (!) further processing

    case WM_ENABLE:
        // modal child dialog should disable main window too
        EnableWindow(Globals.hwndMain, (BOOL)wParam);
        return TRUE;

    case WM_CLOSE:
        // Language switching needs the modeless dialog destroyed before the resource DLL is unloaded.
        DestroyWindow(hwnd);
        return TRUE;

    case WM_DESTROY: {

        KillTimer(hwnd, pTimerIdentifier);
        pTimerIdentifier = 0;

        _SetSearchFlags(hwnd, s_pEfrData); // sync
        DuplicateEFR(&(Settings.EFR_Data), s_pEfrData);

        if (!s_bSwitchedFindReplace) {
            s_bHasReplacementDraft = false;
            if (s_anyMatch == MATCH) {
                // Save MRUs
                if (StrgIsNotEmpty(s_pEfrData->chFindPattern)) {
                    MRU_Add(Globals.pMRUfind, StrgGet(s_pEfrData->chFindPattern), 0, -1, -1, NULL);
                    SetFindPattern(StrgGet(s_pEfrData->chFindPattern));
                }
                if (s_bIsReplaceDlg) {
                    MRU_Add(Globals.pMRUreplace, StrgGet(s_pEfrData->chReplaceTemplate), 0, -1, -1, NULL);
                }
            }

            Globals.iReplacedOccurrences = 0;
            Globals.FindReplaceMatchFoundState = FND_NOP;

            Settings.MarkOccurrences = s_SaveMarkOccurrences;
            Settings.MarkOccurrencesMatchVisible = s_SaveMarkMatchVisible;
            EnableCmd(GetMenu(Globals.hwndMain), IDM_VIEW_MARKOCCUR_ONOFF, true);

            if (FocusedView.HideNonMatchedLines) {
                EditToggleView(s_pEfrData->hwnd);
            }

            if (IsMarkOccurrencesEnabled()) {
                MarkAllOccurrences(_MQ_STD, true);
            } else {
                EditClearAllOccurrenceMarkers(s_pEfrData->hwnd);
            }

            if (s_InitialTopLine >= 0) {
                SciCall_SetFirstVisibleLine(s_InitialTopLine);
                s_InitialTopLine = -1;  // reset
            } else {
                if (s_anyMatch == NO_MATCH) {
                    EditSetSelectionEx(s_InitialAnchorPos, s_InitialCaretPos, -1, -1);
                } else {
                    Sci_ScrollSelectionToView();
                }
            }

            CmdMessageQueue_t* pmqc = NULL;
            CmdMessageQueue_t* dummy;
            DL_FOREACH_SAFE(MessageQueue, pmqc, dummy) {
                DL_DELETE(MessageQueue, pmqc);
                FreeMem(pmqc);
            }
        }

        DeleteObject(hBrushRed);
        DeleteObject(hBrushGreen);
        DeleteObject(hBrushBlue);
        GetSystemMenu(hwnd, TRUE);

        s_pEfrData = NULL;
        Globals.hwndDlgFindReplace = NULL;
    }
    return FALSE;


    case WM_DPICHANGED:
        UpdateWindowLayoutForDPI(hwnd, (RECT*)lParam, LOWORD(wParam));
        return TRUE; // further processing


#ifdef D_NP3_WIN10_DARK_MODE

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORBTN:
    //~case WM_CTLCOLOREDIT:
    //~case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC:
        return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
        break;

    case WM_SETTINGCHANGE:
        if (IsDarkModeSupported() && IsColorSchemeChangeMessage(lParam)) {
            SendMessage(hwnd, WM_THEMECHANGED, 0, 0);
        }
        break;

    case WM_THEMECHANGED:
        if (IsDarkModeSupported()) {
            bool const darkModeEnabled = CheckDarkModeEnabled();
            AllowDarkModeForWindowEx(hwnd, darkModeEnabled);
            RefreshTitleBarThemeColor(hwnd);
            int const ctlx[] = { IDOK, IDCANCEL, IDC_FINDPREV, IDC_REPLACE, IDC_SWAPSTRG,
                                 IDC_REPLACEALL, IDC_REPLACEINSEL, IDC_TOGGLE_VISIBILITY
                               };
            for (int i = 0; i < COUNTOF(ctlx); ++i) {
                HWND const hBtn = GetDlgItem(hwnd, ctlx[i]);
                AllowDarkModeForWindowEx(hBtn, darkModeEnabled);
                SendMessage(hBtn, WM_THEMECHANGED, 0, 0);
            }
            UpdateWindowEx(hwnd);
        }
        break;

#endif

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        if (!s_pEfrData) {
            return false;
        }
        HWND hCheck = (HWND)lParam;
        HDC hDC = (HDC)wParam;

        HWND hComboBox = GetDlgItem(hwnd, IDC_FINDTEXT);
        COMBOBOXINFO ci = { sizeof(COMBOBOXINFO) };
        GetComboBoxInfo(hComboBox, &ci);

        //if (hCheck == ci.hwndItem || hCheck == ci.hwndList)
        if (hCheck == ci.hwndItem) {
            SetBkMode(hDC, TRANSPARENT);
            INT_PTR hBrush;
            switch (s_anyMatch) {
            case MATCH:
                //SetTextColor(hDC, green);
                SetBkColor(hDC, rgbGreenColorRef);
                hBrush = (INT_PTR)hBrushGreen;
                break;
            case NO_MATCH:
                //SetTextColor(hDC, blue);
                SetBkColor(hDC, rgbBlueColorRef);
                hBrush = (INT_PTR)hBrushBlue;
                break;
            case INVALID:
            default:
                //SetTextColor(hDC, red);
                SetBkColor(hDC, rgbRedColorRef);
                hBrush = (INT_PTR)hBrushRed;
                break;
            }
            return hBrush;
        }
    }
#ifdef D_NP3_WIN10_DARK_MODE
    return SetDarkModeCtlColors((HDC)wParam, UseDarkMode());
#else
    return FALSE;
#endif


    case WM_ACTIVATE: {
        if (!s_pEfrData) {
            return false;
        }

        switch (LOWORD(wParam)) {
        case WA_INACTIVE:
            SetWindowTransparentMode(hwnd, Settings.FindReplaceTransparentMode, Settings2.FindReplaceOpacityLevel);
            break;

        case WA_CLICKACTIVE:
        // mouse click activation
        case WA_ACTIVE:
            SetWindowTransparentMode(hwnd, false, 100);

            // selection changed ?
            if ((SciCall_GetCurrentPos() != s_InitialCaretPos) ||
                    (SciCall_GetAnchor() != s_InitialAnchorPos)) {
                EditSetCaretToSelectionStart();
                s_InitialAnchorPos = SciCall_GetAnchor();
                s_InitialCaretPos = SciCall_GetCurrentPos();
                s_InitialTopLine = SciCall_GetFirstVisibleLine();
                s_InitialSearchStart = s_InitialCaretPos;
                if (s_pEfrData) {
                    s_pEfrData->bStateChanged = true;
                }
            }

            bool const bEnableReplInSel = !(SciCall_IsSelectionEmpty() || Sci_IsMultiOrRectangleSelection());
            DialogEnableControl(hwnd, IDC_REPLACEINSEL, bEnableReplInSel);

            _DelayMarkAll(_MQ_STD);

            if (!SciCall_IsSelectionEmpty()) {
                Sci_ScrollSelectionToView();
            }
            /// don't do:  ///~SendWMCommandEx(hwnd, IDC_FINDTEXT, CBN_EDITCHANGE);
            break;

        default:
            break;
        }
    }
    return false;


    case WM_COMMAND: {
        if (!s_pEfrData) {
            return FALSE;
        }

        switch (LOWORD(wParam)) {

        case IDC_DOC_MODIFIED:
            s_InitialSearchStart = SciCall_GetSelectionStart();
            s_InitialTopLine = -1;  // reset
            s_pEfrData->bStateChanged = true;
            _DelayMarkAll(_MQ_STD);
            break;


        case IDC_FINDTEXT:
        case IDC_REPLACETEXT: {

            bool bFndPatternChanged = s_pEfrData->bStateChanged;

            switch (HIWORD(wParam)) {

            case CBN_EDITUPDATE:
                if (SendDlgItemMessage(hwnd, LOWORD(wParam), CB_GETDROPPEDSTATE, 0, 0)) {
                    //~SendDlgItemMessage(hwnd, LOWORD(wParam), CB_SETCURSEL, (WPARAM)-1, 0); // clear
                    SendDlgItemMessage(hwnd, LOWORD(wParam), CB_SHOWDROPDOWN, 0, 0); // hide
                }
                break;

            //~case CBN_SELCHANGE:
            case CBN_CLOSEUP: {
                int const cursel = (int)SendDlgItemMessage(hwnd, LOWORD(wParam), CB_GETCURSEL, 0, 0);
                if (cursel >= 0) {
                    SendDlgItemMessage(hwnd, LOWORD(wParam), CB_SETCURSEL, (WPARAM)cursel, 0);
                }
            }
            // [fallthrough]
            case CBN_SELENDOK:
            case CBN_EDITCHANGE:
                bFndPatternChanged = (LOWORD(wParam) == IDC_FINDTEXT);
                break;

            case CBN_KILLFOCUS:
            case CBN_SELENDCANCEL:
            default:
                break;
            }

            if (Globals.bFindReplCopySelOrClip) {
                // reset first: setting the combo text below re-enters this handler (CBN_EDITCHANGE)
                Globals.bFindReplCopySelOrClip = false;
                char*        lpszSelection = NULL;
                DocPos const cchSelection = SciCall_GetSelText(NULL);
                // WM_SETTEXT ignores CB_LIMITTEXT: an oversized selection freezes the combo's edit control
                if ((cchSelection > 0) && (cchSelection <= FNDRPL_BUFFER) && (LOWORD(wParam) != IDC_REPLACETEXT)) {
                    lpszSelection = AllocMem((cchSelection + 1), HEAP_ZERO_MEMORY);
                    SciCall_GetSelText(lpszSelection);
                } else { // no (usable) selection
                    // nothing is selected in the editor:
                    // if first time you bring up find/replace dialog,
                    // use most recent search pattern to find box
                    // in case of no history: paste clipboard
                    LPCWSTR wchFind = _EditGetFindStrg(Globals.hwndEdit, s_pEfrData, true);
                    if (StrIsNotEmpty(wchFind)) {
                        int const size = WideCharToMultiByte(Encoding_SciCP, 0, wchFind, -1, NULL, 0, NULL, NULL);
                        lpszSelection = AllocMem(size * sizeof(CHAR), HEAP_ZERO_MEMORY);
                        WideCharToMultiByte(Encoding_SciCP, 0, wchFind, -1, lpszSelection, size, NULL, NULL);
                    }
                }
                if (lpszSelection) {
                    ComboBox_SetTextMB2W(hwnd, IDC_FINDTEXT, lpszSelection);
                    FreeMem(lpszSelection);
                    lpszSelection = NULL;
                    bFndPatternChanged = true;
                }
                s_InitialTopLine = -1; // reset

            } // Globals.bFindReplCopySelOrClip

            _SetSearchFlags(hwnd, s_pEfrData);
            SetFindPattern(StrgGet(s_pEfrData->chFindPattern));

            // ------------------------

            if (!(bFndPatternChanged || s_pEfrData->bStateChanged)) {
                break; // return
            }
            // ------------------------

            EditClearAllOccurrenceMarkers(s_pEfrData->hwnd);

            if (_EnableFRDlgCtrls(hwnd)) {
                EditSetSelectionEx(s_InitialAnchorPos, s_InitialCaretPos, -1, -1);
            }

            DocPos start = s_InitialSearchStart;
            DocPos end = Sci_GetDocEndPosition();

            LPCWSTR wchFind = _EditGetFindStrg(s_pEfrData->hwnd, s_pEfrData, false);
            DocPos const iPos = _FindInTarget(wchFind, (int)(s_pEfrData->fuFlags), &start, &end, false, FRMOD_NORM);

            if (s_pEfrData->bInstantIncrementalSearch) {
                if (iPos >= 0) {
                    if (s_bIsReplaceDlg) {
                        SciCall_ScrollRange(end, iPos);
                    }
                    else {
                        EditSetSelectionEx(end, iPos, -1, -1);
                    }
                    if (iPos == end) {
                        _ShowZeroLengthCallTip(iPos);
                    }
                }
                else {
                    if (s_bIsReplaceDlg) {
                        SciCall_ScrollRange(s_InitialAnchorPos, s_InitialCaretPos);
                    }
                    if (s_InitialTopLine >= 0) {
                        SciCall_SetFirstVisibleLine(s_InitialTopLine);
                    }
                }
            }
            _DelayMarkAll(_MQ_STD);
        }
        break;

        case IDT_TIMER_CALLBACK_MRKALL: {
            bool const bRestoreFocusedView = s_bRestoreFocusedView;
            s_bRestoreFocusedView = false;
            static wchar_t s_lastFind[FNDRPL_BUFFER] = { L'\0' };
            bool const bRefreshMarks = s_pEfrData->bMarkOccurences &&
                (s_pEfrData->bStateChanged || (StringCchCompareX(s_lastFind, StrgGet(s_pEfrData->chFindPattern)) != 0));
            if (bRefreshMarks) {
                StringCchCopyW(s_lastFind, COUNTOF(s_lastFind), StrgGet(s_pEfrData->chFindPattern));
            }
            s_anyMatch = _FindHasMatch(s_pEfrData->hwnd, s_pEfrData, 0, bRefreshMarks);
            InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, TRUE); // coloring
            if (bRefreshMarks) {
                if (bRestoreFocusedView && s_anyMatch == MATCH && (Settings.FocusViewMarkerMode & FVMM_FOLD)) {
                    // Fold the rebuilt matches without re-running Focused View's bookmark action.
                    FocusedView.HideNonMatchedLines = true;
                    EditFoldMarkedLineRange(s_pEfrData->hwnd, true);
                    UpdateDocumentReadOnlyState();
                }
                UpdateStatusbar(false);
            } else if (!s_pEfrData->bMarkOccurences && s_pEfrData->bStateChanged) {
                if (FocusedView.HideNonMatchedLines) {
                    SendWMCommand(hwnd, IDC_TOGGLE_VISIBILITY);
                } else {
                    EditClearAllOccurrenceMarkers(s_pEfrData->hwnd);
                    InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, TRUE);
                }
            }
            s_pEfrData->bStateChanged = false;
        }
        break;

        case IDC_ALL_OCCURRENCES: {
            _SetSearchFlags(hwnd, s_pEfrData);

            if (IsButtonChecked(hwnd, IDC_ALL_OCCURRENCES)) {
                DialogEnableControl(hwnd, IDC_TOGGLE_VISIBILITY, true);
                _DelayMarkAll(_MQ_STD);
            } else { // switched OFF
                DialogEnableControl(hwnd, IDC_TOGGLE_VISIBILITY, false);
                if (FocusedView.HideNonMatchedLines) {
                    EditToggleView(s_pEfrData->hwnd);
                }
                EditClearAllOccurrenceMarkers(s_pEfrData->hwnd);
                InvalidateRect(GetDlgItem(hwnd, IDC_FINDTEXT), NULL, TRUE);
            }
        }
        break;

        case IDC_INST_INCR_SEARCH:
            _SetSearchFlags(hwnd, s_pEfrData);
            _DelayMarkAll(_MQ_STD);
            break;

        case IDC_TOGGLE_VISIBILITY:
            if (s_pEfrData) {
                s_bRestoreFocusedView = false;
                EditToggleView(s_pEfrData->hwnd);
                if (!FocusedView.HideNonMatchedLines) {
                    s_pEfrData->bStateChanged = true;
                    s_InitialTopLine = -1;  // reset
                    EditClearAllOccurrenceMarkers(s_pEfrData->hwnd);
                    _DelayMarkAll(_MQ_STD);
                }
            }
            break;

        case IDC_FINDREGEXP:
            if (IsButtonChecked(hwnd, IDC_FINDREGEXP)) {
                DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, true);
                // Can not use wild-card search together with RegExp
                CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED);
                // transform BS handled by regex
                CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
                DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, false);
            } else { // unchecked
                DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, false);
                DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, true);
                CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_pEfrData->bTransformBS));
            }
            _SetSearchFlags(hwnd, s_pEfrData);
            _DelayMarkAll(_MQ_STD);
            break;

        case IDC_DOT_MATCH_ALL:
            _SetSearchFlags(hwnd, s_pEfrData);
            _DelayMarkAll(_MQ_STD);
            break;

        case IDC_WILDCARDSEARCH: {
            if (IsButtonChecked(hwnd, IDC_WILDCARDSEARCH)) {
                // Can not use wild-card search together with RegExp
                CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
                DialogEnableControl(hwnd, IDC_DOT_MATCH_ALL, false);
                // transform BS handled by regex (wild-card search based on):
                CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_CHECKED);
                DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, false);
            } else { // unchecked
                DialogEnableControl(hwnd, IDC_FINDTRANSFORMBS, true);
                CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, SetBtn(s_pEfrData->bTransformBS));
            }
            _SetSearchFlags(hwnd, s_pEfrData);
            _DelayMarkAll(_MQ_STD);
        }
        break;

        case IDC_FINDTRANSFORMBS: {
            _SetSearchFlags(hwnd, s_pEfrData);
            _DelayMarkAll(_MQ_STD);
        }
        break;

        case IDC_FINDCASE:
            _SetSearchFlags(hwnd, s_pEfrData);
            _DelayMarkAll(_MQ_STD);
            break;

        case IDC_FINDWORD:
            _SetSearchFlags(hwnd, s_pEfrData);
            _DelayMarkAll(_MQ_STD);
            break;

        case IDC_FINDSTART:
            _SetSearchFlags(hwnd, s_pEfrData);
            _DelayMarkAll(_MQ_STD);
            break;

        case IDC_TRANSPARENT:
            Settings.FindReplaceTransparentMode = IsButtonChecked(hwnd, IDC_TRANSPARENT);
            break;

        case IDC_REPLACE:
        case IDC_REPLACEALL:
        case IDC_REPLACEINSEL:
            Globals.iReplacedOccurrences = 0;
        case IDOK:
        case IDC_FINDPREV:
        case IDACC_SELTONEXT:
        case IDACC_SELTOPREV: {
            if (!_EnableFRDlgCtrls(hwnd)) {
                return true;
            }

            _SetSearchFlags(hwnd, s_pEfrData);

            // Save MRUs
            if (StrgIsNotEmpty(s_pEfrData->chFindPattern)) {
                MRU_Add(Globals.pMRUfind, StrgGet(s_pEfrData->chFindPattern), 0, -1, -1, NULL);
                SetFindPattern(StrgGet(s_pEfrData->chFindPattern));
            }
            if (s_bIsReplaceDlg) {
                MRU_Add(Globals.pMRUreplace, StrgGet(s_pEfrData->chReplaceTemplate), 0, -1, -1, NULL);
                _AddPatternToComboMRU(hwnd, IDC_REPLACETEXT, StrgGet(s_pEfrData->chReplaceTemplate));
            }
            _AddPatternToComboMRU(hwnd, IDC_FINDTEXT, StrgGet(s_pEfrData->chFindPattern));
            SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetFocus()), 1);

            bool bFound = false;

            switch (LOWORD(wParam)) {
            case IDOK: // find next
            case IDACC_SELTONEXT:
                if (s_bIsReplaceDlg) {
                    Globals.bReplaceInitialized = true;
                }
                bFound = EditFindNext(s_pEfrData->hwnd, s_pEfrData, (LOWORD(wParam) == IDACC_SELTONEXT), IsKeyDown(VK_F3), true);
                SET_INITIAL_ANCHORS()
                break;

            case IDC_FINDPREV: // find previous
            case IDACC_SELTOPREV:
                if (s_bIsReplaceDlg) {
                    Globals.bReplaceInitialized = true;
                }
                bFound = EditFindPrev(s_pEfrData->hwnd, s_pEfrData, (LOWORD(wParam) == IDACC_SELTOPREV), IsKeyDown(VK_F3), true);
                SET_INITIAL_ANCHORS()
                break;

            case IDC_REPLACE: {
                Globals.bReplaceInitialized = true;
                bFound = EditReplace(s_pEfrData->hwnd, s_pEfrData);
            }
            break;

            case IDC_REPLACEALL:
                Globals.bReplaceInitialized = true;
                bFound = EditReplaceAll(s_pEfrData->hwnd, s_pEfrData, true);
                break;

            case IDC_REPLACEINSEL:
                if (!SciCall_IsSelectionEmpty()) {
                    Globals.bReplaceInitialized = true;
                    bFound = EditReplaceAllInSelection(s_pEfrData->hwnd, s_pEfrData, true);
                }
                break;
            }

            if (bFound && !s_bIsReplaceDlg && (s_pEfrData->bFindClose)) {
                //~EndDialog(hwnd, LOWORD(wParam)); ~ (!) not running on own message loop
                DestroyWindow(hwnd);
            }
            else if (bFound && s_bIsReplaceDlg && s_pEfrData->bReplaceClose) {
                //~EndDialog(hwnd, LOWORD(wParam)); ~ (!) not running on own message loop
                DestroyWindow(hwnd);
            }
        }
        _DelayMarkAll(_MQ_STD);
        break;

        case IDMSG_SWITCHTOFIND:
        case IDMSG_SWITCHTOREPLACE:
            if (s_bIsReplaceDlg == (LOWORD(wParam) == IDMSG_SWITCHTOFIND)) {
                GetDlgPos(hwnd, &s_xFindReplaceDlgSave, &s_yFindReplaceDlgSave);
                s_bSwitchedFindReplace = true;
                s_bHasReplacementDraft |= s_bIsReplaceDlg;
            }
            break;

        case IDCANCEL:
            //~EndDialog(hwnd, IDCANCEL); ~ (!) not running on own message loop
            DestroyWindow(hwnd);
            break;

        case IDC_FINDESCCTRLCHR:
        case IDC_REPLESCCTRLCHR: {
            UINT const ctrl_id = (LOWORD(wParam) == IDC_FINDESCCTRLCHR) ? IDC_FINDTEXT : IDC_REPLACETEXT;
            GetDlgItemTextW(hwnd, ctrl_id, s_wchBufIn, COUNTOF(s_wchBufIn));
            size_t const inLen = StringCchLen(s_wchBufIn, 0);
            if (SlashCtrlW(s_wchBufOut, COUNTOF(s_wchBufOut), s_wchBufIn) == inLen) {
                UnSlashCtrlW(s_wchBufOut);
            }
            SetDlgItemTextW(hwnd, ctrl_id, s_wchBufOut);
            if ((ctrl_id == IDC_FINDTEXT) && (inLen != StringCchLen(s_wchBufOut, 0))) {
                Globals.FindReplaceMatchFoundState = FND_NOP;
                _SetSearchFlags(hwnd, s_pEfrData);
                _DelayMarkAll(_MQ_ASAP);
            }
        }
        break;

        case IDC_SWAPSTRG: {
            GetDlgItemTextW(hwnd, IDC_FINDTEXT, s_wchBufOut, COUNTOF(s_wchBufOut));
            GetDlgItemTextW(hwnd, IDC_REPLACETEXT, s_wchBufIn, COUNTOF(s_wchBufIn));
            SetDlgItemTextW(hwnd, IDC_FINDTEXT, s_wchBufIn);
            SetDlgItemTextW(hwnd, IDC_REPLACETEXT, s_wchBufOut);
            if (StringCchCompareX(s_wchBufIn, s_wchBufOut) != 0) {
                Globals.FindReplaceMatchFoundState = FND_NOP;
                _SetSearchFlags(hwnd, s_pEfrData);
                _DelayMarkAll(_MQ_ASAP);
            }
        }
        break;

        case IDACC_FIND:
            PostWMCommand(GetParent(hwnd), IDM_EDIT_FIND);
            break;

        case IDACC_REPLACE:
            PostWMCommand(GetParent(hwnd), IDM_EDIT_REPLACE);
            break;

        case IDACC_SAVEPOS:
            GetDlgPos(hwnd, &Settings.FindReplaceDlgPosX, &Settings.FindReplaceDlgPosY);
            break;

        case IDACC_RESETPOS:
            CenterDlgInParent(hwnd, false);
            Settings.FindReplaceDlgPosX = Settings.FindReplaceDlgPosY = CW_USEDEFAULT;
            break;

        case IDACC_CLEAR_FIND_HISTORY:
            MRU_Empty(Globals.pMRUfind, false, Globals.bCanSaveIniFile);
            if (s_pEfrData) {
                StrgEmpty(s_pEfrData->chFindPattern, false);
            }
            SetFindPattern(NULL);
            while ((int)SendDlgItemMessage(hwnd, IDC_FINDTEXT, CB_DELETESTRING, 0, 0) > 0) {};
            SetDlgItemText(hwnd, IDC_FINDTEXT, L"");
            break;

        case IDACC_CLEAR_REPL_HISTORY:
            MRU_Empty(Globals.pMRUreplace, false, Globals.bCanSaveIniFile);
            if (s_pEfrData) {
                StrgEmpty(s_pEfrData->chReplaceTemplate, false);
            }
            while ((int)SendDlgItemMessage(hwnd, IDC_REPLACETEXT, CB_DELETESTRING, 0, 0) > 0) {};
            SetDlgItemText(hwnd, IDC_REPLACETEXT, L"");
            break;

        case IDACC_SEARCHCLIPIFEMPTY:
            Settings.SearchByClipboardIfEmpty = !Settings.SearchByClipboardIfEmpty;
            CheckCmd(GetSystemMenu(hwnd, FALSE), IDS_MUI_SEARCHCLIPIFEMPTY, Settings.SearchByClipboardIfEmpty);
            break;

        case IDACC_REPLCLIPTAG:
            Settings.ReplaceByClipboardTag = !Settings.ReplaceByClipboardTag;
            CheckCmd(GetSystemMenu(hwnd, FALSE), IDS_MUI_REPLCLIPTAG, Settings.ReplaceByClipboardTag);
            break;

        case IDACC_FINDNEXT:
            PostWMCommand(hwnd, IDOK);
            break;

        case IDACC_FINDPREV:
            PostWMCommand(hwnd, IDC_FINDPREV);
            break;

        case IDACC_REPLACENEXT:
            if (s_bIsReplaceDlg) {
                PostWMCommand(hwnd, IDC_REPLACE);
            }
            break;

        case IDACC_SAVEFIND:
            Globals.FindReplaceMatchFoundState = FND_NOP;
            SendWMCommand(Globals.hwndMain, IDM_EDIT_SAVEFIND);
            ComboBox_SetTextHW(hwnd, IDC_FINDTEXT, s_pEfrData->chFindPattern);
            CheckDlgButton(hwnd, IDC_FINDREGEXP, BST_UNCHECKED);
            CheckDlgButton(hwnd, IDC_DOT_MATCH_ALL, BST_UNCHECKED);
            CheckDlgButton(hwnd, IDC_WILDCARDSEARCH, BST_UNCHECKED);
            CheckDlgButton(hwnd, IDC_FINDTRANSFORMBS, BST_UNCHECKED);
            PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)(GetDlgItem(hwnd, IDC_FINDTEXT)), 1);
            break;

        case IDACC_VIEWSCHEMECONFIG:
            PostWMCommand(GetParent(hwnd), IDM_VIEW_SCHEMECONFIG);
            break;

        default:
            return FALSE;
        }

    } // WM_COMMAND:
    return TRUE;


    case WM_SYSCOMMAND:
        if (wParam == IDS_MUI_SAVEPOS) {
            PostWMCommand(hwnd, IDACC_SAVEPOS);
            return TRUE;
        } else if (wParam == IDS_MUI_RESETPOS) {
            PostWMCommand(hwnd, IDACC_RESETPOS);
            return TRUE;
        } else if (wParam == IDS_MUI_CLEAR_FIND_HISTORY) {
            PostWMCommand(hwnd, IDACC_CLEAR_FIND_HISTORY);
            return TRUE;
        } else if (wParam == IDS_MUI_CLEAR_REPL_HISTORY) {
            PostWMCommand(hwnd, IDACC_CLEAR_REPL_HISTORY);
            return TRUE;
        } else if (wParam == IDS_MUI_SEARCHCLIPIFEMPTY) {
            PostWMCommand(hwnd, IDACC_SEARCHCLIPIFEMPTY);
            return TRUE;
        } else if (wParam == IDS_MUI_REPLCLIPTAG) {
            PostWMCommand(hwnd, IDACC_REPLCLIPTAG);
            return TRUE;
        }
        break;

    case WM_NOTIFY: {
        LPNMHDR pnmhdr = (LPNMHDR)lParam;
        switch (pnmhdr->code) {
        case NM_CLICK:
        case NM_RETURN:
            switch (pnmhdr->idFrom) {
            case IDC_TOGGLEFINDREPLACE:
                if (s_bIsReplaceDlg) {
                    PostWMCommand(GetParent(hwnd), IDM_EDIT_FIND);
                } else {
                    PostWMCommand(GetParent(hwnd), IDM_EDIT_REPLACE);
                }
                break;
            case IDC_FINDESCCTRLCHR:
                SendWMCommand(hwnd, IDC_FINDESCCTRLCHR);
                break;
            case IDC_REPLESCCTRLCHR:
                SendWMCommand(hwnd, IDC_REPLESCCTRLCHR);
                break;
            case IDC_BACKSLASHHELP:
                // Display help messages in the find/replace windows
                DisplayHelpDlgLng(hwnd, IDS_MUI_BACKSLASHHELP);
                break;
            case IDC_REGEXPHELP:
                DisplayHelpDlgLng(hwnd, IDS_MUI_REGEXPHELP);
                break;
            case IDC_WILDCARDHELP:
                DisplayHelpDlgLng(hwnd, IDS_MUI_WILDCARDHELP);
                break;
            default:
                break;
            }
            break;

        default:
            return false;
        }
    }
    break;

    default:
        break;

    } // switch(umsg)

    return FALSE; // message handled
}


//=============================================================================
//
//  EditFindReplaceDlg()
//
static void EditFindReplaceDlg(bool bReplace, const FindReplaceDlgRestore* pRestore)
{
    s_FindReplaceData.hwnd = Globals.hwndEdit;
    FindReplaceDlgInit const init = { &s_FindReplaceData, pRestore };
    HWND hDlg = CreateThemedDialogParam(Globals.hLngResContainer,
                                        (bReplace) ? MAKEINTRESOURCEW(IDD_MUI_REPLACE) : MAKEINTRESOURCEW(IDD_MUI_FIND),
                                        GetParent(s_FindReplaceData.hwnd),
                                        EditFindReplaceDlgProc,
                                        (LPARAM)&init);

    if (IS_VALID_HANDLE(hDlg)) {
        ShowWindow(hDlg, SW_SHOW);
        if (pRestore) {
            _EnableFRDlgCtrls(hDlg);
        }
        UpdateStatusbar(true);
    }
}


//=============================================================================
//
//  FindReplaceShowDialog()
//
void FindReplaceShowDialog(bool bReplace)
{
    FindReplacePrepare();
    HWND const hwndDlg = Globals.hwndDlgFindReplace;
    if (!IsWindow(hwndDlg)) {
        // Don't overwrite cmd-line search pattern with editor selection.
        Globals.bFindReplCopySelOrClip = !g_flagMatchText;
        EditFindReplaceDlg(bReplace, NULL);
    } else {
        Globals.bFindReplCopySelOrClip = (GetForegroundWindow() != hwndDlg);
        if ((GetDlgItem(hwndDlg, IDC_REPLACE) != NULL) != bReplace) {
            SendWMCommand(hwndDlg, bReplace ? IDMSG_SWITCHTOREPLACE : IDMSG_SWITCHTOFIND);
            DestroyWindow(Globals.hwndDlgFindReplace);
            EditFindReplaceDlg(bReplace, NULL);
        } else {
            SetForegroundWindow(hwndDlg);
            PostMessage(Globals.hwndDlgFindReplace, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(Globals.hwndDlgFindReplace, IDC_FINDTEXT), 1);
        }
    }
}

bool FindReplaceCaptureDialog(FindReplaceDlgRestore* pRestore)
{
    HWND const hwndDlg = Globals.hwndDlgFindReplace;
    if (!IsWindow(hwndDlg)) {
        return false;
    }
    pRestore->bReplace = GetDlgItem(hwndDlg, IDC_REPLACE) != NULL;
    GetWindowRect(hwndDlg, &pRestore->windowRect);
    pRestore->bFocusedView = FocusedView.HideNonMatchedLines;
    return true;
}

void FindReplaceRestoreDialog(const FindReplaceDlgRestore* pRestore)
{
    if (!IsWindow(Globals.hwndDlgFindReplace)) {
        Globals.bFindReplCopySelOrClip = false;
        EditFindReplaceDlg(pRestore->bReplace, pRestore);
    }
}


//=============================================================================
//
//  EditFindNext()
//
bool EditFindNext(HWND hwnd, const LPEDITFINDREPLACE lpefr, bool bExtendSelection, bool bFocusWnd, bool bFillEmpty)
{
    bool bSuppressNotFound = false;
    bool bFoundWrapAround = false;

    if (bFocusWnd) {
        SetFocus(hwnd);
    }

    LPCWSTR wchFind = _EditGetFindStrg(hwnd, lpefr, bFillEmpty);

    if (StrIsEmpty(wchFind)) {
        return false;
    }
    int const sFlags = (int)(lpefr->fuFlags);
    bool const bIsRegExpr = (sFlags & SCFIND_REGEXP);

    DocPos const iDocEndPos = Sci_GetDocEndPosition();

    EditSetCaretToSelectionEnd(); // fluent switch between Prev/Next
    DocPos const curPos = SciCall_GetCurrentPos();
    DocPos       start = (curPos < iDocEndPos) ? curPos : 0;

    DocPos end = iDocEndPos;

    Sci_CallTipCancelEx();

    DocPos iPos = _FindInTarget(wchFind, sFlags, &start, &end, true, FRMOD_NORM);

    if (bIsRegExpr && Sci_ConsumeRegexWarnStatus()) {
        InfoBoxLng(MB_ICONWARNING, Constants.SuppressKey.MsgInvalidRegex, IDS_MUI_REGEX_INVALID);
        bSuppressNotFound = true;
    } else if ((iPos < 0LL) && (start >= 0LL) && !bExtendSelection) {
        UpdateStatusbar(false);
        if (!lpefr->bNoFindWrap && !bSuppressNotFound) {
            Globals.iSelectionMarkNumber = 0;
            DocPos const _start = start;
            //DocPos const _end = end;
            end = min_p(start, iDocEndPos);
            start = 0LL;
            iPos = _FindInTarget(wchFind, sFlags, &start, &end, false, FRMOD_WRAPED);

            if ((iPos < 0LL) || (end == _start)) {
                if (bIsRegExpr && Sci_ConsumeRegexWarnStatus()) {
                    InfoBoxLng(MB_ICONWARNING, Constants.SuppressKey.MsgInvalidRegex, IDS_MUI_REGEX_INVALID);
                    bSuppressNotFound = true;
                }
            } else {
                LONG const result = InfoBoxLng(MB_OKCANCEL, Constants.SuppressKey.MsgFindWrap1, IDS_MUI_FIND_WRAPFW);
                if (!IsYesOkay(result)) {
                    iPos = NOT_FOUND;
                    bSuppressNotFound = true;
                }
                bFoundWrapAround = (INFOBOX_MODE(result) != 0);
            }
        }
    }

    if (iPos < 0LL) {
        if (!bSuppressNotFound) {
            InfoBoxLng(MB_OK, Constants.SuppressKey.MsgNotFound, IDS_MUI_NOTFOUND);
        }
        return false;
    }

    if (bExtendSelection) {
        DocPos const iSelPos = SciCall_GetCurrentPos();
        DocPos const iSelAnchor = SciCall_GetAnchor();
        EditSetSelectionEx(min_p(iSelAnchor, iSelPos), end, -1, -1);
    } else {
        EditSetSelectionEx(iPos, end, -1, -1);
    }

    if (!Globals.iSelectionMarkNumber && lpefr->bMarkOccurences) {
        _FindHasMatch(hwnd, lpefr, 0, true);
    }
    else {
        ++Globals.iSelectionMarkNumber;
    }

    if (iPos == end) {
        _ShowZeroLengthCallTip(iPos);
    }
    if ((iPos+1) == end) {
        char const p = SciCall_GetCharAt(iPos);
        char const e = SciCall_GetCharAt(end);
        if (p == 0x0d && e == 0x0a) {
            _ShowZeroLengthCallTip(iPos);
        }
    }
    if (bFoundWrapAround) {
        ShowWrapAroundCallTip(true);
    }

    return true;
}


//=============================================================================
//
//  EditFindPrev()
//
bool EditFindPrev(HWND hwnd, LPEDITFINDREPLACE lpefr, bool bExtendSelection, bool bFocusWnd, bool bFillEmpty)
{
    bool bSuppressNotFound = false;
    bool bFoundWrapAround = false;

    if (bFocusWnd) {
        SetFocus(hwnd);
    }

    LPCWSTR wchFind = _EditGetFindStrg(hwnd, lpefr, bFillEmpty);

    if (StrIsEmpty(wchFind)) {
        return false;
    }
    int const sFlags = (int)(lpefr->fuFlags);
    bool const bIsRegExpr = (sFlags & SCFIND_REGEXP);

    DocPos const iDocEndPos = Sci_GetDocEndPosition();

    EditSetCaretToSelectionStart(); // fluent switch between Next/Prev
    DocPos const curPos = SciCall_GetCurrentPos();
    DocPos       start = (curPos > 0) ? (SciCall_IsSelectionEmpty() ? curPos : SciCall_PositionBefore(curPos)) : SciCall_PositionBefore(iDocEndPos);
    DocPos       end = 0LL;

    Sci_CallTipCancelEx();

    DocPos iPos = _FindInTarget(wchFind, sFlags, &start, &end, true, FRMOD_NORM);

    if (bIsRegExpr && Sci_ConsumeRegexWarnStatus()) {
        InfoBoxLng(MB_ICONWARNING, Constants.SuppressKey.MsgInvalidRegex, IDS_MUI_REGEX_INVALID);
        bSuppressNotFound = true;
    } else if ((iPos < 0LL) && (start <= iDocEndPos) &&  !bExtendSelection) {
        UpdateStatusbar(false);
        if (!lpefr->bNoFindWrap && !bSuppressNotFound) {
            Globals.iSelectionMarkNumber = Globals.iMarkOccurrencesCount + 1;
            DocPos const _start = start;
            //DocPos const _end = end;
            end = max_p(start, 0LL);
            start = iDocEndPos;
            iPos = _FindInTarget(wchFind, sFlags, &start, &end, false, FRMOD_WRAPED);

            if ((iPos < 0LL) || (start == _start)) {
                if (bIsRegExpr && Sci_ConsumeRegexWarnStatus()) {
                    InfoBoxLng(MB_ICONWARNING, Constants.SuppressKey.MsgInvalidRegex, IDS_MUI_REGEX_INVALID);
                    bSuppressNotFound = true;
                }
            } else {
                LONG const result = InfoBoxLng(MB_OKCANCEL, Constants.SuppressKey.MsgFindWrap2, IDS_MUI_FIND_WRAPRE);
                if (!IsYesOkay(result)) {
                    iPos = NOT_FOUND;
                    bSuppressNotFound = true;
                }
                bFoundWrapAround = (INFOBOX_MODE(result) != 0);
            }
        }
    }

    if (iPos < 0LL) {
        if (!bSuppressNotFound) {
            InfoBoxLng(MB_OK, Constants.SuppressKey.MsgNotFound, IDS_MUI_NOTFOUND);
        }
        return false;
    }

    if (bExtendSelection) {
        DocPos const iSelPos = SciCall_GetCurrentPos();
        DocPos const iSelAnchor = SciCall_GetAnchor();
        EditSetSelectionEx(max_p(iSelPos, iSelAnchor), iPos, -1, -1);
    } else {
        EditSetSelectionEx(end, iPos, -1, -1);
    }

    if (!Globals.iSelectionMarkNumber && lpefr->bMarkOccurences) {
        _FindHasMatch(hwnd, lpefr, 0, true);
    }
    else {
        --Globals.iSelectionMarkNumber;
    }

    if (iPos == end) {
        _ShowZeroLengthCallTip(iPos);
    }
    if ((iPos + 1) == end) {
        char const p = SciCall_GetCharAt(iPos);
        char const e = SciCall_GetCharAt(end);
        if (p == 0x0d && e == 0x0a) {
            _ShowZeroLengthCallTip(iPos);
        }
    }
    if (bFoundWrapAround) {
        ShowWrapAroundCallTip(false);
    }

    return true;
}


static int GetMarkAllOccSearchFlags(void)
{
    return (Settings.MarkOccurrencesMatchCase ? SCFIND_MATCHCASE : 0) |
           ((Settings.MarkOccurrencesCurrentWord || Settings.MarkOccurrencesMatchWholeWords) ? SCFIND_WHOLEWORD : 0);
}

//=============================================================================
//
//  EditMarkAllOccurrences()
//
void EditMarkAllOccurrences(HWND hwnd, bool bForceClear)
{
    if (bForceClear) {
        EditClearAllOccurrenceMarkers(hwnd);
    }

    if (!IsMarkOccurrencesEnabled()) {
        return;
    }

    int const searchFlags = GetMarkAllOccSearchFlags();

    if (Settings.MarkOccurrencesMatchVisible) {

        // get visible lines for update
        DocLn const iStartLine = SciCall_DocLineFromVisible(SciCall_GetFirstVisibleLine());
        DocLn const iEndLine = min_ln((iStartLine + SciCall_LinesOnScreen()), Sci_GetLastDocLineNumber());
        DocPos const iPosStart = SciCall_PositionFromLine(iStartLine);
        DocPos const iPosEnd = SciCall_GetLineEndPosition(iEndLine);

        // !!! don't clear all marks, else this method is re-called
        // !!! on UpdateUI notification on drawing indicator mark
        EditMarkAll(NULL, searchFlags, iPosStart, iPosEnd, false);
    } else {
        EditMarkAll(NULL, searchFlags, 0, Sci_GetDocEndPosition(), false);
    }
}


//=============================================================================
//
//  EditSelectionMultiSelectAll()
//
void EditSelectionMultiSelectAll(void)
{
    if (SciCall_GetSelText(NULL) > 0) {
        SciCall_TargetWholeDocument();
        SciCall_SetSearchFlags(GetMarkAllOccSearchFlags());
        SciCall_MultipleSelectAddEach();
        SciCall_SetMainSelection(0);
        if (SciCall_GetSelectionNAnchor(0) > SciCall_GetSelectionNCaret(0)) {
            SciCall_SwapMainAnchorCaret();
        }
        Sci_ScrollSelectionToView();
    }
}


//=============================================================================
//
//  EditSelectionMultiSelectAllEx()
//
void EditSelectionMultiSelectAllEx(HWND hwnd, CLPCEDITFINDREPLACE edFndRpl)
{
    EDITFINDREPLACE efr = INIT_EFR_DATA;
    DuplicateEFR(&efr, edFndRpl);

    if (IsWindow(Globals.hwndDlgFindReplace)) {
        _SetSearchFlags(Globals.hwndDlgFindReplace, &efr);
    } else {
        efr.fuFlags = GetMarkAllOccSearchFlags();
    }

    LPCWSTR wchFind = _EditGetFindStrg(hwnd, &efr, false);

    if (StrIsNotEmpty(wchFind)) {
        UndoTransActionBegin();
        EditMarkAll(wchFind, efr.fuFlags, 0, Sci_GetDocEndPosition(), true);
        EndUndoTransAction();
    }
    ReleaseEFR(&efr);
}


//=============================================================================
//
//  EditReplace()
//
bool EditReplace(HWND hwnd, LPEDITFINDREPLACE lpefr)
{
    int iReplaceMsg = SciCall_GetChangeHistory() ? SCI_REPLACETARGETMINIMAL : SCI_REPLACETARGET;
    DocPos const selBeg = SciCall_GetSelectionStart();
    DocPos const selEnd = SciCall_GetSelectionEnd();

    // redo find to get group ranges filled
    DocPos start = (SciCall_IsSelectionEmpty() ? SciCall_GetCurrentPos() : selBeg);
    DocPos end = Sci_GetDocEndPosition();
    DocPos _start = start;
    Globals.iReplacedOccurrences = 0;

    LPCWSTR wchFind = _EditGetFindStrg(hwnd, lpefr, false);

    if (StrIsEmpty(wchFind)) {
        return false;
    }

    char* const pszReplace = _GetReplaceString(hwnd, lpefr, &iReplaceMsg);
    if (!pszReplace) {
        return false; // recoding of clipboard canceled
    }

    int const    sFlags = (int)(lpefr->fuFlags);
    DocPos const iPos = _FindInTarget(wchFind, sFlags, &start, &end, false, FRMOD_NORM);

    // w/o selection, replacement string is put into current position
    // but this maybe not intended here
    if (SciCall_IsSelectionEmpty()) {
        if ((iPos < 0LL) || (_start != start) || (_start != end)) {
            // empty-replace was not intended
            FreeMem(pszReplace);
            if (iPos < 0LL) {
                return EditFindNext(hwnd, lpefr, false, false, false);
            }
            EditSetSelectionEx(start, end, -1, -1);
            return true;
        }
    }
    // Only replace a selection when the search matched it.
    else if ((iPos < 0LL) || (selBeg != start) || (selEnd != end)) {
        FreeMem(pszReplace);
        SciCall_SetCurrentPos(selBeg);
        return EditFindNext(hwnd, lpefr, false, false, false);
    }

    UndoTransActionBegin();

    SciCall_TargetFromSelection();
    DocPos const posTargetStart = SciCall_GetTargetStart();
    Sci_ReplaceTargetEx(iReplaceMsg, -1, pszReplace);
    // move caret behind replacement
    if (SCI_REPLACETARGETMINIMAL == iReplaceMsg) {
        SciCall_SetCurrentPos(posTargetStart + strlen(pszReplace));
    }
    else {
        SciCall_SetCurrentPos(SciCall_GetTargetEnd());
    }

    Globals.iReplacedOccurrences = 1;

    EndUndoTransAction();

    FreeMem(pszReplace);

    return EditFindNext(hwnd, lpefr, false, false, false);
}



//=============================================================================
//
//  EditReplaceAllInRange()
//
//
static DocPosU EditReplaceAllInRange(HWND hwnd, LPEDITFINDREPLACE lpefr, DocPos iStartPos, DocPos iEndPos, DocPos *enlargement)
{
    if (iStartPos > iEndPos) {
        swapos(&iStartPos, &iEndPos);
    }
    DocPos const iOrigEndPos = iEndPos; // remember

    LPCWSTR wchFind = _EditGetFindStrg(hwnd, lpefr, false);

    if (StrIsEmpty(wchFind)) {
        return 0;
    }

    int const sFlags = (int)(lpefr->fuFlags);
    bool const bIsRegExpr = (sFlags & SCFIND_REGEXP);
    bool const bRegexStartOfLine = bIsRegExpr && (wchFind[0] == L'^');

    // SCI_REPLACETARGET or SCI_REPLACETARGETRE
    int iReplaceMsg = SCI_REPLACETARGET;
    char* const pszReplace = _GetReplaceString(hwnd, lpefr, &iReplaceMsg);
    if (!pszReplace) {
        return 0; // recoding of clipboard canceled
    }

    DocPos const _saveTargetBeg_ = SciCall_GetTargetStart();
    DocPos const _saveTargetEnd_ = SciCall_GetTargetEnd();

    DocPos start = iStartPos;
    DocPos end   = iEndPos;
    DocPos iPos = _FindInTarget(wchFind, sFlags, &start, &end, false, FRMOD_NORM);

    if (bIsRegExpr && Sci_ConsumeRegexWarnStatus()) {
        FreeMem(pszReplace);
        InfoBoxLng(MB_ICONWARNING, Constants.SuppressKey.MsgInvalidRegex, IDS_MUI_REGEX_INVALID);
        return 0;
    }

    DocPosU iCount = 0;
    LimitNotifyEvents();
    UndoTransActionBegin();

    while ((iPos >= 0LL) && (start <= iEndPos)) {
        DocLn const lnCnt = bRegexStartOfLine ? SciCall_GetLineCount() : 0;
        SciCall_SetTargetRange(iPos, end);
        DocPos const replLen = Sci_ReplaceTargetEx(iReplaceMsg, -1, pszReplace);
        ++iCount;
        // start next search behind replacement
        if (SCI_REPLACETARGETMINIMAL == iReplaceMsg) {
            DocPos const replLength = strlen(pszReplace);
            iStartPos = iPos + replLength;
            iEndPos += replLength - (end - iPos);
        }
        else {
            iStartPos = SciCall_GetTargetEnd();
            if (bRegexStartOfLine && (SciCall_GetLineCount() == lnCnt) && (Sci_GetLineStartPosition(iStartPos) == iStartPos)) {
                iStartPos = SciCall_PositionAfter(iStartPos);
            }
            iEndPos += replLen - (end - iPos);
        }
        start = iStartPos;
        end   = iEndPos;
        iPos = (start <= end) ? _FindInTarget(wchFind, sFlags, &start, &end, true, FRMOD_NORM) : NOT_FOUND;
    }

    EndUndoTransAction();

    FreeMem(pszReplace);

    *enlargement = (iEndPos - iOrigEndPos);
    SciCall_SetTargetRange(_saveTargetBeg_, _saveTargetEnd_ + *enlargement); //restore

    RestoreNotifyEvents();

    return iCount;
}


//=============================================================================
//
//  EditReplaceAll()
//
bool EditReplaceAll(HWND hwnd, LPEDITFINDREPLACE lpefr, bool bShowInfo)
{
    DocPos const start = 0;
    DocPos const end = Sci_GetDocEndPosition();
    DocPos enlargement = 0;

    BeginWaitCursorUID(true, IDS_MUI_SB_REPLACE_ALL);
    Globals.iReplacedOccurrences = EditReplaceAllInRange(hwnd, lpefr, start, end, &enlargement);
    EndWaitCursor();

    WCHAR wchOcc[64] = { L'\0' };
    StringCchPrintf(wchOcc, COUNTOF(wchOcc), DOCPOSFMTW, Globals.iReplacedOccurrences);
    FormatNumberStr(wchOcc, COUNTOF(wchOcc), 0);

    if (bShowInfo) {
        if (Globals.iReplacedOccurrences) {
            InfoBoxLng(MB_OK, Constants.SuppressKey.MsgReplaceCount, IDS_MUI_REPLCOUNT, wchOcc);
        } else {
            InfoBoxLng(MB_OK, Constants.SuppressKey.MsgNotFound, IDS_MUI_NOTFOUND);
        }
    }

    return (bool)Globals.iReplacedOccurrences;
}


//=============================================================================
//
//  EditReplaceAllInSelection()
//
bool EditReplaceAllInSelection(HWND hwnd, LPEDITFINDREPLACE lpefr, bool bShowInfo)
{
    if (Sci_IsMultiOrRectangleSelection()) {
        InfoBoxLng(MB_ICONWARNING, NULL, IDS_MUI_SELRECTORMULTI);
        return false;
    }

    const DocPos start = SciCall_GetSelectionStart();
    const DocPos end = SciCall_GetSelectionEnd();
    const DocPos currPos = SciCall_GetCurrentPos();
    const DocPos anchorPos = SciCall_GetAnchor();
    DocPos enlargement = 0;

    BeginWaitCursorUID(true, IDS_MUI_SB_REPLACE_ALL);
    Globals.iReplacedOccurrences = EditReplaceAllInRange(hwnd, lpefr, start, end, &enlargement);
    EndWaitCursor();

    if (Globals.iReplacedOccurrences) {
        if (currPos < anchorPos) {
            SciCall_SetSel(anchorPos + enlargement, currPos);
        } else {
            SciCall_SetSel(anchorPos, currPos + enlargement);
        }
        if (bShowInfo) {
            WCHAR wchOcc[64] = { L'\0' };
            StringCchPrintf(wchOcc, COUNTOF(wchOcc), DOCPOSFMTW, Globals.iReplacedOccurrences);
            FormatNumberStr(wchOcc, COUNTOF(wchOcc), 0);
            InfoBoxLng(MB_OK, Constants.SuppressKey.MsgReplaceCount, IDS_MUI_REPLCOUNT, wchOcc);
        }
    }
    else if (bShowInfo) {
        InfoBoxLng(MB_OK, Constants.SuppressKey.MsgNotFound, IDS_MUI_NOTFOUND);
    }

    return (bool)Globals.iReplacedOccurrences;
}



//=============================================================================
//
//  EditClearAllOccurrenceMarkers()
//
void EditClearAllOccurrenceMarkers(HWND hwnd)
{
    UNREFERENCED_PARAMETER(hwnd);

    LimitNotifyEvents();

    SciCall_SetIndicatorCurrent(INDIC_NP3_MARK_OCCURANCE);
    SciCall_IndicatorClearRange(0, Sci_GetDocEndPosition());

    SciCall_MarkerDeleteAll(MARKER_NP3_OCCURRENCE);

    Globals.iSelectionMarkNumber = Globals.iMarkOccurrencesCount = 0;

    RestoreNotifyEvents();
}


//=============================================================================
//
//  EditMarkAll()
//  Mark all occurrences of the matching text in range (by Aleksandar Lekov)
//
static void EditMarkAll(LPCWSTR wchFind, int sFlags, DocPos rangeStart, DocPos rangeEnd, bool bMultiSel)
{
    char  chText[FNDRPL_BUFFER] = { L'\0' };
    WCHAR wchText[FNDRPL_BUFFER] = { L'\0' };

    DocPos       iFindLength = 0;

    LimitNotifyEvents();

    if (StrIsEmpty(wchFind)) {
        if (SciCall_IsSelectionEmpty()) {
            // nothing selected, get word under caret if flagged
            if (Settings.MarkOccurrencesCurrentWord && (sFlags & SCFIND_WHOLEWORD)) {
                DocPos const iCurPos = SciCall_GetCurrentPos();
                DocPos iWordStart = SciCall_WordStartPosition(iCurPos, true);
                DocPos iWordEnd = SciCall_WordEndPosition(iCurPos, true);
                if (iWordStart == iWordEnd) {
                    goto observe;
                }
                iFindLength = (iWordEnd - iWordStart);

                StringCchCopyNA(chText, COUNTOF(chText), SciCall_GetRangePointer(iWordStart, iFindLength), iFindLength);

            } else {
                goto observe; // no pattern, no selection and no word mark chosen
            }
        } else { // we have a selection

            if (Sci_IsMultiSelection()) {
                goto observe;
            }

            // get current selection
            DocPos const iSelStart = SciCall_GetSelectionStart();
            DocPos const iSelEnd = SciCall_GetSelectionEnd();
            //DocPos const iSelCount = (iSelEnd - iSelStart);

            // if multiple lines are selected exit
            if (SciCall_LineFromPosition(iSelStart) != SciCall_LineFromPosition(iSelEnd)) {
                goto observe;
            }

            DocPosU const iSelLen = SciCall_GetSelText(NULL);
            if (iSelLen >= COUNTOF(chText)) {
                goto observe;
            }

            iFindLength = SciCall_GetSelText(chText);
            chText[iFindLength] = '\0';

            // exit if selection is not a word and Match whole words only is enabled
            if ((sFlags & SCFIND_WHOLEWORD) && EditContainsWordDelimiter(chText, iSelLen + 1)) {
                goto observe;
            }
        }

        MultiByteToWideChar(Encoding_SciCP, 0, chText, (int)iFindLength, wchText, (int)COUNTOF(wchText));
        wchFind = wchText;
    }

    if (StrIsNotEmpty(wchFind)) {

        if (bMultiSel) {
            SciCall_ClearSelections();
        }
        DocPos const iSelStart = SciCall_GetSelectionStart();
        DocPos const iSelEnd = SciCall_GetSelectionEnd();

        DocPos const iTextEnd = Sci_GetDocEndPosition();
        rangeStart = max_p(0, rangeStart);
        rangeEnd = min_p(rangeEnd, iTextEnd);

        DocPos start = rangeStart;
        DocPos end = rangeEnd;
        DocPos iPos = _FindInTarget(wchFind, sFlags, &start, &end, false, FRMOD_NORM);

        if (!bMultiSel) {
            SciCall_SetIndicatorCurrent(INDIC_NP3_MARK_OCCURANCE); // once: doc-change notifications are disabled (LimitNotifyEvents)
        }

        DocPosU count = 0;
        DocPosU found = 0;
        while ((iPos >= 0LL) && (start <= rangeEnd)) {

            if (bMultiSel) {
                if (count) {
                    SciCall_AddSelection(end, iPos);
                } else {
                    SciCall_SetSelection(end, iPos);
                }
            } else {
                // mark this match if not done before
                SciCall_IndicatorFillRange(iPos, (end - start));
                SciCall_MarkerAdd(SciCall_LineFromPosition(iPos), MARKER_NP3_OCCURRENCE);
            }
            ++count;
            if (!found && ((iSelStart <= end) && (start <= iSelEnd))) {
                // intersect = (max_p(iSelStart, start) < min_p(iSelEnd, end))
                found = count;
            }
            start = end;
            end = rangeEnd;
            iPos = _FindInTarget(wchFind, sFlags, &start, &end, true, FRMOD_NORM);
        };
        Globals.iSelectionMarkNumber = found;
        Globals.iMarkOccurrencesCount = count;
        assert(!bMultiSel || (SciCall_GetSelections() == count));
    }

observe:

    RestoreNotifyEvents();
}


// Decode \0oo without treating a lone \0 as an escape instead of a backreference.
static void _UnSlashLowOctalA(LPSTR s)
{
    LPSTR o = s;
    while (*s) {
        if ((s[0] == '\\') && (s[1] == '\\')) {
            *o++ = *s++;
            *o = *s;
        } else if ((s[0] == '\\') && (s[1] == '0') && IsOctalDigitA(s[2]) && IsOctalDigitA(s[3])) {
            *o = (CHAR)(8 * (s[2] - '0') + (s[3] - '0'));
            s += 3;
        } else {
            *o = *s;
        }
        ++o;
        if (*s) {
            ++s;
        }
    }
    *o = '\0';
}

static int _CheckRegExReplTargetA(LPCSTR pszInput)
{
    while (*pszInput) {
        if (*pszInput == '$') {
            ++pszInput;
            if (((*pszInput >= '0') && (*pszInput <= '9')) || (*pszInput == '+') || (*pszInput == '{')) {
                return SCI_REPLACETARGETRE;
            }
        } else if (*pszInput == '\\') {
            ++pszInput;
            if ((*pszInput >= '0') && (*pszInput <= '9')) {
                return SCI_REPLACETARGETRE;
            }
        } else {
            ++pszInput;
        }
    }
    return SciCall_GetChangeHistory() ? SCI_REPLACETARGETMINIMAL : SCI_REPLACETARGET;
}

static void _TransformReplaceBackslashes(LPSTR pszInput, bool bRegEx, int* iReplaceMsg)
{
    if (bRegEx) {
        _UnSlashLowOctalA(pszInput);
        *iReplaceMsg = _CheckRegExReplTargetA(pszInput);
    } else {
        *iReplaceMsg = SciCall_GetChangeHistory() ? SCI_REPLACETARGETMINIMAL : SCI_REPLACETARGET;
    }
    if (*iReplaceMsg != SCI_REPLACETARGETRE) {
        UnSlashA(pszInput, CP_UTF8);
    }
}
