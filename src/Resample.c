// Resample.cpp : Defines the entry point for the DLL application.
//

//#include "stdafx.h"

#define _USE_MATH_DEFINES

#include <math.h>
#include <assert.h>
#include "Helpers.h"
#include "Resample.h"

/* Quite arbitrary */
#define MAX_FILTER_RADIUS   16.0
#define MIN_RESAMPLE_WIDTH  0x00000001
#define MAX_RESAMPLE_WIDTH  0x00001000
#define MIN_RESAMPLE_HEIGHT 0x00000001
#define MAX_RESAMPLE_HEIGHT 0x00001000

/* RGBA */
#define COLOR_COMPONENTS 4

/* Each core filter has its own radius */
#define DEFAULT_LANCZOS8_RADIUS         8.0
#define DEFAULT_LANCZOS3_RADIUS         3.0
#define DEFAULT_HERMITE_RADIUS          1.0
#define DEFAULT_BOX_RADIUS              0.5
#define DEFAULT_TRIANGLE_RADIUS         1.0
#define DEFAULT_BELL_RADIUS             1.5
#define DEFAULT_CUBICSPLINE_RADIUS      2.0
#define DEFAULT_MITCHELL_RADIUS         2.0
#define DEFAULT_COSINE_RADIUS           1.0
#define DEFAULT_CATMULLROM_RADIUS       2.0
#define DEFAULT_QUADRATIC_RADIUS        1.5
#define DEFAULT_QUADRATICBSPLINE_RADIUS 1.5
#define DEFAULT_CUBICCONVOLUTION_RADIUS 3.0

/* Filter function type */
typedef double (*PFN_FILTER)(double);

/* Core filters */
static double _Lanczos8(double);
static double _Lanczos3(double);
static double _Hermite(double);
static double _Box(double);
static double _Triangle(double);
static double _Bell(double);
static double _CubicSpline(double);
static double _Mitchell(double);
static double _Cosine(double);
static double _CatmullRom(double);
static double _Quadratic(double);
static double _QuadraticBSpline(double);
static double _CubicConvolution(double);

/* helper functions */
static BOOL    _setResampleFilter(DWORD dwFilter, PFN_FILTER *ppFnFilter, double *pdRadius);
static HBITMAP _createResampledBitmap(HDC hdc, HBITMAP hBmpSource, DWORD dwWidth, DWORD dwHeight, PFN_FILTER pFnFilter, double dRadius);
static BOOL    _fillBITMAPINFO(HBITMAP hBmp, BITMAPINFO *pBinfo);
static BOOL    _resample(BYTE *ibuf, LONG iw, LONG ih, BYTE *obuf, LONG ow, LONG oh, PFN_FILTER pFnFilter, double dRadius);

#ifdef _MANAGED
#  pragma managed(push, off)
#endif

//BOOL APIENTRY DllMain(HMODULE hModule,
//                      DWORD   ul_reason_for_call,
//                      LPVOID  lpReserved)
//  UNUSED(lpReserved);
//  UNUSED(lpReserved);
//  switch (ul_reason_for_call) {
//    case DLL_PROCESS_ATTACH:
//    case DLL_THREAD_ATTACH:
//    case DLL_THREAD_DETACH:
//    case DLL_PROCESS_DETACH:
//      break;
//  }
//  return TRUE;
//}

#ifdef _MANAGED
#  pragma managed(pop)
#endif

/*-> The exported functions */
/* CreateResampledBitmap
	Creates a resampled bitmap given the original one, neew dimensions, the index of the core filter.
	The function is in fact a wrapper for the pair of helper functions _setResampleFilter and _createResampledBitmap.
	ARGS:
	hdc [IN] given device context
	hBmpSource [IN] original bitmap handle
	dwWidth [IN] resampled bitmap width
	dwHeight [IN] resampled bitmap height
	dwFilter [IN] index of the choosen core filter
	RETURN VALUE:
	the handle (HBITMAP) of the resampled bitmap on success, NULL on failure
*/

