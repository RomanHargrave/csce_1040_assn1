#include <stddef.h>
#include <stdbool.h>
#include "../debug.h"

#include "linkedlist.h"

#define lreal(list) ((GBListReal*)list)
#define ireal(iter) ((GBIteratorReal*)iter)

// Internal List Data Structures ---------------------------------------------------------------------------------------

/*
 * A node in a list.
 * On their own, nodes form the entire list.
 * This is referred to throughout the code in this file as a `chain'
 */
typedef struct S_GBList_Node {

    struct S_GBList_Node* prior;

    struct S_GBList_Node* next;

    void* data;

} Node;

/*
 * Real list broker.
 * This model exists for each list.
 * It holds a size, which is updated by each function which modifies the number of links. The size is available in client code
 * It also holds references to both the head and tail, allowing for O(1) prepend, append, eject, and pop.
 *
 * Due to C's memory model, it can be cast to GBList, which will work because GBList's only element, size, is in the same
 * place as cachedSize, and is also of the same type. This is used to the advantage of the list library, because it makes
 * corrupting the node order from client code impossible without intentionally doing so.
 */
typedef struct S_GBList_Real {

    size_t size;

    Node* head;

    Node* tail;

} GBListReal;

/*
 * Real Iterator.
 * Created by GBList_iterator and destroyed by GBIterator_destroy.
 *
 * It holds a void pointer that represents the data in the current node, and which is available to client code.
 *
 * In the internal implementation, it holds a pointer to the last node passed over, and the next node.
 * By default, it is meant to do forward iteration, but can be initialized for backward iteration using GBList_tailIterator
 * or for circular iteration using GBList_loopIterator.
 */
typedef struct S_GBList_Iterator_Real {

    void* data;

    Node* prior;
    Node* next;

} GBIteratorReal;

// Internal helpers ----------------------------------------------------------------------------------------------------

// -- Node Helpers -----------------------------------------------------------------------------------------------------

/*
 * Recursively traverse to the last node in a chain
 */
$Internal $NotNull()
Node* Node_tail(Node* head) {
    unless(head) return NULL;
    unless(head->next) return head;
    return Node_tail(head->next);
}

$Internal $NotNull() $Optimize(2)
Node* Node_head(Node* tail) {
    unless(tail) return NULL;
    unless(tail->prior) return tail;
    return Node_head(tail->prior);
}

$Internal $Optimize(2)
size_t Node_countLinks(Node* head) {
    unless(head) return 0;
    return 1 + Node_countLinks(head->next);
}

$Internal $WrapperFunction
bool Node_eq(Node* a, Node* b) {
    return (a->prior == b->prior)
           && (a->data == b->data)
           && (a->next == b->next);
}

// -- List Helpers -----------------------------------------------------------------------------------------------------

/*
 * Traverse to the Ith node in a given GBListReal.
 * In attempt to optimize traversal time (such that it may be O(N/2) as opposed to O(N)),
 * the center of the list, P, will be calculated by calculating half of size, and determining weather
 *  I lies before or after that pivot.
 * If I lies before the pivot, the node chain will be traversed from the first element in the chain until I is reached
 * If I lies after the pivot, the node chain will be traversed backwards from the last element in the chain until
 *  I is reached.
 *
 *
 *  If  I = P then backward reversal will be the default mode of traversal.
 */
$Internal $NotNull()
Node* List_nthNode(GBListReal* list, size_t index) {

    if (list->size < 2) return list->head;
    if (index > list->size) return NULL;

    size_t pivot = list->size / 2;

    /*
     * If index is after the pivot value, traverse up, otherwise down
     */
    Node* current = NULL;
    if (index >= pivot) {
        current = list->tail;
        for (size_t idx = list->size - 1;
             idx > index; --idx) {
            current = current->prior;
        }
    } else {
        current = list->head;
        for (size_t idx = 0; idx < index; ++idx) current = current->next;
    }

    return current;
}

$Internal $WrapperFunction
void List_onListUpdate(GBListReal* list) {
    d_printf("onListUpdate\n");
    if (!list->head && list->tail) {
        d_printf("fix head\n");
        list->head = Node_head(list->tail);
    } else if (!list->tail && list->head) {
        d_printf("fix tail\n");
        list->tail = Node_tail(list->head);
    } else if (!list->head && !list->head) {
        d_printf("bob list\n");
        list->size = 0;
    }

    if (list->size < 1 && list->head) {
        d_printf("fix size\n");
        list->size = Node_countLinks(list->head);
    }
}

/*
 * Allocated a new Node
 */
$Internal $Allocator $WrapperFunction $NoProfiler
Node* Node_allocate(void* data) {
    Node* node = gb_new(Node);
    node->data = data;
    return node;
}

/*
 * Perform necessary procedures to dispose of a node
 */
