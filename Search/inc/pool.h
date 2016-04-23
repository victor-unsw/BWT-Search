//
// Created by Demon on 21/04/16.
//

#ifndef SEARCH_POOL_H
#define SEARCH_POOL_H

#include "algorithm"
#include "cmath"
#include "fstream"
#include "iostream"

class Buffer{

public:
    char*   b;                                      // buffer address

    Buffer(int size = 8000):b(new char[size]){
        memset((void*)b,0,size);
    }

    ~Buffer(){
        delete [] b;
    }
};

class BPool{

private:
    std::ifstream*  fin;                                                 // file input stream
    const unsigned  BUFFER_SIZE;
    unsigned        POOL_SIZE;
    const unsigned  MEMORY_CAP;
    const unsigned  CAPACITY;
    const unsigned  TOTAL_PARTITIONS;

    Buffer      **  pool;                                               // buffer pool ( as hashmap, load = 1)
    unsigned    *   popularity;                                         // popularity of each buffer
    int             pivot;                                              // popularity pivot

    unsigned        SIZE;                                               // total buffer's in pool.
    unsigned        max_popular;                                        // max popularity
    unsigned        total_safe;                                         // total partitions returned which were already present
    unsigned        total_attempt;                                      // total partition access attempt
    unsigned        total_access;                                       // total real partition access from disk
    unsigned        total_replacement;                                  // total replacement of buffers


    //============================================================
    // CORE METHODS
    //============================================================

    /*
     * replacePage(i,o).
     * - if page already exist; clears the page.
     *   else new memory is allocated.
     * - reads the data from disk into the memory
     *
     * returns : false if any failure. else true.
     */
    inline bool getPage(int partition);

    /*
     * releasePage(p).
     * - releases given partition buffer
     *   from the pool.
     */
    inline void releasePage(int partition);

    /*
     * victim(p).
     * - starts from partition 'p' index
     *   and finds next unpopular page.
     *
     * cp := current pivot
     */
    inline int victim(int partition);

    //============================================================
    // UTILITY METHODS
    //============================================================

    /*
     * updateMax(p).
     * - update the max_popular if 'p'
     *   partition is more popular.
     * - update pivot value.
     */
    inline void updateMax(int partition);

public:
    //============================================================
    // CONSTRUCTOR & DESTRUCTOR
    //============================================================
    /*
     * default partition size   := 8000 bytes a.k.a 8 KB.
     * default memory cap       := 1,00,0000 bytes a.k.a 10 MB.
     *
     */
    BPool(std::ifstream* stream,const unsigned PSize,const unsigned TOTAL_P,const unsigned CAP):BUFFER_SIZE(PSize),
                                                                                                fin(stream),
                                                                                                MEMORY_CAP(CAP),
                                                                                                TOTAL_PARTITIONS(TOTAL_P),
                                                                                                CAPACITY(CAP/PSize),
                                                                                                POOL_SIZE(0),
                                                                                                pool(NULL),
                                                                                                popularity(NULL),
                                                                                                pivot(0),
                                                                                                max_popular(0),
                                                                                                total_access(0),
                                                                                                total_attempt(0),
                                                                                                total_replacement(0),
                                                                                                SIZE(0) {
        POOL_SIZE = CAPACITY*BUFFER_SIZE;

        // initialize the buffer pool as null
        pool    = new Buffer*[TOTAL_PARTITIONS];
        for (int i = 0; i < TOTAL_PARTITIONS; ++i)
            pool[i] = NULL;

        // initialize popularity index as 0
        popularity = new unsigned[TOTAL_PARTITIONS];
        for (int j = 0; j < TOTAL_PARTITIONS; ++j)
            popularity[j] = 0;
    }

    ~BPool(){
        fin = NULL;
        delete [] popularity;
        for (int i = 0; i < TOTAL_PARTITIONS; ++i)
            if (pool[i]!=NULL)
                delete pool[i];
        delete [] pool;
    }

    //============================================================
    // CORE METHODS
    //============================================================

    /*
     * getPartition().
     *
     *
     * returns : char *.
     *          - address of buffer
     */
    const char* getBuffer(int partition);


    void stats(){
        std::cout << "\n\t--------xxxxxxBUFFERxxxxxxx--------\n";
        printf("%20s :: [%d]\n","CAPACITY",CAPACITY);
        printf("%20s :: [%d]\n","SIZE",SIZE);
        printf("%20s :: [%d]\n","PIVOT",pivot);
        printf("%20s :: [%d]\n","MAX POPULAR",max_popular);
        printf("%20s :: [%d]\n","TOTAL ATTEMPTS",total_attempt);
        printf("%20s :: [%d]\n","TOTAL ACCESS",total_access);
        printf("%20s :: [%d]\n","TOTAL REPLACEMENT",total_replacement);
        printf("%20s :: [%d]\n","TOTAL SAFE",total_safe);
        std::cout << "\t--------xxxxxxBUFFERxxxxxxx--------\n\n";
    }

};

#endif //SEARCH_POOL_H
