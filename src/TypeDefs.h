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

#if defined(SCI_LARGE_FILE_SUPPORT)
  typedef Sci_Position   DocPos;
  typedef Sci_PositionU  DocPosU;
  typedef Sci_PositionCR DocCR;
  typedef Sci_Position   DocLn;  // Sci_Line?
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


  enum BufferSizes
  {
    MICRO_BUFFER = 32,
    MINI_BUFFER = 64,
    SMALL_BUFFER = 128,
    MIDSZ_BUFFER = 256,
    LARGE_BUFFER = 512,
    HUGE_BUFFER = 1024,
    XHUGE_BUFFER = 2048,

    FILE_ARG_BUF = MAX_PATH + 2,
    FNDRPL_BUFFER = 512
  };

//=============================================================================

#endif //_NP3_TYPEDEFS_H_
