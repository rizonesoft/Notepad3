/* Copyright (c) 2010-2019 Christopher Swenson. */
/* Copyright (c) 2012 Vojtech Fried. */
/* Copyright (c) 2012 Google Inc. All Rights Reserved. */
/* https://github.com/swenson/sort */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef SORT_NAME
#error "Must declare SORT_NAME"
#endif

#ifndef SORT_TYPE
#error "Must declare SORT_TYPE"
#endif

#ifndef SORT_CMP
#define SORT_CMP(x, y)  ((x) < (y) ? -1 : ((y) < (x) ? 1 : 0))
#endif

#ifdef __cplusplus
#ifndef SORT_SAFE_CPY
#define SORT_SAFE_CPY 0
#endif
#else
#undef SORT_SAFE_CPY
#define SORT_SAFE_CPY 0
#endif

#ifndef TIM_SORT_STACK_SIZE
#define TIM_SORT_STACK_SIZE 128
#endif

#ifndef TIM_SORT_MIN_GALLOP
#define TIM_SORT_MIN_GALLOP 7
#endif

#ifndef SORT_SWAP
#define SORT_SWAP(x,y) {SORT_TYPE _sort_swap_temp = (x); (x) = (y); (y) = _sort_swap_temp;}
#endif

/* Common, type-agnostic functions and constants that we don't want to declare twice. */
#ifndef SORT_COMMON_H
#define SORT_COMMON_H

#ifndef MAX
#define MAX(x,y) (((x) > (y) ? (x) : (y)))
#endif

#ifndef MIN
#define MIN(x,y) (((x) < (y) ? (x) : (y)))
#endif

static int compute_minrun(const uint64_t);

/* From http://oeis.org/classic/A102549 */
static const uint64_t shell_gaps[48] = {1, 4, 10, 23, 57, 132, 301, 701, 1750, 4376, 10941, 27353, 68383, 170958, 427396, 1068491, 2671228, 6678071, 16695178, 41737946, 104344866, 260862166, 652155416, 1630388541, 4075971353LL, 10189928383LL, 25474820958LL, 63687052396LL, 159217630991LL, 398044077478LL, 995110193696LL, 2487775484241LL, 6219438710603LL, 15548596776508LL, 38871491941271LL, 97178729853178LL, 242946824632946LL, 607367061582366LL, 1518417653955916LL, 3796044134889791LL, 9490110337224478LL, 23725275843061196LL, 59313189607652991LL, 148282974019132478LL, 370707435047831196LL, 926768587619577991LL, 2316921469048944978LL, 5792303672622362446LL};

#ifndef CLZ
/* clang-only */
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#if __has_builtin(__builtin_clzll) || (defined(__GNUC__) && ((__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 3)))
#define CLZ __builtin_clzll
#else

static int clzll(uint64_t);

/* adapted from Hacker's Delight */
static int clzll(uint64_t x) {
  int n;

  if (x == 0) {
    return 64;
  }

  n = 0;

  if (x <= 0x00000000FFFFFFFFL) {
    n = n + 32;
    x = x << 32;
  }

  if (x <= 0x0000FFFFFFFFFFFFL) {
    n = n + 16;
    x = x << 16;
  }

  if (x <= 0x00FFFFFFFFFFFFFFL) {
    n = n + 8;
    x = x << 8;
  }

  if (x <= 0x0FFFFFFFFFFFFFFFL) {
    n = n + 4;
    x = x << 4;
  }

  if (x <= 0x3FFFFFFFFFFFFFFFL) {
    n = n + 2;
    x = x << 2;
  }

  if (x <= 0x7FFFFFFFFFFFFFFFL) {
    n = n + 1;
  }

  return n;
}

#define CLZ clzll
#endif
#endif

static __inline int compute_minrun(const uint64_t size) {
  const int top_bit = 64 - CLZ(size);
  const int shift = MAX(top_bit, 6) - 6;
  const int minrun = (int)(size >> shift);
  const uint64_t mask = (1ULL << shift) - 1;

  if (mask & size) {
    return minrun + 1;
  }

  return minrun;
}

static __inline size_t rbnd(size_t len) {
  int k;

  if (len < 16) {
    return 2;
  }

  k = 62 - CLZ(len);
  return 1ULL << ((2 * k) / 3);
}

#endif /* SORT_COMMON_H */

#define SORT_CONCAT(x, y) x ## _ ## y
#define SORT_MAKE_STR1(x, y) SORT_CONCAT(x,y)
#define SORT_MAKE_STR(x) SORT_MAKE_STR1(SORT_NAME,x)

#ifndef SMALL_SORT_BND
#define SMALL_SORT_BND 16
#endif
#ifndef SMALL_SORT
#define SMALL_SORT BITONIC_SORT
/*#define SMALL_SORT BINARY_INSERTION_SORT*/
#endif

#define SORT_TYPE_CPY                  SORT_MAKE_STR(sort_type_cpy)
#define SORT_TYPE_MOVE                 SORT_MAKE_STR(sort_type_move)
#define SORT_NEW_BUFFER                SORT_MAKE_STR(sort_new_buffer)
#define SORT_DELETE_BUFFER             SORT_MAKE_STR(sort_delete_buffer)
#define BITONIC_SORT                   SORT_MAKE_STR(bitonic_sort)
#define BINARY_INSERTION_FIND          SORT_MAKE_STR(binary_insertion_find)
#define BINARY_INSERTION_SORT_START    SORT_MAKE_STR(binary_insertion_sort_start)
#define BINARY_INSERTION_SORT          SORT_MAKE_STR(binary_insertion_sort)
#define REVERSE_ELEMENTS               SORT_MAKE_STR(reverse_elements)
#define COUNT_RUN                      SORT_MAKE_STR(count_run)
#define CHECK_INVARIANT                SORT_MAKE_STR(check_invariant)
#define TIM_SORT                       SORT_MAKE_STR(tim_sort)
#define TIM_SORT_GALLOP                SORT_MAKE_STR(tim_sort_gallop)
#define TIM_SORT_RESIZE                SORT_MAKE_STR(tim_sort_resize)
#define TIM_SORT_MERGE                 SORT_MAKE_STR(tim_sort_merge)
#define TIM_SORT_MERGE_LEFT            SORT_MAKE_STR(tim_sort_merge_left)
#define TIM_SORT_MERGE_RIGHT           SORT_MAKE_STR(tim_sort_merge_right)
#define TIM_SORT_COLLAPSE              SORT_MAKE_STR(tim_sort_collapse)
#define HEAP_SORT                      SORT_MAKE_STR(heap_sort)
#define MEDIAN                         SORT_MAKE_STR(median)
#define QUICK_SORT                     SORT_MAKE_STR(quick_sort)
#define MERGE_SORT                     SORT_MAKE_STR(merge_sort)
#define MERGE_SORT_RECURSIVE           SORT_MAKE_STR(merge_sort_recursive)
#define MERGE_SORT_IN_PLACE            SORT_MAKE_STR(merge_sort_in_place)
#define MERGE_SORT_IN_PLACE_RMERGE     SORT_MAKE_STR(merge_sort_in_place_rmerge)
#define MERGE_SORT_IN_PLACE_BACKMERGE  SORT_MAKE_STR(merge_sort_in_place_backmerge)
#define MERGE_SORT_IN_PLACE_FRONTMERGE SORT_MAKE_STR(merge_sort_in_place_frontmerge)
#define MERGE_SORT_IN_PLACE_ASWAP      SORT_MAKE_STR(merge_sort_in_place_aswap)
#define SELECTION_SORT                 SORT_MAKE_STR(selection_sort)
#define SHELL_SORT                     SORT_MAKE_STR(shell_sort)
#define QUICK_SORT_PARTITION           SORT_MAKE_STR(quick_sort_partition)
#define QUICK_SORT_RECURSIVE           SORT_MAKE_STR(quick_sort_recursive)
#define HEAP_SIFT_DOWN                 SORT_MAKE_STR(heap_sift_down)
#define HEAPIFY                        SORT_MAKE_STR(heapify)
#define TIM_SORT_RUN_T                 SORT_MAKE_STR(tim_sort_run_t)
#define TEMP_STORAGE_T                 SORT_MAKE_STR(temp_storage_t)
#define PUSH_NEXT                      SORT_MAKE_STR(push_next)
#define GRAIL_SWAP1                    SORT_MAKE_STR(grail_swap1)
#define REC_STABLE_SORT                SORT_MAKE_STR(rec_stable_sort)
#define GRAIL_REC_MERGE                SORT_MAKE_STR(grail_rec_merge)
#define GRAIL_SORT_DYN_BUFFER          SORT_MAKE_STR(grail_sort_dyn_buffer)
#define GRAIL_SORT_FIXED_BUFFER        SORT_MAKE_STR(grail_sort_fixed_buffer)
#define GRAIL_COMMON_SORT              SORT_MAKE_STR(grail_common_sort)
#define GRAIL_SORT                     SORT_MAKE_STR(grail_sort)
#define GRAIL_COMBINE_BLOCKS           SORT_MAKE_STR(grail_combine_blocks)
#define GRAIL_LAZY_STABLE_SORT         SORT_MAKE_STR(grail_lazy_stable_sort)
#define GRAIL_MERGE_WITHOUT_BUFFER     SORT_MAKE_STR(grail_merge_without_buffer)
#define GRAIL_ROTATE                   SORT_MAKE_STR(grail_rotate)
#define GRAIL_BIN_SEARCH_LEFT          SORT_MAKE_STR(grail_bin_search_left)
#define GRAIL_BUILD_BLOCKS             SORT_MAKE_STR(grail_build_blocks)
#define GRAIL_FIND_KEYS                SORT_MAKE_STR(grail_find_keys)
#define GRAIL_MERGE_BUFFERS_LEFT_WITH_X_BUF SORT_MAKE_STR(grail_merge_buffers_left_with_x_buf)
#define GRAIL_BIN_SEARCH_RIGHT         SORT_MAKE_STR(grail_bin_search_right)
#define GRAIL_MERGE_BUFFERS_LEFT       SORT_MAKE_STR(grail_merge_buffers_left)
#define GRAIL_SMART_MERGE_WITH_X_BUF   SORT_MAKE_STR(grail_smart_merge_with_x_buf)
#define GRAIL_MERGE_LEFT_WITH_X_BUF    SORT_MAKE_STR(grail_merge_left_with_x_buf)
#define GRAIL_SMART_MERGE_WITHOUT_BUFFER SORT_MAKE_STR(grail_smart_merge_without_buffer)
#define GRAIL_SMART_MERGE_WITH_BUFFER  SORT_MAKE_STR(grail_smart_merge_with_buffer)
#define GRAIL_MERGE_RIGHT              SORT_MAKE_STR(grail_merge_right)
#define GRAIL_MERGE_LEFT               SORT_MAKE_STR(grail_merge_left)
#define GRAIL_SWAP_N                   SORT_MAKE_STR(grail_swap_n)
#define SQRT_SORT                      SORT_MAKE_STR(sqrt_sort)
#define SQRT_SORT_BUILD_BLOCKS         SORT_MAKE_STR(sqrt_sort_build_blocks)
#define SQRT_SORT_MERGE_BUFFERS_LEFT_WITH_X_BUF SORT_MAKE_STR(sqrt_sort_merge_buffers_left_with_x_buf)
#define SQRT_SORT_MERGE_DOWN           SORT_MAKE_STR(sqrt_sort_merge_down)
#define SQRT_SORT_MERGE_LEFT_WITH_X_BUF SORT_MAKE_STR(sqrt_sort_merge_left_with_x_buf)
#define SQRT_SORT_MERGE_RIGHT          SORT_MAKE_STR(sqrt_sort_merge_right)
#define SQRT_SORT_SWAP_N               SORT_MAKE_STR(sqrt_sort_swap_n)
#define SQRT_SORT_SWAP_1               SORT_MAKE_STR(sqrt_sort_swap_1)
#define SQRT_SORT_SMART_MERGE_WITH_X_BUF SORT_MAKE_STR(sqrt_sort_smart_merge_with_x_buf)
#define SQRT_SORT_SORT_INS             SORT_MAKE_STR(sqrt_sort_sort_ins)
#define SQRT_SORT_COMBINE_BLOCKS       SORT_MAKE_STR(sqrt_sort_combine_blocks)
#define SQRT_SORT_COMMON_SORT          SORT_MAKE_STR(sqrt_sort_common_sort)
#define BUBBLE_SORT                    SORT_MAKE_STR(bubble_sort)

#ifndef MAX
#define MAX(x,y) (((x) > (y) ? (x) : (y)))
#endif
#ifndef MIN
#define MIN(x,y) (((x) < (y) ? (x) : (y)))
#endif
#ifndef SORT_CSWAP
#define SORT_CSWAP(x, y) { if(SORT_CMP((x),(y)) > 0) {SORT_SWAP((x),(y));}}
#endif

typedef struct {
  size_t start;
  size_t length;
} TIM_SORT_RUN_T;


