#include "bptree.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <unordered_map>

#include "storage.h"

extern void *startAddress;

typedef unsigned char uchar;

using namespace std;

InternalNode::InternalNode(int maxKeys) {
    keys = new int[maxKeys];
    pointers = new Node *[maxKeys + 1];
    numKeys = 0;
    isLeaf = false;
}

LeafNode::LeafNode(int maxKeys) {
    keys = new int[maxKeys];
    pointers = new vector<void *> *[maxKeys];
    for (int i = 0; i < maxKeys; i++) {
        pointers[i] = new vector<void *>();
    }
    numKeys = 0;
    isLeaf = true;
    nextLeaf = nullptr;
}

LeafNode *BPTree::search(int key_value) {
    // Tree is empty.
    if (root == nullptr) {
        throw std::logic_error("Tree is empty!");
    }
    // Else iterate through root node and follow the keys to find the correct
    // key.
    else {
        Node *cursor = root;

        bool found = false;

        // While we haven't hit a leaf node.
        while (cursor->isLeaf == false) {
            // Iterate through each key in the current node.
            for (int i = 0; i < cursor->numKeys; i++) {
                // If key_value is less than current key, go to the left
                // pointer's node to continue searching.
                if (key_value < cursor->keys[i]) {
                    cursor = ((InternalNode *)cursor)->pointers[i];
                    break;
                }
                // If we reached the end of all keys in this node (larger than
                // all), then go to the right pointer's node to continue
                // searching.
                if (i == cursor->numKeys - 1) {
                    cursor = ((InternalNode *)cursor)->pointers[i + 1];
                    break;
                }
            }
        }

        for (int i = 0; i < cursor->numKeys; i++) {
            if (cursor->keys[i] == key_value) {
                return (LeafNode *)cursor;
            }
        }
        throw std::logic_error("Not found!");
    }
}

void BPTree::insertInternal(int newKey, InternalNode *cur, Node *child) {
    if (cur->numKeys < maxKeys) {
        // cur not full, search for position to insert
        int i = 0;
        while (newKey > cur->keys[i] && i < cur->numKeys) {
            i++;
        }
        // move keys and pointers to make space for new key at pos i
        for (int j = cur->numKeys; j > i; j--) {
            cur->keys[j] = cur->keys[j - 1];
            cur->pointers[j + 1] = cur->pointers[j];
        }
        cur->keys[i] = newKey;
        cur->numKeys++;
        cur->pointers[i + 1] = child;
    } else {
        // new internal node
        InternalNode *newInternal = new InternalNode(maxKeys);
        numNodes++;

        // virtual node to store all values temporary
        int vKey[maxKeys + 1];
        Node *vPtr[maxKeys + 2];
        // copy to vNode
        for (int i = 0; i < maxKeys; i++) {
            vKey[i] = cur->keys[i];
            vPtr[i] = cur->pointers[i];
        }
        vPtr[maxKeys] = cur->pointers[maxKeys];

        int i = 0;
        int j;
        // searching for newKey position
        while (newKey > vKey[i] && i < maxKeys) {
            i++;
        }
        // moving keys and pointers to make space at newKey position
        for (int j = maxKeys; j > i; j--) {
            vKey[j] = vKey[j - 1];
            vPtr[j + 1] = vPtr[j];
        }
        // insert new key and pointer
        vKey[i] = newKey;
        vPtr[i + 1] = child;

        // spilt into 2 nodes
        cur->numKeys = (maxKeys + 1) / 2;
        newInternal->numKeys = maxKeys - (maxKeys + 1) / 2;

        // move keys and pointers to new nodes
        for (i = 0; i < cur->numKeys; i++) {
            cur->keys[i] = vKey[i];
            cur->pointers[i] = vPtr[i];
        }
        cur->pointers[cur->numKeys] = vPtr[i];

        for (i = 0, j = cur->numKeys + 1; i < newInternal->numKeys; i++, j++) {
            newInternal->keys[i] = vKey[j];
            newInternal->pointers[i] = vPtr[j];
        }
        newInternal->pointers[newInternal->numKeys] = vPtr[j];

        // cur is root node
        if (root == cur) {
            // new Root node
            InternalNode *newRoot = new InternalNode(maxKeys);
            numNodes++;
            newRoot->pointers[0] = cur;
            newRoot->pointers[1] = newInternal;
            int smallKey = getSmallestKey(newInternal);
            newRoot->keys[0] = smallKey;
            newRoot->isLeaf = false;
            newRoot->numKeys = 1;
            root = newRoot;
        } else {
            // recursive call
            insertInternal(getSmallestKey(newInternal),
                           (InternalNode *)findParent(root, cur), newInternal);
        }
    }
}