RESAMPLE_API HBITMAP CreateResampledBitmap(HDC     hdc,
                                           HBITMAP hBmpSource,
                                           DWORD dwWidth, DWORD dwHeight,
                                           DWORD dwFilter)
{
  double (*pFnFilter)(double);
  double dRadius;
  if (_setResampleFilter(dwFilter, &pFnFilter, &dRadius) == FALSE) {
    SetLastError(E_UNABLE_TO_SET_FILTER);
    return NULL;
  }

  return _createResampledBitmap(hdc, hBmpSource, dwWidth, dwHeight, pFnFilter, dRadius);
}

/* CreateResampledBitmap
	Creates a resampled bitmap given the original one, neew dimensions, the index of the core filter
	ARGS:
	hdc [IN] given device context
	hBmpSource [IN] original bitmap handle
	dwWidth [IN] resampled bitmap width
	dwHeight [IN] resampled bitmap height
	dwFilter [IN] index of the choosen core filter
	pFnCustomFilter [IN] custom filter function pointer
	dRadius [IN] radius of the custom filter
	RETURN VALUE:
	the handle (HBITMAP) of the resampled bitmap on success, NULL on failure
*/

RESAMPLE_API HBITMAP CreateUserFilterResampledBitmap(HDC     hdc,
                                                     HBITMAP hBmpSource,
                                                     DWORD dwWidth, DWORD                      dwHeight,
                                                     double (*pFnCustomFilter)(double), double dRadius)
{
  if (!pFnCustomFilter || dRadius < 0.0 || dRadius > MAX_FILTER_RADIUS) {
    SetLastError(E_UNABLE_TO_SET_FILTER);
    return NULL;
  }
  return _createResampledBitmap(hdc, hBmpSource, dwWidth, dwHeight, pFnCustomFilter, dRadius);
}

/* Sets proper (core) filter and radius given filter index
		ARGS:
		dwFilter [IN] filter index
		ppFnFilter [OUT] filter function
		pdRadius [OUT] filter radius
*/
BOOL _setResampleFilter(DWORD dwFilter, PFN_FILTER *ppFnFilter, double *pdRadius)
{
  BOOL fResult;

  fResult  = TRUE;
  dwFilter = dwFilter % STOCK_FILTERS;
  switch (dwFilter) {
    case STOCK_FILTER_LANCZOS3:
      *ppFnFilter = _Lanczos3;
      *pdRadius   = DEFAULT_LANCZOS3_RADIUS;
      break;
    case STOCK_FILTER_LANCZOS8:
      *ppFnFilter = _Lanczos8;
      *pdRadius   = DEFAULT_LANCZOS8_RADIUS;
      break;
    case STOCK_FILTER_HERMITE:
      *ppFnFilter = _Hermite;
      *pdRadius   = DEFAULT_HERMITE_RADIUS;
      break;
    case STOCK_FILTER_BOX:
      *ppFnFilter = _Box;
      *pdRadius   = DEFAULT_BOX_RADIUS;
      break;
    case STOCK_FILTER_TRIANGLE:
      *ppFnFilter = _Triangle;
      *pdRadius   = DEFAULT_TRIANGLE_RADIUS;
      break;
    case STOCK_FILTER_BELL:
      *ppFnFilter = _Bell;
      *pdRadius   = DEFAULT_BELL_RADIUS;
      break;
    case STOCK_FILTER_CUBICSPLINE:
      *ppFnFilter = _CubicSpline;
      *pdRadius   = DEFAULT_CUBICSPLINE_RADIUS;
      break;
    case STOCK_FILTER_MITCHELL:
      *ppFnFilter = _Mitchell;
      *pdRadius   = DEFAULT_MITCHELL_RADIUS;
      break;
    case STOCK_FILTER_COSINE:
      *ppFnFilter = _Cosine;
      *pdRadius   = DEFAULT_COSINE_RADIUS;
      break;
    case STOCK_FILTER_CATMULLROM:
      *ppFnFilter = _CatmullRom;
      *pdRadius   = DEFAULT_CATMULLROM_RADIUS;
      break;
    case STOCK_FILTER_QUADRATIC:
      *ppFnFilter = _Quadratic;
      *pdRadius   = DEFAULT_QUADRATIC_RADIUS;
      break;
    case STOCK_FILTER_QUADRATICBSPLINE:
      *ppFnFilter = _QuadraticBSpline;
      *pdRadius   = DEFAULT_QUADRATICBSPLINE_RADIUS;
      break;
    case STOCK_FILTER_CUBICCONVOLUTION:
      *ppFnFilter = _CubicConvolution;
      *pdRadius   = DEFAULT_CUBICCONVOLUTION_RADIUS;
      break;
    default:
      assert(0);
      fResult = FALSE;
  }
  return fResult;
}

