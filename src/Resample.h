/*
                             Resample

	Carlo Pallini, Dec. 2007 

  Original Source: https://www.codeproject.com/Articles/22271/Plain-C-Resampling-DLL

	This DLL exports the following two functions that allows quality resampling of true color bitmaps
	(1) CreateResampledBitmap
  (2) CreateUserFilterResampledBitmap  
	Function (1) Creates a resampled bitmap (HBITMAP) given the original one , the new dimensions
	and the choosen filter (13 'default' filters are provided) (a HDC is needed too).
	Functon (2) accepts  a custom filter (a function pointer) and related radius to perform a similar
	task.
	In both cases, the new HBITMAP is returned on success (NULL on failure). When such a HBITMAP is
	no longer need,  call DeleteObject on it.


 * AKNOWLEDGMENTS:
  
	This work is based on the Libor Tinka's article "Image Resizing - outperform GDI+" that
	may be found at CodeProject:
  https://www.codeproject.com/KB/GDI-plus/imgresizoutperfgdiplus.aspx
	I've ported his original algo to 100% pure unmanaged C code.
	There are some modification and the resulting code (thought not optimized)
	is a bit faster than the original one.
	
	Dec. 17th 2007
*/

#include "TypeDefs.h"

/*
	The following ifdef block is the standard way of creating macros which make exporting 
  from a DLL simpler. All files within this DLL are compiled with the RESAMPLE_EXPORTS
  symbol defined on the command line. this symbol should not be defined on any project
  that uses this DLL. This way any other project whose source files include this file see 
  RESAMPLE_API functions as being imported from a DLL, whereas this DLL sees symbols
  defined with this macro as being exported.
*/

//#ifdef RESAMPLE_EXPORTS
//#define RESAMPLE_API __declspec(dllexport)
//#else
//#define RESAMPLE_API __declspec(dllimport)
//#endif
#define RESAMPLE_API 

/* Stock (i.e. already defined) filters */
#define STOCK_FILTER_BELL								0x00000000
#define STOCK_FILTER_BOX								0x00000001
#define STOCK_FILTER_CATMULLROM         0x00000002
#define STOCK_FILTER_COSINE             0x00000003
#define STOCK_FILTER_CUBICCONVOLUTION   0x00000004
#define STOCK_FILTER_CUBICSPLINE        0x00000005
#define STOCK_FILTER_HERMITE						0x00000006					
#define STOCK_FILTER_LANCZOS3						0x00000007
#define STOCK_FILTER_LANCZOS8						0x00000008
#define STOCK_FILTER_MITCHELL           0x00000009
#define STOCK_FILTER_QUADRATIC          0x0000000A
#define STOCK_FILTER_QUADRATICBSPLINE   0x0000000B
#define STOCK_FILTER_TRIANGLE						0x0000000C

#define STOCK_FILTERS                   0x0000000D


/* Errors */
#define E_INVALID_BITMAP                0x20000001
#define E_INVALID_BITMAP_DATA           0x20000002
#define E_UNABLE_TO_LOAD_BITMAP_BITS		0x20000003
#define E_UNABLE_TO_CREATE_BITMAP				0x20000004
#define	E_INVALID_OUT_BITMAP_DATA				0x20000005
#define E_MEMORY_ERROR									0x20000006
#define E_RESAMPLE_ERROR                0x20000007
#define E_UNABLE_TO_SET_BITMAP          0x20000008
#define E_UNABLE_TO_SET_FILTER          0x20000009

/* The following block allows compilation with both C++ and C code */
#ifdef __cplusplus
extern "C"
{
#endif
RESAMPLE_API HBITMAP CreateResampledBitmap(HDC hdc, HBITMAP hBmpSource, DWORD dwWidth, DWORD dwHeight, DWORD dwFilter);
RESAMPLE_API HBITMAP CreateUserFilterResampledBitmap(HDC hdc, HBITMAP hBmpSource, DWORD dwWidth, DWORD dwHeight, double (*CustomFilterFunc)(double), double dRadius);
#ifdef __cplusplus
}
#endif