int BPTree::getSmallestKey(Node *cur) {
    while (cur->isLeaf == false) {
        cur = ((InternalNode *)cur)->pointers[0];
    }
    return cur->keys[0];
}

int BPTree::removeInternal(int targetKey, Node *parent, void *child) {
    int numDeletions = 0;
    int pos;

    for (pos = 0; pos < parent->numKeys; pos++) {
        if (parent->keys[pos] >= targetKey) {
            break;
        }
    }
    for (int i = pos; i < parent->numKeys - 1; i++) {
        parent->keys[i] = parent->keys[i + 1];
    }

    for (pos = 0; pos < parent->numKeys + 1; pos++) {
        if (((InternalNode *)parent)->pointers[pos] == child) {
            break;
        }
    }
    for (int i = pos; i < parent->numKeys + 1; i++) {
        if (parent->isLeaf) {
            ((LeafNode *)parent)->pointers[i] =
                ((LeafNode *)parent)->pointers[i + 1];
        } else {
            ((InternalNode *)parent)->pointers[i] =
                ((InternalNode *)parent)->pointers[i + 1];
        }
    }

    parent->numKeys--;

    // If minimum number of keys/pointers met, can just return
    if (parent == root) return numDeletions;
    if ((parent->isLeaf && parent->numKeys >= (maxKeys + 1) / 2) ||
        (!parent->isLeaf && parent->numKeys >= (maxKeys + 2) / 2 - 1)) {
        return numDeletions;
    }

    // Else, we need to recursively update the tree
    InternalNode *parentPtr = (InternalNode *)findParentInclLeaf(root, parent);
    int leftPos, rightPos;
    for (int pos = 0; pos < parentPtr->numKeys + 1; pos++) {
        if (parentPtr->pointers[pos] == parent) {
            leftPos = pos - 1;
            rightPos = pos + 1;
            break;
        }
    }

    // Sharing keys/pointers
    if (leftPos >= 0) {
        Node *leftNode = parentPtr->pointers[leftPos];
        if ((leftNode->isLeaf && leftNode->numKeys > (maxKeys + 1) / 2) ||
            (!leftNode->isLeaf && leftNode->numKeys > (maxKeys + 2) / 2 - 1)) {
            for (int i = parent->numKeys; i > 0; i--) {
                parent->keys[i] = parent->keys[i - 1];
            }
            parent->keys[0] = leftNode->keys[leftNode->numKeys - 1];
            parentPtr->keys[leftPos] = leftNode->keys[leftNode->numKeys - 1];

            for (int i = parent->numKeys + 1; i > 0; i--) {
                if (parent->isLeaf) {
                    ((LeafNode *)parent)->pointers[i] =
                        ((LeafNode *)parent)->pointers[i - 1];
                } else {
                    ((InternalNode *)parent)->pointers[i] =
                        ((InternalNode *)parent)->pointers[i - 1];
                }
            }
            if (parent->isLeaf) {
                ((LeafNode *)parent)->pointers[0] =
                    ((LeafNode *)leftNode)->pointers[leftNode->numKeys];
            } else {
                ((InternalNode *)parent)->pointers[0] =
                    ((InternalNode *)leftNode)->pointers[leftNode->numKeys];
            }

            parent->numKeys++;
            leftNode->numKeys--;
            return numDeletions;
        }
    }
    if (rightPos <= parentPtr->numKeys) {
        Node *rightNode = parentPtr->pointers[rightPos];
        if ((rightNode->isLeaf && rightNode->numKeys > (maxKeys + 1) / 2) ||
            (!rightNode->isLeaf &&
             rightNode->numKeys > (maxKeys + 2) / 2 - 1)) {
            parent->keys[parent->numKeys] = rightNode->keys[0];
            for (int i = 0; i < rightNode->numKeys - 1; i++) {
                rightNode->keys[i] = rightNode->keys[i + 1];
            }
            parentPtr->keys[rightPos - 1] = rightNode->keys[0];

            if (parent->isLeaf) {
                ((LeafNode *)parent)->pointers[parent->numKeys + 1] =
                    ((LeafNode *)rightNode)->pointers[0];
            } else {
                ((InternalNode *)parent)->pointers[parent->numKeys + 1] =
                    ((InternalNode *)rightNode)->pointers[0];
            }
            for (int i = 0; i < rightNode->numKeys; ++i) {
                if (rightNode->isLeaf) {
                    ((LeafNode *)rightNode)->pointers[i] =
                        ((LeafNode *)rightNode)->pointers[i + 1];
                } else {
                    ((InternalNode *)rightNode)->pointers[i] =
                        ((InternalNode *)rightNode)->pointers[i + 1];
                }
            }

            parent->numKeys++;
            rightNode->numKeys--;
            return numDeletions;
        }
    }

    // Merge nodes
    if (leftPos >= 0) {
        Node *leftNode = parentPtr->pointers[leftPos];
        if (!leftNode->isLeaf) {
            leftNode->keys[leftNode->numKeys] = parentPtr->keys[leftPos];

            for (int i = leftNode->numKeys + 1, j = 0; j < parent->numKeys;
                 i++, j++) {
                leftNode->keys[i] = parent->keys[j];
            }

            for (int i = leftNode->numKeys + 1, j = 0; j <= parent->numKeys;
                 i++, j++) {
                if ((parent->isLeaf &&
                     ((LeafNode *)parent)->pointers[j] == child) ||
                    (!parent->isLeaf &&
                     ((InternalNode *)parent)->pointers[j] == child)) {
                    i--;
                    continue;
                }
                if (leftNode->isLeaf) {
                    ((LeafNode *)leftNode)->pointers[i] =
                        ((LeafNode *)parent)->pointers[j];
                    ((LeafNode *)parent)->pointers[j] = NULL;
                } else {
                    ((InternalNode *)leftNode)->pointers[i] =
                        ((InternalNode *)parent)->pointers[j];
                    ((InternalNode *)parent)->pointers[j] = NULL;
                }
            }

            leftNode->numKeys += parent->numKeys + 1;
        } else {
            for (int i = leftNode->numKeys, j = 0; j < parent->numKeys;
                 i++, j++) {
                leftNode->keys[i] = parent->keys[j];
            }

            for (int i = leftNode->numKeys + 1, j = 0; j < parent->numKeys + 1;
                 i++, j++) {
                if (leftNode->isLeaf) {
                    ((LeafNode *)leftNode)->pointers[i] =
                        ((LeafNode *)parent)->pointers[j];
                    ((LeafNode *)parent)->pointers[j] = NULL;
                } else {
                    ((InternalNode *)leftNode)->pointers[i] =
                        ((InternalNode *)parent)->pointers[j];
                    ((InternalNode *)parent)->pointers[j] = NULL;
                }
            }

            leftNode->numKeys += parent->numKeys;
        }
        parent->numKeys = 0;

        numDeletions +=
            1 + removeInternal(parentPtr->keys[leftPos], parentPtr, parent);
    } else if (rightPos <= parentPtr->numKeys) {
        Node *rightNode = parentPtr->pointers[rightPos];
        if (!rightNode->isLeaf) {
            parent->keys[parent->numKeys] = parentPtr->keys[rightPos - 1];

            for (int i = parent->numKeys + 1, j = 0; j < rightNode->numKeys;
                 i++, j++) {
                parent->keys[i] = rightNode->keys[j];
            }

            for (int i = parent->numKeys + 1, j = 0; j < rightNode->numKeys;
                 i++, j++) {
                if ((rightNode->isLeaf &&
                     ((LeafNode *)rightNode)->pointers[j] == child) ||
                    (!rightNode->isLeaf &&
                     ((InternalNode *)rightNode)->pointers[j] == child)) {
                    i--;
                    continue;
                }
                if (rightNode->isLeaf) {
                    ((LeafNode *)parent)->pointers[i] =
                        ((LeafNode *)rightNode)->pointers[j];
                    ((LeafNode *)rightNode)->pointers[j] = NULL;
                } else {
                    ((InternalNode *)parent)->pointers[i] =
                        ((InternalNode *)rightNode)->pointers[j];
                    ((InternalNode *)rightNode)->pointers[j] = NULL;
                }
            }

            parent->numKeys += rightNode->numKeys + 1;
        } else {
            for (int i = parent->numKeys, j = 0; j < rightNode->numKeys;
                 i++, j++) {
                parent->keys[i] = rightNode->keys[j];
            }

            for (int i = parent->numKeys + 1, j = 0; j < rightNode->numKeys + 1;
                 i++, j++) {
                if (rightNode->isLeaf) {
                    ((LeafNode *)parent)->pointers[i] =
                        ((LeafNode *)rightNode)->pointers[j];
                    ((LeafNode *)rightNode)->pointers[j] = NULL;
                } else {
                    ((InternalNode *)parent)->pointers[i] =
                        ((InternalNode *)rightNode)->pointers[j];
                    ((InternalNode *)rightNode)->pointers[j] = NULL;
                }
            }

            parent->numKeys += rightNode->numKeys;
        }
        rightNode->numKeys = 0;

        numDeletions += 1 + removeInternal(parentPtr->keys[rightPos - 1],
                                           parentPtr, rightNode);
    }
    return numDeletions;
}

