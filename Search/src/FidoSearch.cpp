//
// Created by Demon on 19/04/16.
//

//
//  ' we have to fix 'a' query from tiny.bwt

#include <iostream>
#include <cstring>
#include <fstream>
#include <set>

#include "../inc/preprocess.h"
#include "../inc/FidoSearch.h"

using namespace std;

//============================================================
// CORE METHODS
//============================================================

/*
 * C(c).
 * - core method.
 * - finds the total number of text chars in 'T'
 *   which are alphabetically smaller than 'c'.
 *
 * RETURNS :-
 * - integer; i.e. total count of smaller characters.
 */
inline unsigned FidoSearch::C(const char c) {
    return sortBucket.freq[c];
}

/*
 * Occ(c,q).
 * - core method
 * - finds number of occurance of char 'c' in prefix L[1,q]
 *
 * c := character of which we need to find occurance
 * q := exact index till which you wish to count.
 *
 * RETURNS :-
 * - integer; i.e. total occurance of char 'c' in L[1,q].
 */
unsigned FidoSearch::Occ(const char c, const unsigned q) {
    unsigned    current_partition   = (q/PARTITION_SIZE);
    int         pre_partition       = current_partition - 1;
    unsigned    frequency           = 0;
    FBucket     tempBucket;

    // fill bucket with pre partition frequencies
    if (pre_partition >= 0){
        fillBucket(&tempBucket,unsigned(pre_partition));
        frequency = tempBucket.freq[c];
    }


    // do linear frequency count over current partition
    unsigned    start   = current_partition * PARTITION_SIZE;                           // start position for read in 'bwt' file

    const char* buffer  = pool->getBuffer(current_partition);                           // get buffer from pool

    if (buffer == NULL){
        std::cout << "no buffer received";
        exit(1);
    }

    int LIMIT = (q-start)+1;
    for (int i = 0; i < LIMIT; ++i)
        if (buffer[i] == c)
            frequency++;

    return frequency;
}

/*
 * BS(P)
 * - backward search.
 * - returns total occurrance of pattern 'P'.
 * - return -1 if no occurrance found.
 */
int FidoSearch::BS(const std::string P) {

    size_t      loc     = P.size()-1;
    char        c       = P[loc];
    char        n       = nextAlive(c);                 // next character in global bucket

    unsigned    FIRST   = C(c);
    unsigned    LAST    = (int(n) == 0) ? C(c) + globalBucket.freq[c] - 1 : C(n) - 1;
    while ((FIRST <= LAST) && loc >= 1){
        c       = P[--loc];
        FIRST   = C(c) + Occ(c,FIRST == 0 ? 0 : FIRST-1);
        LAST    = C(c) + Occ(c,LAST) - 1;
    }

    if (FIRST > LAST) {
        cout << "First > Last: returning -1\n";
        return -1;
    }
    int total_records = (LAST-FIRST+1);

    if (OBJECTIVE == R || OBJECTIVE == A) {
        for (unsigned i = FIRST, j = 1; i <= LAST; ++i, j++)
            decode(i);
        if (OBJECTIVE == A){
            for(auto it=list.begin();it!=list.end();it++)
                cout<<"["<< *it << "]\n";
        } else{
            cout << list.size() << endl;
        }
    }else{
        cout << total_records << endl;
    }

    return total_records;
}

/*
 * decode(i).
 * - core method.
 * - decodes the bwt text from index
 *   until '[' found.
 */
void FidoSearch::decode(unsigned index) {
    char c = 0;
    unsigned next = index;

    std::string value;value.reserve(100);
    bool fill = false;
    while (c != '['){
        int p = int(next/PARTITION_SIZE);                                               // partition of character we need

        int PSTART = int(p * PARTITION_SIZE);
        int target = next - PSTART;

        const char * buffer = pool->getBuffer(p);

        if (buffer == NULL){
            std::cout << "no buffer received";
            exit(1);
        }
        c = buffer[target];
        if (fill)
            value.insert(value.begin(),c);

        unsigned o = Occ(c,next);                               // major time con
        unsigned u = C(c);
        next = u + o - 1;

        if (c == ']') {
            fill = true;
        }
    }
    value.erase(value.begin());
    list.insert(atoi(value.c_str()));
}

