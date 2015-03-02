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
#include "../util.h"

typedef int (*GBLL_Comparator)(const void* a, const void* b);

typedef struct S_GBLinkedList {

    struct S_GBLinkedList* prior;

    struct S_GBLinkedList* next;

    void* data;

} GBLinkedList;

/*
 * Handy-dandy preprocessor macro that iterates through a list and handles the boiler plate!
 * This borders on preprocessor abuse (don't flog me!)
 */
#define GBLinkedList_with(head, with, lambda) {\
    GBLinkedList* with = head;\
    do {\
     lambda;\
     with = with->next;\
    } while ( with );\
}

/*
 * Preprocessor macro that allows you to perform a Map function with `lambda` where lambda is an expression.
 * `x` will define the name of the variable visible inside the lambda which represents the current value.
 *
 * There is no question is to whether this is preprocessor abuse.
 * It is __flagrant__ preprocessor abuse.
 *
 * It also uses GCC compound statements.
 */
#define GBLinkedList_mapWith(head, x, lambda) ({\
    GBLinkedList* mapped = NULL;\
    GBLinkedList_with(head, currentNode, {\
        void* x = currentNode->data;\
        void* yield = (void*) ( lambda );\
        if(mapped) {\
            mapped = GBLinkedList_prepend(mapped, yield);\
        } else {\
            mapped = GBLinkedList_new(yield);\
        }\
    });\
    GBLinkedList_reverse(&mapped);\
    mapped;\
})


/*
 * Initialize a new GBLinkedList
 */
F_CONSTRUCTOR
GBLinkedList* GBLinkedList_new(void* initialData);

/*
 * Free a GBLinkedList (note that this will not free #data)
 */
ARGS_EXIST(1)
void GBLinkedList_free(GBLinkedList* list) ;

/*
 * Determines the size of the list by counting the number of nodes in
 * the list
 */
ARGS_EXIST(1)
size_t GBLinkedList_size(GBLinkedList* list);

/*
 * Inserts in to the position in the list where the comparator says that this should go
 */
ARGS_EXIST(1)
GBLinkedList* GBLinkedList_append(GBLinkedList* list, void* ptr);

/*
 * Counts nodes up to `index`, returning false if the chain ends before `index` is reached.
 * Once `index` is reached, the node at the position will have its value updated to
 * `ptr` instead of its previous value, after which a value of true will be returned.
 *
 * You shouldn't use this on large collections
 *
 * Warning: I am O(k) where `k` is the number of nodes (!)
 */
ARGS_EXIST(1)
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
ARGS_EXIST(1)
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
ARGS_EXIST(1)
bool GBLinkedList_spliceAll(GBLinkedList* list, void* ptrs[], size_t nPtrs, size_t index);

/*
 * Puts `ptr` at the top of the list
 */
ARGS_EXIST(1)
GBLinkedList* GBLinkedList_prepend(GBLinkedList* list, void* ptr);

/*
 * Pop from the tail and return the value that was previously at the tail.
 * Returns NULL if the list is 0 nodes long, or that element should have a NULL value.
 */
ALL_ARGS_EXIST
void* GBLinkedList_eject(GBLinkedList* list);

/*
 * Returns the value at the top of the list
 */
ALL_ARGS_EXIST
void* GBLinkedList_first(GBLinkedList* list);

/*
 * Returns the value at the end of the list
 */
ALL_ARGS_EXIST
void* GBLinkedList_last(GBLinkedList* list);

/*
 * Performs an operation on each element in the list taking two arguments:
 *
 *      (x, y) => f(x,y)
 *
 *      where
 *          for the first iteration
 *              x is provided to the function call
 *          for all other iterations
 *              x is the result of the previous operation
 *          y is the nth element in the list
 *
 *  A an example use case might be summing all values in a list:
 *
 *      let Sum(x, y) in
 *          x + y
 *
 *      FoldRight(Sequence(1, 2, 3), 0, sum) ==
 *
 *          ( 0 + 1 -> 1
 *            1 + 2 -> 3
 *            3 + 3 -> 6 )
 *
 *          == 6
 *
 *
 *      Essentially, this can be unwound as follows:
 *
 *          # Where `N` is the nth index of Sequence(1, 2, 3)
 *
 *          let x 0
 *
 *          # N = 0, x = 0
 *          let x Sum(x, 1)
 *
 *          # N = 1, x = 1
 *          let x Sum(x, 2)
 *
 *          # N = 2, x = 3
 *          let x Sum(x, 3)
 *
 *          x = 6
 *
 *      Alternatively,
 *
 *          (sum 3
 *              (sum 2
 *                  (sum 1 0)))
 *
 *      Which simplifies to:
 *
 *          (sum 3 2 1)
 *
 *          ;; or
 *          (sum 1 2 3)
 *
 */
ARGS_EXIST(1, 3)
void* GBLinkedList_foldRight(GBLinkedList*, const void*, void*(*)(const void*, const void*));

/*
 * Maps the output of a function applied to element N in a list
 *
 * Example:
 *
 *      let Product(x)(y) in
 *          x * y
 *
 *      Map(Sequence(1, 2, 3), Product(2)) = Sequence(2, 4, 6)
 *
 *  Rules:
 *
 *      # Where Application(x) is an arbitrary function that is both pure and deterministic
 *      # such that Application(x) = 'x , and Inverse(Application)('x) = x are both true at any given time
 *      let Application(x)
 *
 *      let a Sequence(1, 2, 3)
 *      let b Map(a, Application)
 *
 *      # Where N is an arbitrary index within both `a` and `b`
 *      Take(b, N) = Application(Take(a, N))
 *
 */
ALL_ARGS_EXIST
GBLinkedList* GBLinkedList_map(GBLinkedList*, void*(*)(const void*));

/*
 * Performs the following operations:
 *  - GBLinkedList_findNode(list, key)
 *  - NULL? Return false
 *  - GBLinkedList_unlink(node)
 *  - free(node)
 *  - Return true
 */
ARGS_EXIST(1, 3)
bool GBLinkedList_remove(GBLinkedList*, const void*, GBLL_Comparator);

/*
 * Counts nodes up to index and returns the value of the node at that point.
 * Will return NULL if index exceeds the number of nodes in the chain
 */
ARGS_EXIST(1)
void* GBLinkedList_get(GBLinkedList* list, size_t index);

/*
 * Performs GBLinkedList_findNode and then returns the value of that node.
 * Returns NULL if no value is found
 */
ARGS_EXIST(1)
GBLinkedList* GBLinkedList_find(GBLinkedList*, const void*, GBLL_Comparator);

/*
 * Sort the specified list using the internal comparator
 */
ALL_ARGS_EXIST
GBLinkedList* GBLinkedList_sort(GBLinkedList*, GBLL_Comparator);

/*
 * Returns true if GBLinkedList_findNode(...) exists, and (...)->value is non-NULL
 */
ARGS_EXIST(1, 3)
bool GBLinkedList_contains(GBLinkedList*, const void*, GBLL_Comparator);

#endif