InternalNode *BPTree::findParent(Node *cur, Node *child) {
    Node *parent;

    // end of tree
    if (cur->isLeaf || (((InternalNode *)cur)->pointers[0])->isLeaf) {
        return NULL;
    }

    for (int i = 0; i < cur->numKeys + 1; i++) {
        if (((InternalNode *)cur)->pointers[i] == child) {
            parent = cur;
            return (InternalNode *)parent;
        } else {
            parent = findParent(((InternalNode *)cur)->pointers[i], child);
            if (parent != NULL) {
                return (InternalNode *)parent;
            }
        }
    }
    return (InternalNode *)parent;
}

Node *BPTree::findParentInclLeaf(Node *cur, Node *child) {
    Node *parent;

    // end of tree
    if (cur->isLeaf) {
        return NULL;
    }

    // cur is InternalNode
    for (int i = 0; i < cur->numKeys + 1; i++) {
        if (((InternalNode *)cur)->pointers[i] == child) {
            parent = cur;
            return parent;
        } else {
            parent =
                findParentInclLeaf(((InternalNode *)cur)->pointers[i], child);
            if (parent != NULL) {
                return parent;
            }
        }
    }
    return parent;
}

BPTree::BPTree(int blockCapacity) {
    root = nullptr;
    height = 0;
    nodeSize = 0;
    __blockCapacity = blockCapacity;

    // calculate max size of a node
    // cout << "size of node* = " << sizeof(Node*) << endl;
    // cout << "size of KEY = " << sizeof(Key) << endl;
    maxKeys = floor((__blockCapacity - sizeof(Node *)) /
                    (sizeof(vector<void *> *) + sizeof(int)));
}

