#include "sort_compare.h"
#include <stdlib.h>

#define SIZE_INT sizeof(int)
#define SIZE_PTR sizeof(int *)
#define SIZE_STATS_PTR sizeof(SortStats *)
#define SIZE_ALGO sizeof(Algorithm)

static void enter_memory(SortStats *s, size_t bytes)
{
    s->current_memory += bytes;
    if (s->current_memory > s->peak_memory)
        s->peak_memory = s->current_memory;
}

static void leave_memory(SortStats *s, size_t bytes)
{
    s->current_memory -= bytes;
}

static void swap(int *a, int *b, SortStats *s)
{
    int temp;
    enter_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + SIZE_INT);
    temp = *a;
    *a = *b;
    *b = temp;
    leave_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + SIZE_INT);
}

/* root 노드를 아래로 내려 힙 속성을 복구한다. 반복문만 사용하고 재귀는 쓰지 않는다.
   root < n / 2 는 "root가 자식을 가진다"는 뜻이다(인덱스 n/2 이상은 모두 리프). */
static void sift_down(int a[], int n, int root, SortStats *s)
{
    int child, largest;
    enter_memory(s, SIZE_PTR + SIZE_STATS_PTR + 4 * SIZE_INT);
    while (root < n / 2)
    {
        child = 2 * root + 1;
        largest = child;
        if (child + 1 < n)
        {
            s->comparisons++;
            if (a[child + 1] > a[child])
                largest = child + 1;
        }
        s->comparisons++;
        if (a[root] >= a[largest])
            break;
        swap(&a[root], &a[largest], s);
        root = largest;
    }
    leave_memory(s, SIZE_PTR + SIZE_STATS_PTR + 4 * SIZE_INT);
}

static void heap_sort(int a[], int n, SortStats *s)
{
    int start, end;
    enter_memory(s, SIZE_PTR + SIZE_STATS_PTR + 3 * SIZE_INT);
    for (start = n / 2; start > 0; start--)
        sift_down(a, n, start - 1, s);
    for (end = n - 1; end > 0; end--)
    {
        swap(&a[0], &a[end], s);
        sift_down(a, end, 0, s);
    }
    leave_memory(s, SIZE_PTR + SIZE_STATS_PTR + 3 * SIZE_INT);
}

static void merge(int a[], int temp[], int left, int mid, int right, SortStats *s)
{
    int i = left, j = mid + 1, k = left;
    enter_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + 6 * SIZE_INT);
    while (i <= mid && j <= right)
    {
        s->comparisons++;
        if (a[i] <= a[j])
            temp[k++] = a[i++];
        else
            temp[k++] = a[j++];
    }
    while (i <= mid)
        temp[k++] = a[i++];
    while (j <= right)
        temp[k++] = a[j++];
    for (i = left; i <= right; i++)
        a[i] = temp[i];
    leave_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + 6 * SIZE_INT);
}

static void merge_recursive(int a[], int temp[], int left, int right, SortStats *s)
{
    int mid;
    enter_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + 3 * SIZE_INT);
    if (left < right)
    {
        mid = left + (right - left) / 2;
        merge_recursive(a, temp, left, mid, s);
        merge_recursive(a, temp, mid + 1, right, s);
        merge(a, temp, left, mid, right, s);
    }
    leave_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + 3 * SIZE_INT);
}

static int merge_sort(int a[], int n, SortStats *s)
{
    int *temp;
    enter_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + SIZE_INT);
    temp = malloc((size_t)n * sizeof(int));
    if (temp == NULL)
    {
        leave_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + SIZE_INT);
        return 0;
    }
    enter_memory(s, (size_t)n * sizeof(int));
    merge_recursive(a, temp, 0, n - 1, s);
    free(temp);
    leave_memory(s, (size_t)n * sizeof(int));
    leave_memory(s, 2 * SIZE_PTR + SIZE_STATS_PTR + SIZE_INT);
    return 1;
}

static int choose_pivot(int a[], int left, int right, Algorithm mode, SortStats *s)
{
    int mid;
    enter_memory(s, SIZE_PTR + SIZE_STATS_PTR + 3 * SIZE_INT + SIZE_ALGO);
    if (mode == QUICK_FIRST)
        mid = left;
    else if (mode == QUICK_RANDOM)
        mid = left + rand() % (right - left + 1);
    else
    {
        mid = left + (right - left) / 2;
        /* 실제 실행한 값 비교만 센다. 두 번 또는 세 번 비교한다. */
        s->comparisons++;
        if (a[left] < a[mid])
        {
            s->comparisons++;
            /* left < mid 이고 mid < right 이면 mid가 중앙값이므로 그대로 둔다. */
            if (a[mid] >= a[right])
            {
                s->comparisons++;
                mid = (a[left] < a[right]) ? right : left;
            }
        }
        else
        {
            s->comparisons++;
            if (a[left] < a[right])
                mid = left;
            else
            {
                s->comparisons++;
                mid = (a[mid] < a[right]) ? right : mid;
            }
        }
    }
    leave_memory(s, SIZE_PTR + SIZE_STATS_PTR + 3 * SIZE_INT + SIZE_ALGO);
    return mid;
}

/* Lomuto 분할: 피벗을 맨 오른쪽으로 옮긴 뒤, 피벗 이하의 값을 왼쪽으로 모은다. */
static int partition(int a[], int left, int right, Algorithm mode, SortStats *s)
{
    int pivot_index, pivot, i, j;
    enter_memory(s, SIZE_PTR + SIZE_STATS_PTR + 6 * SIZE_INT + SIZE_ALGO);
    pivot_index = choose_pivot(a, left, right, mode, s);
    swap(&a[pivot_index], &a[right], s);
    pivot = a[right];
    i = left;
    for (j = left; j < right; j++)
    {
        s->comparisons++;
        if (a[j] <= pivot)
        {
            swap(&a[i], &a[j], s);
            i++;
        }
    }
    swap(&a[i], &a[right], s);
    leave_memory(s, SIZE_PTR + SIZE_STATS_PTR + 6 * SIZE_INT + SIZE_ALGO);
    return i;
}

static void quick_recursive(int a[], int left, int right, Algorithm mode, SortStats *s)
{
    int p;
    enter_memory(s, SIZE_PTR + SIZE_STATS_PTR + 3 * SIZE_INT + SIZE_ALGO);
    if (left < right)
    {
        p = partition(a, left, right, mode, s);
        quick_recursive(a, left, p - 1, mode, s);
        quick_recursive(a, p + 1, right, mode, s);
    }
    leave_memory(s, SIZE_PTR + SIZE_STATS_PTR + 3 * SIZE_INT + SIZE_ALGO);
}

int run_sort(int a[], int n, Algorithm algorithm, SortStats *stats)
{
    /* 호출자는 유효한 배열, n >= 0, 올바른 Algorithm을 전달한다. */
    stats->comparisons = 0;
    stats->current_memory = 0;
    stats->peak_memory = 0;
    enter_memory(stats, SIZE_PTR + SIZE_STATS_PTR + SIZE_INT + SIZE_ALGO);
    if (n > 1)
    {
        if (algorithm == HEAP)
            heap_sort(a, n, stats);
        else if (algorithm == MERGE)
        {
            if (!merge_sort(a, n, stats))
            {
                leave_memory(stats, SIZE_PTR + SIZE_STATS_PTR + SIZE_INT + SIZE_ALGO);
                return 0;
            }
        }
        else
            quick_recursive(a, 0, n - 1, algorithm, stats);
    }
    leave_memory(stats, SIZE_PTR + SIZE_STATS_PTR + SIZE_INT + SIZE_ALGO);
    return 1;
}