void SHELL_SORT(SORT_TYPE *dst, const size_t size);
void BINARY_INSERTION_SORT(SORT_TYPE *dst, const size_t size);
void HEAP_SORT(SORT_TYPE *dst, const size_t size);
void QUICK_SORT(SORT_TYPE *dst, const size_t size);
void MERGE_SORT(SORT_TYPE *dst, const size_t size);
void MERGE_SORT_IN_PLACE(SORT_TYPE *dst, const size_t size);
void SELECTION_SORT(SORT_TYPE *dst, const size_t size);
void TIM_SORT(SORT_TYPE *dst, const size_t size);
void BUBBLE_SORT(SORT_TYPE *dst, const size_t size);
void BITONIC_SORT(SORT_TYPE *dst, const size_t size);
void REC_STABLE_SORT(SORT_TYPE *dst, const size_t size);
void GRAIL_SORT_DYN_BUFFER(SORT_TYPE *dst, const size_t size);
void GRAIL_SORT_FIXED_BUFFER(SORT_TYPE *dst, const size_t size);
void GRAIL_SORT(SORT_TYPE *dst, const size_t size);
void SQRT_SORT(SORT_TYPE *dst, const size_t size);

/* The full implementation of a bitonic sort is not here. Since we only want to use
   sorting networks for small length lists we create optimal sorting networks for
   lists of length <= 16 and call out to BINARY_INSERTION_SORT for anything larger
   than 16.
   Optimal sorting networks for small length lists.
   Taken from https://pages.ripco.net/~jgamble/nw.html */
#define BITONIC_SORT_2          SORT_MAKE_STR(bitonic_sort_2)
static __inline void BITONIC_SORT_2(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
}


#define BITONIC_SORT_3          SORT_MAKE_STR(bitonic_sort_3)
static __inline void BITONIC_SORT_3(SORT_TYPE *dst) {
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[0], dst[1]);
}


#define BITONIC_SORT_4          SORT_MAKE_STR(bitonic_sort_4)
static __inline void BITONIC_SORT_4(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[1], dst[2]);
}


#define BITONIC_SORT_5          SORT_MAKE_STR(bitonic_sort_5)
static __inline void BITONIC_SORT_5(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[0], dst[3]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[1], dst[2]);
}


#define BITONIC_SORT_6          SORT_MAKE_STR(bitonic_sort_6)
static __inline void BITONIC_SORT_6(SORT_TYPE *dst) {
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[2], dst[5]);
  SORT_CSWAP(dst[0], dst[3]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[2], dst[3]);
}


#define BITONIC_SORT_7          SORT_MAKE_STR(bitonic_sort_7)
static __inline void BITONIC_SORT_7(SORT_TYPE *dst) {
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[0], dst[3]);
  SORT_CSWAP(dst[2], dst[5]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[2], dst[3]);
}


#define BITONIC_SORT_8          SORT_MAKE_STR(bitonic_sort_8)
static __inline void BITONIC_SORT_8(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[5], dst[7]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[3], dst[7]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[3], dst[6]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[3], dst[4]);
}


#define BITONIC_SORT_9          SORT_MAKE_STR(bitonic_sort_9)
static __inline void BITONIC_SORT_9(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[7], dst[8]);
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[2], dst[5]);
  SORT_CSWAP(dst[0], dst[3]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[5], dst[8]);
  SORT_CSWAP(dst[3], dst[6]);
  SORT_CSWAP(dst[4], dst[7]);
  SORT_CSWAP(dst[2], dst[5]);
  SORT_CSWAP(dst[0], dst[3]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[5], dst[7]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[2], dst[3]);
}


#define BITONIC_SORT_10          SORT_MAKE_STR(bitonic_sort_10)
static __inline void BITONIC_SORT_10(SORT_TYPE *dst) {
  SORT_CSWAP(dst[4], dst[9]);
  SORT_CSWAP(dst[3], dst[8]);
  SORT_CSWAP(dst[2], dst[7]);
  SORT_CSWAP(dst[1], dst[6]);
  SORT_CSWAP(dst[0], dst[5]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[6], dst[9]);
  SORT_CSWAP(dst[0], dst[3]);
  SORT_CSWAP(dst[5], dst[8]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[3], dst[6]);
  SORT_CSWAP(dst[7], dst[9]);
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[5], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[7], dst[8]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[2], dst[5]);
  SORT_CSWAP(dst[6], dst[8]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[4], dst[7]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[4], dst[5]);
}


#define BITONIC_SORT_11          SORT_MAKE_STR(bitonic_sort_11)
static __inline void BITONIC_SORT_11(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[5], dst[7]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[8], dst[10]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[3], dst[7]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[6], dst[10]);
  SORT_CSWAP(dst[4], dst[8]);
  SORT_CSWAP(dst[5], dst[9]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[3], dst[8]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[6], dst[10]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[7], dst[10]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[6], dst[8]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[7], dst[9]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[7], dst[8]);
}


#define BITONIC_SORT_12          SORT_MAKE_STR(bitonic_sort_12)
static __inline void BITONIC_SORT_12(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[10], dst[11]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[5], dst[7]);
  SORT_CSWAP(dst[9], dst[11]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[8], dst[10]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[7], dst[11]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[6], dst[10]);
  SORT_CSWAP(dst[3], dst[7]);
  SORT_CSWAP(dst[4], dst[8]);
  SORT_CSWAP(dst[5], dst[9]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[7], dst[11]);
  SORT_CSWAP(dst[3], dst[8]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[6], dst[10]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[7], dst[10]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[6], dst[8]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[7], dst[9]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[7], dst[8]);
}


#define BITONIC_SORT_13          SORT_MAKE_STR(bitonic_sort_13)
static __inline void BITONIC_SORT_13(SORT_TYPE *dst) {
  SORT_CSWAP(dst[1], dst[7]);
  SORT_CSWAP(dst[9], dst[11]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[5], dst[8]);
  SORT_CSWAP(dst[0], dst[12]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[8], dst[11]);
  SORT_CSWAP(dst[7], dst[12]);
  SORT_CSWAP(dst[5], dst[9]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[3], dst[7]);
  SORT_CSWAP(dst[10], dst[11]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[6], dst[12]);
  SORT_CSWAP(dst[7], dst[8]);
  SORT_CSWAP(dst[11], dst[12]);
  SORT_CSWAP(dst[4], dst[9]);
  SORT_CSWAP(dst[6], dst[10]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[10], dst[11]);
  SORT_CSWAP(dst[1], dst[7]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[9], dst[11]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[4], dst[7]);
  SORT_CSWAP(dst[8], dst[10]);
  SORT_CSWAP(dst[0], dst[5]);
  SORT_CSWAP(dst[2], dst[5]);
  SORT_CSWAP(dst[6], dst[8]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[7], dst[8]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[5], dst[6]);
}


#define BITONIC_SORT_14          SORT_MAKE_STR(bitonic_sort_14)
static __inline void BITONIC_SORT_14(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[10], dst[11]);
  SORT_CSWAP(dst[12], dst[13]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[8], dst[10]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[5], dst[7]);
  SORT_CSWAP(dst[9], dst[11]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[8], dst[12]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[9], dst[13]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[3], dst[7]);
  SORT_CSWAP(dst[0], dst[8]);
  SORT_CSWAP(dst[1], dst[9]);
  SORT_CSWAP(dst[2], dst[10]);
  SORT_CSWAP(dst[3], dst[11]);
  SORT_CSWAP(dst[4], dst[12]);
  SORT_CSWAP(dst[5], dst[13]);
  SORT_CSWAP(dst[5], dst[10]);
  SORT_CSWAP(dst[6], dst[9]);
  SORT_CSWAP(dst[3], dst[12]);
  SORT_CSWAP(dst[7], dst[11]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[4], dst[8]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[7], dst[13]);
  SORT_CSWAP(dst[2], dst[8]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[11], dst[13]);
  SORT_CSWAP(dst[3], dst[8]);
  SORT_CSWAP(dst[7], dst[12]);
  SORT_CSWAP(dst[6], dst[8]);
  SORT_CSWAP(dst[10], dst[12]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[7], dst[9]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[7], dst[8]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[11], dst[12]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
}


#define BITONIC_SORT_15          SORT_MAKE_STR(bitonic_sort_15)
static __inline void BITONIC_SORT_15(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[10], dst[11]);
  SORT_CSWAP(dst[12], dst[13]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[8], dst[10]);
  SORT_CSWAP(dst[12], dst[14]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[5], dst[7]);
  SORT_CSWAP(dst[9], dst[11]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[8], dst[12]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[9], dst[13]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[10], dst[14]);
  SORT_CSWAP(dst[3], dst[7]);
  SORT_CSWAP(dst[0], dst[8]);
  SORT_CSWAP(dst[1], dst[9]);
  SORT_CSWAP(dst[2], dst[10]);
  SORT_CSWAP(dst[3], dst[11]);
  SORT_CSWAP(dst[4], dst[12]);
  SORT_CSWAP(dst[5], dst[13]);
  SORT_CSWAP(dst[6], dst[14]);
  SORT_CSWAP(dst[5], dst[10]);
  SORT_CSWAP(dst[6], dst[9]);
  SORT_CSWAP(dst[3], dst[12]);
  SORT_CSWAP(dst[13], dst[14]);
  SORT_CSWAP(dst[7], dst[11]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[4], dst[8]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[7], dst[13]);
  SORT_CSWAP(dst[2], dst[8]);
  SORT_CSWAP(dst[11], dst[14]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[11], dst[13]);
  SORT_CSWAP(dst[3], dst[8]);
  SORT_CSWAP(dst[7], dst[12]);
  SORT_CSWAP(dst[6], dst[8]);
  SORT_CSWAP(dst[10], dst[12]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[7], dst[9]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[7], dst[8]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[11], dst[12]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
}