// insert new Key
void BPTree::insert(Key newKey) {
    // first key
    if (root == nullptr) {
        root = new LeafNode(maxKeys);
        root->keys[0] = newKey.key_value;
        ((LeafNode *)root)->pointers[0]->push_back(newKey.address[0]);
        root->numKeys = 1;
        numNodes++;
        return;
    } else {
        Node *cur = root;
        InternalNode *parent;

        // traverse till leaf node
        while (!cur->isLeaf) {
            parent = (InternalNode *)cur;

            if (newKey.key_value >= cur->keys[cur->numKeys - 1]) {
                cur = ((InternalNode *)cur)->pointers[cur->numKeys];
            } else {
                int i = 0;
                while (newKey.key_value >= cur->keys[i]) {
                    i++;
                }
                cur = ((InternalNode *)cur)->pointers[i];
            }
        }

        // currently at leaf node
        // if key_value is already in the B+ tree
        for (int i = 0; i < cur->numKeys; i++) {
            if (cur->keys[i] == newKey.key_value) {
                ((LeafNode *)cur)->pointers[i]->push_back(newKey.address[0]);
                return;
            }
        }

        // if key_value is not yet in the B+ tree
        if (cur->numKeys < maxKeys) {  // Node is not full
            int i = 0;

            if (newKey.key_value > cur->keys[cur->numKeys - 1]) {
                i = cur->numKeys;
            }

            // find position for newKey
            while (newKey.key_value > cur->keys[i] && i < cur->numKeys) {
                i++;
            }

            // shifting of keys and pointers
            for (int j = cur->numKeys; j > i; j--) {
                cur->keys[j] = cur->keys[j - 1];
                ((LeafNode *)cur)->pointers[j] =
                    ((LeafNode *)cur)->pointers[j - 1];
            }

            // inserting newKey
            cur->keys[i] = newKey.key_value;
            cur->numKeys++;


            ((LeafNode *)cur)->pointers[i] = new vector<void *>();
            ((LeafNode *)cur)->pointers[i]->push_back(newKey.address[0]);
            return;

        } 
        else {  // need to create new Node
            LeafNode *newLeaf = new LeafNode(maxKeys);
            int vNode[maxKeys + 1];
            vector<void *> *vPtr[maxKeys + 1];

            for (int i = 0; i < maxKeys; i++) {
                vNode[i] = cur->keys[i];
                vPtr[i] = ((LeafNode *)cur)->pointers[i];
            }

            if (newKey.key_value > vNode[maxKeys - 1]) {
                vNode[maxKeys] = newKey.key_value;
                vPtr[maxKeys] = new vector<void *>();
                vPtr[maxKeys]->push_back(newKey.address[0]);
                numNodes++;
            } 
            else {
                int i = 0;
                while (newKey.key_value > vNode[i]) {
                    i++;
                }
                numNodes++;

                // shifting of keys in vNode
                for (int j = maxKeys; j > i; j--) {
                    vNode[j] = vNode[j - 1];
                    vPtr[j] = vPtr[j - 1];
                }

                // insert into vNode
                vNode[i] = newKey.key_value;
                vPtr[i] = new vector<void *>();
                vPtr[i]->push_back(newKey.address[0]);
            }
            cur->numKeys = (maxKeys + 1) / 2;
            newLeaf->numKeys = maxKeys + 1 - (maxKeys + 1) / 2;

            newLeaf->nextLeaf = ((LeafNode *)cur)->nextLeaf;
            ((LeafNode *)cur)->nextLeaf = newLeaf;

            // moving keys from vNode back to cur and newLeaf
            int i;
            for (i = 0; i < cur->numKeys; i++) {
                cur->keys[i] = vNode[i];
                ((LeafNode *)cur)->pointers[i] = vPtr[i];
            }

            for (int j = 0; j < newLeaf->numKeys; i++, j++) {
                newLeaf->keys[j] = vNode[i];
                ((LeafNode *)newLeaf)->pointers[j] = vPtr[i];
            }

            // updating parent node
            if (cur == root) {
                // create new root node
                InternalNode *newRoot = new InternalNode(maxKeys);
                numNodes++;

                newRoot->keys[0] = newLeaf->keys[0];
                newRoot->pointers[0] = cur;
                newRoot->pointers[1] = newLeaf;
                newRoot->isLeaf = false;
                newRoot->numKeys = 1;
                root = newRoot;
            } 
            else {
                // insert in parent Node
                insertInternal(newLeaf->keys[0], parent, newLeaf);
            }
        }
    }
}

