// C program for Huffman Coding
// Please refer to: https://www.geeksforgeeks.org/huffman-coding-greedy-algo-3
#include "tree_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This constant can be avoided by explicitly
// calculating height of Huffman Tree
#define MAX_TREE_HT 100

// A Huffman tree node
struct MinHeapNode
{

    // One of the input characters
    char data;

    // Frequency of the character
    unsigned freq;

    // Left and right child of this node
    struct MinHeapNode *left, *right;
};

// A Min Heap:  Collection of
// min-heap (or Huffman tree) nodes
struct MinHeap
{

    // Current size of min heap
    unsigned size;

    // capacity of min heap
    unsigned capacity;

    // Array of minheap node pointers
    struct MinHeapNode **array;
};


/**
 * @brief Utility function to allocate a new min heap node
 * 
 * @param data character
 * @param freq frequency
 * 
 * @return the min heap node
 * **/
struct MinHeapNode *newNode(char data, unsigned freq)
{
    struct MinHeapNode *temp = (struct MinHeapNode *)malloc(
        sizeof(struct MinHeapNode));

    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;

    return temp;
}


/**
 * @bried Utility function to create a min heap
 * 
 * @param capacity capacity of the min heap
 * 
 * @return the created min heap
 */
struct MinHeap *createMinHeap(unsigned capacity)

{

    struct MinHeap *minHeap = (struct MinHeap *)malloc(sizeof(struct MinHeap));

    // current size is 0
    minHeap->size = 0;

    minHeap->capacity = capacity;

    minHeap->array = (struct MinHeapNode **)malloc(
        minHeap->capacity * sizeof(struct MinHeapNode *));
    return minHeap;
}


/**
 * @brief Utility function to swap two min heap nodes
 * 
 * @param a the first node
 * @param b the secodn node
 */
void swapMinHeapNode(struct MinHeapNode **a,
                     struct MinHeapNode **b)

{

    struct MinHeapNode *t = *a;
    *a = *b;
    *b = t;
}

/**
 * @brief the standard minHeapify function
 * 
 * @param minHeap the min heap
 * @param idx current index
 */
void minHeapify(struct MinHeap *minHeap, int idx)

{

    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx)
    {
        swapMinHeapNode(&minHeap->array[smallest],
                        &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}


/**
 * @brief Utility function to check ifsize of the heap is 1 or not
 * 
 * @param minHeap the min heap
 * 
 * @return 1 if true, 0 otherwise
 */ 
int isSizeOne(struct MinHeap *minHeap)
{
    return (minHeap->size == 1);
}


/**
 * @brief A standard function to extract minimum value node from heap
 * 
 * @param minHeap the min heap
 * 
 * @return the node with minimun value 
 */
struct MinHeapNode *extractMin(struct MinHeap *minHeap)

{

    struct MinHeapNode *temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];

    --minHeap->size;
    minHeapify(minHeap, 0);

    return temp;
}

// A utility function to insert
// a new node to Min Heap
/**
 * @brief Utility function to insert a new node to Min Heap
 * 
 * @param minHeap the min heap
 * @param minHeapNode the node to add
 *  
 */
void insertMinHeap(struct MinHeap *minHeap,
                   struct MinHeapNode *minHeapNode)

{

    ++minHeap->size;
    int i = minHeap->size - 1;

    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq)
    {

        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    minHeap->array[i] = minHeapNode;
}

/**
 * @brief Standard fuction to build min heap
 * 
 * @param minHeap the min heap
 */ 
void buildMinHeap(struct MinHeap *minHeap)

{

    int n = minHeap->size - 1;
    int i;

    for (i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}


/** 
 * @brief Utility function to check if this node is leaf
 * 
 * @param root the node
 * 
 * @return 1 if is a leaf, 0 otherwise
 */
int isLeaf(struct MinHeapNode *root)

{

    return !(root->left) && !(root->right);
}


/** 
 * @brief Creates a min heap and inserts all character of data[] in min heap
 * 
 * @param data array of characters
 * @param freq array of frequencies
 * @param size capacity of min heap
 * 
 * @return the min heap
 */
struct MinHeap *createAndBuildMinHeap(char data[],
                                      int freq[], int size)

{

    int i;
    struct MinHeap *minHeap = createMinHeap(size);

    for (i = 0; i < size; ++i)
        minHeap->array[i] = newNode(data[i], freq[i]);

    minHeap->size = size;
    buildMinHeap(minHeap);

    return minHeap;
}



/**
 * @brief The main function that builds Huffman tree
 * 
 * @param data array of character
 * @param freq array of corresponding frequences
 * @param size size of the previous arrays
 * 
 * @return the root of the tree
 */
struct MinHeapNode *buildHuffmanTree(char data[],
                                     int freq[], int size)

{
    struct MinHeapNode *left, *right, *top;

    // Step 1: Create a min heap of capacity
    // equal to size.  Initially, there are
    // modes equal to size.
    struct MinHeap *minHeap = createAndBuildMinHeap(data, freq, size);

    // Iterate while size of heap doesn't become 1
    while (!isSizeOne(minHeap))
    {

        // Step 2: Extract the two minimum
        // freq items from min heap
        left = extractMin(minHeap);
        right = extractMin(minHeap);

        // Step 3:  Create a new internal
        // node with frequency equal to the
        // sum of the two nodes frequencies.
        // Make the two extracted node as
        // left and right children of this new node.
        // Add this node to the min heap
        // '$' is a special value for internal nodes, not
        // used
        top = newNode('$', left->freq + right->freq);

        top->left = left;
        top->right = right;

        insertMinHeap(minHeap, top);
    }

    // Step 4: The remaining node is the
    // root node and the tree is complete.
    return extractMin(minHeap);
}

// The main function that builds a
// Huffman Tree and print codes by traversing
// the built Huffman Tree
struct MinHeapNode *HuffmanCodes(char data[], int freq[], int size)
{

    // Construct Huffman Tree
    struct MinHeapNode *root = buildHuffmanTree(data, freq, size);

    return root;
}
