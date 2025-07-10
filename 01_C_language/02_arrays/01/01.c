#include <stdio.h>

#define N 3

int main(void)
{
    int m[N][N];
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            m[i][j] = N * i + j + 1;
        }
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%d ", m[i][j]);
        }
        printf("\n");
    }
    return 0;
}