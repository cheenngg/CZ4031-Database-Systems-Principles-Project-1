#ifndef BPTREE_H
#define BPTREE_H

#include <vector>

#include "storage.h"

typedef unsigned char uchar;

struct Key {
    int key_value;
    vector<void *> address;  // array of address for records with same key_value
};

class Node {
   public:
    int *keys;  
    void **pointers;
    int numKeys;  // number of keys in this node
    bool isLeaf; //if node is leaf or internal
    friend class BPTree;
};

class InternalNode : private Node {
   private:
    Node **pointers;  // array of pointers to other Nodes

   public:
    // Constructor
    InternalNode(int maxKeys);
    friend class BPTree;
};

class LeafNode : private Node {
   private:
    vector<void *> **pointers;  // array of addresses to data in memory
    Node *nextLeaf;

   public:
    // Constructor
    LeafNode(int maxKeys);
    friend class BPTree;
};

class BPTree {
   private:
    Node *root;    // Root Node Pointer
    int maxKeys;   // Max num of keys in a node
    int height;    // Height of B+ Tree
    int numNodes;  // Num of nodes in B+ Tree
    int nodeSize;  // Size of a Node
    int __blockCapacity;

    //insert internal nodes into b+ tree
    void insertInternal(int newKey, InternalNode *parent, Node *child);

    //remove internal nodes from b+ tree
    int removeInternal(int targetKey, Node *parent, void *child);

    //get smallest key in node
    int getSmallestKey(Node *cur);

    //find parent node of current node
    InternalNode *findParent(Node *cur, Node *child);

    //find parent including leaf
    Node *findParentInclLeaf(Node *cur, Node *child);

    //get address 
    vector<void *> getAddresses(int key_value);

   public:
    // Constructor
    BPTree(int blockCapacity);

    // insert new Key
    void insert(Key newKey);

    //remove Node
    int remove(int key_value);

    //serach for key
    LeafNode *search(int key_value);

    // print for experiment 2
    void displayBlock(Node *cur);

    // print tree
    void displayTree(Node *head);

    //get root
    Node *getRoot();

    //search for expriment 3 and 4
    tuple<int, int, float> searchExp(int lowerBoundKey, int upperBoundKey, string filename);

    //get height of b+ tree
    int getHeight(Node *cur);

    //get number of nodes in b+ tree
    int getNumNodes(Node *cur);

    //get maximum keys in node
    int getMaxKeys();

};
#endif