tuple<int, int, float> BPTree::searchExp(int lowerBoundKey, int upperBoundKey, string filename) {
    int numIndexBlockAccessed = 0;
    float averageRating = 0;
    tuple<int, int, float> results;
    vector<float> allRatings;
    fill(allRatings.begin(), allRatings.end(), 0.0);  // need to make sure this is cleared before starting next experiment 4
    std::ofstream fout(filename + ".txt");
    set<int> blockSet;
    int numInternal = 0;
    int numLeaf = 0;
    int numRecords = 0;

    // If tree is not empty
    if (root != nullptr) {
        Node *cursor = root;  // the variable cursor will hold address of root

        // To begin search starting from internal nodes i.e not leaf nodes
        while (cursor->isLeaf == false) {

            // Search through the keys in current node
            for (int i = 0; i < cursor->numKeys; i++) {
                
                // If key is greater than lowerBound provided, proceed to left subtree & continue the search
                if (lowerBoundKey < cursor->keys[i]) {

                    // std::cout << "Index node (Internal Node) successfully accessed in process. This is the content: ---- |";
                    fout << "Index node (Internal Node) successfully accessed in process. This is the content: ---- |";
                    for (int x = 0; x < cursor->numKeys; x++) {

                        // cout << cursor->keys[x] << "|";
                        fout << cursor->keys[x] << "|" ;
                
                    }
                    fout << "\n";
                    // cout <<"\n";
                    cursor = ((InternalNode *)cursor)->pointers[i];

                    numIndexBlockAccessed++;
                    numInternal++;

                    break;
                }
                // If last key in node is reached (all keys in this node smaller than lowerBound), proceed to right subtree & continue the search
                if (i == (cursor->numKeys) - 1) {

                    // std::cout << "Index node (Internal Node) successfully accessed in process. This is the content: ---- |";
                    fout << "Index node (Internal Node) successfully accessed in process. This is the content: ---- |";

                    for (int x = 0; x < cursor->numKeys; x++) {

                        // cout << cursor->keys[x] << "|";
                        fout << cursor->keys[x] << "|";
                             
                    }
                    fout << "\n";
                    // cout <<"\n";
                    cursor = ((InternalNode *)cursor)->pointers[i + 1];  // To go to the right subtree

                    numIndexBlockAccessed++;
                    numInternal++;

                    break;
                }
            }
        }

        // At this point, cursor has reached isLeaf = true, now we search through leaf node to find matching key or range
        bool stopSearch = false;
        bool flag = true;
        LeafNode* leafCursor;
        leafCursor = (LeafNode* ) cursor;

        // Search from this point onwards are searching within leaf nodes hence this layer onwards is dense index
        while (stopSearch == false) {
            int i;
            // Key that is within given range is found, hence need to iterate through entire range until we reach upperBoundKey
            for (i = 0; i < leafCursor->numKeys; i++) {

                if (leafCursor->keys[i] > upperBoundKey) {

                    stopSearch = true;
                    break;
                }
                // Meets range criteria or key value matches requirement
                if (leafCursor->keys[i] >= lowerBoundKey && leafCursor->keys[i] <= upperBoundKey) {

                    //flag is to ensure leaf node nt printed repeatedly in range query
                    if (flag == true) {
                        // printf("\n");
                        // std::cout << "Index node (Leaf Node) successfully accessed in process. This is the content: ---- |";
                        fout << "Index node (Leaf Node) successfully accessed in process. This is the content: ---- |";

                        for (int x = 0; x < leafCursor->numKeys; x++) {

                            // cout << cursor->keys[x] << "|";
                            fout << leafCursor->keys[x] << "|";
    
                        }
                        fout << "\n";
                        // cout <<"\n";

                        numIndexBlockAccessed++;
                        numLeaf++;
                    }
                    // std::cout << endl;

                    // Since now we are at leaf node and this key matches criteria, hence will now access the datablocks with this key value
                    int size = ((LeafNode *)leafCursor)->pointers[i]->size();  // since 1 key can have many different records, need to find total number of records of this key value

                    // printf("\n");
                    int blockNum;
                    for (int j = 0; j < size; ++j) {

                        fout << "\n";
                        // printf("Data block: ");
                        // printf("%p", (uchar*)
                        // cursor->keys[i].address[j]); printf("\n"); cout << "tconst: " << (*(Record *)cursor->keys[i].address[j]).tconst << "Rating: " << (*(Record *)cursor->keys[i].address[j]).averageRating << "numVotes: " << (*(Record *)cursor->keys[i].address[j]).numVotes << "\n";
                        void *recordAddress = (*((LeafNode *)leafCursor)->pointers[i])[j];
                        allRatings.push_back((*(Record *)recordAddress).averageRating);
                        blockNum = (int)((uchar *)(Record *)recordAddress - (uchar *)startAddress) / __blockCapacity;
                        fout << "Data Block Num: " << blockNum << endl;
                        fout << "Record Address: " << recordAddress << std::endl;
                        fout << "tconst: " << (*(Record *)recordAddress).tconst << " Rating: " << (*(Record *)recordAddress).averageRating << " numVotes: " << (*(Record *)recordAddress).numVotes << "\n" << std::endl;
                        blockSet.insert(blockNum);
                        numRecords++;
                    }

                    if (upperBoundKey == lowerBoundKey) {  // for search query

                        averageRating = std::accumulate(allRatings.begin(),allRatings.end(), 0.0) / allRatings.size();
                        get<0>(results) = numIndexBlockAccessed;
                        get<1>(results) = blockSet.size();
                        get<2>(results) = averageRating;
                        fout << "Number of index nodes the process accessed: " << numIndexBlockAccessed << endl;
                        fout << "Number of data blocks the process accessed: " << blockSet.size() << endl;
                        fout << "Number of records accessed: " << numRecords << endl;
                        fout << "Average Rating of all records returned : " << averageRating << endl;
                        return results;
                    }

                    if (upperBoundKey != lowerBoundKey){  // for range query
                        flag = false;
                        
                    }
                }
            }
             // Since stopsearch is false, hence will continue searching through each key in next unexplored leaf node in the for loop above
            if (leafCursor->nextLeaf != nullptr && leafCursor->keys[(leafCursor->numKeys) - 1] != upperBoundKey){

                leafCursor = (LeafNode *) leafCursor -> nextLeaf;
                flag = true;

            } 
            else{
                stopSearch = true;
            }
        }

    } 
    else{
        std::cout << "Tree is empty, no content accessed.";
    }

    averageRating = std::accumulate(allRatings.begin(), allRatings.end(), 0.0) / allRatings.size();
    get<0>(results) = numIndexBlockAccessed;
    get<1>(results) = blockSet.size();
    get<2>(results) = averageRating;
    fout << "Number of index nodes the process accessed: " << numIndexBlockAccessed << endl;
    fout << "Number of data blocks the process accessed: " << blockSet.size() << endl;
    fout << "Number of records accessed: " << numRecords << endl;
    fout << "Average Rating of all records returned : " << averageRating << endl;

    return results;
}