$Internal
void Node_destroy(Node* node) {
    gb_free(node);
}

$Internal $Optimize(2)
void Node_destroyChain(Node* head) {

    unless(head) return;

    Node* next = head->next;

    head->next = NULL;
    head->prior = NULL;
    Node_destroy(head);

    Node_destroyChain(next);
}

// ---------------------------------------------------------------------------------------------------------------------

// -- Iterator API -----------------------------------------------------------------------------------------------------

// ---- Iterator Management --------------------------------------------------------------------------------------------

$NoProfiler
GBIterator* GBList_iterator(GBList* list) {
    unless(list->size > 0) return NULL;

    GBIteratorReal* iterator = gb_new(GBIteratorReal);

    iterator->data = NULL;
    iterator->next = lreal(list)->head;

    return (GBIterator*) iterator;
}

$NoProfiler
GBIterator* GBList_tailIterator(GBList* list) {
    unless(list->size > 0) return NULL;

    GBIteratorReal* iterator = gb_new(GBIteratorReal);

    iterator->data = NULL;
    iterator->prior = lreal(list)->tail;

    return (GBIterator*) iterator;
}

$NoProfiler
GBIterator* GBList_loopIterator(GBList* list) {
    GBIteratorReal* iterator = ireal(GBList_iterator(list));
    unless(iterator) return NULL;

    iterator->prior = lreal(list)->tail;

    return (GBIterator*) iterator;
}

$NoProfiler
void GBIterator_destroy(GBIterator* iterator) {
    gb_free(ireal(iterator));
}

// ---- Iterator Access ------------------------------------------------------------------------------------------------

$WrapperFunction $NoProfiler
bool GBIterator_hasNext(GBIterator* iterator) {
    d_printf("GBIterator_hasNext() => iterator->next = %p\n", ireal(iterator)->next);
    return ireal(iterator)->next != NULL;
}

$NoProfiler
void* GBIterator_next(GBIterator* iterator) {
    unless(GBIterator_hasNext(iterator)) return NULL;

    GBIteratorReal* real = ireal(iterator);

    real->data = real->next->data;

    Node* next = real->next->next;
    real->prior = real->next;
    real->next = next;

    return real->data;
}

$NoProfiler
bool GBIterator_hasPrior(GBIterator* iterator) {
    return ireal(iterator)->prior != NULL;
}

$NoProfiler
void* GBIterator_prior(GBIterator* iterator) {
    unless(GBIterator_hasPrior(iterator)) return NULL;

    GBIteratorReal* real = ireal(iterator);

    real->data = real->prior->data;

    Node* next = real->prior->prior;
    real->next = real->prior;
    real->prior = next;

    return real->data;
}

// -- List API ---------------------------------------------------------------------------------------------------------

// ---- Constructors / Factories ---------------------------------------------------------------------------------------

GBList* GBList_new() {

    GBListReal* listBroker = gb_new(GBListReal);
    listBroker->head = NULL;
    listBroker->tail = NULL;
    listBroker->size = 0;

    return (GBList*) listBroker;
}

$WrapperFunction
GBList* GBList_new_with(void* initialData) {
    GBList* list = GBList_new();
    GBList_append(list, initialData);
    return list;
}

// ---- Destructors ----------------------------------------------------------------------------------------------------

void GBList_free(GBList* list) {
    Node_destroyChain(lreal(list)->head);
    gb_free(lreal(list));
}

// ---- List Inspection ------------------------------------------------------------------------------------------------

$WrapperFunction
GBListSize GBList_size(GBList* list) {
    return list->size;
}

$WrapperFunction
void* GBList_first(GBList* list) {
    unless(lreal(list)->head) return NULL;
    return ((GBListReal*) list)->head->data;
}

$WrapperFunction
void* GBList_last(GBList* list) {
    unless(lreal(list)->tail) return NULL;
    return ((GBListReal*) list)->tail->data;
}

$WrapperFunction
void* GBList_get(GBList* list, size_t index) {
    Node* node = List_nthNode(lreal(list), index);
    unless(node) return NULL;
    return node->data;
}

GBList* GBList_find(GBList* list, const void* key, GBLL_Comparator comparator) {

    GBList* found = GBList_new();

    GBList_with(list, current, {
        if (comparator(key, current) == 0) GBList_append(found, current);
    });

    return found;
}

bool GBList_contains(GBList* list, const void* key, GBLL_Comparator comparator) {

    bool wasFound = false;

    GBList_with(list, value, {
        if (comparator(key, value) == 0) {
            wasFound = true;
            break;
        }
    })

    return wasFound;
}

// ---- List Manipulation ----------------------------------------------------------------------------------------------

// ------ Addition / Insertion -----------------------------------------------------------------------------------------

