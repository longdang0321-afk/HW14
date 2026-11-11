#ifndef SORT_COMPARE_H
#define SORT_COMPARE_H

#include <stddef.h>

typedef enum
{
    HEAP,
    MERGE,
    QUICK_FIRST,
    QUICK_RANDOM,
    QUICK_MEDIAN3
} Algorithm;

typedef struct
{
    unsigned long long comparisons;
    size_t current_memory;
    size_t peak_memory;
} SortStats;

/* 성공: 1, 임시 배열 할당 실패: 0 */
int run_sort(int a[], int n, Algorithm algorithm, SortStats *stats);

#endif