int BPTree::remove(int key_value){

    Node *cur = search(key_value);
    int i, numDeletions;

    for (i = 0; i < cur->numKeys; i++) {
        if (cur->keys[i] == key_value) {
            break;
        }
    }
    vector<void *> *child = ((LeafNode *)cur)->pointers[i];

    numDeletions = removeInternal(key_value, cur, child);

    if (i == 0) {
        int newKey = cur->keys[i];

        if (root != nullptr) {
            Node *cursor = root;

            bool found = false;

            while (!cursor->isLeaf) {
                for (int i = 0; i < cursor->numKeys; i++) {

                    if (key_value == cursor->keys[i]) {
                        cursor->keys[i] = newKey;
                    }

                    if (key_value < cursor->keys[i]) {
                        cursor = ((InternalNode *)cursor)->pointers[i];
                        break;
                    }
                    // If we reached the end of all keys in this node(larger than all), then go to the right pointer's node to continue searching.
                    if (i == cursor->numKeys - 1) {
                        // Set cursor to the child node, now loaded in main memory.
                        cursor = ((InternalNode *)cursor)->pointers[i + 1];
                        break;
                    }
                }
            }
        }
    }
    return numDeletions;
}

vector<void *> BPTree::getAddresses(int key_value){

    LeafNode *cursor = search(key_value);

    for (int i = 0; i < cursor->numKeys; i++) {
        if (cursor->keys[i] == key_value) {
            return *(cursor->pointers[i]);
        }
    }
    return vector<void *>();
}

