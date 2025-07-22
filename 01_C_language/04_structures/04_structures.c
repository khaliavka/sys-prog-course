#include <stdio.h>
#include <string.h>

#define N 100

struct abonent
{
    char name[10];
    char second_name[10];
    char tel[10];
};

struct abonent *search_free_record(struct abonent *begin, struct abonent *end)
{
    while (begin < end)
    {
        if (begin->name[0] == 0)
            return begin;
        ++begin;
    }
    return NULL;
}

struct abonent *search_by_name(struct abonent *begin, struct abonent *end, char *name)
{
    while (begin < end)
    {
        if (strcmp(begin->name, name) == 0)
            return begin;
        ++begin;
    }
    return begin;
}

void print_prompt()
{
    printf("\n1) Add abonent\n"
           "2) Remove abonent\n"
           "3) Search by name\n"
           "4) Print all\n"
           "5) Exit\n");
}

void print_abonent(struct abonent *ab, int id)
{
    printf("Id: %d\n"
           "Name: %s\n"
           "Second name: %s\n"
           "Phone number: %s\n",
           id, ab->name, ab->second_name, ab->tel);
}

void print_found(struct abonent *begin, struct abonent *end, char *name)
{
    struct abonent *cur = search_by_name(begin, end, name);
    if (cur == end)
    {
        printf("Name not found.\n");
        return;
    }
    print_abonent(cur, cur - begin);

    while (cur < end)
    {
        cur = search_by_name(cur + 1, end, name);
        if (cur == end)
            return;
        print_abonent(cur, cur - begin);
    }
}

void print_all(struct abonent *begin, struct abonent *end)
{
    struct abonent *cur = begin;
    while (cur < end)
    {
        if (cur->name[0])
            print_abonent(cur, cur - begin);
        ++cur;
    }
}

int add_abonent(struct abonent *book)
{
    struct abonent *free_record = search_free_record(book, book + N);
    if (!free_record)
        return 1;
    printf("Enter a name: ");
    scanf("%9s", free_record->name);
    printf("Enter second name: ");
    scanf("%9s", free_record->second_name);
    printf("Enter a phone number: ");
    scanf("%9s", free_record->tel);
    return 0;
}

void remove_abonent(struct abonent *begin, int id)
{
    memset(begin + id, 0, sizeof(*begin));
}

int main(void)
{
    struct abonent book[N] = {0};
    char name[10];
    int id;
    int option = 0;

    while (1)
    {
        print_prompt();

        if (scanf("%d", &option) != 1)
        {
            printf("Bad input.\n");
            return 1;
        }

        switch (option)
        {
        case 1:
            if (add_abonent(book))
                printf("Cannot add a new contact. "
                       "The phone book is full.\n");
            break;
        case 2:
            printf("Enter id: ");
            scanf("%d", &id);
            if (id < N)
                remove_abonent(book, id);
            else
                printf("Bad id.\n");
            break;
        case 3:
            printf("Enter a name: ");
            scanf("%s", name);
            print_found(book, book + N, name);
            break;
        case 4:
            print_all(book, book + N);
            break;
        case 5:
            return 0;
        default:
            perror("Error.\n");
            return 1;
        }
    }

    return 0;
}