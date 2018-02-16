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

#if defined(SCI_LARGE_FILE_SUPPORT)
typedef Sci_Position   DocPos;
typedef Sci_PositionCR DocPosCR;
typedef int            DocLn;  // Sci_Line?
#else
typedef int  DocPos;
//typedef ptrdiff_t DocPos; // compile test
typedef long DocPosCR;
typedef int  DocLn;
//typedef ptrdiff_t DocLn; // compile test
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
  FILE_ARG_BUF = MAX_PATH + 2
};

//=============================================================================

#endif //_NP3_TYPEDEFS_H_
