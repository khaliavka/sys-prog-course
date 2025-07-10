#include <stdio.h>

#define N 5

int main(void)
{
    int a[N];
    for (int i = 0; i < N; ++i) {
        if (scanf("%d", a + i) != 1)
            return 1;
    }
    for (int i = 0; i < (N) / 2; ++i) {
        int t = a[i];
        a[i] = a[N - 1 - i];
        a[N - 1 - i] = t;
    }
    for (int i = 0; i < N; ++i) {
        printf("%d ", a[i]);
    }
    printf("\n");
    return 0;
}