void BPTree::displayTree(Node *head){

    if (head == NULL) return;

    queue<Node *> q;

    q.push(head);

    while (!q.empty()) {
        int qSize = q.size();
        for (int i = 0; i < qSize; ++i) {
            Node *node = q.front();

            for (int j = 0; j < node->numKeys; ++j) {
                cout << node->keys[j] << "|";

                if (!node->isLeaf) {
                    q.push(((InternalNode *)node)->pointers[j]);
                }
            }
            cout << endl;

            if (!node->isLeaf) {
                q.push(((InternalNode *)node)->pointers[node->numKeys]);
            }
            q.pop();
        }
        cout << endl;
    }
}

// print Block
void BPTree::displayBlock(Node *ptr){

    cout << "Root: " << endl;
    for (int i = 0; i < ptr->numKeys; ++i) {
        cout << ptr->keys[i] << "|";
    }
    cout << endl;

    if (!ptr->isLeaf) {

        cout << "First child node: " << endl;
        ptr = ((InternalNode *)ptr)->pointers[0];

        for (int i = 0; i < ptr->numKeys; ++i) {
            cout << ptr->keys[i] << "|";
        }
        cout << endl;
    }
}

Node *BPTree::getRoot(){
    return root; 
}

int BPTree::getHeight(Node *cur){
    if (cur->isLeaf) {
        return 1;
    } else {
        return getHeight(((InternalNode *)cur)->pointers[0]) + 1;
    }
    return 0;
}

int BPTree::getNumNodes(Node *cur){
    if (cur == NULL) {
        return 0;
    }
    queue<Node *> q;
    q.push(cur);

    int numNodes = 0;
    while (!q.empty()) {
        int qSize = q.size();
        numNodes += qSize;

        for (int i = 0; i < qSize; i++) {

            Node *temp = q.front();
            for (int i = 0; i < temp->numKeys; i++) {

                if (!temp->isLeaf) {
                    q.push(((InternalNode *)temp)->pointers[i]);
                }
            }

            if (!temp->isLeaf) {
                q.push(((InternalNode *)temp)->pointers[temp->numKeys]);
            }
            q.pop();
        }
    }
    return numNodes;
}

int BPTree::getMaxKeys(){ 
    return maxKeys; 
}