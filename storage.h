#ifndef STORAGE_H
#define STORAGE_H

#include <iostream>
#include <vector>
#include <tuple>
#include <cstring>

typedef unsigned char uchar;

using namespace std;

// Record structure
struct Record {
    char tconst[10]; // 9 chars + \0
    float averageRating;
    int numVotes; 
};

class Storage {
    private:
        //Storage variables
        uchar *__storagePtr;
        int __storageCapacity; //max capacity of the storage
        int __storageSizeAllocated; //allocated size of storage for blocks
        int __storageSizeUsed; //used size of storage for records
        int __numRecords; //num of records

        //Block variables
        uchar *__blockPtr;
        int __blockCapacity; //max capacity of a block
        int __blockSizeUsed; //used size of a block
        int __blocksAvail; //num of blocks available
        int __blocksUsed; //num of blocks used

    public:
        //constructor
        Storage(int storageCapacity, int blockCapacity);
        
        //destructor
        ~Storage();

        int getStorageCapacity();

        int getStorageSizeUsed();

        int getNumRecords();

        int getBlockCapacity();

        int getBlockSizeUsed();

        int getBlocksUsed();

        bool createBlock();
        
        tuple<uchar*, int> addRecord(int recordSize);


};

#endif