#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include "linkedlist.h"
#include "../debug.h"

GBLinkedList* GBLinkedList_new(void* initialData) {

    GBLinkedList* listHead = gb_new0(GBLinkedList);

    if (initialData) {
        listHead->data = initialData;
    }

    return listHead;
}

void GBLinkedList_free(GBLinkedList* list) {

    // Traverse to the end of the list, freeing each node
    GBLinkedList* currentNode = list;
    do {
        GBLinkedList* nextNode = currentNode->next;
        gb_free(currentNode);
        currentNode = nextNode;
    } while (currentNode);

    GBLinkedList_with(list, current, {

    });

    gb_free(currentNode);
}

size_t GBLinkedList_size(GBLinkedList* list) {

    d_printf("%s(0x%X)\n", __FUNCTION__, list);

    size_t listSize = 0;

    // Traverse list to the end in order to count nodes
    for (GBLinkedList* current = list; current; current = current->next) ++listSize;

    return listSize;
}

static GBLinkedList* GBLinkedList_tail(GBLinkedList* list) {

    for (GBLinkedList* tail = list; tail; tail = tail->next) if (!tail->next) return tail;

    // This should not be reachable
    return list;
}

static GBLinkedList* GBLinkedList_nth(GBLinkedList* list, const size_t position) {

    GBLinkedList* current = list;
    for (size_t index = 0; index < position; ++index) {
        unless(current->next) return NULL;
        current = current->next;
    }

    return current;
}

GBLinkedList* GBLinkedList_append(GBLinkedList* list, void* ptr) {

    GBLinkedList* tail = GBLinkedList_tail(list);
    tail->next = GBLinkedList_new(ptr);

    return tail->next;
}


void GBLinkedList_reverse(GBLinkedList** list) {

    GBLinkedList* current = *list;
    GBLinkedList* swap;

    do {
        swap = current->prior;
        current->prior = current->next;
        current->next = swap;
        current = current->prior;
    } while (current);

    if (swap) *list = swap->prior;
}


bool GBLinkedList_set(GBLinkedList* list, void* ptr, size_t index) {

    GBLinkedList* targetNode = GBLinkedList_nth(list, index);

    unless(targetNode) return false;

    targetNode->data = ptr;

    return true;
}

bool GBLinkedList_splice(GBLinkedList* list, void* ptr, size_t index) {

    GBLinkedList* position = GBLinkedList_nth(list, index);

    unless(position) return false;

    GBLinkedList* insertion = GBLinkedList_new(ptr);

    insertion->prior = position;
    insertion->next = position->next;

    insertion->next->prior = insertion;

    position->next = insertion;

    return true;
}

bool GBLinkedList_spliceAll(GBLinkedList* list, void* ptrs[], size_t nPtrs, size_t index) {

    // Check that more than one insertion is being made

    unless(nPtrs > 1) {
        return GBLinkedList_splice(list, ptrs[0], index);
    }

    // Before bothering with expensive operations, check that it's worth it
    GBLinkedList* insertionPoint = GBLinkedList_nth(list, index);

    unless(insertionPoint) return false;

    void** workingPointer = ptrs;

    // Do this in reverse, as it's easier
    GBLinkedList* tail = GBLinkedList_new(*workingPointer--);
    GBLinkedList* head = GBLinkedList_new(NULL);
    tail->prior = head;

    // Go backwards from the second-to-last pointer and build a new linked list
    for (size_t workingIndex = nPtrs - 2; workingIndex < nPtrs; --workingIndex) {

        GBLinkedList* nextNode = GBLinkedList_new(NULL);
        nextNode->next = head;
        head->prior = nextNode;

        head->data = *workingPointer;

        head = nextNode;

        --workingPointer;
    }

    // Splice the head of the new linked list in as the next node in the chain, etc...

    GBLinkedList* oldNext = insertionPoint->next;

    // Such that insertionPoint precedes the head of the new list
    insertionPoint->next = head;
    head->prior = insertionPoint;

    // Such that oldNext proceeds the tail of the new list, and knows that the tail precedes it
    tail->next = oldNext;
    tail->next->prior = tail;

    return true;
}

GBLinkedList* GBLinkedList_prepend(GBLinkedList* list, void* ptr) {

    GBLinkedList* newHead = GBLinkedList_new(ptr);

    newHead->next = list;
    list->prior = newHead;

    return newHead;
}

