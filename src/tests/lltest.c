#define _GB_DEBUG yes
#include "../collections/linkedlist.h"
#include "../debug.h"

F_HIDDEN F_COMPARATOR
int compareInt(const void* a, const void* b) {
    if (a && b) {
        return *((int*) a) - *((int*) b);
    } else {
        return (!a && !b) ? 0 : (a ? -1 : 1);
    }
}

/*
 * Just a simple hack to avoid dealing with number allocation
 */

F_INTERNAL
typedef struct _int_wrapper {
    int number;
} IntWrapper;

F_INTERNAL
const int NPTRS = 100;

int main(void) {

    d_printf("Debugging enabled\n");
    fflush(stdout);

    printf("Making data\n");

    int* numberPointers[NPTRS];
    for (int idx = 0; idx < NPTRS; ++idx) {
        numberPointers[idx] = gb_new(int);
        *numberPointers[idx] = idx;
    }

    GBLinkedList* numberList = GBLinkedList_new(numberPointers[0]);

    printf("Populating list\n");

    for (int idx = 1; idx < NPTRS; ++idx) {
        numberList = GBLinkedList_prepend(numberList, numberPointers[idx]);
    }

    const size_t listSize = GBLinkedList_size(numberList);

    printf("List is %lu elements long\n", listSize);

    printf("First element is `%i`, last is `%i`\n",
           *((int*)numberList->data),
            *((int*)GBLinkedList_last(numberList)));

    printf("Sorting list\n");

    GBLinkedList* sorted = GBLinkedList_sort(numberList, compareInt);

    printf("First element is `%i`, last is `%i`\n",
           *((int*)sorted->data),
            *((int*)GBLinkedList_last(sorted)));

    GBLinkedList_with(sorted, node, {
        printf("%i, ", *((int*) node->data));
    });
    printf("\n");

    return 0;
}