#define BITONIC_SORT_16          SORT_MAKE_STR(bitonic_sort_16)
static __inline void BITONIC_SORT_16(SORT_TYPE *dst) {
  SORT_CSWAP(dst[0], dst[1]);
  SORT_CSWAP(dst[2], dst[3]);
  SORT_CSWAP(dst[4], dst[5]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
  SORT_CSWAP(dst[10], dst[11]);
  SORT_CSWAP(dst[12], dst[13]);
  SORT_CSWAP(dst[14], dst[15]);
  SORT_CSWAP(dst[0], dst[2]);
  SORT_CSWAP(dst[4], dst[6]);
  SORT_CSWAP(dst[8], dst[10]);
  SORT_CSWAP(dst[12], dst[14]);
  SORT_CSWAP(dst[1], dst[3]);
  SORT_CSWAP(dst[5], dst[7]);
  SORT_CSWAP(dst[9], dst[11]);
  SORT_CSWAP(dst[13], dst[15]);
  SORT_CSWAP(dst[0], dst[4]);
  SORT_CSWAP(dst[8], dst[12]);
  SORT_CSWAP(dst[1], dst[5]);
  SORT_CSWAP(dst[9], dst[13]);
  SORT_CSWAP(dst[2], dst[6]);
  SORT_CSWAP(dst[10], dst[14]);
  SORT_CSWAP(dst[3], dst[7]);
  SORT_CSWAP(dst[11], dst[15]);
  SORT_CSWAP(dst[0], dst[8]);
  SORT_CSWAP(dst[1], dst[9]);
  SORT_CSWAP(dst[2], dst[10]);
  SORT_CSWAP(dst[3], dst[11]);
  SORT_CSWAP(dst[4], dst[12]);
  SORT_CSWAP(dst[5], dst[13]);
  SORT_CSWAP(dst[6], dst[14]);
  SORT_CSWAP(dst[7], dst[15]);
  SORT_CSWAP(dst[5], dst[10]);
  SORT_CSWAP(dst[6], dst[9]);
  SORT_CSWAP(dst[3], dst[12]);
  SORT_CSWAP(dst[13], dst[14]);
  SORT_CSWAP(dst[7], dst[11]);
  SORT_CSWAP(dst[1], dst[2]);
  SORT_CSWAP(dst[4], dst[8]);
  SORT_CSWAP(dst[1], dst[4]);
  SORT_CSWAP(dst[7], dst[13]);
  SORT_CSWAP(dst[2], dst[8]);
  SORT_CSWAP(dst[11], dst[14]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[2], dst[4]);
  SORT_CSWAP(dst[11], dst[13]);
  SORT_CSWAP(dst[3], dst[8]);
  SORT_CSWAP(dst[7], dst[12]);
  SORT_CSWAP(dst[6], dst[8]);
  SORT_CSWAP(dst[10], dst[12]);
  SORT_CSWAP(dst[3], dst[5]);
  SORT_CSWAP(dst[7], dst[9]);
  SORT_CSWAP(dst[3], dst[4]);
  SORT_CSWAP(dst[5], dst[6]);
  SORT_CSWAP(dst[7], dst[8]);
  SORT_CSWAP(dst[9], dst[10]);
  SORT_CSWAP(dst[11], dst[12]);
  SORT_CSWAP(dst[6], dst[7]);
  SORT_CSWAP(dst[8], dst[9]);
}

void BITONIC_SORT(SORT_TYPE *dst, const size_t size) {
  switch (size) {
  case 0:
  case 1:
    break;

  case 2:
    BITONIC_SORT_2(dst);
    break;

  case 3:
    BITONIC_SORT_3(dst);
    break;

  case 4:
    BITONIC_SORT_4(dst);
    break;

  case 5:
    BITONIC_SORT_5(dst);
    break;

  case 6:
    BITONIC_SORT_6(dst);
    break;

  case 7:
    BITONIC_SORT_7(dst);
    break;

  case 8:
    BITONIC_SORT_8(dst);
    break;

  case 9:
    BITONIC_SORT_9(dst);
    break;

  case 10:
    BITONIC_SORT_10(dst);
    break;

  case 11:
    BITONIC_SORT_11(dst);
    break;

  case 12:
    BITONIC_SORT_12(dst);
    break;

  case 13:
    BITONIC_SORT_13(dst);
    break;

  case 14:
    BITONIC_SORT_14(dst);
    break;

  case 15:
    BITONIC_SORT_15(dst);
    break;

  case 16:
    BITONIC_SORT_16(dst);
    break;

  default:
    BINARY_INSERTION_SORT(dst, size);
  }
}

#if SORT_SAFE_CPY

void SORT_TYPE_CPY(SORT_TYPE *dst, SORT_TYPE *src, const size_t size) {
  size_t i = 0;

  for (; i < size; ++i) {
    dst[i] = src[i];
  }
}

void SORT_TYPE_MOVE(SORT_TYPE *dst, SORT_TYPE *src, const size_t size) {
  size_t i;

  if (dst < src) {
    SORT_TYPE_CPY(dst, src, size);
  } else if (dst != src && size > 0) {
    for (i = size - 1; i > 0; --i) {
      dst[i] = src[i];
    }

    *dst = *src;
  }
}

#else

#undef SORT_TYPE_CPY
#define SORT_TYPE_CPY(dst, src, size) memcpy((dst), (src), (size) * sizeof(SORT_TYPE))
#undef SORT_TYPE_MOVE
#define SORT_TYPE_MOVE(dst, src, size) memmove((dst), (src), (size) * sizeof(SORT_TYPE))

#endif

SORT_TYPE* SORT_NEW_BUFFER(size_t size) {
#if SORT_SAFE_CPY
  return new SORT_TYPE[size];
#else
  return (SORT_TYPE*)malloc(size * sizeof(SORT_TYPE));
#endif
}

void SORT_DELETE_BUFFER(SORT_TYPE* pointer) {
#if SORT_SAFE_CPY
  delete[] pointer;
#else
  free(pointer);
#endif
}


/* Shell sort implementation based on Wikipedia article
   http://en.wikipedia.org/wiki/Shell_sort
*/
void SHELL_SORT(SORT_TYPE *dst, const size_t size) {
  /* don't bother sorting an array of size 0 or 1 */
  /* TODO: binary search to find first gap? */
  int inci = 47;
  size_t inc = shell_gaps[inci];
  size_t i;

  if (size <= 1) {
    return;
  }

  while (inc > (size >> 1)) {
    inc = shell_gaps[--inci];
  }

  while (1) {
    for (i = inc; i < size; i++) {
      SORT_TYPE temp = dst[i];
      size_t j = i;

      while ((j >= inc) && (SORT_CMP(dst[j - inc], temp) > 0)) {
        dst[j] = dst[j - inc];
        j -= inc;
      }

      dst[j] = temp;
    }

    if (inc == 1) {
      break;
    }

    inc = shell_gaps[--inci];
  }
}

/* Function used to do a binary search for binary insertion sort */
static __inline size_t BINARY_INSERTION_FIND(SORT_TYPE *dst, const SORT_TYPE x,
    const size_t size) {
  size_t l, c, r;
  SORT_TYPE cx;
  l = 0;
  r = size - 1;
  c = r >> 1;

  /* check for out of bounds at the beginning. */
  if (SORT_CMP(x, dst[0]) < 0) {
    return 0;
  } else if (SORT_CMP(x, dst[r]) > 0) {
    return r;
  }

  cx = dst[c];

  while (1) {
    const int val = SORT_CMP(x, cx);

    if (val < 0) {
      if (c - l <= 1) {
        return c;
      }

      r = c;
    } else { /* allow = for stability. The binary search favors the right. */
      if (r - c <= 1) {
        return c + 1;
      }

      l = c;
    }

    c = l + ((r - l) >> 1);
    cx = dst[c];
  }
}

/* Binary insertion sort, but knowing that the first "start" entries are sorted.  Used in timsort. */
static void BINARY_INSERTION_SORT_START(SORT_TYPE *dst, const size_t start, const size_t size) {
  size_t i;

  for (i = start; i < size; i++) {
    size_t j;
    SORT_TYPE x;
    size_t location;

    /* If this entry is already correct, just move along */
    if (SORT_CMP(dst[i - 1], dst[i]) <= 0) {
      continue;
    }

    /* Else we need to find the right place, shift everything over, and squeeze in */
    x = dst[i];
    location = BINARY_INSERTION_FIND(dst, x, i);

    for (j = i - 1; j >= location; j--) {
      dst[j + 1] = dst[j];

      if (j == 0) { /* check edge case because j is unsigned */
        break;
      }
    }

    dst[location] = x;
  }
}

/* Binary insertion sort */
void BINARY_INSERTION_SORT(SORT_TYPE *dst, const size_t size) {
  /* don't bother sorting an array of size <= 1 */
  if (size <= 1) {
    return;
  }

  BINARY_INSERTION_SORT_START(dst, 1, size);
}

/* Selection sort */
void SELECTION_SORT(SORT_TYPE *dst, const size_t size) {
  size_t i, j;

  /* don't bother sorting an array of size <= 1 */
  if (size <= 1) {
    return;
  }

  for (i = 0; i < size; i++) {
    for (j = i + 1; j < size; j++) {
      if (SORT_CMP(dst[j], dst[i]) < 0) {
        SORT_SWAP(dst[i], dst[j]);
      }
    }
  }
}

/* In-place mergesort */
void MERGE_SORT_IN_PLACE_ASWAP(SORT_TYPE * dst1, SORT_TYPE * dst2, size_t len) {
  do {
    SORT_SWAP(*dst1, *dst2);
    dst1++;
    dst2++;
  } while (--len);
}

void MERGE_SORT_IN_PLACE_FRONTMERGE(SORT_TYPE *dst1, size_t l1, SORT_TYPE *dst2, size_t l2) {
  SORT_TYPE *dst0 = dst2 - l1;

  if (SORT_CMP(dst1[l1 - 1], dst2[0]) <= 0) {
    MERGE_SORT_IN_PLACE_ASWAP(dst1, dst0, l1);
    return;
  }

  do {
    while (SORT_CMP(*dst2, *dst1) > 0) {
      SORT_SWAP(*dst1, *dst0);
      dst1++;
      dst0++;

      if (--l1 == 0) {
        return;
      }
    }

    SORT_SWAP(*dst2, *dst0);
    dst2++;
    dst0++;
  } while (--l2);

  do {
    SORT_SWAP(*dst1, *dst0);
    dst1++;
    dst0++;
  } while (--l1);
}

size_t MERGE_SORT_IN_PLACE_BACKMERGE(SORT_TYPE * dst1, size_t l1, SORT_TYPE * dst2, size_t l2) {
  size_t res;
  SORT_TYPE *dst0 = dst2 + l1;

  if (SORT_CMP(dst1[1 - l1], dst2[0]) >= 0) {
    MERGE_SORT_IN_PLACE_ASWAP(dst1 - l1 + 1, dst0 - l1 + 1, l1);
    return l1;
  }

  do {
    while (SORT_CMP(*dst2, *dst1) < 0) {
      SORT_SWAP(*dst1, *dst0);
      dst1--;
      dst0--;

      if (--l1 == 0) {
        return 0;
      }
    }

    SORT_SWAP(*dst2, *dst0);
    dst2--;
    dst0--;
  } while (--l2);

  res = l1;

  do {
    SORT_SWAP(*dst1, *dst0);
    dst1--;
    dst0--;
  } while (--l1);

  return res;
}

/* merge dst[p0..p1) by buffer dst[p1..p1+r) */
void MERGE_SORT_IN_PLACE_RMERGE(SORT_TYPE *dst, size_t len, size_t lp, size_t r) {
  size_t i, lq;
  int cv;

  if (SORT_CMP(dst[lp], dst[lp - 1]) >= 0) {
    return;
  }

  lq = lp;

  for (i = 0; i < len; i += r) {
    /* select smallest dst[p0+n*r] */
    size_t q = i, j;

    for (j = lp; j <= lq; j += r) {
      cv = SORT_CMP(dst[j], dst[q]);

      if (cv == 0) {
        cv = SORT_CMP(dst[j + r - 1], dst[q + r - 1]);
      }

      if (cv < 0) {
        q = j;
      }
    }

    if (q != i) {
      MERGE_SORT_IN_PLACE_ASWAP(dst + i, dst + q, r); /* swap it with current position */

      if (q == lq && q < (len - r)) {
        lq += r;
      }
    }

    if (i != 0 && SORT_CMP(dst[i], dst[i - 1]) < 0) {
      MERGE_SORT_IN_PLACE_ASWAP(dst + len, dst + i, r); /* swap current position with buffer */
      MERGE_SORT_IN_PLACE_BACKMERGE(dst + (len + r - 1), r, dst + (i - 1),
                                    r);  /* buffer :merge: dst[i-r..i) -> dst[i-r..i+r) */
    }

    if (lp == i) {
      lp += r;
    }
  }
}

/* In-place Merge Sort implementation. (c)2012, Andrey Astrelin, astrelin@tochka.ru */
void MERGE_SORT_IN_PLACE(SORT_TYPE *dst, const size_t len) {
  /* don't bother sorting an array of size <= 1 */
  size_t r = rbnd(len);
  size_t lr = (len / r - 1) * r;
  SORT_TYPE *dst1 = dst - 1;
  size_t p, m, q, q1, p0;

  if (len <= 1) {
    return;
  }

  if (len <= SMALL_SORT_BND) {
    SMALL_SORT(dst, len);
    return;
  }

  for (p = 2; p <= lr; p += 2) {
    dst1 += 2;

    if (SORT_CMP(dst1[0], dst1[-1]) < 0) {
      SORT_SWAP(dst1[0], dst1[-1]);
    }

    if (p & 2) {
      continue;
    }

    m = len - p;
    q = 2;

    while ((p & q) == 0) {
      if (SORT_CMP(dst1[1 - q], dst1[-(int) q]) < 0) {
        break;
      }

      q *= 2;
    }

    if (p & q) {
      continue;
    }

    if (q < m) {
      p0 = len - q;
      MERGE_SORT_IN_PLACE_ASWAP(dst + p - q, dst + p0, q);

      for (;;) {
        q1 = 2 * q;

        if ((q1 > m) || (p & q1)) {
          break;
        }

        p0 = len - q1;
        MERGE_SORT_IN_PLACE_FRONTMERGE(dst + (p - q1), q, dst + p0 + q, q);
        q = q1;
      }

      MERGE_SORT_IN_PLACE_BACKMERGE(dst + (len - 1), q, dst1 - q, q);
      q *= 2;
    }

    q1 = q;

    while (q1 > m) {
      q1 /= 2;
    }

    while ((q & p) == 0) {
      q *= 2;
      MERGE_SORT_IN_PLACE_RMERGE(dst + (p - q), q, q / 2, q1);
    }
  }

  q1 = 0;

  for (q = r; q < lr; q *= 2) {
    if ((lr & q) != 0) {
      q1 += q;

      if (q1 != q) {
        MERGE_SORT_IN_PLACE_RMERGE(dst + (lr - q1), q1, q, r);
      }
    }
  }

  m = len - lr;
  MERGE_SORT_IN_PLACE(dst + lr, m);
  MERGE_SORT_IN_PLACE_ASWAP(dst, dst + lr, m);
  m += MERGE_SORT_IN_PLACE_BACKMERGE(dst + (m - 1), m, dst + (lr - 1), lr - m);
  MERGE_SORT_IN_PLACE(dst, m);
}

/* Standard merge sort */
void MERGE_SORT_RECURSIVE(SORT_TYPE *newdst, SORT_TYPE *dst, const size_t size) {
  const size_t middle = size / 2;
  size_t out = 0;
  size_t i = 0;
  size_t j = middle;

  /* don't bother sorting an array of size <= 1 */
  if (size <= 1) {
    return;
  }

  if (size <= SMALL_SORT_BND) {
    BINARY_INSERTION_SORT(dst, size);
    return;
  }

  MERGE_SORT_RECURSIVE(newdst, dst, middle);
  MERGE_SORT_RECURSIVE(newdst, &dst[middle], size - middle);

  while (out != size) {
    if (i < middle) {
      if (j < size) {
        if (SORT_CMP(dst[i], dst[j]) <= 0) {
          newdst[out] = dst[i++];
        } else {
          newdst[out] = dst[j++];
        }
      } else {
        newdst[out] = dst[i++];
      }
    } else {
      newdst[out] = dst[j++];
    }

    out++;
  }

  SORT_TYPE_CPY(dst, newdst, size);
}

/* Standard merge sort */
void MERGE_SORT(SORT_TYPE *dst, const size_t size) {
  SORT_TYPE *newdst;

  /* don't bother sorting an array of size <= 1 */
  if (size <= 1) {
    return;
  }

  if (size <= SMALL_SORT_BND) {
    BINARY_INSERTION_SORT(dst, size);
    return;
  }

  newdst = SORT_NEW_BUFFER(size);
  MERGE_SORT_RECURSIVE(newdst, dst, size);
  SORT_DELETE_BUFFER(newdst);
}


static __inline size_t QUICK_SORT_PARTITION(SORT_TYPE *dst, const size_t left,
    const size_t right, const size_t pivot) {
  SORT_TYPE value = dst[pivot];
  size_t index = left;
  size_t i;
  int not_all_same = 0;
  /* move the pivot to the right */
  SORT_SWAP(dst[pivot], dst[right]);

  for (i = left; i < right; i++) {
    int cmp = SORT_CMP(dst[i], value);
    /* check if everything is all the same */
    not_all_same |= cmp;

    if (cmp < 0) {
      SORT_SWAP(dst[i], dst[index]);
      index++;
    }
  }

  SORT_SWAP(dst[right], dst[index]);

  /* avoid degenerate case */
  if (not_all_same == 0) {
    return SIZE_MAX;
  }

  return index;
}

/* Based on Knuth vol. 3
static __inline size_t QUICK_SORT_HOARE_PARTITION(SORT_TYPE *dst, const size_t l,
    const size_t r, const size_t pivot) {
  SORT_TYPE value;
  size_t i = l + 1;
  size_t j = r;

  if (pivot != l) {
    SORT_SWAP(dst[pivot], dst[l]);
  }
  value = dst[l];

  while (1) {
    while (SORT_CMP(dst[i], value) < 0) {
      i++;
    }
    while (SORT_CMP(value, dst[j]) < 0) {
      j--;
    }
    if (j <= i) {
      SORT_SWAP(dst[l], dst[j]);
      return j;
    }
    SORT_SWAP(dst[i], dst[j]);
    i++;
    j--;
  }
  return 0;
}
*/


/* Return the median index of the objects at the three indices. */
static __inline size_t MEDIAN(const SORT_TYPE *dst, const size_t a, const size_t b,
                              const size_t c) {
  const int AB = SORT_CMP(dst[a], dst[b]) < 0;

  if (AB) {
    /* a < b */
    const int BC = SORT_CMP(dst[b], dst[c]) < 0;

    if (BC) {
      /* a < b < c */
      return b;
    } else {
      /* a < b, c < b */
      const int AC = SORT_CMP(dst[a], dst[c]) < 0;

      if (AC) {
        /* a < c < b */
        return c;
      } else {
        /* c < a < b */
        return a;
      }
    }
  } else {
    /* b < a */
    const int AC = SORT_CMP(dst[a], dst[b]) < 0;

    if (AC) {
      /* b < a < c */
      return a;
    } else {
      /* b < a, c < a */
      const int BC = SORT_CMP(dst[b], dst[c]) < 0;

      if (BC) {
        /* b < c < a */
        return c;
      } else {
        /* c < b < a */
        return b;
      }
    }
  }
}

static void QUICK_SORT_RECURSIVE(SORT_TYPE *dst, const size_t original_left,
                                 const size_t original_right) {
  size_t left;
  size_t right;
  size_t pivot;
  size_t new_pivot;
  size_t middle;
  int loop_count = 0;
  const int max_loops = 64 - CLZ(original_right - original_left); /* ~lg N */
  left = original_left;
  right = original_right;

  while (1) {
    if (right <= left) {
      return;
    }

    if ((right - left + 1U) <= SMALL_SORT_BND) {
      SMALL_SORT(&dst[left], right - left + 1U);
      return;
    }

    if (++loop_count >= max_loops) {
      /* we have recursed / looped too many times; switch to heap sort */
      HEAP_SORT(&dst[left], right - left + 1U);
      return;
    }

    /* median of 5 */
    middle = left + ((right - left) >> 1);
    pivot = MEDIAN((const SORT_TYPE *) dst, left, middle, right);
    pivot = MEDIAN((const SORT_TYPE *) dst, left + ((middle - left) >> 1), pivot,
                   middle + ((right - middle) >> 1));
    new_pivot = QUICK_SORT_PARTITION(dst, left, right, pivot);

    /* check for partition all equal */
    if (new_pivot == SIZE_MAX) {
      return;
    }

    /* recurse only on the small part to avoid degenerate stack sizes */
    /* and manually do tail call on the large part */
    if (new_pivot - 1U - left > right - new_pivot - 1U) {
      /* left is bigger than right */
      QUICK_SORT_RECURSIVE(dst, new_pivot + 1U, right);
      /* tail call for left */
      right = new_pivot - 1U;
    } else {
      /* right is bigger than left */
      QUICK_SORT_RECURSIVE(dst, left, new_pivot - 1U);
      /* tail call for right */
      left = new_pivot + 1U;
    }
  }
}

void QUICK_SORT(SORT_TYPE *dst, const size_t size) {
  /* don't bother sorting an array of size 1 */
  if (size <= 1) {
    return;
  }

  QUICK_SORT_RECURSIVE(dst, 0U, size - 1U);
}


/* timsort implementation, based on timsort.txt */

static __inline void REVERSE_ELEMENTS(SORT_TYPE *dst, size_t start, size_t end) {
  while (1) {
    if (start >= end) {
      return;
    }

    SORT_SWAP(dst[start], dst[end]);
    start++;
    end--;
  }
}

static size_t COUNT_RUN(SORT_TYPE *dst, const size_t start, const size_t size) {
  size_t curr;

  if (size - start == 1) {
    return 1;
  }

  if (start >= size - 2) {
    if (SORT_CMP(dst[size - 2], dst[size - 1]) > 0) {
      SORT_SWAP(dst[size - 2], dst[size - 1]);
    }

    return 2;
  }

  curr = start + 2;

  if (SORT_CMP(dst[start], dst[start + 1]) <= 0) {
    /* increasing run */
    while (1) {
      if (curr == size - 1) {
        break;
      }

      if (SORT_CMP(dst[curr - 1], dst[curr]) > 0) {
        break;
      }

      curr++;
    }

    return curr - start;
  } else {
    /* decreasing run */
    while (1) {
      if (curr == size - 1) {
        break;
      }

      if (SORT_CMP(dst[curr - 1], dst[curr]) <= 0) {
        break;
      }

      curr++;
    }

    /* reverse in-place */
    REVERSE_ELEMENTS(dst, start, curr - 1);
    return curr - start;
  }
}

static int CHECK_INVARIANT(TIM_SORT_RUN_T *stack, const int stack_curr) {
  size_t A, B, C;

  if (stack_curr < 2) {
    return 1;
  }

  if (stack_curr == 2) {
    const size_t A1 = stack[stack_curr - 2].length;
    const size_t B1 = stack[stack_curr - 1].length;

    if (A1 <= B1) {
      return 0;
    }

    return 1;
  }

  A = stack[stack_curr - 3].length;
  B = stack[stack_curr - 2].length;
  C = stack[stack_curr - 1].length;

  if ((A <= B + C) || (B <= C)) {
    return 0;
  }

  return 1;
}

typedef struct {
  size_t alloc;
  SORT_TYPE *storage;
} TEMP_STORAGE_T;

static void TIM_SORT_RESIZE(TEMP_STORAGE_T *store, const size_t new_size) {
  if ((store->storage == NULL) || (store->alloc < new_size)) {
    SORT_TYPE *tempstore = (SORT_TYPE *)realloc(store->storage, new_size * sizeof(SORT_TYPE));

    if (tempstore == NULL) {
      fprintf(stderr, "Error allocating temporary storage for tim sort: need %lu bytes",
              (unsigned long)(sizeof(SORT_TYPE) * new_size));
      exit(1);
    }

    store->storage = tempstore;
    store->alloc = new_size;
  }
}


static size_t TIM_SORT_GALLOP(SORT_TYPE *dst, const size_t size, const SORT_TYPE key, size_t anchor,
                              int right) {
  int last_ofs = 0;
  int ofs, max_ofs, ofs_sign, cmp;
  size_t  l, c, r;
  cmp = SORT_CMP(key, dst[anchor]);

  if (cmp < 0 || (!right && cmp == 0)) {
    /* short cut */
    if (anchor == 0) {
      return 0;
    }

    ofs = -1;
    ofs_sign = -1;
    max_ofs = -(int)anchor; /* ensure anchor+max_ofs is valid idx */
  } else {
    if (anchor == size - 1) {
      return size;
    }

    ofs = 1;
    ofs_sign = 1;
    max_ofs = (int)(size - anchor - 1);
  }

  for (;;) {
    /* deal with overflow */
    if (max_ofs / ofs <= 1) {
      ofs = max_ofs;

      if (ofs < 0) {
        cmp = SORT_CMP(key, dst[0]);

        if ((right && cmp < 0) || (!right && cmp <= 0)) {
          return 0;
        }
      } else {
        cmp = SORT_CMP(dst[size - 1], key);

        if ((right && cmp <= 0) || (!right && cmp < 0)) {
          return size;
        }
      }

      break;
    }

    c = anchor + ofs;
    /* right, 0<ofs:  dst[anchor+last_ofs] <=  key  <  dst[anchor+ofs]      */
    /* left,  0<ofs:  dst[anchor+last_ofs] <   key  <= dst[anchor+ofs]      */
    /* right, ofs<0:  dst[anchor+ofs]      <=  key  <  dst[anchor+last_ofs] */
    /* left,  ofs<0:  dst[anchor+ofs]      <   key  <= dst[anchor+last_ofs] */
    cmp = SORT_CMP(key, dst[c]);

    if (0 < ofs) {
      if ((right && cmp < 0) || (!right && cmp <= 0)) {
        break;
      }
    } else {
      if ((right && 0 <= cmp) || (!right && 0 < cmp)) {
        break;
      }
    }

    last_ofs = ofs;
    ofs = (ofs << 1) + ofs_sign;
  }

  /* key in region (l, r) , both l and r have already been compared */
  if (ofs < 0) {
    l = anchor + ofs;
    r = anchor + last_ofs;
  } else {
    l = anchor + last_ofs;
    r = anchor + ofs;
  }

  while (1 < r - l) {
    c = l + ((r - l) >> 1);
    cmp = SORT_CMP(key, dst[c]);

    if ((right && cmp < 0) || (!right && cmp <= 0)) {
      r = c;
    } else {
      l = c;
    }
  }

  return r;
}



static void TIM_SORT_MERGE_LEFT(SORT_TYPE *A_src, SORT_TYPE *B_src, const size_t A, const size_t B,
                                SORT_TYPE* storage, int *min_gallop_p) {
  size_t pdst, pa, pb, k;
  int a_count, b_count;
  int min_gallop = *min_gallop_p;
  SORT_TYPE *dst = A_src;
  SORT_TYPE_CPY(storage, dst, A);
  A_src = storage;
  pdst = pa = pb = 0;
  /* first element must in B, otherwise skipped in the caller  */
  dst[pdst++] = B_src[pb++];

  if (B == 1) {
    goto copyA;
  }

  for (;;) {
    a_count = b_count = 0;

    for (;;) {
      if (SORT_CMP(A_src[pa], B_src[pb]) <= 0) {
        dst[pdst++] = A_src[pa++];
        ++a_count;
        b_count = 0;

        /* No need to check if pa == A because the last element must be in A
         * so pb will reach to B first. You can check pa == A-1 and do
         * some optimization if you wish.*/
        if (min_gallop <= a_count) {
          break;
        }
      } else {
        dst[pdst++] = B_src[pb++];
        ++b_count;
        a_count = 0;

        if (pb == B) {
          goto copyA;
        }

        if (min_gallop <= b_count) {
          break;
        }
      }
    }

    ++min_gallop;

    for (;;) {
      if (min_gallop != 0) {
        min_gallop --;
      }

      k = TIM_SORT_GALLOP(&A_src[pa], A - pa, B_src[pb], 0, 1);
      SORT_TYPE_CPY(&dst[pdst], &A_src[pa], k);
      pdst += k;
      pa += k;
      /* now we know the next must be in B */
      dst[pdst++] = B_src[pb++];

      if (pb == B) {
        goto copyA;
      }

      if (a_count && k < TIM_SORT_MIN_GALLOP) {
        ++min_gallop;
        break;
      }

      k = TIM_SORT_GALLOP(&B_src[pb], B - pb, A_src[pa], 0, 0);
      SORT_TYPE_MOVE(&dst[pdst], &B_src[pb], k);
      pdst += k;
      pb += k;

      if (pb == B) {
        goto copyA;
      }

      dst[pdst++] = A_src[pa++];

      if (b_count && k < TIM_SORT_MIN_GALLOP) {
        ++min_gallop;
        break;
      }
    }
  }

copyA:
  SORT_TYPE_CPY(&dst[pdst], &A_src[pa], A - pa);
  *min_gallop_p = min_gallop;
  return;
}


static void TIM_SORT_MERGE_RIGHT(SORT_TYPE *A_src, SORT_TYPE *B_src, const size_t A, const size_t B,
                                 SORT_TYPE* storage, int *min_gallop_p) {
  size_t k;
  int pdst, pa, pb, a_count, b_count;
  int min_gallop = *min_gallop_p;
  SORT_TYPE *dst = A_src;
  pa = (int)(A - 1);
  pb = (int)(B - 1);
  pdst = (int)(A + B - 1);
  SORT_TYPE_CPY(storage, B_src, B);
  B_src = storage;
  /* last element must in A, otherwise skipped in the caller  */
  dst[pdst--] = A_src[pa--];

  if (A == 1) {
    goto copyB;
  }

  for (;;) {
    a_count = b_count = 0;

    for (;;) {
      if (SORT_CMP(A_src[pa], B_src[pb]) <= 0) {
        dst[pdst--] = B_src[pb--];
        ++b_count;
        a_count = 0;

        if (min_gallop <= b_count) {
          break;
        }

        /* No need to check if pb == -1 because the first element must be in B
         * so pa will reach to -1 first. You can check pb == 0 and do
         * some optimization if you wish.*/
      } else {
        dst[pdst--] = A_src[pa--];
        ++a_count;
        b_count = 0;

        if (pa == -1) {
          goto copyB;
        }

        if (min_gallop <= a_count) {
          break;
        }
      }
    }

    ++min_gallop;

    for (;;) {
      if (min_gallop != 0) {
        min_gallop --;
      }

      k = TIM_SORT_GALLOP(A_src, pa + 1, B_src[pb], pa, 1);
      /* Understand the margin by considering k==0 */
      SORT_TYPE_MOVE(&dst[pb + k + 1], &A_src[k], pa + 1 - k);
      pdst = pb + (int)k;
      pa = (int)(k - 1);

      if (pa == -1) {
        goto copyB;
      }

      /* now we know the next must be in B */
      dst[pdst--] = B_src[pb--];

      if (a_count && pa + 1 - k < TIM_SORT_MIN_GALLOP) {
        ++min_gallop;
        break;
      }

      k = TIM_SORT_GALLOP(B_src, pb + 1, A_src[pa], pb, 0);
      SORT_TYPE_CPY(&dst[pa + k + 1], &B_src[k], pb + 1 - k);
      pdst = pa + (int)k;
      pb = (int)(k - 1);
      dst[pdst--] = A_src[pa--];

      if (pa == -1) {
        goto copyB;
      }

      if (b_count && pb + 1 - k < TIM_SORT_MIN_GALLOP) {
        ++min_gallop;
        break;
      }
    }
  }

copyB:
  SORT_TYPE_CPY(dst, B_src, pb + 1);
  *min_gallop_p = min_gallop;
  return;
}


static void TIM_SORT_MERGE(SORT_TYPE *dst, const TIM_SORT_RUN_T *stack, const int stack_curr,
                           TEMP_STORAGE_T *store, int* min_gallop_p) {
  size_t A = stack[stack_curr - 2].length;
  size_t B = stack[stack_curr - 1].length;
  size_t A_start = stack[stack_curr - 2].start;
  size_t B_start = stack[stack_curr - 1].start;
  SORT_TYPE *storage;
  size_t k;
  /* A[k-1] <= B[0] < A[k] */
  k = TIM_SORT_GALLOP(&dst[A_start], A, dst[B_start], 0, 1);
  A_start += k;
  A -= k;

  if (A == 0) {
    *min_gallop_p /= 2;
    return;
  }

  /* B[k-1] < A[A-1] <= B[k] */
  k = TIM_SORT_GALLOP(&dst[B_start], B, dst[B_start - 1], B - 1, 0);
  B = k;
  TIM_SORT_RESIZE(store, MIN(A, B));
  storage = store->storage;

  if (A < B) {
    TIM_SORT_MERGE_LEFT(&dst[A_start], &dst[B_start], A, B, storage, min_gallop_p);
  } else {
    TIM_SORT_MERGE_RIGHT(&dst[A_start], &dst[B_start], A, B, storage, min_gallop_p);
  }
}

static int TIM_SORT_COLLAPSE(SORT_TYPE *dst, TIM_SORT_RUN_T *stack, int stack_curr,
                             TEMP_STORAGE_T *store, const size_t size, int* min_gallop_p) {
  while (1) {
    size_t A, B, C, D;
    int ABC, BCD, CD;

    /* if the stack only has one thing on it, we are done with the collapse */
    if (stack_curr <= 1) {
      break;
    }

    /* if this is the last merge, just do it */
    if ((stack_curr == 2) && (stack[0].length + stack[1].length == size)) {
      TIM_SORT_MERGE(dst, stack, stack_curr, store, min_gallop_p);
      stack[0].length += stack[1].length;
      stack_curr--;
      break;
    }
    /* check if the invariant is off for a stack of 2 elements */
    else if ((stack_curr == 2) && (stack[0].length <= stack[1].length)) {
      TIM_SORT_MERGE(dst, stack, stack_curr, store, min_gallop_p);
      stack[0].length += stack[1].length;
      stack_curr--;
      break;
    } else if (stack_curr == 2) {
      break;
    }

    B = stack[stack_curr - 3].length;
    C = stack[stack_curr - 2].length;
    D = stack[stack_curr - 1].length;

    if (stack_curr >= 4) {
      A = stack[stack_curr - 4].length;
      ABC = (A <= B + C);
    } else {
      ABC = 0;
    }

    BCD = (B <= C + D) || ABC;
    CD = (C <= D);

    /* Both invariants are good */
    if (!BCD && !CD) {
      break;
    }

    /* left merge */
    if (BCD && !CD) {
      TIM_SORT_MERGE(dst, stack, stack_curr - 1, store, min_gallop_p);
      stack[stack_curr - 3].length += stack[stack_curr - 2].length;
      stack[stack_curr - 2] = stack[stack_curr - 1];
      stack_curr--;
    } else {
      /* right merge */
      TIM_SORT_MERGE(dst, stack, stack_curr, store, min_gallop_p);
      stack[stack_curr - 2].length += stack[stack_curr - 1].length;
      stack_curr--;
    }
  }

  return stack_curr;
}

static __inline int PUSH_NEXT(SORT_TYPE *dst,
                              const size_t size,
                              TEMP_STORAGE_T *store,
                              const size_t minrun,
                              TIM_SORT_RUN_T *run_stack,
                              size_t *stack_curr,
                              size_t *curr,
                              int *min_gallop_p) {
  size_t len = COUNT_RUN(dst, *curr, size);
  size_t run = minrun;

  if (run > size - *curr) {
    run = size - *curr;
  }

  if (run > len) {
    BINARY_INSERTION_SORT_START(&dst[*curr], len, run);
    len = run;
  }

  run_stack[*stack_curr].start = *curr;
  run_stack[*stack_curr].length = len;
  (*stack_curr)++;
  *curr += len;

  if (*curr == size) {
    /* finish up */
    while (*stack_curr > 1) {
      TIM_SORT_MERGE(dst, run_stack, (int)*stack_curr, store, min_gallop_p);
      run_stack[*stack_curr - 2].length += run_stack[*stack_curr - 1].length;
      (*stack_curr)--;
    }

    if (store->storage != NULL) {
      free(store->storage);
      store->storage = NULL;
    }

    return 0;
  }

  return 1;
}

void TIM_SORT(SORT_TYPE *dst, const size_t size) {
  size_t minrun;
  TEMP_STORAGE_T _store, *store;
  TIM_SORT_RUN_T run_stack[TIM_SORT_STACK_SIZE];
  size_t stack_curr = 0;
  size_t curr = 0;
  int min_gallop = TIM_SORT_MIN_GALLOP;

  /* don't bother sorting an array of size 1 */
  if (size <= 1) {
    return;
  }

  if (size < 64) {
    SMALL_SORT(dst, size);
    return;
  }

  /* compute the minimum run length */
  minrun = compute_minrun(size);
  /* temporary storage for merges */
  store = &_store;
  store->alloc = 0;
  store->storage = NULL;

  if (!PUSH_NEXT(dst, size, store, minrun, run_stack, &stack_curr, &curr, &min_gallop)) {
    return;
  }

  if (!PUSH_NEXT(dst, size, store, minrun, run_stack, &stack_curr, &curr, &min_gallop)) {
    return;
  }

  if (!PUSH_NEXT(dst, size, store, minrun, run_stack, &stack_curr, &curr, &min_gallop)) {
    return;
  }

  while (1) {
    if (!CHECK_INVARIANT(run_stack, (int)stack_curr)) {
      stack_curr = TIM_SORT_COLLAPSE(dst, run_stack, (int)stack_curr, store, size, &min_gallop);
      continue;
    }

    if (!PUSH_NEXT(dst, size, store, minrun, run_stack, &stack_curr, &curr, &min_gallop)) {
      return;
    }
  }
}

/* heap sort: based on wikipedia */

static __inline void HEAP_SIFT_DOWN(SORT_TYPE *dst, const size_t start, const size_t end) {
  size_t root = start;

  while ((root << 1) <= end) {
    size_t child = root << 1;

    if ((child < end) && (SORT_CMP(dst[child], dst[child + 1]) < 0)) {
      child++;
    }

    if (SORT_CMP(dst[root], dst[child]) < 0) {
      SORT_SWAP(dst[root], dst[child]);
      root = child;
    } else {
      return;
    }
  }
}

static __inline void HEAPIFY(SORT_TYPE *dst, const size_t size) {
  size_t start = size >> 1;

  while (1) {
    HEAP_SIFT_DOWN(dst, start, size - 1);

    if (start == 0) {
      break;
    }

    start--;
  }
}

void HEAP_SORT(SORT_TYPE *dst, const size_t size) {
  size_t end = size - 1;

  /* don't bother sorting an array of size <= 1 */
  if (size <= 1) {
    return;
  }

  HEAPIFY(dst, size);

  while (end > 0) {
    SORT_SWAP(dst[end], dst[0]);
    HEAP_SIFT_DOWN(dst, 0, end - 1);
    end--;
  }
}

/********* Sqrt sorting *********************************/
/*                                                       */
/* (c) 2014 by Andrey Astrelin                           */
/*                                                       */
/*                                                       */
/* Stable sorting that works in O(N*log(N)) worst time   */
/* and uses O(sqrt(N)) extra memory                      */
/*                                                       */
/* Define SORT_TYPE and SORT_CMP                         */
/* and then call SqrtSort() function                     */
/*                                                       */
/*********************************************************/

#define SORT_CMP_A(a,b) SORT_CMP(*(a),*(b))

static __inline void SQRT_SORT_SWAP_1(SORT_TYPE *a, SORT_TYPE *b) {
  SORT_TYPE c = *a;
  *a++ = *b;
  *b++ = c;
}

static __inline void SQRT_SORT_SWAP_N(SORT_TYPE *a, SORT_TYPE *b, int n) {
  while (n--) {
    SQRT_SORT_SWAP_1(a++, b++);
  }
}


static void SQRT_SORT_MERGE_RIGHT(SORT_TYPE *arr, int L1, int L2, int M) {
  int p0 = L1 + L2 + M - 1, p2 = L1 + L2 - 1, p1 = L1 - 1;

  while (p1 >= 0) {
    if (p2 < L1 || SORT_CMP_A(arr + p1, arr + p2) > 0) {
      arr[p0--] = arr[p1--];
    } else {
      arr[p0--] = arr[p2--];
    }
  }

  if (p2 != p0) while (p2 >= L1) {
      arr[p0--] = arr[p2--];
    }
}

/* arr[M..-1] - free, arr[0,L1-1]++arr[L1,L1+L2-1] -> arr[M,M+L1+L2-1] */
static void SQRT_SORT_MERGE_LEFT_WITH_X_BUF(SORT_TYPE *arr, int L1, int L2, int M) {
  int p0 = 0, p1 = L1;
  L2 += L1;

  while (p1 < L2) {
    if (p0 == L1 || SORT_CMP_A(arr + p0, arr + p1) > 0) {
      arr[M++] = arr[p1++];
    } else {
      arr[M++] = arr[p0++];
    }
  }

  if (M != p0) while (p0 < L1) {
      arr[M++] = arr[p0++];
    }
}

/* arr[0,L1-1] ++ arr2[0,L2-1] -> arr[-L1,L2-1],  arr2 is "before" arr1 */
static void SQRT_SORT_MERGE_DOWN(SORT_TYPE *arr, SORT_TYPE *arr2, int L1, int L2) {
  int p0 = 0, p1 = 0, M = -L2;

  while (p1 < L2) {
    if (p0 == L1 || SORT_CMP_A(arr + p0, arr2 + p1) >= 0) {
      arr[M++] = arr2[p1++];
    } else {
      arr[M++] = arr[p0++];
    }
  }

  if (M != p0) while (p0 < L1) {
      arr[M++] = arr[p0++];
    }
}

static void SQRT_SORT_SMART_MERGE_WITH_X_BUF(SORT_TYPE *arr, int *alen1, int *atype, int len2,
    int lkeys) {
  int p0 = -lkeys, p1 = 0, p2 = *alen1, q1 = p2, q2 = p2 + len2;
  int ftype = 1 - *atype; /* 1 if inverted */

  while (p1 < q1 && p2 < q2) {
    if (SORT_CMP_A(arr + p1, arr + p2) - ftype < 0) {
      arr[p0++] = arr[p1++];
    } else {
      arr[p0++] = arr[p2++];
    }
  }

  if (p1 < q1) {
    *alen1 = q1 - p1;

    while (p1 < q1) {
      arr[--q2] = arr[--q1];
    }
  } else {
    *alen1 = q2 - p2;
    *atype = ftype;
  }
}


/*
  arr - starting array. arr[-lblock..-1] - buffer (if havebuf).
  lblock - length of regular blocks. First nblocks are stable sorted by 1st elements and key-coded
  keys - arrays of keys, in same order as blocks. key<midkey means stream A
  nblock2 are regular blocks from stream A. llast is length of last (irregular) block from stream B, that should go before nblock2 blocks.
  llast=0 requires nblock2=0 (no irregular blocks). llast>0, nblock2=0 is possible.
*/
static void SQRT_SORT_MERGE_BUFFERS_LEFT_WITH_X_BUF(int *keys, int midkey, SORT_TYPE *arr,
    int nblock, int lblock, int nblock2, int llast) {
  int l, prest, lrest, frest, pidx, cidx, fnext;

  if (nblock == 0) {
    l = nblock2 * lblock;
    SQRT_SORT_MERGE_LEFT_WITH_X_BUF(arr, l, llast, -lblock);
    return;
  }

  lrest = lblock;
  frest = keys[0] < midkey ? 0 : 1;
  pidx = lblock;

  for (cidx = 1; cidx < nblock; cidx++, pidx += lblock) {
    prest = pidx - lrest;
    fnext = keys[cidx] < midkey ? 0 : 1;

    if (fnext == frest) {
      SORT_TYPE_CPY(arr + prest - lblock, arr + prest, lrest);
      prest = pidx;
      lrest = lblock;
    } else {
      SQRT_SORT_SMART_MERGE_WITH_X_BUF(arr + prest, &lrest, &frest, lblock, lblock);
    }
  }

  prest = pidx - lrest;

  if (llast) {
    if (frest) {
      SORT_TYPE_CPY(arr + prest - lblock, arr + prest, lrest);
      prest = pidx;
      lrest = lblock * nblock2;
      frest = 0;
    } else {
      lrest += lblock * nblock2;
    }

    SQRT_SORT_MERGE_LEFT_WITH_X_BUF(arr + prest, lrest, llast, -lblock);
  } else {
    SORT_TYPE_CPY(arr + prest - lblock, arr + prest, lrest);
  }
}

/*
  build blocks of length K
  input: [-K,-1] elements are buffer
  output: first K elements are buffer, blocks 2*K and last subblock sorted
*/
static void SQRT_SORT_BUILD_BLOCKS(SORT_TYPE *arr, int L, int K) {
  int m, u, h, p0, p1, rest, restk, p;

  for (m = 1; m < L; m += 2) {
    u = 0;

    if (SORT_CMP_A(arr + (m - 1), arr + m) > 0) {
      u = 1;
    }

    arr[m - 3] = arr[m - 1 + u];
    arr[m - 2] = arr[m - u];
  }

  if (L % 2) {
    arr[L - 3] = arr[L - 1];
  }

  arr -= 2;

  for (h = 2; h < K; h *= 2) {
    p0 = 0;
    p1 = L - 2 * h;

    while (p0 <= p1) {
      SQRT_SORT_MERGE_LEFT_WITH_X_BUF(arr + p0, h, h, -h);
      p0 += 2 * h;
    }

    rest = L - p0;

    if (rest > h) {
      SQRT_SORT_MERGE_LEFT_WITH_X_BUF(arr + p0, h, rest - h, -h);
    } else {
      for (; p0 < L; p0++) {
        arr[p0 - h] = arr[p0];
      }
    }

    arr -= h;
  }

  restk = L % (2 * K);
  p = L - restk;

  if (restk <= K) {
    SORT_TYPE_CPY(arr + p + K, arr + p, restk);
  } else {
    SQRT_SORT_MERGE_RIGHT(arr + p, K, restk - K, K);
  }

  while (p > 0) {
    p -= 2 * K;
    SQRT_SORT_MERGE_RIGHT(arr + p, K, K, K);
  }
}


static void SQRT_SORT_SORT_INS(SORT_TYPE *arr, int len) {
  int i, j;

  for (i = 1; i < len; i++) {
    for (j = i - 1; j >= 0 && SORT_CMP_A(arr + (j + 1), arr + j) < 0; j--) {
      SQRT_SORT_SWAP_1(arr + j, arr + (j + 1));
    }
  }
}

/*
  keys are on the left of arr. Blocks of length LL combined. We'll combine them in pairs
  LL and nkeys are powers of 2. (2*LL/lblock) keys are guarantied
*/
static void SQRT_SORT_COMBINE_BLOCKS(SORT_TYPE *arr, int len, int LL, int lblock, int *tags) {
  int M, b, NBlk, midkey, lrest, u, i, p, v, kc, nbl2, llast;
  SORT_TYPE *arr1;
  M = len / (2 * LL);
  lrest = len % (2 * LL);

  if (lrest <= LL) {
    len -= lrest;
    lrest = 0;
  }

  for (b = 0; b <= M; b++) {
    if (b == M && lrest == 0) {
      break;
    }

    arr1 = arr + b * 2 * LL;
    NBlk = (b == M ? lrest : 2 * LL) / lblock;
    u = NBlk + (b == M ? 1 : 0);

    for (i = 0; i <= u; i++) {
      tags[i] = i;
    }

    midkey = LL / lblock;

    for (u = 1; u < NBlk; u++) {
      p = u - 1;

      for (v = u; v < NBlk; v++) {
        kc = SORT_CMP_A(arr1 + p * lblock, arr1 + v * lblock);

        if (kc > 0 || (kc == 0 && tags[p] > tags[v])) {
          p = v;
        }
      }

      if (p != u - 1) {
        SQRT_SORT_SWAP_N(arr1 + (u - 1)*lblock, arr1 + p * lblock, lblock);
        i = tags[u - 1];
        tags[u - 1] = tags[p];
        tags[p] = i;
      }
    }

    nbl2 = llast = 0;

    if (b == M) {
      llast = lrest % lblock;
    }

    if (llast != 0) {
      while (nbl2 < NBlk && SORT_CMP_A(arr1 + NBlk * lblock, arr1 + (NBlk - nbl2 - 1)*lblock) < 0) {
        nbl2++;
      }
    }

    SQRT_SORT_MERGE_BUFFERS_LEFT_WITH_X_BUF(tags, midkey, arr1, NBlk - nbl2, lblock, nbl2, llast);
  }

  for (p = len; --p >= 0;) {
    arr[p] = arr[p - lblock];
  }
}


static void SQRT_SORT_COMMON_SORT(SORT_TYPE *arr, int Len, SORT_TYPE *extbuf, int *Tags) {
  int lblock, cbuf;

  if (Len < 16) {
    SQRT_SORT_SORT_INS(arr, Len);
    return;
  }

  lblock = 1;

  while (lblock * lblock < Len) {
    lblock *= 2;
  }

  SORT_TYPE_CPY(extbuf, arr, lblock);
  SQRT_SORT_COMMON_SORT(extbuf, lblock, arr, Tags);
  SQRT_SORT_BUILD_BLOCKS(arr + lblock, Len - lblock, lblock);
  cbuf = lblock;

  while (Len > (cbuf *= 2)) {
    SQRT_SORT_COMBINE_BLOCKS(arr + lblock, Len - lblock, cbuf, lblock, Tags);
  }

  SQRT_SORT_MERGE_DOWN(arr + lblock, extbuf, Len - lblock, lblock);
}

void SQRT_SORT(SORT_TYPE *arr, size_t Len) {
  int L = 1;
  SORT_TYPE *ExtBuf;
  int *Tags;
  int NK;

  while (L * L < Len) {
    L *= 2;
  }

  NK = (int)((Len - 1) / L + 2);
  ExtBuf = SORT_NEW_BUFFER(L);

  if (ExtBuf == NULL) {
    return;  /* fail */
  }

  Tags = (int*)malloc(NK * sizeof(int));

  if (Tags == NULL) {
    return;
  }

  SQRT_SORT_COMMON_SORT(arr, (int)Len, ExtBuf, Tags);
  free(Tags);
  SORT_DELETE_BUFFER(ExtBuf);
}

/********* Grail sorting *********************************/
/*                                                       */
/* (c) 2013 by Andrey Astrelin                           */
/*                                                       */
/*                                                       */
/* Stable sorting that works in O(N*log(N)) worst time   */
/* and uses O(1) extra memory                            */
/*                                                       */
/* Define SORT_TYPE and SORT_CMP                         */
/* and then call GrailSort() function                    */
/*                                                       */
/* For sorting with fixed external buffer (512 items)    */
/* use GrailSortWithBuffer()                             */
/*                                                       */
/* For sorting with dynamic external buffer (O(sqrt(N)) items) */
/* use GrailSortWithDynBuffer()                          */
/*                                                       */
/* Also classic in-place merge sort is implemented       */
/* under the name of RecStableSort()                     */
/*                                                       */
/*********************************************************/

#define GRAIL_EXT_BUFFER_LENGTH 512

static __inline void GRAIL_SWAP1(SORT_TYPE *a, SORT_TYPE *b) {
  SORT_TYPE c = *a;
  *a = *b;
  *b = c;
}

static __inline void GRAIL_SWAP_N(SORT_TYPE *a, SORT_TYPE *b, int n) {
  while (n--) {
    GRAIL_SWAP1(a++, b++);
  }
}

static void GRAIL_ROTATE(SORT_TYPE *a, int l1, int l2) {
  while (l1 && l2) {
    if (l1 <= l2) {
      GRAIL_SWAP_N(a, a + l1, l1);
      a += l1;
      l2 -= l1;
    } else {
      GRAIL_SWAP_N(a + (l1 - l2), a + l1, l2);
      l1 -= l2;
    }
  }
}

static int GRAIL_BIN_SEARCH_LEFT(SORT_TYPE *arr, int len, SORT_TYPE *key) {
  int a = -1, b = len, c;

  while (a < b - 1) {
    c = a + ((b - a) >> 1);

    if (SORT_CMP_A(arr + c, key) >= 0) {
      b = c;
    } else {
      a = c;
    }
  }

  return b;
}
static int GRAIL_BIN_SEARCH_RIGHT(SORT_TYPE *arr, int len, SORT_TYPE *key) {
  int a = -1, b = len, c;

  while (a < b - 1) {
    c = a + ((b - a) >> 1);

    if (SORT_CMP_A(arr + c, key) > 0) {
      b = c;
    } else {
      a = c;
    }
  }

  return b;
}

/* cost: 2*len+nk^2/2 */
static int GRAIL_FIND_KEYS(SORT_TYPE *arr, int len, int nkeys) {
  int h = 1, h0 = 0; /* first key is always here */
  int u = 1, r;

  while (u < len && h < nkeys) {
    r = GRAIL_BIN_SEARCH_LEFT(arr + h0, h, arr + u);

    if (r == h || SORT_CMP_A(arr + u, arr + (h0 + r)) != 0) {
      GRAIL_ROTATE(arr + h0, h, u - (h0 + h));
      h0 = u - h;
      GRAIL_ROTATE(arr + (h0 + r), h - r, 1);
      h++;
    }

    u++;
  }

  GRAIL_ROTATE(arr, h0, h);
  return h;
}

/* cost: min(L1,L2)^2+max(L1,L2) */
static void GRAIL_MERGE_WITHOUT_BUFFER(SORT_TYPE *arr, int len1, int len2) {
  int h;

  if (len1 < len2) {
    while (len1) {
      h = GRAIL_BIN_SEARCH_LEFT(arr + len1, len2, arr);

      if (h != 0) {
        GRAIL_ROTATE(arr, len1, h);
        arr += h;
        len2 -= h;
      }

      if (len2 == 0) {
        break;
      }

      do {
        arr++;
        len1--;
      } while (len1 && SORT_CMP_A(arr, arr + len1) <= 0);
    }
  } else {
    while (len2) {
      h = GRAIL_BIN_SEARCH_RIGHT(arr, len1, arr + (len1 + len2 - 1));

      if (h != len1) {
        GRAIL_ROTATE(arr + h, len1 - h, len2);
        len1 = h;
      }

      if (len1 == 0) {
        break;
      }

      do {
        len2--;
      } while (len2 && SORT_CMP_A(arr + len1 - 1, arr + len1 + len2 - 1) <= 0);
    }
  }
}

/* arr[M..-1] - buffer, arr[0,L1-1]++arr[L1,L1+L2-1] -> arr[M,M+L1+L2-1] */
static void GRAIL_MERGE_LEFT(SORT_TYPE *arr, int L1, int L2, int M) {
  int p0 = 0, p1 = L1;
  L2 += L1;

  while (p1 < L2) {
    if (p0 == L1 || SORT_CMP_A(arr + p0, arr + p1) > 0) {
      GRAIL_SWAP1(arr + (M++), arr + (p1++));
    } else {
      GRAIL_SWAP1(arr + (M++), arr + (p0++));
    }
  }

  if (M != p0) {
    GRAIL_SWAP_N(arr + M, arr + p0, L1 - p0);
  }
}
static void GRAIL_MERGE_RIGHT(SORT_TYPE *arr, int L1, int L2, int M) {
  int p0 = L1 + L2 + M - 1, p2 = L1 + L2 - 1, p1 = L1 - 1;

  while (p1 >= 0) {
    if (p2 < L1 || SORT_CMP_A(arr + p1, arr + p2) > 0) {
      GRAIL_SWAP1(arr + (p0--), arr + (p1--));
    } else {
      GRAIL_SWAP1(arr + (p0--), arr + (p2--));
    }
  }

  if (p2 != p0) while (p2 >= L1) {
      GRAIL_SWAP1(arr + (p0--), arr + (p2--));
    }
}

static void GRAIL_SMART_MERGE_WITH_BUFFER(SORT_TYPE *arr, int *alen1, int *atype, int len2,
    int lkeys) {
  int p0 = -lkeys, p1 = 0, p2 = *alen1, q1 = p2, q2 = p2 + len2;
  int ftype = 1 - *atype; /* 1 if inverted */

  while (p1 < q1 && p2 < q2) {
    if (SORT_CMP_A(arr + p1, arr + p2) - ftype < 0) {
      GRAIL_SWAP1(arr + (p0++), arr + (p1++));
    } else {
      GRAIL_SWAP1(arr + (p0++), arr + (p2++));
    }
  }

  if (p1 < q1) {
    *alen1 = q1 - p1;

    while (p1 < q1) {
      GRAIL_SWAP1(arr + (--q1), arr + (--q2));
    }
  } else {
    *alen1 = q2 - p2;
    *atype = ftype;
  }
}
static void GRAIL_SMART_MERGE_WITHOUT_BUFFER(SORT_TYPE *arr, int *alen1, int *atype, int _len2) {
  int len1, len2, ftype, h;

  if (!_len2) {
    return;
  }

  len1 = *alen1;
  len2 = _len2;
  ftype = 1 - *atype;

  if (len1 && SORT_CMP_A(arr + (len1 - 1), arr + len1) - ftype >= 0) {
    while (len1) {
      h = ftype ? GRAIL_BIN_SEARCH_LEFT(arr + len1, len2, arr) : GRAIL_BIN_SEARCH_RIGHT(arr + len1, len2,
          arr);

      if (h != 0) {
        GRAIL_ROTATE(arr, len1, h);
        arr += h;
        len2 -= h;
      }

      if (len2 == 0) {
        *alen1 = len1;
        return;
      }

      do {
        arr++;
        len1--;
      } while (len1 && SORT_CMP_A(arr, arr + len1) - ftype < 0);
    }
  }

  *alen1 = len2;
  *atype = ftype;
}

/***** Sort With Extra Buffer *****/

/* arr[M..-1] - free, arr[0,L1-1]++arr[L1,L1+L2-1] -> arr[M,M+L1+L2-1] */
static void GRAIL_MERGE_LEFT_WITH_X_BUF(SORT_TYPE *arr, int L1, int L2, int M) {
  int p0 = 0, p1 = L1;
  L2 += L1;

  while (p1 < L2) {
    if (p0 == L1 || SORT_CMP_A(arr + p0, arr + p1) > 0) {
      arr[M++] = arr[p1++];
    } else {
      arr[M++] = arr[p0++];
    }
  }

  if (M != p0) while (p0 < L1) {
      arr[M++] = arr[p0++];
    }
}

static void GRAIL_SMART_MERGE_WITH_X_BUF(SORT_TYPE *arr, int *alen1, int *atype, int len2,
    int lkeys) {
  int p0 = -lkeys, p1 = 0, p2 = *alen1, q1 = p2, q2 = p2 + len2;
  int ftype = 1 - *atype; /* 1 if inverted */

  while (p1 < q1 && p2 < q2) {
    if (SORT_CMP_A(arr + p1, arr + p2) - ftype < 0) {
      arr[p0++] = arr[p1++];
    } else {
      arr[p0++] = arr[p2++];
    }
  }

  if (p1 < q1) {
    *alen1 = q1 - p1;

    while (p1 < q1) {
      arr[--q2] = arr[--q1];
    }
  } else {
    *alen1 = q2 - p2;
    *atype = ftype;
  }
}

/*
  arr - starting array. arr[-lblock..-1] - buffer (if havebuf).
  lblock - length of regular blocks. First nblocks are stable sorted by 1st elements and key-coded
  keys - arrays of keys, in same order as blocks. key<midkey means stream A
  nblock2 are regular blocks from stream A. llast is length of last (irregular) block from stream B, that should go before nblock2 blocks.
  llast=0 requires nblock2=0 (no irregular blocks). llast>0, nblock2=0 is possible.
*/
static void GRAIL_MERGE_BUFFERS_LEFT_WITH_X_BUF(SORT_TYPE *keys, SORT_TYPE *midkey, SORT_TYPE *arr,
    int nblock, int lblock, int nblock2, int llast) {
  int l, prest, lrest, frest, pidx, cidx, fnext;

  if (nblock == 0) {
    l = nblock2 * lblock;
    GRAIL_MERGE_LEFT_WITH_X_BUF(arr, l, llast, -lblock);
    return;
  }

  lrest = lblock;
  frest = SORT_CMP_A(keys, midkey) < 0 ? 0 : 1;
  pidx = lblock;

  for (cidx = 1; cidx < nblock; cidx++, pidx += lblock) {
    prest = pidx - lrest;
    fnext = SORT_CMP_A(keys + cidx, midkey) < 0 ? 0 : 1;

    if (fnext == frest) {
      SORT_TYPE_CPY(arr + prest - lblock, arr + prest, lrest);
      prest = pidx;
      lrest = lblock;
    } else {
      GRAIL_SMART_MERGE_WITH_X_BUF(arr + prest, &lrest, &frest, lblock, lblock);
    }
  }

  prest = pidx - lrest;

  if (llast) {
    if (frest) {
      SORT_TYPE_CPY(arr + prest - lblock, arr + prest, lrest);
      prest = pidx;
      lrest = lblock * nblock2;
      frest = 0;
    } else {
      lrest += lblock * nblock2;
    }

    GRAIL_MERGE_LEFT_WITH_X_BUF(arr + prest, lrest, llast, -lblock);
  } else {
    SORT_TYPE_CPY(arr + prest - lblock, arr + prest, lrest);
  }
}

/***** End Sort With Extra Buffer *****/

/*
  build blocks of length K
  input: [-K,-1] elements are buffer
  output: first K elements are buffer, blocks 2*K and last subblock sorted
*/
static void GRAIL_BUILD_BLOCKS(SORT_TYPE *arr, int L, int K, SORT_TYPE *extbuf, int LExtBuf) {
  int m, u, h, p0, p1, rest, restk, p, kbuf;
  kbuf = K < LExtBuf ? K : LExtBuf;

  while (kbuf & (kbuf - 1)) {
    kbuf &= kbuf - 1;  /* max power or 2 - just in case */
  }

  if (kbuf) {
    SORT_TYPE_CPY(extbuf, arr - kbuf, kbuf);

    for (m = 1; m < L; m += 2) {
      u = 0;

      if (SORT_CMP_A(arr + (m - 1), arr + m) > 0) {
        u = 1;
      }

      arr[m - 3] = arr[m - 1 + u];
      arr[m - 2] = arr[m - u];
    }

    if (L % 2) {
      arr[L - 3] = arr[L - 1];
    }

    arr -= 2;

    for (h = 2; h < kbuf; h *= 2) {
      p0 = 0;
      p1 = L - 2 * h;

      while (p0 <= p1) {
        GRAIL_MERGE_LEFT_WITH_X_BUF(arr + p0, h, h, -h);
        p0 += 2 * h;
      }

      rest = L - p0;

      if (rest > h) {
        GRAIL_MERGE_LEFT_WITH_X_BUF(arr + p0, h, rest - h, -h);
      } else {
        for (; p0 < L; p0++) {
          arr[p0 - h] = arr[p0];
        }
      }

      arr -= h;
    }

    SORT_TYPE_CPY(arr + L, extbuf, kbuf);
  } else {
    for (m = 1; m < L; m += 2) {
      u = 0;

      if (SORT_CMP_A(arr + (m - 1), arr + m) > 0) {
        u = 1;
      }

      GRAIL_SWAP1(arr + (m - 3), arr + (m - 1 + u));
      GRAIL_SWAP1(arr + (m - 2), arr + (m - u));
    }

    if (L % 2) {
      GRAIL_SWAP1(arr + (L - 1), arr + (L - 3));
    }

    arr -= 2;
    h = 2;
  }

  for (; h < K; h *= 2) {
    p0 = 0;
    p1 = L - 2 * h;

    while (p0 <= p1) {
      GRAIL_MERGE_LEFT(arr + p0, h, h, -h);
      p0 += 2 * h;
    }

    rest = L - p0;

    if (rest > h) {
      GRAIL_MERGE_LEFT(arr + p0, h, rest - h, -h);
    } else {
      GRAIL_ROTATE(arr + p0 - h, h, rest);
    }

    arr -= h;
  }

  restk = L % (2 * K);
  p = L - restk;

  if (restk <= K) {
    GRAIL_ROTATE(arr + p, restk, K);
  } else {
    GRAIL_MERGE_RIGHT(arr + p, K, restk - K, K);
  }

  while (p > 0) {
    p -= 2 * K;
    GRAIL_MERGE_RIGHT(arr + p, K, K, K);
  }
}

/*
  arr - starting array. arr[-lblock..-1] - buffer (if havebuf).
  lblock - length of regular blocks. First nblocks are stable sorted by 1st elements and key-coded
  keys - arrays of keys, in same order as blocks. key<midkey means stream A
  nblock2 are regular blocks from stream A. llast is length of last (irregular) block from stream B, that should go before nblock2 blocks.
  llast=0 requires nblock2=0 (no irregular blocks). llast>0, nblock2=0 is possible.
*/
static void GRAIL_MERGE_BUFFERS_LEFT(SORT_TYPE *keys, SORT_TYPE *midkey, SORT_TYPE *arr, int nblock,
                                     int lblock, int havebuf, int nblock2, int llast) {
  int l, prest, lrest, frest, pidx, cidx, fnext;

  if (nblock == 0) {
    l = nblock2 * lblock;

    if (havebuf) {
      GRAIL_MERGE_LEFT(arr, l, llast, -lblock);
    } else {
      GRAIL_MERGE_WITHOUT_BUFFER(arr, l, llast);
    }

    return;
  }

  lrest = lblock;
  frest = SORT_CMP_A(keys, midkey) < 0 ? 0 : 1;
  pidx = lblock;

  for (cidx = 1; cidx < nblock; cidx++, pidx += lblock) {
    prest = pidx - lrest;
    fnext = SORT_CMP_A(keys + cidx, midkey) < 0 ? 0 : 1;

    if (fnext == frest) {
      if (havebuf) {
        GRAIL_SWAP_N(arr + prest - lblock, arr + prest, lrest);
      }

      prest = pidx;
      lrest = lblock;
    } else {
      if (havebuf) {
        GRAIL_SMART_MERGE_WITH_BUFFER(arr + prest, &lrest, &frest, lblock, lblock);
      } else {
        GRAIL_SMART_MERGE_WITHOUT_BUFFER(arr + prest, &lrest, &frest, lblock);
      }
    }
  }

  prest = pidx - lrest;

  if (llast) {
    if (frest) {
      if (havebuf) {
        GRAIL_SWAP_N(arr + prest - lblock, arr + prest, lrest);
      }

      prest = pidx;
      lrest = lblock * nblock2;
      frest = 0;
    } else {
      lrest += lblock * nblock2;
    }

    if (havebuf) {
      GRAIL_MERGE_LEFT(arr + prest, lrest, llast, -lblock);
    } else {
      GRAIL_MERGE_WITHOUT_BUFFER(arr + prest, lrest, llast);
    }
  } else {
    if (havebuf) {
      GRAIL_SWAP_N(arr + prest, arr + (prest - lblock), lrest);
    }
  }
}

static void GRAIL_LAZY_STABLE_SORT(SORT_TYPE *arr, int L) {
  int m, h, p0, p1, rest;

  for (m = 1; m < L; m += 2) {
    if (SORT_CMP_A(arr + m - 1, arr + m) > 0) {
      GRAIL_SWAP1(arr + (m - 1), arr + m);
    }
  }

  for (h = 2; h < L; h *= 2) {
    p0 = 0;
    p1 = L - 2 * h;

    while (p0 <= p1) {
      GRAIL_MERGE_WITHOUT_BUFFER(arr + p0, h, h);
      p0 += 2 * h;
    }

    rest = L - p0;

    if (rest > h) {
      GRAIL_MERGE_WITHOUT_BUFFER(arr + p0, h, rest - h);
    }
  }
}

/*
  keys are on the left of arr. Blocks of length LL combined. We'll combine them in pairs
  LL and nkeys are powers of 2. (2*LL/lblock) keys are guarantied
*/
static void GRAIL_COMBINE_BLOCKS(SORT_TYPE *keys, SORT_TYPE *arr, int len, int LL, int lblock,
                                 int havebuf, SORT_TYPE *xbuf) {
  int M, b, NBlk, midkey, lrest, u, p, v, kc, nbl2, llast;
  SORT_TYPE *arr1;
  M = len / (2 * LL);
  lrest = len % (2 * LL);

  if (lrest <= LL) {
    len -= lrest;
    lrest = 0;
  }

  if (xbuf) {
    SORT_TYPE_CPY(xbuf, arr - lblock, lblock);
  }

  for (b = 0; b <= M; b++) {
    if (b == M && lrest == 0) {
      break;
    }

    arr1 = arr + b * 2 * LL;
    NBlk = (b == M ? lrest : 2 * LL) / lblock;
    SMALL_SORT(keys, NBlk + (b == M ? 1 : 0));
    midkey = LL / lblock;

    for (u = 1; u < NBlk; u++) {
      p = u - 1;

      for (v = u; v < NBlk; v++) {
        kc = SORT_CMP_A(arr1 + p * lblock, arr1 + v * lblock);

        if (kc > 0 || (kc == 0 && SORT_CMP_A(keys + p, keys + v) > 0)) {
          p = v;
        }
      }

      if (p != u - 1) {
        GRAIL_SWAP_N(arr1 + (u - 1)*lblock, arr1 + p * lblock, lblock);
        GRAIL_SWAP1(keys + (u - 1), keys + p);

        if (midkey == u - 1 || midkey == p) {
          midkey ^= (u - 1)^p;
        }
      }
    }

    nbl2 = llast = 0;

    if (b == M) {
      llast = lrest % lblock;
    }

    if (llast != 0) {
      while (nbl2 < NBlk && SORT_CMP_A(arr1 + NBlk * lblock, arr1 + (NBlk - nbl2 - 1)*lblock) < 0) {
        nbl2++;
      }
    }

    if (xbuf) {
      GRAIL_MERGE_BUFFERS_LEFT_WITH_X_BUF(keys, keys + midkey, arr1, NBlk - nbl2, lblock, nbl2, llast);
    } else {
      GRAIL_MERGE_BUFFERS_LEFT(keys, keys + midkey, arr1, NBlk - nbl2, lblock, havebuf, nbl2, llast);
    }
  }

  if (xbuf) {
    for (p = len; --p >= 0;) {
      arr[p] = arr[p - lblock];
    }

    SORT_TYPE_CPY(arr - lblock, xbuf, lblock);
  } else if (havebuf) {
    while (--len >= 0) {
      GRAIL_SWAP1(arr + len, arr + len - lblock);
    }
  }
}


static void GRAIL_COMMON_SORT(SORT_TYPE *arr, int Len, SORT_TYPE *extbuf, int LExtBuf) {
  int lblock, nkeys, findkeys, ptr, cbuf, lb, nk;
  int havebuf, chavebuf;
  long long s;

  if (Len <= SMALL_SORT_BND) {
    SMALL_SORT(arr, Len);
    return;
  }

  lblock = 1;

  while (lblock * lblock < Len) {
    lblock *= 2;
  }

  nkeys = (Len - 1) / lblock + 1;
  findkeys = GRAIL_FIND_KEYS(arr, Len, nkeys + lblock);
  havebuf = 1;

  if (findkeys < nkeys + lblock) {
    if (findkeys < 4) {
      GRAIL_LAZY_STABLE_SORT(arr, Len);
      return;
    }

    nkeys = lblock;

    while (nkeys > findkeys) {
      nkeys /= 2;
    }

    havebuf = 0;
    lblock = 0;
  }

  ptr = lblock + nkeys;
  cbuf = havebuf ? lblock : nkeys;

  if (havebuf) {
    GRAIL_BUILD_BLOCKS(arr + ptr, Len - ptr, cbuf, extbuf, LExtBuf);
  } else {
    GRAIL_BUILD_BLOCKS(arr + ptr, Len - ptr, cbuf, NULL, 0);
  }

  /* 2*cbuf are built */
  while (Len - ptr > (cbuf *= 2)) {
    lb = lblock;
    chavebuf = havebuf;

    if (!havebuf) {
      if (nkeys > 4 && nkeys / 8 * nkeys >= cbuf) {
        lb = nkeys / 2;
        chavebuf = 1;
      } else {
        nk = 1;
        s = (long long)cbuf * findkeys / 2;

        while (nk < nkeys && s != 0) {
          nk *= 2;
          s /= 8;
        }

        lb = (2 * cbuf) / nk;
      }
    }

    GRAIL_COMBINE_BLOCKS(arr, arr + ptr, Len - ptr, cbuf, lb, chavebuf, chavebuf
                         && lb <= LExtBuf ? extbuf : NULL);
  }

  SMALL_SORT(arr, ptr);
  GRAIL_MERGE_WITHOUT_BUFFER(arr, ptr, Len - ptr);
}

void GRAIL_SORT(SORT_TYPE *arr, size_t Len) {
  GRAIL_COMMON_SORT(arr, (int)Len, NULL, 0);
}

void GRAIL_SORT_FIXED_BUFFER(SORT_TYPE *arr, size_t Len) {
  SORT_TYPE ExtBuf[GRAIL_EXT_BUFFER_LENGTH];
  GRAIL_COMMON_SORT(arr, (int)Len, ExtBuf, GRAIL_EXT_BUFFER_LENGTH);
}

void GRAIL_SORT_DYN_BUFFER(SORT_TYPE *arr, size_t Len) {
  int L = 1;
  SORT_TYPE *ExtBuf;

  while (L * L < Len) {
    L *= 2;
  }

  ExtBuf = SORT_NEW_BUFFER(L);

  if (ExtBuf == NULL) {
    GRAIL_SORT_FIXED_BUFFER(arr, Len);
  } else {
    GRAIL_COMMON_SORT(arr, (int)Len, ExtBuf, L);
    SORT_DELETE_BUFFER(ExtBuf);
  }
}

/****** classic MergeInPlace *************/

static void GRAIL_REC_MERGE(SORT_TYPE *A, int L1, int L2) {
  int K, k1, k2, m1, m2;

  if (L1 < 3 || L2 < 3) {
    GRAIL_MERGE_WITHOUT_BUFFER(A, L1, L2);
    return;
  }

  if (L1 < L2) {
    K = L1 + L2 / 2;
  } else {
    K = L1 / 2;
  }

  k1 = k2 = GRAIL_BIN_SEARCH_LEFT(A, L1, A + K);

  if (k2 < L1 && SORT_CMP_A(A + k2, A + K) == 0) {
    k2 = GRAIL_BIN_SEARCH_RIGHT(A + k1, L1 - k1, A + K) + k1;
  }

  m1 = GRAIL_BIN_SEARCH_LEFT(A + L1, L2, A + K);
  m2 = m1;

  if (m2 < L2 && SORT_CMP_A(A + L1 + m2, A + K) == 0) {
    m2 = GRAIL_BIN_SEARCH_RIGHT(A + L1 + m1, L2 - m1, A + K) + m1;
  }

  if (k1 == k2) {
    GRAIL_ROTATE(A + k2, L1 - k2, m2);
  } else {
    GRAIL_ROTATE(A + k1, L1 - k1, m1);

    if (m2 != m1) {
      GRAIL_ROTATE(A + (k2 + m1), L1 - k2, m2 - m1);
    }
  }

  GRAIL_REC_MERGE(A + (k2 + m2), L1 - k2, L2 - m2);
  GRAIL_REC_MERGE(A, k1, m1);
}

void REC_STABLE_SORT(SORT_TYPE *arr, size_t L) {
  int m, h, p0, p1, rest;

  for (m = 1; m < L; m += 2) {
    if (SORT_CMP_A(arr + m - 1, arr + m) > 0) {
      GRAIL_SWAP1(arr + (m - 1), arr + m);
    }
  }

  for (h = 2; h < L; h *= 2) {
    p0 = 0;
    p1 = (int)(L - 2 * h);

    while (p0 <= p1) {
      GRAIL_REC_MERGE(arr + p0, h, h);
      p0 += 2 * h;
    }

    rest = (int)(L - p0);

    if (rest > h) {
      GRAIL_REC_MERGE(arr + p0, h, rest - h);
    }
  }
}

/* Bubble sort implementation based on Wikipedia article
   https://en.wikipedia.org/wiki/Bubble_sort
*/
void BUBBLE_SORT(SORT_TYPE *dst, const size_t size) {
  size_t n = size;

  while (n) {
    size_t i, newn = 0U;

    for (i = 1U; i < n; ++i) {
      if (SORT_CMP(dst[i - 1U], dst[i]) > 0) {
        SORT_SWAP(dst[i - 1U], dst[i]);
        newn = i;
      }
    }

    n = newn;
  }
}

#undef SORT_SAFE_CPY
#undef SORT_TYPE_CPY
#undef SORT_TYPE_MOVE
#undef SORT_NEW_BUFFER
#undef SORT_DELETE_BUFFER
#undef QUICK_SORT
#undef MEDIAN
#undef SORT_CONCAT
#undef SORT_MAKE_STR1
#undef SORT_MAKE_STR
#undef SORT_NAME
#undef SORT_TYPE
#undef SORT_CMP
#undef TEMP_STORAGE_T
#undef TIM_SORT_RUN_T
#undef PUSH_NEXT
#undef SORT_SWAP
#undef SORT_CONCAT
#undef SORT_MAKE_STR1
#undef SORT_MAKE_STR
#undef BINARY_INSERTION_FIND
#undef BINARY_INSERTION_SORT_START
#undef BINARY_INSERTION_SORT
#undef REVERSE_ELEMENTS
#undef COUNT_RUN
#undef TIM_SORT
#undef TIM_SORT_RESIZE
#undef TIM_SORT_COLLAPSE
#undef TIM_SORT_RUN_T
#undef TEMP_STORAGE_T
#undef MERGE_SORT
#undef MERGE_SORT_RECURSIVE
#undef MERGE_SORT_IN_PLACE
#undef MERGE_SORT_IN_PLACE_RMERGE
#undef MERGE_SORT_IN_PLACE_BACKMERGE
#undef MERGE_SORT_IN_PLACE_FRONTMERGE
#undef MERGE_SORT_IN_PLACE_ASWAP
#undef GRAIL_SWAP1
#undef REC_STABLE_SORT
#undef GRAIL_REC_MERGE
#undef GRAIL_SORT_DYN_BUFFER
#undef GRAIL_SORT_FIXED_BUFFER
#undef GRAIL_COMMON_SORT
#undef GRAIL_SORT
#undef GRAIL_COMBINE_BLOCKS
#undef GRAIL_LAZY_STABLE_SORT
#undef GRAIL_MERGE_WITHOUT_BUFFER
#undef GRAIL_ROTATE
#undef GRAIL_BIN_SEARCH_LEFT
#undef GRAIL_BUILD_BLOCKS
#undef GRAIL_FIND_KEYS
#undef GRAIL_MERGE_BUFFERS_LEFT_WITH_X_BUF
#undef GRAIL_BIN_SEARCH_RIGHT
#undef GRAIL_MERGE_BUFFERS_LEFT
#undef GRAIL_SMART_MERGE_WITH_X_BUF
#undef GRAIL_MERGE_LEFT_WITH_X_BUF
#undef GRAIL_SMART_MERGE_WITHOUT_BUFFER
#undef GRAIL_SMART_MERGE_WITH_BUFFER
#undef GRAIL_MERGE_RIGHT
#undef GRAIL_MERGE_LEFT
#undef GRAIL_SWAP_N
#undef SQRT_SORT
#undef SQRT_SORT_BUILD_BLOCKS
#undef SQRT_SORT_MERGE_BUFFERS_LEFT_WITH_X_BUF
#undef SQRT_SORT_MERGE_DOWN
#undef SQRT_SORT_MERGE_LEFT_WITH_X_BUF
#undef SQRT_SORT_MERGE_RIGHT
#undef SQRT_SORT_SWAP_N
#undef SQRT_SORT_SWAP_1
#undef SQRT_SORT_SMART_MERGE_WITH_X_BUF
#undef SQRT_SORT_SORT_INS
#undef SQRT_SORT_COMBINE_BLOCKS
#undef SQRT_SORT_COMMON_SORT
#undef SORT_CMP_A
#undef BUBBLE_SORT
