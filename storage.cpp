#include "storage.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <cstring>

typedef unsigned char uchar;

using namespace std;

//Storage Constructor
Storage::Storage(int storageCapacity, int blockCapacity){
    __storageCapacity = storageCapacity;
    __blockCapacity = blockCapacity;

    __storagePtr = new uchar[storageCapacity];
    __storageSizeAllocated = 0;
    __storageSizeUsed = 0;
    __blockPtr = nullptr;
    __blockSizeUsed = 0;
    __blocksAvail = storageCapacity / blockCapacity;
    __blocksUsed = 0;
    __numRecords = 0;
}

//Storage Destructor
Storage::~Storage(){
    delete __storagePtr;
    __storagePtr = nullptr;
}

//Get max size of Storage
int Storage::getStorageCapacity(){
    return __storageCapacity;
}

//Get space utilized by records within Storage
int Storage::getStorageSizeUsed(){
    return __storageSizeUsed;
}

//Get num of records within Storage
int Storage::getNumRecords(){
    return __numRecords;
}

//GEt max size of a Block
int Storage::getBlockCapacity(){
    return __blockCapacity;
}

//Get space utilized within a Block
int Storage::getBlockSizeUsed(){
    return __blockSizeUsed;
}

//Get number of Blocks utilized in Storage
int Storage::getBlocksUsed(){
    return __blocksUsed;
}

//Create a new Block in the Storage
bool Storage::Storage::createBlock(){
    if (__blocksAvail > 0){
        __blockPtr = (uchar*) __storagePtr + (__blocksUsed * __blockCapacity); //point to new block.
        __blocksUsed++; // +1 num of blocks used
        __blocksAvail--; // -1 num of blocks available
        __storageSizeAllocated += __blockCapacity; //empty block but storage size allocated.
        __blockSizeUsed = 0; //new block, 0 / blockCapacity used.
        return true;
    }
    else{
        return false; // insufficient storage space for new block.
    }
}

//Returns address of the added record
tuple <uchar *, int> Storage::addRecord(int recordSize){

    if(__blockCapacity - __blockSizeUsed < recordSize || __blocksUsed == 0){ //unable to fit in current block
        if(!createBlock()){ //unable to create new block
            cout << "Insufficient space in Storage for record" << endl;
        }
    }

    if( recordSize > __blockCapacity){ //recordSize exceed block capacity
        cout << "RecordSize exceeded Block Capacity" << endl;
    }

    //record can be written
    //new record address = blockPtr + offset, <current block address , offset>    
    tuple <uchar* , int> recordAddInfo((uchar*)__blockPtr, __blockSizeUsed);
    __storageSizeUsed += recordSize; //increment Storage size used by record
    __numRecords ++; //increment num of records within storage
    __blockSizeUsed += recordSize; //increment Block size used by record

    return recordAddInfo;
}