/* Creates the resampled bitmap
		ARGS:
		hdc [IN] provided HDC
		hBmpSource [IN] original bitmap handle
		dwWidth [IN] resampled bitmap width
		dwHeight [IN] resampled bitmap height
		pnFnFilter [IN] filter function
		dRAdius [IN] filter radius
		RETURN VALUE:
		Handle (HBITMAP) of the resampled bitmap
*/

HBITMAP _createResampledBitmap(HDC hdc, HBITMAP hBmpSource,
                               DWORD dwWidth, DWORD dwHeight,
                               PFN_FILTER pFnFilter, double dRadius)
{
  BOOL fResult;

  BITMAPINFO binfoSource;
  BYTE *     pbSource;

  HBITMAP    hBmpTarget;
  BITMAPINFO binfoTarget;
  BYTE *     pbTarget;

  /* Bare initialization */
  fResult  = FALSE;
  pbSource = NULL;

  hBmpTarget = NULL;
  pbTarget   = NULL;

  /*<- Bare initialization */

  if (!pFnFilter) {
    SetLastError(E_UNABLE_TO_SET_FILTER);
    return FALSE;
  }

  if (!hBmpSource) {
    SetLastError(E_INVALID_BITMAP);
    return FALSE;
  }

  if (dwWidth < MIN_RESAMPLE_WIDTH) {
    dwWidth = MIN_RESAMPLE_WIDTH;
  }
  else if (dwWidth > MAX_RESAMPLE_WIDTH) {
    dwWidth = MAX_RESAMPLE_WIDTH;
  }

  if (dwHeight < MIN_RESAMPLE_HEIGHT) {
    dwHeight = MIN_RESAMPLE_HEIGHT;
  }
  else if (dwHeight > MAX_RESAMPLE_HEIGHT) {
    dwHeight = MAX_RESAMPLE_HEIGHT;
  }

  if (_fillBITMAPINFO(hBmpSource, &binfoSource) == FALSE) {
    SetLastError(E_INVALID_BITMAP_DATA);
    return FALSE;
  }

  /* Creating target bitmap */
  hBmpTarget = CreateCompatibleBitmap(hdc, dwWidth, dwHeight);
  if (!hBmpTarget) {
    SetLastError(E_UNABLE_TO_CREATE_BITMAP);
    return FALSE;
  }

  /* Getting info about the target bitmap */
  if (_fillBITMAPINFO(hBmpTarget, &binfoTarget) == FALSE) {
    SetLastError(E_INVALID_OUT_BITMAP_DATA);
    goto Cleanup;
  }

  /* Allocating buffer for the Source image bits */
  pbSource = AllocMem(binfoSource.bmiHeader.biSizeImage, HEAP_ZERO_MEMORY);
  if (!pbSource) {
    SetLastError(E_MEMORY_ERROR);
    goto Cleanup;
  }

  /* Getting data of the source bitmap  */
  if (binfoSource.bmiHeader.biHeight != GetDIBits(hdc, hBmpSource, 0, binfoSource.bmiHeader.biHeight, pbSource, &binfoSource, DIB_RGB_COLORS)) {
    SetLastError(E_UNABLE_TO_LOAD_BITMAP_BITS);
    goto Cleanup;
  }

  /* Allocating  buffer for the Target image bits */
  pbTarget = AllocMem(binfoTarget.bmiHeader.biSizeImage, HEAP_ZERO_MEMORY);
  if (!pbTarget) {
    SetLastError(E_MEMORY_ERROR);
    goto Cleanup;
  }

  if (_resample(pbSource, binfoSource.bmiHeader.biWidth, binfoSource.bmiHeader.biHeight,
                pbTarget, binfoTarget.bmiHeader.biWidth, binfoTarget.bmiHeader.biHeight,
                pFnFilter, dRadius) == FALSE) {
    SetLastError(E_RESAMPLE_ERROR);
    goto Cleanup;
  }

  if (binfoTarget.bmiHeader.biHeight != SetDIBits(hdc, hBmpTarget, 0, binfoTarget.bmiHeader.biHeight, pbTarget, &binfoTarget, DIB_RGB_COLORS)) {
    SetLastError(E_UNABLE_TO_SET_BITMAP);
    goto Cleanup;
  }

  fResult = TRUE;

Cleanup:

  if (pbSource) FreeMem(pbSource);

  if (pbTarget) FreeMem(pbTarget);

  if (fResult == FALSE) {
    if (hBmpTarget) {
      DeleteObject(hBmpTarget);
      hBmpTarget = NULL;
    }
  }

  return hBmpTarget;
} /* <- CreateResampledBitmap */

