#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct abonent
{
    struct abonent *prev;
    struct abonent *next;
    unsigned long id;
    char name[10];
    char second_name[10];
    char tel[10];
};

struct abonent *insert_node(struct abonent *head)
{
    struct abonent *node = malloc(sizeof(*node));
    if (!node)
        return NULL;

    node->prev = head;
    node->next = head->next;

    head->next->prev = node;
    head->next = node;

    return node;
}

struct abonent *erase_node(struct abonent *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;

    struct abonent *nn = node->next;
    free(node);
    return nn;
}

void cleanup(struct abonent *head)
{
    struct abonent *node = head->next;
    while (node != node->next)
    {
        node = erase_node(node);
    }
}

struct abonent *next(struct abonent *node)
{
    return node->next;
}

unsigned long generate_id(void)
{
    static unsigned long id = 0;
    return id++;
}

struct abonent *search_by_id(struct abonent *begin, struct abonent *end, unsigned long id)
{
    while (begin != end)
    {
        if (begin->id == id)
            return begin;
        begin = next(begin);
    }
    return begin;
}

struct abonent *search_by_name(struct abonent *begin, struct abonent *end, char *name)
{
    while (begin != end)
    {
        if (strcmp(begin->name, name) == 0)
            return begin;
        begin = next(begin);
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

void print_abonent(struct abonent *ab)
{
    printf("Id: %lu\n"
           "Name: %s\n"
           "Second name: %s\n"
           "Phone number: %s\n",
           ab->id, ab->name, ab->second_name, ab->tel);
}

void print_found(struct abonent *begin, struct abonent *end, char *name)
{
    struct abonent *cur = search_by_name(begin, end, name);
    if (cur == end)
    {
        printf("Name not found.\n");
        return;
    }
    print_abonent(cur);

    while (cur != end)
    {
        cur = search_by_name(next(cur), end, name);
        if (cur == end)
            return;
        print_abonent(cur);
    }
}

void print_all(struct abonent *begin, struct abonent *end)
{
    if (begin == end)
    {
        printf("Nothing to print.\n");
        return;
    }
    struct abonent *cur = begin;
    while (cur != end)
    {
        print_abonent(cur);
        cur = next(cur);
    }
}

int add_abonent(struct abonent *head)
{
    struct abonent *ab = insert_node(head);
    if (!ab)
        return 1;
    ab->id = generate_id();
    printf("Enter a name: ");
    scanf("%s", ab->name);
    printf("Enter second name: ");
    scanf("%s", ab->second_name);
    printf("Enter a phone number: ");
    scanf("%s", ab->tel);
    return 0;
}

int remove_abonent(struct abonent *head, unsigned long id)
{
    struct abonent *cur = search_by_id(head->next, head, id);
    if (cur == head)
        return 1;
    erase_node(cur);
    return 0;
}

void test_linked_list(void)
{
    {
        struct abonent head;
        head.prev = &head;
        head.next = &head;
        head.id = 0;

        for (int i = 0; i < 12; ++i)
        {
            struct abonent *node = insert_node(&head);
            node->id = generate_id();
        }
        erase_node(head.prev);
        erase_node(head.next);
        cleanup(&head);
    }
}

int test1_add_abonent(struct abonent *head)
{
    struct abonent *ab = insert_node(head);
    if (!ab)
    {
        cleanup(head);
        exit(1);
    }
    ab->id = generate_id();
    ab->name[0] = 'N';
    ab->name[1] = 0;
    ab->second_name[0] = 'S';
    ab->second_name[1] = 0;
    ab->tel[0] = '4';
    ab->tel[1] = 0;
    return 0;
}

void test1(void)
{
    {
        struct abonent head;
        head.prev = &head;
        head.next = &head;
        head.id = 0;

        for (int i = 0; i < 8; ++i)
        {
            test1_add_abonent(&head);
        }
        printf("/----------/\n");
        print_all(head.next, &head);
        printf("/----------/\n");
        print_found(head.next, &head, "N");
        printf("/----------/\n");

        remove_abonent(&head, head.prev->id);
        remove_abonent(&head, head.prev->id);
        remove_abonent(&head, head.next->id);

        print_all(head.next, &head);

        cleanup(&head);
    }
}

int main(void)
{
    // test_linked_list();
    // test1();

    struct abonent book;
    book.prev = &book;
    book.next = &book;
    book.id = generate_id();
    char name[10];
    unsigned long id;
    int option = 0;
    while (1)
    {
        print_prompt();

        if (scanf("%d", &option) != 1)
        {
            printf("Bad input.\n");
            cleanup(&book);
            return 1;
        }

        switch (option)
        {
        case 1:
            if (add_abonent(&book))
                printf("Cannot add a new contact. "
                       "The phone book is full.\n");
            break;
        case 2:
            printf("Enter id: ");
            scanf("%lu", &id);
            if (remove_abonent(&book, id))
                printf("Bad id.\n");
            break;
        case 3:
            printf("Enter a name: ");
            scanf("%s", name);
            print_found(book.next, &book, name);
            break;
        case 4:
            print_all(book.next, &book);
            break;
        case 5:
            cleanup(&book);
            return 0;
        default:
            printf("Bad input.\n");
        }
    }

    return 0;
}