/*
 * crunch().
 * - core method to search & decode
 *   pattern.
 */
void FidoSearch::crunch(const char* P) {
    clock_t t = clock();
    if (!INDEX_EXISTS){
        indexer = new PreProcess(fin,INDEX_FILE,FILE_SIZE,PARTITION_SIZE,TOTAL_PARTITIONS);
        indexer->index();
        delete indexer;
        index.open(INDEX_FILE);
    }

    if (!index.is_open()){
        std::cout << "ERROR IN OPENING Index File\n";
        exit(1);
    }

    // fill global frequency bucket
    fillBucket(&globalBucket,TOTAL_PARTITIONS-1);
    index.seekg(0);
    if (globalBucket.isEmpty()){
        std::cout << "Global bucket empty\n";
        exit(1);
    } else{
        // fill the sort bucket frequency
        int size = sortBucket.getSize();
        unsigned count = 0;
        for (int i = 0; i < size; ++i) {
            if (globalBucket.freq[i] == 0)
                continue;
            sortBucket.freq[i] = count;
            count += globalBucket.freq[i];
        }
    }

    std::string pattern(P);
    int r = BS(pattern);
    if (r == -1) {
        std::cout << "WARNING [no pattern exist]\n";
        exit(1);
    }
    t = clock() - t;
}


//============================================================
// UTILITY METHODS
//============================================================

/*
 * fillBucket(b,o).
 * - fills the bucket from 'Index file' according
 *   to given offset
 */
inline void FidoSearch::fillBucket(FBucket *bucket, unsigned partition) {
    if (index.eof())
        index.clear();

    index.seekg(partition* unsigned(bucket->getSize()* sizeof(unsigned)),std::ios_base::beg);
    index.read((char*)bucket->freq,bucket->getSize()* sizeof(unsigned));
}

/*
 * nextAlive(c).
 * - return next character 'c'
 *   available in globalBucket.
 *
 * - return 0 if no next bucket alive.
 */
inline char FidoSearch::nextAlive(const char c) {
    for (int i = c+1; i < globalBucket.getSize(); ++i)
        if (globalBucket.freq[i] != 0)
            return char(i);
    return 0;
}


void FidoSearch::showStats() {
    std::cout << "\n\t--------xxxxxxxxxxxxx--------\n";
    printf("%20s :: [%u]\n","FILE SIZE",FILE_SIZE);
    printf("%20s :: [%d]\n","PARTITION SIZE",PARTITION_SIZE);
    printf("%20s :: [%d]\n","TOTAL PARTITIONS",TOTAL_PARTITIONS);
    std::cout << "\t--------xxxxxxxxxxxxx--------\n\n";
}



//============================================================
// main method
//============================================================

int main(int argc,char** argv) {

    ifstream            fin;
    const char*         arg         = NULL;
    const char*         indexFile   = NULL;
    const char*         search      = NULL;
    int                 TASK        = 0;
    int                 buffer      = 0;
    int                 cap         = 0;

    if (argc >= 5){
        arg         = argv[1];
        if(!strcmp(arg,"-n"))
            TASK    = N;
        else if(!strcmp(arg,"-r"))
            TASK    = R;
        else if(!strcmp(arg,"-a"))
            TASK    = A;
        else{
            cout << "wrong argument\n";
            exit(1);
        }
        fin.open(argv[2],ios::in);
        indexFile   = argv[3];
        search      = argv[4];
        buffer  = 2000;
        cap     = 7000000;
    }else{
        cout << "Insufficient Arguments.\nUsage : bwtsearch bwt_file indx_file 'search'\n";
        exit(1);
    }


    FidoSearch fido(&fin,indexFile,unsigned (buffer),unsigned (cap),TASK);
    fido.crunch(search);

    return 0;
}