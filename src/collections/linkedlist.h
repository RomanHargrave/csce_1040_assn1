/*
 * Roman Hargrave. 2015, Apache License w/ reserved privileges for author when stated.
 * Created for use in CSCE 1040 project at UNT.
 *
 * roman@hargrave.info.
 *
 * Not published.
 *
 * It's a list... It's a deque.. It's GBLinkedList!
 *
 * Provides a doubly linked collection with deque-like features. Each value inserted is
 * encapsulated in a GBLinkedNode (which should not be used externally), which is memory
 * managed internally by the linked list library.
 */

#ifndef _H_LINKED_LIST_GB
    #define _H_LINKED_LIST_GB
    #include <stddef.h>
    #include <stdbool.h>

typedef int (*GBLL_Comparator)(const void* a, const void* b);

typedef struct S_GBLinkedNode GBLinkedNode;

typedef struct S_GBLinkedNode {

    GBLinkedNode* preceeding;

    void* value;

    GBLinkedNode* proceeding;

} GBLinkedNode;

typedef struct S_GBLinkedList {

    GBLinkedNode* head;

    GBLinkedNode* tail;

    GBLL_Comparator comparator;

} GBLinkedList;

/*
 * Determines the size of the list by counting the number of nodes in
 * the list
 */
size_t GBLinkedList_size(GBLinkedList* list);

/*
 * Inserts in to the position in the list where the comparator says that this should go
 */
void GBLinkedList_add(GBLinkedList* list, void* ptr);

/*
 * Counts nodes up to `index`, returning false if the chain ends before `index` is reached.
 * Once `index` is reached, the node at the position will have its value updated to
 * `ptr` instead of its previous value, after which a value of true will be returned.
 *
 * You shouldn't use this on large collections
 *
 * Warning: I am O(k) where `k` is the number of nodes (!)
 */
bool GBLinkedList_set(GBLinkedList* list, void* ptr, size_t index);

/*
 * Like `_insert`, however, instead of replacing the preexisting node, a new node will be initialized
 * and 'spliced' between index->preceding and index itself, thus meaning that index->preceding is the
 * node containing `ptr`.
 *
 * This additionally means that index is shifted right by one index
 *
 * Warning: I am O(k) where `k` is the number of nodes (!)
 */
bool GBLinkedList_splice(GBLinkedList* list, void* ptr, size_t index);

/*
 * Like `splice`, but creates nPtrs new nodes, inserting such that the last node will precede the node
 * previously at `index`.
 *
 * This will shift index nPtrs right, meaning that ptrs[0] will now occupy the numerical index
 * specified.
 *
 * Warning: I am O(k + nPtrs) where k is the number of nodes
 */
bool GBLinkedList_spliceAll(GBLinkedList* list, void* ptrs[], size_t nPtrs, size_t index);

/*
 * Appends `ptr` to the end of the list
 */
void GBLinkedList_enqueue(GBLinkedList* list, void* ptr);

/*
 * Puts `ptr` at the top of the list
 */
void GBLinkedList_push(GBLinkedList* list, void* ptr);

/*
 * Pop from the tail and return the value that was previously at the tail.
 * Returns NULL if the list is 0 nodes long, or that element should have a NULL value.
 */
void* GBLinkedList_eject(GBLinkedList* list);

/*
 * Returns the value at the top of the list
 */
void* GBLinkedList_peekFront(GBLinkedList* list);

/*
 * Returns the value at the end of the list
 */
void* GBLinkedList_peekLast(GBLinkedList* list);

/*
 * Removes and returns the value at the top of the list
 * If no more values are available, NULL will be returning.
 */
void* GBLinkedList_pop(GBLinkedList* list);

/*
 * Unlink the specified node.
 * This is accomplished by unlinking this node, and linking the preceding node to
 * the proceeding node.
 *
 * You probably want to use remove with the value of your node, because that will
 * free the memory allocated to the node (if any).
 */
void GBLinkedList_unlink(GBLinkedNode* node);

/*
 * Performs the following operations:
 *  - GBLinkedList_findNode(list, key)
 *  - NULL? Return false
 *  - GBLinkedList_unlink(node)
 *  - free(node)
 *  - Return true
 */
bool GBLinkedList_remove(GBLinkedList* list, void* key);

/*
 * Counts nodes up to index and returns the value of the node at that point.
 * Will return NULL if index exceeds the number of nodes in the chain
 */
void* GBLinkedList_get(GBLinkedList* list, size_t index);

/*
 * Performs GBLinkedList_findNode and then returns the value of that node.
 * Returns NULL if no value is found
 */
void* GBLinkedList_find(GBLinkedList* list, const void* key);

/*
 * Traverse the list until the internal comparator returns a difference of 0 for key and node value
 * Returns the node that has a matching value, or NULL if not found.
 */
GBLinkedNode GBLinkedList_findNode(GBLinkedList* list, const void* key);

/*
 * Sort the specified list using the internal comparator
 */
void GBLinkedList_sort(GBLinkedList* list);

/*
 * Returns true if GBLinkedList_findNode(...) exists, and (...)->value is non-NULL
 */
bool GBLinkedList_contains(GBLinkedList* list, const void* key);

#endif