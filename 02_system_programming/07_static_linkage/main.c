#include <stdio.h>

#include "ops.h"

void print_prompt(void)
{
    printf("\n1) Add\n"
           "2) Subtract\n"
           "3) Multiply\n"
           "4) Divide\n"
           "5) Exit\n");
}

void read_numbers(int *a, int *b)
{
    printf("Enter numbers: ");
    if (scanf("%d %d", a, b) != 2)
        printf("Bad input.\n");
}

void print_answer(int ans)
{
    printf("Answer: %d\n", ans);
}

int main(void)
{

    int option = 0;
    int a, b;
    while (1)
    {
        print_prompt();

        if (scanf("%d", &option) != 1)
            printf("Bad input.\n");

        switch (option)
        {
        case 1:
            read_numbers(&a, &b);
            print_answer(add(a, b));
            break;
        case 2:
            read_numbers(&a, &b);
            print_answer(sub(a, b));
            break;
        case 3:
            read_numbers(&a, &b);
            print_answer(mul(a, b));
            break;
        case 4:
            read_numbers(&a, &b);
            print_answer(div(a, b));
            break;
        case 5:
            printf("Quit.\n");
            return 0;
        default:
            printf("Bad input.\n");
        }
    }

    return 0;
}
