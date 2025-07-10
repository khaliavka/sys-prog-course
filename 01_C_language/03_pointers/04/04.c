#include <assert.h>
#include <stdio.h>

#define N 1000

char *search_substr(char *str, char *substr)
{
    while (*str)
    {
        char *ps = str;
        char *psub = substr;
        while (*psub && *ps == *psub)
        {
            ++ps;
            ++psub;
        }
        if (!(*psub))
            return str;
        ++str;
    }
    return NULL;
}

void tests(void)
{
    {
        char *str = "aaabbbccc";
        char *substr = "aaabbbccc";
        char *ans = search_substr(str, substr);
        assert(ans == str);
    }
    {
        char *str = "aaabbbccc";
        char *substr = "aaabbb";
        char *ans = search_substr(str, substr);
        assert(ans == str);
    }
    {
        char *str = "aaabbbccc";
        char *substr = "bbbccc";
        char *ans = search_substr(str, substr);
        assert(ans == str + 3);
    }
    {
        char *str = "aaabbbccc";
        char *substr = "bab";
        char *ans = search_substr(str, substr);
        assert(ans == NULL);
    }
    {
        char *str = "aaabbbccc";
        char *substr = "aaabbbcccaaa";
        char *ans = search_substr(str, substr);
        assert(ans == NULL);
    }
    {
        char *str = "aaabbbccc";
        char *substr = "ccaaa";
        char *ans = search_substr(str, substr);
        assert(ans == NULL);
    }
    {
        char *str = "aaabbbcccp";
        char *substr = "p";
        char *ans = search_substr(str, substr);
        assert(ans == str + 9);
    }
}

int main(void)
{
    char str[N];
    char substr[N];

    tests();

    if (scanf("%s %s", str, substr) != 2)
        return 1;

    char *ans = search_substr(str, substr);
    if (ans)    
        printf("%s\n", ans);
    else
        printf("No match.\n");
    
    return 0;
}