/* _resample
  This function does the real resampling stuff.
	ARGS:
	ibuf [IN] pointer of original bitmap bits
	iw [IN] original image width
	ih [IN] original image height
	obuf [OUT] resampled image bits
	ow [IN] resampled image width
	oh [IN] resampled image height
	pFnFilter [IN] filter function pointer
	dRadius [IN] filter radius
	RETURN VALUE
	TRUE on success
*/
BOOL _resample(BYTE *ibuf, LONG iw, LONG ih, BYTE *obuf, LONG ow, LONG oh, PFN_FILTER pFnFilter, double dRadius)
{
  BOOL fSuccess = FALSE;

  LONG   i, j, n, c;
  double xScale, yScale;

  /* Alias (pointer to DWORD) for ibuf */
  DWORD *ib;
  /* Alias (pointer to DWORD ) for obuf */
  DWORD *ob;

  // Temporary values
  DWORD val = 0;
  int   col; /* This should remain int (a bit tricky stuff) */

  double *h_weight; // Weight contribution    [ow][MAX_CONTRIBS]
  LONG *  h_pixel;  // Pixel that contributes [ow][MAX_CONTRIBS]
  LONG *  h_count;  // How many contribution for the pixel [ow]
  double *h_wsum;   // Sum of weights [ow]

  double *v_weight; // Weight contribution    [oh][MAX_CONTRIBS]
  LONG *  v_pixel;  // Pixel that contributes [oh][MAX_CONTRIBS]
  LONG *  v_count;  // How many contribution for the pixel [oh]
  double *v_wsum;   // Sum of weights [oh]

  DWORD *tb; // Temporary (intermediate buffer)

  double intensity[COLOR_COMPONENTS]; // RGBA component intensities

  double center; // Center of current sampling
  double weight; // Current wight
  LONG   left;   // Left of current sampling
  LONG   right;  // Right of current sampling

  double *p_weight; // Temporary pointer
  LONG *  p_pixel;  // Temporary pointer

  LONG   MAX_CONTRIBS;  // Almost-const: max number of contribution for current sampling
  double SCALED_RADIUS; // Almost-const: scaled radius for downsampling operations
  double FILTER_FACTOR; // Almost-const: filter factor for downsampling operations

  /* Preliminary (redundant ? ) check */
  if (iw < 1 || ih < 1 || ibuf == NULL || ow < 1 || oh < 1 || obuf == NULL) {
    return FALSE;
  }

  /* Aliasing buffers */
  ib = (DWORD *)ibuf;
  ob = (DWORD *)obuf;

  if (ow == iw && oh == ih) { /* Aame size, no resampling */
    CopyMemory(ob, ib, iw * ih * sizeof(COLORREF));
    return TRUE;
  }

  xScale = ((double)ow / iw);
  yScale = ((double)oh / ih);

  h_weight = NULL;
  h_pixel  = NULL;
  h_count  = NULL;
  h_wsum   = NULL;

  v_weight = NULL;
  v_pixel  = NULL;
  v_count  = NULL;
  v_wsum   = NULL;

  tb = NULL;

  tb = (DWORD *)AllocMem(ow * ih * sizeof(DWORD), HEAP_ZERO_MEMORY);

  if (!tb) goto Cleanup;

  if (xScale > 1.0) {
    /* Horizontal upsampling */
    FILTER_FACTOR = 1.0;
    SCALED_RADIUS = dRadius;
  }
  else { /* Horizontal downsampling */
    FILTER_FACTOR = xScale;
    SCALED_RADIUS = dRadius / xScale;
  }
  /* The maximum number of contributions for a target pixel */
  MAX_CONTRIBS = (int)(2 * SCALED_RADIUS + 1);

  /* Pre-allocating all of the needed memory */
  h_weight = (double *)AllocMem(ow * MAX_CONTRIBS * sizeof(double), HEAP_ZERO_MEMORY); /* weights */
  h_pixel  = (LONG *)AllocMem(ow * MAX_CONTRIBS * sizeof(int), HEAP_ZERO_MEMORY);       /* the contributing pixels */
  h_count  = (LONG *)AllocMem(ow * sizeof(int), HEAP_ZERO_MEMORY);                      /* how may contributions for the target pixel */
  h_wsum   = (double *)AllocMem(ow * sizeof(double), HEAP_ZERO_MEMORY);                /* sum of the weights for the target pixel */

  if (!(h_weight && h_pixel || h_count || h_wsum)) goto Cleanup;

  /* Pre-calculate weights contribution for a row */
  for (i = 0; i < ow; i++) {
    p_weight = h_weight + i * MAX_CONTRIBS;
    p_pixel  = h_pixel + i * MAX_CONTRIBS;

    h_count[i] = 0;
    h_wsum[i]  = 0.0;

    center = ((double)i) / xScale;
    left   = (int)((center + .5) - SCALED_RADIUS);
    right  = (int)(left + 2 * SCALED_RADIUS);

    for (j = left; j <= right; j++) {
      if (j < 0 || j >= iw) continue;

      weight = (*pFnFilter)((center - j) * FILTER_FACTOR);

      if (weight == 0.0) continue;

      n           = h_count[i]; /* Since h_count[i] is our current index */
      p_pixel[n]  = j;
      p_weight[n] = weight;
      h_wsum[i] += weight;
      h_count[i]++; /* Increment contribution count */
    }               /* j */
  }                 /* i */

  /* Filter horizontally from input to temporary buffer */
  for (n = 0; n < ih; n++) {
    /* Here 'n' runs on the vertical coordinate */
    for (i = 0; i < ow; i++) { /* i runs on the horizontal coordinate */
      p_weight = h_weight + i * MAX_CONTRIBS;
      p_pixel  = h_pixel + i * MAX_CONTRIBS;

      for (c = 0; c < COLOR_COMPONENTS; c++) {
        intensity[c] = 0.0;
      }
      for (j = 0; j < h_count[i]; j++) {
        weight = p_weight[j];
        val    = ib[p_pixel[j] + n * iw]; /* Using val as temporary storage */
        /* Acting on color components */
        for (c = 0; c < COLOR_COMPONENTS; c++) {
          intensity[c] += (val & 0xFF) * weight;
          val = val >> 8;
        }
      }
      /* val is already 0 */
      for (c = 0; c < COLOR_COMPONENTS; c++) {
        val = val << 8;
        col = (int)(intensity[COLOR_COMPONENTS - c - 1] / h_wsum[i]);
        if (col < 0) col = 0;
        if (col > 255) col = 255;
        val |= col;
      }
      tb[i + n * ow] = val; /* Temporary buffer ow x ih */
    }                       /* i */
  }                         /* n */

  /* Going to vertical stuff */
  if (yScale > 1.0) {
    FILTER_FACTOR = 1.0;
    SCALED_RADIUS = dRadius;
  }
  else {
    FILTER_FACTOR = yScale;
    SCALED_RADIUS = dRadius / yScale;
  }
  MAX_CONTRIBS = (int)(2 * SCALED_RADIUS + 1);

  /* Pre-calculate filter contributions for a column */
  v_weight = (double *)AllocMem(oh * MAX_CONTRIBS * sizeof(double), HEAP_ZERO_MEMORY); /* Weights */
  v_pixel  = (LONG *)AllocMem(oh * MAX_CONTRIBS * sizeof(int), HEAP_ZERO_MEMORY);       /* The contributing pixels */
  v_count  = (LONG *)AllocMem(oh * sizeof(int), HEAP_ZERO_MEMORY);                      /* How may contributions for the target pixel */
  v_wsum   = (double *)AllocMem(oh * sizeof(double), HEAP_ZERO_MEMORY);                /* Sum of the weights for the target pixel */

  if (!(v_weight && v_pixel && v_count && v_wsum)) goto Cleanup;

  for (i = 0; i < oh; i++) {
    p_weight = v_weight + i * MAX_CONTRIBS;
    p_pixel  = v_pixel + i * MAX_CONTRIBS;

    v_count[i] = 0;
    v_wsum[i]  = 0.0;

    center = ((double)i) / yScale;
    left   = (int)(center + .5 - SCALED_RADIUS);
    right  = (int)(left + 2 * SCALED_RADIUS);

    for (j = left; j <= right; j++) {
      if (j < 0 || j >= ih) continue;

      weight = (*pFnFilter)((center - j) * FILTER_FACTOR);

      if (weight == 0.0) continue;
      n           = v_count[i]; /* Our current index */
      p_pixel[n]  = j;
      p_weight[n] = weight;
      v_wsum[i] += weight;
      v_count[i]++; /* Increment the contribution count */
    }               /* j */
  }                 /* i */

  /* Filter vertically from work to output */
  for (n = 0; n < ow; n++) {
    for (i = 0; i < oh; i++) {
      p_weight = v_weight + i * MAX_CONTRIBS;
      p_pixel  = v_pixel + i * MAX_CONTRIBS;

      for (c = 0; c < COLOR_COMPONENTS; c++) {
        intensity[c] = 0.0;
      }

      for (j = 0; j < v_count[i]; j++) {
        weight = p_weight[j];
        val    = tb[n + ow * p_pixel[j]]; /* Using val as temporary storage */
        /* Acting on color components */
        for (c = 0; c < COLOR_COMPONENTS; c++) {
          intensity[c] += (val & 0xFF) * weight;
          val = val >> 8;
        }
      }
      /* val is already 0 */
      for (c = 0; c < COLOR_COMPONENTS; c++) {
        val = val << 8;
        col = (int)(intensity[COLOR_COMPONENTS - c - 1] / v_wsum[i]);
        if (col < 0) col = 0;
        if (col > 255) col = 255;
        val |= col;
      }
      ob[n + i * ow] = val;
    } /* i */
  }   /* n */

  fSuccess = TRUE;

Cleanup: /* CLEANUP */

  if (tb) FreeMem(tb);

  if (h_weight) FreeMem(h_weight);
  if (h_pixel) FreeMem(h_pixel);
  if (h_count) FreeMem(h_count);
  if (h_wsum) FreeMem(h_wsum);

  if (v_weight) FreeMem(v_weight);
  if (v_pixel) FreeMem(v_pixel);
  if (v_count) FreeMem(v_count);
  if (v_wsum) FreeMem(v_wsum);

  return fSuccess;
} /* _resample */