void* GBLinkedList_eject(GBLinkedList* list) {

    GBLinkedList* tail = GBLinkedList_tail(list);

    void* data = tail->data;

    // Only do the following if the provided head is not the end of the list
    unless(tail == list) {
        if (tail->prior) tail->prior->next = NULL;
        gb_free(tail);
    }

    return data;
}

F_BRIDGE // This function is just a bridge to list->data, so __make__ the compiler inline it.
void* GBLinkedList_first(GBLinkedList* list) {
    return list->data;
}

void* GBLinkedList_last(GBLinkedList* list) {
    GBLinkedList* tail = GBLinkedList_tail(list);
    unless(tail) return NULL;
    return tail->data;
}

void* GBLinkedList_get(GBLinkedList* list, size_t index) {
    GBLinkedList* node = GBLinkedList_nth(list, index);
    unless(node) return NULL;
    return node->data;
}

GBLinkedList* GBLinkedList_find(GBLinkedList* list, const void* key, GBLL_Comparator comparator) {

    GBLinkedList* found = NULL;

    GBLinkedList* current = list;
    do {

        if (comparator(key, current->data) == 0) {
            if (found) {
                found = GBLinkedList_prepend(found, current->data);
            } else {
                found = GBLinkedList_new(current->data);
            }
        }

        current = current->next;
    } while (current);

    if (found) {
        GBLinkedList_reverse(&found);
    }

    return found;
}

void* GBLinkedList_foldRight(GBLinkedList* list, const void* x, void* (* lambda)(const void*, const void*)) {

    // Do the first iteration to work out a celtic knot
    void* accum = lambda(x, list->data);

    GBLinkedList* current = list->next;
    do {

        void* lastAccum = accum;
        accum = lambda(accum, current->data);
        gb_free(lastAccum);

        current = current->next;
    } while (current);

    return accum;
}

GBLinkedList* GBLinkedList_map(GBLinkedList* list, void* (* lambda)(const void*)) {
    return GBLinkedList_mapWith(list, x, lambda(x));
}

// Sort Logic & Helpers -------------------------------------------------------------------------


#define printList(list) {\
if(list){\
    d_printf("{ ");\
    GBLinkedList_with(list, node, {\
        if(node->data) d_printf("%i", *((int*)node->data));\
        else d_printf("(nil)");\
        if(node->next) d_printf(", ");\
    });\
    d_printf(" }");\
} else {\
    d_printf("nil");\
}\
}

F_INTERNAL ALL_ARGS_EXIST
GBLinkedList* sort_Tail(GBLinkedList* head) {
    if(head->next) {
        return sort_Tail(head->next);
    } else {
        return head;
    }
}

F_INTERNAL ALL_ARGS_EXIST
void sort_AppendAll(GBLinkedList* appendTo, GBLinkedList* appendFrom) {

    d_printf("- %s(", __FUNCTION__);
    printList(appendTo);
    d_printf(", ");
    printList(appendFrom);
    d_printf(") = ");

    GBLinkedList* current = appendFrom;

    GBLinkedList* tail = sort_Tail(appendTo);
    tail->next = GBLinkedList_new(NULL);
    tail = tail->next;

    GBLinkedList* last = NULL;

    do {
        tail->data = current->data;

        if (last) tail->prior = last;

        current = current->next;

        if (current) {
            last = tail;
            tail = GBLinkedList_new(NULL);
            last->next = tail;
        }
    } while (current);

    printList(appendTo);
    d_printf("\n");
}

