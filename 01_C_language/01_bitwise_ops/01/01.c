#include <limits.h>
#include <stdio.h>

int main(void) {
    while (1) {
        
        int x;
        if(scanf("%d", &x) != 1) 
            return 1;
        
        int bit_sz = sizeof(x) * CHAR_BIT;
        unsigned mask = 1u << (bit_sz - 1);
        
        for (int i = 0; i < bit_sz; ++i) {
            if (!(i % 4)) printf(" ");
            
            printf("%d", !!(mask & x));
            mask >>= 1;
        }
        printf("\n");

    }
    return 0;
}