/* _fillBITMAPINFO helper function
  fills a BITMAPINFO struct given a HBITMAP
	ARGS:
	hBmp [IN] handle of the bitmap
	pBinfo [INOUT] pointer to a valid BITMAPINFO struct
	RETURN VALUE:
	TRUE on success
*/
BOOL _fillBITMAPINFO(HBITMAP hBmp, BITMAPINFO *pBinfo)
{
  BITMAP bmp;

  ZeroMemory(&bmp, sizeof(bmp));

  if (!GetObject(hBmp, sizeof(BITMAP), &bmp)) {
    return FALSE;
  }
  if (bmp.bmPlanes != 1 || bmp.bmBitsPixel < 24) {
    return FALSE;
  }

  /* Getting info about the source bitmap
	*/
  ZeroMemory(pBinfo, sizeof(*pBinfo));

  pBinfo->bmiHeader.biSize        = sizeof(pBinfo->bmiHeader);
  pBinfo->bmiHeader.biWidth       = bmp.bmWidth;
  pBinfo->bmiHeader.biHeight      = bmp.bmHeight;
  pBinfo->bmiHeader.biBitCount    = bmp.bmBitsPixel;
  pBinfo->bmiHeader.biPlanes      = 1;
  pBinfo->bmiHeader.biCompression = BI_RGB;
  pBinfo->bmiHeader.biSizeImage   = bmp.bmBitsPixel * bmp.bmWidth * bmp.bmHeight / 8;

  return TRUE;
}

