//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by Notepad3.rc
//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#ifndef WINVER
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif

// ==========================================
// How to build a NON MUI Notepad3 version for desired Language 
// -----------------------------------------------------
// 1. Comment-Out the following line if Language Menu is not wanted
#define HAVE_DYN_LOAD_LIBS_MUI_LNGS 1
// -----------------------------------------------------
// 2. Uncomment one of the following lines according to
//    the desired Language (use en-US if undefined)
//    Hint: better use VS-Solution's 
//          C/C++ _and_ Resources (compiler) Preprocessor Define
///#define MUI_BASE_LNG_EN_US 1
///#define MUI_BASE_LNG_AF_ZA 1
///#define MUI_BASE_LNG_BE_BY 1
///#define MUI_BASE_LNG_DE_DE 1
///#define MUI_BASE_LNG_EL_GR 1
///#define MUI_BASE_LNG_EN_GB 1
///#define MUI_BASE_LNG_ES_ES 1
///#define MUI_BASE_LNG_FR_FR 1
///#define MUI_BASE_LNG_HI_IN 1
///#define MUI_BASE_LNG_HU_HU 1
///#define MUI_BASE_LNG_ID_ID 1
///#define MUI_BASE_LNG_IT_IT 1
///#define MUI_BASE_LNG_JA_JP 1
///#define MUI_BASE_LNG_KO_KR 1
///#define MUI_BASE_LNG_NL_NL 1
///#define MUI_BASE_LNG_PL_PL 1
///#define MUI_BASE_LNG_PT_BR 1
///#define MUI_BASE_LNG_PT_PT 1
///#define MUI_BASE_LNG_RU_RU 1
///#define MUI_BASE_LNG_SK_SK 1
///#define MUI_BASE_LNG_SV_SE 1
///#define MUI_BASE_LNG_TR_TR 1
///#define MUI_BASE_LNG_VI_VN 1
///#define MUI_BASE_LNG_ZH_CN 1
///#define MUI_BASE_LNG_ZH_TW 1

// -----------------------------------------------------
// 3. In case of a new language:
//    Extent Notepad3.rc file accordingly
//    and the language_id mapping in ../language/common_res.h
// -----------------------------------------------------

#ifndef IDC_STATIC
#define IDC_STATIC                     (-1)
#endif

//#define IDR_RT_MANIFEST                 1

#define IDR_POPUPMENU                   115
#define IDR_ACCFINDREPLACE              126
#define IDC_MODWEBPAGE2                 163
#define IDC_MOD_PAGE2                   165
#define IDC_AUTHORNAME                  168
#define IDC_WEBPAGE4                    169
#define IDC_MODWEBPAGE                  170
#define IDC_EMAIL                       174
#define IDC_NOTE2WEBPAGE                175
#define IDC_EMAIL2                      178
#define IDC_NOTE2WEBPAGE2               179
#define IDC_EMAIL3                      184
#define IDC_MOD_PAGE                    190
#define IDC_WEBPAGE3                    194

#define IDT_TIMER_MRKALL                215
#define IDT_TIMER_CALLBACK_MRKALL       218
#define IDC_CHECK_OCC                   228
#define IDR_ACCCUSTOMSCHEMES            231
#define IDC_DOC_MODIFIED                235
#define IDT_TIMER_UPDATE_STATUSBAR      236
#define IDT_TIMER_UPDATE_TOOLBAR        237
#define IDT_TIMER_CLEAR_CALLTIP         238
#define IDT_TIMER_UNDO_TRANSACTION      239
#define IDT_TIMER_UPDATE_TITLEBAR       240

#define IDACC_FIND                      302
#define IDACC_REPLACE                   303
#define IDACC_SAVEPOS                   304
#define IDACC_RESETPOS                  305
#define IDACC_FINDNEXT                  306
#define IDACC_FINDPREV                  307
#define IDACC_REPLACENEXT               308
#define IDACC_SAVEFIND                  309
#define IDACC_SELTONEXT                 310
#define IDACC_SELTOPREV                 311
#define IDACC_VIEWSCHEMECONFIG          312
#define IDACC_PREVIEW                   313
#define IDACC_CLEAR_FIND_HISTORY        314
#define IDACC_CLEAR_REPL_HISTORY        315
#define IDACC_SEARCHCLIPIFEMPTY         316
#define IDACC_REPLCLIPTAG               317

#define IDC_SETCURLEXERTV               402

// ------------------------------------------
#define ___SCI_CMD__SCEN_KILLFOCUS___   256
#define ___SCI_CMD__SCEN_SETFOCUS___    512
#define ___SCI_CMD__SCEN_CHANGE___      768
// ------------------------------------------

#define IDS_WARN_PREF_LNG_NOT_AVAIL     4000

// ==========================================

#include "../language/common_res.h"

// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NO_MFC                     1
#define _APS_NEXT_RESOURCE_VALUE        0
#define _APS_NEXT_COMMAND_VALUE         0
#define _APS_NEXT_CONTROL_VALUE         0
#define _APS_NEXT_SYMED_VALUE           0
#endif
#endif
