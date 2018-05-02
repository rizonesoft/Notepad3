/******************************************************************************
*                                                                             *
*                                                                             *
* Notepad3                                                                    *
*                                                                             *
* TypeDefs.h                                                                  *
*                                                                             *
*                                                  (c) Rizonesoft 2008-2018   *
*                                                    https://rizonesoft.com   *
*                                                                             *
*                                                                             *
*******************************************************************************/
#pragma once
#ifndef _NP3_TYPEDEFS_H_
#define _NP3_TYPEDEFS_H_

#include <stdbool.h>
#include "Sci_Position.h"

//~#define NP3_COMPILE_TEST 1

//
// SC_DOCUMENTOPTION_TEXT_LARGE
//
#if defined(SCI_LARGE_FILE_SUPPORT)
  typedef Sci_Position   DocPos;
  typedef Sci_PositionU  DocPosU;
  typedef Sci_PositionCR DocCR;
  typedef Sci_Line       DocLn;
#else

  #ifdef NP3_COMPILE_TEST
    typedef ptrdiff_t DocPos;
    typedef size_t DocPosU;
    typedef long DocPosCR;
    typedef ptrdiff_t DocLn;
  #else
    typedef int  DocPos;
    typedef unsigned int DocPosU;
    typedef long DocPosCR;
    typedef int  DocLn;
  #endif

#endif

// --------------------------------------------------------------------------
    
typedef struct _wi
{
  int x;
  int y;
  int cx;
  int cy;
  int max;
} WININFO;

// --------------------------------------------------------------------------

typedef enum BufferSizes
{
  MICRO_BUFFER = 32,
  MINI_BUFFER = 64,
  SMALL_BUFFER = 128,
  MIDSZ_BUFFER = 256,
  LARGE_BUFFER = 512,
  HUGE_BUFFER = 1024,
  XHUGE_BUFFER = 2048,
  XXXL_BUFFER = 4096,

  FILE_ARG_BUF = MAX_PATH + 2,
  FNDRPL_BUFFER = 1024,
  TEMPLINE_BUFFER = 4096

} BUFFER_SIZES;


typedef enum { FND_NOP = 0, NXT_NOT_FND, NXT_FND, NXT_WRP_FND, PRV_NOT_FND, PRV_FND, PRV_WRP_FND } FR_STATES;
typedef enum { FRMOD_IGNORE = 0, FRMOD_NORM, FRMOD_WRAPED } FR_UPD_MODES;
typedef enum { MBINFO = 0, MBWARN, MBYESNO, MBYESNOWARN, MBYESNOCANCEL, MBOKCANCEL, MBRETRYCANCEL } MBTYPES;

//==== Statusbar ==============================================================

typedef WCHAR prefix_t[MICRO_BUFFER];

typedef enum {
  STATUS_DOCLINE = 0, STATUS_DOCCOLUMN, STATUS_SELECTION, STATUS_SELCTBYTES, STATUS_SELCTLINES, STATUS_OCCURRENCE,
  STATUS_DOCSIZE, STATUS_CODEPAGE, STATUS_EOLMODE, STATUS_OVRMODE, STATUS_2ND_DEF, STATUS_LEXER,
  STATUS_SECTOR_COUNT,
  STATUS_HELP = 255
} STATUS_SECTOR_T;

#define SBS_INIT_ZERO  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } 
#define SBS_INIT_MINUS { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } 
#define SBS_INIT_ORDER { 0, 1, 2, 3, 4, 5, 6, 7. 8. 9, 10, 11 }

#define STATUSBAR_EXTION_PREFIXES L"Ln  ,Col  ,Sel  ,Sb  ,SLn  ,Occ  ,,,,,,,"
#define STATUSBAR_DEFAULT_IDS  L"0 1 2 4 5 6 7 8 9 10 11"
#define STATUSBAR_SECTION_WIDTH_SPECS L"30 20 20 20 20 20 0 0 0 0 0 0"
#define STAUSBAR_RIGHT_MARGIN 20

// --------------------------------------------------------------------------

typedef struct _editfindreplace
{
  char szFind[FNDRPL_BUFFER];
  char szReplace[FNDRPL_BUFFER];
  UINT fuFlags;
  bool bTransformBS;
  bool bFindClose;
  bool bReplaceClose;
  bool bNoFindWrap;
  bool bWildcardSearch;
  bool bMarkOccurences;
  bool bHideNonMatchedLines;
  bool bDotMatchAll;
  bool bStateChanged;
  HWND hwnd;

} EDITFINDREPLACE, *LPEDITFINDREPLACE, *LPCEDITFINDREPLACE;

#define EFR_INIT_DATA  { "", "", 0, false, false, false, false, false, false, false, false, true, NULL }
#define IDMSG_SWITCHTOFIND    300
#define IDMSG_SWITCHTOREPLACE 301

// --------------------------------------------------------------------------

typedef struct _cmq
{
  HWND hwnd;
  UINT  cmd;
  WPARAM wparam;
  LPARAM lparam;
  int delay;
  struct _cmq* next;
  struct _cmq* prev;

} CmdMessageQueue_t;

#define MESSAGE_QUEUE_INIT = { NULL, WM_COMMAND, NULL, NULL, -1 };

// --------------------------------------------------------------------------


// --------------------------------------------------------------------------

#define MARKER_NP3_BOOKMARK      1
#define MARKER_NP3_OCCUR_LINE    2

#define INDIC_NP3_MARK_OCCURANCE 1
#define INDIC_NP3_MATCH_BRACE    2
#define INDIC_NP3_BAD_BRACE      3

// --------------------------------------------------------------------------

//=============================================================================

#endif //_NP3_TYPEDEFS_H_