void GBList_prepend(GBList* list, void* ptr) {

    // [H]=[]=[T]
    GBListReal* real = lreal(list);

    // [] [H]=[]=[T]
    Node* newHead = Node_allocate(ptr);

    // []-[H]=[]=[T]
    newHead->next = real->head;

    // []=[H]=[]=[T]
    if (real->head) real->head->prior = newHead;

    // [H]=[]=[]=[T]
    real->head = newHead;
    unless(real->tail) real->tail = real->head;

    ++real->size;
}

void GBList_append(GBList* list, void* ptr) {

    d_printf("append ");

    // [H]=[]=[T]
    GBListReal* real = lreal(list);

    printf("(%p) ", real);

    // [H]=[]=[T] []
    Node* newTail = Node_allocate(ptr);

    printf("new tail %p , ", newTail);

    // [H]=[]=[T]-[]
    newTail->prior = real->tail;

    printf("prior %p", real->tail);

    // [H]=[]=[T]=[]
    if (real->tail) {
        printf(", old tail (%p) -> next = %p", real->tail, newTail);
        real->tail->next = newTail;
    }

    // [H]=[]=[]=[T]
    real->tail = newTail;
    unless(real->head) {
        printf(", fix head (%p) = %p", real->head, real->tail);
        real->head = real->tail;
    }

    printf("\n");

    ++real->size;
}

bool GBList_insertBefore(GBList* list, void* ptr, size_t index) {

    GBListReal* real = lreal(list);
    Node* at = List_nthNode(real, index);

    unless(at) return false;

    if (at == real->head) {
        GBList_prepend(list, ptr);
    } else {

        Node* new = Node_allocate(ptr);

        // [1]=[2]=[3]
        Node* oldPrior = at->prior;

        // [1] [4]=[2]=[3]
        at->prior = new;
        new->next = at;

        // [1]=[4]=[2]=[3]
        if (oldPrior) {
            new->prior = oldPrior;
            oldPrior->next = new;
        }

        ++real->size;
    }

    return true;
}

bool GBList_insertAfter(GBList* list, void* ptr, size_t index) {

    GBListReal* real = lreal(list);
    Node* at = List_nthNode(real, index);

    unless(at) return false;

    if (at == real->tail) {
        GBList_append(list, ptr);
    } else {

        Node* new = Node_allocate(ptr);

        // [1]=[2]=[3]
        Node* oldNext = at->next;

        // [1]=[2]=[4] [3]
        at->next = new;
        new->prior = at;

        // [1]=[2]=[4]=[3]
        if (oldNext) {
            new->next = oldNext;
            oldNext->prior = new;
        }

        ++real->size;
    }

    return true;
}

bool GBList_set(GBList* list, void* ptr, size_t index) {

    Node* targetNode = List_nthNode(lreal(list), index);

    unless(targetNode) return false;

    targetNode->data = ptr;

    return true;
}

// ------ Neutral Modification -----------------------------------------------------------------------------------------

void GBList_reverse(GBList* list) {

    GBListReal* real = lreal(list);

    Node* current = real->head;
    Node* swap;

    do {
        swap = current->prior;
        current->prior = current->next;
        current->next = swap;
        current = current->prior;
    } while (current);

    Node* head = real->head;
    real->head = real->tail;
    real->tail = head;
}

// ------ Negative Modification ----------------------------------------------------------------------------------------

void* GBList_pop(GBList* list) {

    GBListReal* real = lreal(list);

    unless(real->head && real->size > 0) return NULL;

    // [<]=[_]=[_]=[>]
    Node* oldHead = real->head;

    // [_]-[<]=[_]=[>]
    real->head = oldHead->next;
    if (real->size == 1) real->head = real->tail;

    --real->size;

    // [_] [<]=[_]=[>]
    if (real->size > 0) real->head->prior = NULL;

    void* oldData = oldHead->data;

    // [<]=[_]=[>]
    Node_destroy(oldHead);

    return oldData;
}

void* GBList_eject(GBList* list) {

    GBListReal* real = lreal(list);

    unless(real->tail && real->size > 0) return NULL;

    // [<]=[_]=[_]=[>]

    Node* oldTail = real->tail;

    // [<]=[_]=[>]-[_]
    real->tail = oldTail->prior;
    if (real->size == 1) real->head = real->tail;

    --real->size;

    // [<]=[_]=[>] [_]
    if (real->size > 0) real->tail->next = NULL;

    void* oldData = oldTail->data;

    // [<]=[_]=[>]
    Node_destroy(oldTail);

    return oldData;
}

