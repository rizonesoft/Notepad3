//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by minipath.rc
//
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#ifndef WINVER
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif

#define IDR_RT_MANIFEST                   1

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
///#define MUI_BASE_LNG_FI_FI 1
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
//    Extent Minipath.rc file accordingly
//    and the language_id mapping in ../language/common_res.h
// -----------------------------------------------------

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
