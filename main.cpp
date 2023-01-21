#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <unordered_map>

#include "bptree.h"
#include "storage.h"

typedef unsigned char uchar;
void *startAddress = NULL;

using namespace std;

int main() {
    // Read data file
    std::ifstream dataStream;
    dataStream.open("data.tsv");

    if (dataStream.is_open()) {

        cout << "File opened" << endl;

        // init Storage
        int storageCapacity = 100000000;
        int blockCapacity;

        cout << "Enter Block Size (in Bytes)" << endl;
        cin >> blockCapacity;

        Storage storage(storageCapacity, blockCapacity);

        vector<tuple<void *, int>> dataEntries;  // vector to hold read data

        string line;
        getline(dataStream, line);  // removing header line
        cout << "Reading data ........" << endl;

        // reading data entries
        int count = 0;
        int count2 = 0;

        while (getline(dataStream, line)) {

            Record record;
            istringstream isStream(line);

            // convert tconst from string to200 char[]
            string tconstStr;
            getline(isStream, tconstStr, '\t');
            strcpy(record.tconst, tconstStr.c_str());

            // process remaining fields
            isStream >> record.averageRating;
            isStream >> record.numVotes;

            tuple<void *, int> dataEntry = storage.addRecord(sizeof(record));

            dataEntries.push_back(dataEntry);

            void *rcdAdr = (uchar *)get<0>(dataEntry) + get<1>(dataEntry);
          
            memcpy(rcdAdr, &record, sizeof(record));
        }
        // end of reading
        dataStream.close();

        // Experiment 1 - Storage Statistic
        cout << "============= Experiment 1 : Storage Statistics =============" << endl;
        cout << endl;
        cout << "Total Storage Size (MegaBytes) : " << storage.getStorageCapacity() / 1000000 << endl;
        cout << "Total Number of Records \t: " << storage.getNumRecords() << endl;
        cout << "Total Size of Records (Bytes) \t: " << storage.getStorageSizeUsed() << endl;
        cout << endl;
        cout << "Capacity of a Block (Bytes) \t: " << storage.getBlockCapacity() << endl;
        cout << "Max Record per Block \t\t: " << floor(storage.getBlockCapacity() / sizeof(Record)) << endl;
        cout << "Number of Blocks Allocated \t: " << storage.getBlocksUsed()<< endl;
        cout << "Max Number of Blocks \t\t: " << storage.getStorageCapacity() / storage.getBlockCapacity() << endl;
        cout << "============================================================="<< endl;
        cout << endl;

        // Experiment 2 - B+ Tree Indexing Component
        BPTree bptree(storage.getBlockCapacity());
        int i = 1;
        
        vector<tuple<void *, int>>::iterator entriesIterator;

        for (entriesIterator = dataEntries.begin();
             entriesIterator != dataEntries.end(); ++entriesIterator) {
            
            void *blockAddress = get<0>(*entriesIterator);
            int offset = get<1>(*entriesIterator);

            if (startAddress == NULL) {
                startAddress = blockAddress;
            }

            
            void *entryAddress = (uchar *)blockAddress + offset;
           
            int votes = (*(Record *)entryAddress).numVotes;

            Key newKey;
            newKey.key_value = votes;
            newKey.address.push_back(entryAddress);
            bptree.insert(newKey);
        }

        std::cout << "============= Experiment 2 : B+ Tree Statistics =============" << endl;
        std::cout << endl;
        std::cout << "Parameter n of B+ Tree : \t" << bptree.getMaxKeys() << endl;
        std::cout << "Number of Nodes of B+ Tree : \t" << bptree.getNumNodes(bptree.getRoot()) << endl;
        std::cout << "Height of B+ Tree : \t\t" << bptree.getHeight(bptree.getRoot()) << endl;
        bptree.displayBlock(bptree.getRoot());
        cout << "=============================================================" << endl;
        cout << endl;

        // Experiment 3 - Search Query
        string filename;
        std::cout << "============= Experiment 3: Retrieve movies with numVotes = 500 =============" << endl;
        std::cout << endl;
        if (blockCapacity == 200) {
            filename = "Exp3Results_200B";
        } else if (blockCapacity == 500) {
            filename = "Exp3Results_500B";
        }

        tuple<int, int, float> finalResults = bptree.searchExp(500, 500, filename);
        std::cout << endl;
        std::cout << "Number of index nodes the process accessed: " << get<0>(finalResults) << endl;
        std::cout << "Number of data blocks the process accessed: " << get<1>(finalResults) << endl;
        std::cout << "Average Rating of all records returned : " << get<2>(finalResults) << endl;
        std::cout << "=============================================================" << endl;
        cout << endl;

        // Experiment 4 - Range Query
        std::cout << "============= Experiment 4: Retrieve movies with 30,000 <= numVotes <= 40,000 =============" << endl;
        std::cout << endl;
        if (blockCapacity == 200) {
            filename = "Exp4Results_200B";
        } else if (blockCapacity == 500) {
            filename = "Exp4Results_500B";
        }

        finalResults = bptree.searchExp(30000, 40000, filename);
        std::cout << endl;
        std::cout << "Number of index nodes the process accessed: " << get<0>(finalResults) << endl;
        std::cout << "Number of data blocks the process accessed: " << get<1>(finalResults) << endl;
        std::cout << "Average Rating of all records returned : " << get<2>(finalResults) << endl;
        std::cout << "=============================================================" << endl;
        std::cout << endl;

        // Experiment 5 - Record deletions
        cout << "============== Experiment 5 : Record deletions ==============" << endl;
        std::cout << endl;
        int numDeletions = bptree.remove(1000);
        std::cout << "No. deletions = " << numDeletions << endl;
        std::cout << "No. nodes = " << bptree.getNumNodes(bptree.getRoot()) << endl;
        std::cout << "Height = " << bptree.getHeight(bptree.getRoot()) << endl;
        bptree.displayBlock(bptree.getRoot());
        std::cout << "=============================================================" << endl;
        std::cout << endl;
    } 
    else{
        cout << "Error opening data.tsv." << endl;
    }
    return 0;
}
