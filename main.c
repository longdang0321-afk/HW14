#include "sort_compare.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_SEED 2026u
#define PIVOT_SEED 12026u
#ifndef REPEAT
#define REPEAT 400
#endif

/* qsort는 결과 검증용이며 실험 대상이 아니다. */
static int compare_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

int main(void)
{
    const int sizes[] = {1000, 2000, 4000, 8000, 16000, 20000};
    const char *names[] = {"Heap Sort", "Merge Sort", "Quick (First)",
                           "Quick (Random)", "Quick (Median3)"};
    int data_index, algorithm, repeat, i;

    printf("Data seed: %u / Pivot seed: %u / Repeat: %d\n", DATA_SEED, PIVOT_SEED, REPEAT);
    printf("Timer: clock() CPU time / Memory: sizeof-based estimate\n");
    printf("sizeof(int)=%zu, sizeof(pointer)=%zu, sizeof(Algorithm)=%zu\n",
           sizeof(int), sizeof(int *), sizeof(Algorithm));
    for (data_index = 0; data_index < 6; data_index++)
    {
        int n = sizes[data_index];
        size_t bytes = (size_t)n * sizeof(int);
        int *original = malloc(bytes);
        int *work = malloc(bytes);
        int *expected = malloc(bytes);
        if (!original || !work || !expected)
        {
            fprintf(stderr, "Memory allocation failed.\n");
            free(original);
            free(work);
            free(expected);
            return EXIT_FAILURE;
        }

        /* 크기별 seed를 고정한다. 피벗의 rand()가 다음 입력에 영향을 주지 않는다. */
        srand(DATA_SEED);
        for (i = 0; i < n; i++)
            original[i] = rand() % 100000;
        memcpy(expected, original, bytes);
        qsort(expected, (size_t)n, sizeof(int), compare_int);

        printf("\nData Size : %d\n", n);
        printf("%-18s %14s %12s %14s\n", "Algorithm", "Comparisons", "Time(ms)", "Memory(Bytes)");
        printf("--------------------------------------------------------------\n");
        /* Algorithm enum 순서(HEAP ~ QUICK_MEDIAN3)가 names[] 순서와 같다. */
        for (algorithm = HEAP; algorithm <= QUICK_MEDIAN3; algorithm++)
        {
            SortStats stats;
            clock_t total_ticks = 0;
            unsigned long long comparisons = 0;
            size_t memory = 0;
            for (repeat = 0; repeat < REPEAT; repeat++)
            {
                clock_t begin, end;
                int ok;
                memcpy(work, original, bytes);
                srand(PIVOT_SEED);
                begin = clock();
                ok = run_sort(work, n, (Algorithm)algorithm, &stats);
                end = clock();
                if (!ok || begin == (clock_t)-1 || end == (clock_t)-1 ||
                    memcmp(work, expected, bytes) != 0 || stats.current_memory != 0)
                {
                    fprintf(stderr, "Sort/timer/memory tracking failed: n=%d, %s\n", n, names[algorithm]);
                    free(original);
                    free(work);
                    free(expected);
                    return EXIT_FAILURE;
                }
                total_ticks += end - begin;
                comparisons = stats.comparisons;
                memory = stats.peak_memory;
            }
            printf("%-18s %14llu %12.6f %14zu\n", names[algorithm], comparisons,
                   (double)total_ticks * 1000.0 / CLOCKS_PER_SEC / REPEAT, memory);
        }
        printf("Verification : PASS (all algorithms, all repetitions)\n");
        free(original);
        free(work);
        free(expected);
    }
    return EXIT_SUCCESS;
}