/* Lanczos8 filter, default radius 8
*/
double _Lanczos8(double x)
{
  const double R = 8.0;
  if (x < 0.0) x = -x;

  if (x == 0.0) return 1;

  if (x < R) {
    x *= M_PI;
    return R * sin(x) * sin(x / R) / (x * x);
  }
  return 0.0;
}

/* Lanczos3 filter, default radius 3
*/
double _Lanczos3(double x)
{
  const double R = 3.0;
  if (x < 0.0) x = -x;

  if (x == 0.0) return 1;

  if (x < R) {
    x *= M_PI;
    return R * sin(x) * sin(x / R) / (x * x);
  }
  return 0.0;
}

/* Hermite filter, default radius 1
*/
double _Hermite(double x)
{
  if (x < 0.0) x = -x;

  if (x < 1.0) return ((2.0 * x - 3) * x * x + 1.0);

  return 0.0;
}

/* Box filter, default radius 0.5
*/
double _Box(double x)
{
  if (x < 0.0) x = -x;

  if (x <= 0.5) return 1.0;

  return 0.0;
}

/* Trangle filter, default radius 1
*/
double _Triangle(double x)
{
  if (x < 0.0) x = -x;
  if (x < 1.0) return (1.0 - x);
  return 0.0;
}

/* Bell filter, default radius 1.5
*/
double _Bell(double x)
{
  if (x < 0.0) x = -x;
  if (x < 0.5) return (0.75 - x * x);
  if (x < 1.5) return (0.5 * pow(x - 1.5, 2.0));
  return 0.0;
}

