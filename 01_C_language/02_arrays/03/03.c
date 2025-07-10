#include <stdio.h>

#define N 7

int main(void)
{
    int m[N][N];
    int zero_count = (N) - 1;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            m[i][j] = (j >= zero_count);
        }
        --zero_count;
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%d ", m[i][j]);
        }
        printf("\n");
    }
    return 0;
}