GBListSize GBList_remove(GBList* list, void const* pVoid, GBLL_Comparator comparator) {

    GBListReal* real = lreal(list);
    GBListSize removalCount = 0;

    Node* current = real->head;
    while (current) {
        if (comparator(pVoid, current->data) == 0) {
            if (current->prior) current->prior->next = current->next;
            if (current->next) current->next->prior = current->prior;
            Node* old = current;
            current = current->next;
            Node_destroy(old);
            ++removalCount;
        } else {
            current = current->next;
        }
    }

    real->size -= removalCount;

    return removalCount;
}

bool GBList_removeAt(GBList* list, GBListSize index) {

    GBListReal* real = lreal(list);
    Node* at = List_nthNode(real, index);

    unless(at) return false;

    at->prior->next = at->next;
    at->next->prior = at->prior;

    Node_destroy(at);

    --real->size;

    return true;
}

$WrapperFunction
void GBList_clear(GBList* list) {
    Node_destroyChain(lreal(list)->head);
    lreal(list)->head = NULL;
    lreal(list)->tail = NULL;
    lreal(list)->size = 0;
}

// ------ List Computations --------------------------------------------------------------------------------------------

void* GBList_foldRight(GBList* list, const void* x, void* (* lambda)(const void*, const void*)) {

    GBIterator* iterator = GBList_iterator(list);

    void* accum = NULL;
    if (GBIterator_hasNext(iterator)) {
        accum = lambda(x, GBIterator_next(iterator));
        while (GBIterator_hasNext(iterator)) {

            void* lastAccum = accum;

            accum = lambda(accum, GBIterator_next(iterator));

            gb_free(lastAccum);

        }
    }

    GBIterator_destroy(iterator);

    return accum;
}

GBList* GBList_map(GBList* list, void* (* lambda)(const void*)) {
    return GBList_mapWith(list, x, lambda(x));
}

// ------ List Sorting -------------------------------------------------------------------------------------------------

// -------- List Sort Helpers ------------------------------------------------------------------------------------------

$Internal $NotNull(1, 3)
void List_sortImpl(GBList* list, GBListSize length, GBLL_Comparator comparator) {

    unless(length > 1) {
        d_printf("list fully reduced\n");
        return;
    }

    GBList* left = GBList_new();
    GBList* right = GBList_new();

    d_printf("list->size = %lu\n", length);

    trace_disable();

    {
        d_printf("append to left\n");
        GBListSize pivotIndex = length / 2;
        GBIterator* iterator = GBList_iterator(list);

        for (GBListSize idx = 0; idx < pivotIndex; ++idx) GBList_append(left, GBIterator_next(iterator));

        d_printf("append to right\n");
        trace_enable();
        while (GBIterator_hasNext(iterator)) GBList_append(right, GBIterator_next(iterator));
        trace_disable();

        GBIterator_destroy(iterator);
    }

    d_printf("pivot is %lu , left size is %lu , right size is %lu\n", length / 2, left->size, right->size);
    trace_enable();
    d_printf("sort left\n");
    List_sortImpl(left, left->size, comparator);
    d_printf("sort right\n");
    List_sortImpl(right, right->size, comparator);

    GBList_clear(list);

    if (comparator(GBList_last(left), GBList_first(right)) <= 0) {

        d_printf("reorder mode (right >> left)\n");
        trace_disable();

        GBIterator* iLeft = GBList_iterator(left);
        GBIterator* iRight = GBList_iterator(right);

        while (GBIterator_hasNext(iLeft)) GBList_append(list, GBIterator_next(iLeft));
        while (GBIterator_hasNext(iRight)) GBList_append(list, GBIterator_next(iRight));

        GBIterator_destroy(iLeft);
        GBIterator_destroy(iRight);

    } else {

        d_printf("merge mode\n");

        while ((left->size > 0) && (right->size > 0)) {
            if (comparator(GBList_first(left), GBList_first(right)) <= 0) {
                GBList_append(list, GBList_pop(left));
            } else {
                GBList_append(list, GBList_pop(right));
            }
        }

        d_printf("merge complete\n");

        if(left->size > 0) {
            d_printf("append left remainder\n");
            GBIterator* iterator = GBList_iterator(left);
            while(GBIterator_hasNext(iterator)) GBList_append(list, GBIterator_next(iterator));
            GBIterator_destroy(iterator);
        }

        if(right->size > 0) {
            d_printf("append right remainder\n");
            GBIterator* iterator =GBList_iterator(right);
            while(GBIterator_hasNext(iterator)) GBList_append(list, GBIterator_next(iterator));
            GBIterator_destroy(iterator);
        }

    }

    trace_enable();

    GBList_free(left);
    GBList_free(right);
}

// -------- List Sort API Implementation -------------------------------------------------------------------------------

$Weak
void GBList_sort(GBList* list, GBLL_Comparator comparator) {
    List_sortImpl(list, list->size, comparator);
}

// ---------------------------------------------------------------------------------------------------------------------