/* CubicSpline filter, default radius 2
*/
double _CubicSpline(double x)
{
  double x2;

  if (x < 0.0) x = -x;
  if (x < 1.0) {
    x2 = x * x;
    return (0.5 * x2 * x - x2 + 2.0 / 3.0);
  }
  if (x < 2.0) {
    x = 2.0 - x;
    return (pow(x, 3.0) / 6.0);
  }
  return 0;
}

/* Mitchell filter, default radius 2.0
*/
double _Mitchell(double x)
{
  const double C = 1.0 / 3.0;
  double       x2;

  if (x < 0.0) x = -x;
  x2 = x * x;
  if (x < 1.0) {
    x = (((12.0 - 9.0 * C - 6.0 * C) * (x * x2)) + ((-18.0 + 12.0 * C + 6.0 * C) * x2) + (6.0 - 2.0 * C));
    return (x / 6.0);
  }
  if (x < 2.0) {
    x = (((-C - 6.0 * C) * (x * x2)) + ((6.0 * C + 30.0 * C) * x2) + ((-12.0 * C - 48.0 * C) * x) + (8.0 * C + 24.0 * C));
    return (x / 6.0);
  }
  return 0.0;
}

/* Cosine filter, default radius 1
*/
double _Cosine(double x)
{
  if ((x >= -1.0) && (x <= 1.0)) return ((cos(x * M_PI) + 1.0) / 2.0);

  return 0;
}

/* CatmullRom filter, default radius 2
*/
double _CatmullRom(double x)
{
  //const double C = 0.5;
  double       x2;
  if (x < 0.0) x = -x;
  x2 = x * x;

  if (x <= 1.0) return (1.5 * x2 * x - 2.5 * x2 + 1);
  if (x <= 2.0) return (-0.5 * x2 * x + 2.5 * x2 - 4 * x + 2);
  return 0;
}

/* Quadratic filter, default radius 1.5
*/
double _Quadratic(double x)
{
  if (x < 0.0) x = -x;
  if (x <= 0.5) return (-2.0 * x * x + 1);
  if (x <= 1.5) return (x * x - 2.5 * x + 1.5);
  return 0.0;
}

/* QuadraticBSpline filter, default radius 1.5
*/
double _QuadraticBSpline(double x)
{
  if (x < 0.0) x = -x;
  if (x <= 0.5) return (-x * x + 0.75);
  if (x <= 1.5) return (0.5 * x * x - 1.5 * x + 1.125);
  return 0.0;
}
/* CubicConvolution filter, default radius 3
*/
double _CubicConvolution(double x)
{
  double x2;
  if (x < 0.0) x = -x;
  x2 = x * x;
  if (x <= 1.0) return ((4.0 / 3.0) * x2 * x - (7.0 / 3.0) * x2 + 1.0);
  if (x <= 2.0) return (-(7.0 / 12.0) * x2 * x + 3 * x2 - (59.0 / 12.0) * x + 2.5);
  if (x <= 3.0) return ((1.0 / 12.0) * x2 * x - (2.0 / 3.0) * x2 + 1.75 * x - 1.5);
  return 0;
}
