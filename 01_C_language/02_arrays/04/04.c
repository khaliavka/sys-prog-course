#include <stdio.h>

#define N 5

void print_array(int (*m)[N])
{
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            printf("%2d ", m[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(void)
{

    int m[N][N] = {0};
    int up = 0, bottom = N, left = 0, right = N;
    int index = 1;
    while (up < bottom && left < right)
    {

        for (int i = left; i < right; ++i)
        {
            m[up][i] = index++;
        }
        ++up;
        print_array(m);
        for (int i = up; i < bottom; ++i)
        {
            m[i][right - 1] = index++;
        }
        --right;
        print_array(m);
        for (int i = right - 1; i > left - 1; --i)
        {
            m[bottom - 1][i] = index++;
        }
        --bottom;
        print_array(m);

        for (int i = bottom - 1; i > up - 1; --i)
        {
            m[i][left] = index++;
        }
        ++left;
        print_array(m);

    }

    return 0;
}