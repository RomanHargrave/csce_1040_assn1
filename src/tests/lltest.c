#define _GNU_SOURCE
#include "../collections/linkedlist.h"
#include "../debug.h"
#include "../util.h"
#include <dlfcn.h>
#include <assert.h>



$Internal
typedef struct s_datum {
    int data;
} Datum;

$Hidden $Comparator
int compareDatum(const void* a, const void* b) {
    if (a && b) {
        return ((Datum*) a)->data - ((Datum*) b)->data;
    } else {
        return (!a && !b) ? 0 : (a ? -1 : 1);
    }
}

$Internal
const int NPTRS = 10;

int main(void) {

    trace_enable();
    trace_root();

    setbuf(stdout, NULL);

    const int PASS_COUNT = 5;

    GBList* testList;

    /*
     * Insure that head & tail are updated on initial addition, removal of last element
     */

    testList = GBList_new();

    assert(testList->size == 0);
    assert(GBList_first(testList) == NULL);
    assert(GBList_last(testList) == NULL);

    printf("Test head/tail repair\n");

    {
        Datum testData = { 9001 };

        GBList_append(testList, &testData);

        assert(testList->size == 1);
        assert(GBList_first(testList) != NULL);
        assert(GBList_last(testList)  != NULL);

        assert(GBList_first(testList) == GBList_last(testList));

        GBList_clear(testList);

        assert(testList->size == 0);
    }

    printf("Testing order integrity, iterator\n");

    {
        Datum testData[10] = { {1}, {3}, {5}, {7}, {9}, {11}, {13}, {15}, {17}, {19} };

        for(GBListSize idx = 0; idx < 10; ++idx) GBList_append(testList, &testData[idx]);

        assert(testList->size == 10);
        assert(((Datum*) GBList_first(testList))->data == testData[0].data);
        assert(((Datum*) GBList_last(testList))->data == testData[9].data);

        GBIterator* iterator = GBList_iterator(testList);

        for(GBListSize idx = 0; idx < 10; ++idx) assert(((Datum*) GBIterator_next(iterator))->data == testData[idx].data);

        GBIterator_destroy(iterator);

        GBList_clear(testList);

        assert(testList->size == 0);
    }

    printf("Testing reverse\n");

    {
        Datum testData[3] = { {1}, {2}, {3} };

        for(GBListSize idx = 0; idx < 3; ++idx) GBList_append(testList, &testData[idx]);

        GBList_reverse(testList);

        GBIterator* iterator = GBList_iterator(testList);

        for(GBListSize idx = 2; idx > 0; --idx) assert(((Datum*) GBIterator_next(iterator))->data == testData[idx].data);

        GBIterator_destroy(iterator);

        GBList_clear(testList);

        assert(testList->size == 0);
    }

    printf("Testing dequeue behavior\n");

    {
        Datum testData[10] = { {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10} };

        for(GBListSize idx = 0; idx < 10; ++idx) {
            GBList_append(testList, &testData[idx]);
            assert(testList->size == 1);
            assert(((Datum*) GBList_eject(testList))->data == testData[idx].data);
        }

        assert(testList->size == 0);

        for(GBListSize idx = 0; idx < 10; ++idx) {
            GBList_prepend(testList, &testData[idx]);
            assert(testList->size == 1);
            assert(((Datum*) GBList_pop(testList))->data == testData[idx].data);
        }

        assert(testList->size == 0);
    }

    printf("Testing random access\n");

    {
        Datum testData[10] = { {1}, {3}, {5}, {7}, {9}, {11}, {13}, {15}, {17}, {19} };

        for(GBListSize idx = 0; idx < 10; ++idx) GBList_append(testList, &testData[idx]);

        for(int pass = 0; pass < 5; ++pass) {
            GBListSize randomIndex = ((GBListSize) rand()) % 9;
            assert(testData[randomIndex].data == ((Datum*) GBList_get(testList, randomIndex))->data);
        }

        GBList_clear(testList);

        assert(testList->size == 0);
    }

    printf("Testing sort\n");

    {
        Datum testData[100];
        for(size_t idx = 0; idx < 100; ++idx) {
            testData[idx] = (Datum) { .data = rand() };
            GBList_append(testList, &testData[idx]);
        }

        GBList_sort(testList, compareDatum);

        {
            Datum* last = NULL;
            GBIterator* iterator = GBList_iterator(testList);
            for(size_t idx = 0; idx < 100; ++idx) {
                Datum* this = (Datum*) GBIterator_next(iterator);
                if(last) assert(this->data >= last->data);
                last = this;
            }
        }
    }

    printf("Done\n");

    GBList_free(testList);

    return 0;
}
