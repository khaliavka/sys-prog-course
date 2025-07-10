#include <limits.h>
#include <stdio.h>

void print_bin_rep(int x)
{

    int bit_sz = sizeof(x) * CHAR_BIT;
    unsigned mask = 1u << (bit_sz - 1);

    for (int i = 0; i < bit_sz; ++i)
    {
        if (!(i % 4))
            printf(" ");

        printf("%d", !!(mask & x));
        mask >>= 1;
    }
    printf("\n");
}

int main(void)
{
    while (1)
    {

        int x;
        int y;
        if (scanf("%d %d", &x, &y) != 2)
            return 1;
        
        print_bin_rep(x);
        print_bin_rep(y);

        *((char*)&x + 2) = *(char*)&y;

        print_bin_rep(x);
    }
    return 0;
}