F_INTERNAL ALL_ARGS_EXIST
GBLinkedList* sort_MergeLists(GBLinkedList* a, GBLinkedList* b, GBLL_Comparator comparator) {

    d_printf("- %s(", __FUNCTION__);
    printList(a);
    d_printf(", ");
    printList(b);
    d_printf(")\n");

    GBLinkedList* result = GBLinkedList_new(NULL);

    GBLinkedList* tail = result;
    GBLinkedList* last = NULL;

    d_printf("- smart merge comparator\n");

    do {

        int diff = comparator(a->data, b->data);

        d_printf("-- dif(%i, %i) = %i\n",
                    *((int*) a->data), *((int*) b->data),
                    diff);

        if (diff <= 0) {
            d_printf("--- result << %i {left} \n", *((int*) a->data));
            tail->data = a->data;
            a = a->next;
        } else {
            d_printf("--- result << %i {right} \n", *((int*) b->data));
            tail->data = b->data;
            b = b->next;
        }

        // Only prepare for the next append operation if there is more data
        if (a && b) {

            // assign last processed as tail
            last = tail;

            // create new tail
            tail = GBLinkedList_new(NULL);

            // set the previous tail as being proceeded by the new tail
            last->next = tail;

            // Set the previous element of the new tail as being the previous tail
            tail->prior = last;
        }
    } while (a && b);

    // If one of the lists was longer than the other, append the remainder to the result list
    if (a) {
        d_printf("- merge all rest a\n");
        sort_AppendAll(result, a);
    }

    if (b) {
        d_printf("- merge all rest b\n");
        sort_AppendAll(result, b);
    }

    d_printf("- result = ");
    GBLinkedList_with(result, node, {
        d_printf("%i, ", *((int*) node->data));
    });
    d_printf("\n");

    return result;
}

GBLinkedList* GBLinkedList_sort(GBLinkedList* list, GBLL_Comparator comparator) {

    d_printf("\n\n\n%s\n", __FUNCTION__);

    size_t length = GBLinkedList_size(list);

    d_printf("size(*list) = %lu\n", length);

    unless(length > 1) return list;

    GBLinkedList* left = GBLinkedList_new(NULL);
    GBLinkedList* right = GBLinkedList_new(NULL);

    GBLinkedList* result;

    size_t pivot = length / 2;
    GBLinkedList* current = list;

    d_printf("pivot = %lu\n", pivot);

    // Populate left division
    {
        GBLinkedList* tail = left;
        GBLinkedList* last = NULL;
        size_t idx = 0;

        do {

            // Set the value of the current node in the builder chain to that
            // of the current list node
            tail->data = current->data;

            d_printf("-- {left} result << %i \n", ({
                int res = -1;
                if (current->data) {
                    res = *((int*) current->data);
                }
                assert(res >= 0);
                res;
            }));

            // If there was a builder node before this, set this nodes prior reference to that
            if (last) tail->prior = last;

            // Update the list node to the next list node
            current = current->next;

            // Avoid performing these operations if the final element in left is being appended
            if (pivot > 1 && (idx < (pivot - 2))) {

                d_printf("--- append tail \n");

                // Create the next node
                tail->next = GBLinkedList_new(NULL);

                // Set this node as the last processed node
                last = tail;

                // Set the current tail node as the next node to exist
                tail = last->next;

            }

            ++idx;

        } while (idx < (pivot - 1));

        d_printf("- {left} = ");
        printList(left);
        d_printf("\n");
    }

    // Populate right division
    {
        GBLinkedList* tail = right;
        GBLinkedList* last = NULL;
        do {
            tail->data = current->data;

            d_printf("-- {right} result << %i \n", ({
                int res = -1;
                if (current->data) {
                    res = *((int*) current->data);
                }
                assert(res >= 0);
                res;
            }));

            if (last) tail->prior = last;

            current = current->next;

            if (current) {
                tail->next = GBLinkedList_new(NULL);

                last = tail;

                tail = last->next;
            }

        } while (current);

        d_printf("- {right} = ");
        printList(right);
        d_printf("\n");
    }

    size_t leftSize = GBLinkedList_size(left);
    size_t rightSize = GBLinkedList_size(right);

    // Recursive sort down to minimum
    left = GBLinkedList_sort(left, comparator);
    d_printf("- {left[%lu]} child sort returned ", leftSize);
    printList(left);
    d_printf("\n");
    right = GBLinkedList_sort(right, comparator);
    d_printf("- {right[%lu]} child sort returned ", rightSize);
    printList(right);
    d_printf("\n");

    bool invertMerge = (comparator(left->data, right->data) <= 0);

    // Determine merge order
    if (invertMerge) {
        result = sort_MergeLists(right, left, comparator);
    } else {
        result = sort_MergeLists(left, right, comparator);
    }

    // Free left and right temp lists
    GBLinkedList_free(left);
    GBLinkedList_free(right);

    // Assign old list head pointer to new list head pointer
    return result;
}

bool GBLinkedList_contains(GBLinkedList* list, const void* key, GBLL_Comparator comparator) {

    for (GBLinkedList* current = list; current; current = current->next) {
        if (comparator(key, current->data) == 0) {
            return true;
        }
    }

    return false;
}
