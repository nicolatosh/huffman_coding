// C program for Huffman Coding

#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include "frequencies_utils.h"

#define RECV_SIZE 200000
#define INPUT_SIZE 200000

// This constant can be avoided by explicitly
// calculating height of Huffman Tree
#define MAX_TREE_HT 100
#define HASHSIZE 100


struct nlist { /* table entry: */
	char name; /* defined char */
	char *code; /* code */
};
int size;
struct nlist *codes_list[HASHSIZE];

/* hash: form hash value for string s */
unsigned hash(char s)
{
    unsigned hashval;
    hashval = s + 31 ;
    return hashval % size;
}


/* lookup: look for s in codes_list */
struct nlist *lookup(char s)
{
    struct nlist *np;
    np = codes_list[hash(s)];
    if ( np != NULL)
        return np; /* found */
    return NULL; /* not found */
}


/* install: put (name, code) in codes_list */
struct nlist *install(char name, char *code)
{
    struct nlist *np;
    unsigned hashval;
    if ((np = lookup(name)) == NULL) { /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL)
          return NULL;
        np->name = name;
        hashval = hash(name);
        codes_list[hashval] = np;
    } else /* already there */
        free((void *) np->code); /*free previous code */
    if ((np->code = strdup(code)) == NULL)
       return NULL;
    return np;
}

// A Huffman tree node
struct MinHeapNode {

    // One of the input characters
    char data;

    // Frequency of the character
    unsigned freq;

    // Left and right child of this node
    struct MinHeapNode *left, *right;
};

// A Min Heap:  Collection of
// min-heap (or Huffman tree) nodes
struct MinHeap {

    // Current size of min heap
    unsigned size;

    // capacity of min heap
    unsigned capacity;

    // Array of minheap node pointers
    struct MinHeapNode** array;
};

// A utility function allocate a new
// min heap node with given character
// and frequency of the character
struct MinHeapNode* newNode(char data, unsigned freq)
{
    struct MinHeapNode* temp = (struct MinHeapNode*)malloc(
        sizeof(struct MinHeapNode));

    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;

    return temp;
}

// A utility function to create
// a min heap of given capacity
struct MinHeap* createMinHeap(unsigned capacity)

{

    struct MinHeap* minHeap
        = (struct MinHeap*)malloc(sizeof(struct MinHeap));

    // current size is 0
    minHeap->size = 0;

    minHeap->capacity = capacity;

    minHeap->array = (struct MinHeapNode**)malloc(
        minHeap->capacity * sizeof(struct MinHeapNode*));
    return minHeap;
}

// A utility function to
// swap two min heap nodes
void swapMinHeapNode(struct MinHeapNode** a,
                     struct MinHeapNode** b)

{

    struct MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

// The standard minHeapify function.
void minHeapify(struct MinHeap* minHeap, int idx)

{

    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size
        && minHeap->array[left]->freq
               < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size
        && minHeap->array[right]->freq
               < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest],
                        &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

// A utility function to check
// if size of heap is 1 or not
int isSizeOne(struct MinHeap* minHeap)
{
    return (minHeap->size == 1);
}

// A standard function to extract
// minimum value node from heap
struct MinHeapNode* extractMin(struct MinHeap* minHeap)

{

    struct MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];

    --minHeap->size;
    minHeapify(minHeap, 0);

    return temp;
}

// A utility function to insert
// a new node to Min Heap
void insertMinHeap(struct MinHeap* minHeap,
                   struct MinHeapNode* minHeapNode)

{

    ++minHeap->size;
    int i = minHeap->size - 1;

    while (i
           && minHeapNode->freq
                  < minHeap->array[(i - 1) / 2]->freq) {

        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    minHeap->array[i] = minHeapNode;
}

// A standard function to build min heap
void buildMinHeap(struct MinHeap* minHeap)

{

    int n = minHeap->size - 1;
    int i;

    for (i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}

// A utility function to print an array of size n
void printArr(char arr[], int n)
{
    int i;
    for (i = 0; i < n; ++i)
        printf("%c", arr[i]);

    printf("\n");
}

// Utility function to check if this node is leaf
int isLeaf(struct MinHeapNode* root)

{

    return !(root->left) && !(root->right);
}

// Creates a min heap of capacity
// equal to size and inserts all character of
// data[] in min heap. Initially size of
// min heap is equal to capacity
struct MinHeap* createAndBuildMinHeap(char data[],
                                      int freq[], int size)

{

    struct MinHeap* minHeap = createMinHeap(size);

    for (int i = 0; i < size; ++i)
        minHeap->array[i] = newNode(data[i], freq[i]);

    minHeap->size = size;
    buildMinHeap(minHeap);

    return minHeap;
}

// The main function that builds Huffman tree
struct MinHeapNode* buildHuffmanTree(char data[],
                                     int freq[], int size)

{
    struct MinHeapNode *left, *right, *top;

    // Step 1: Create a min heap of capacity
    // equal to size.  Initially, there are
    // modes equal to size.
    struct MinHeap* minHeap
        = createAndBuildMinHeap(data, freq, size);

    // Iterate while size of heap doesn't become 1
    while (!isSizeOne(minHeap)) {

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

// Prints huffman codes from the root of Huffman Tree.
// It uses arr[] to store codes
void FillCodesList(struct MinHeapNode* root, char arr[],
                int top)

{

    // Assign 0 to left edge and recur
    if (root->left) {
        #pragma omp task shared(root, top) firstprivate(arr) untied
        arr[top] = '0';
        FillCodesList(root->left, arr, top + 1);
    }

    // Assign 1 to right edge and recur
    if (root->right) {
        #pragma omp task shared(root, top) firstprivate(arr) untied
        arr[top] = '1';
        FillCodesList(root->right, arr, top + 1);
    }

    // If this is a leaf node, then
    // it contains one of the input
    // characters, print the character
    // and its code from arr[]
    #pragma omp taskwait
    if (isLeaf(root)) {
        install(root->data, arr);
    }
}

// The main function that builds a
// Huffman Tree and print codes by traversing
// the built Huffman Tree
void HuffmanCodes(char data[], int freq[], int size)

{
    
    // Construct Huffman Tree
    struct MinHeapNode* root
        = buildHuffmanTree(data, freq, size);
    

    // Print Huffman codes using
    // the Huffman tree built above
    char arr[MAX_TREE_HT], top = 0;

    #pragma omp parallel
    {
        #pragma omp single
        FillCodesList(root, arr, top);
    }
}


// Driver code
int main()
{
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    char alphabeth[] = "ABCDEFGHIJKLMNOPQRSTUVZabcdefghijklmnopqrstuvwxyz ,";
    char *input_string;
    int frequencies[sizeof(alphabeth) / sizeof(char)] = {0};
    int reduce_buff[sizeof(alphabeth) / sizeof(char)] = {0};
    int *out_freq;
    char *out_alphabet;
    char recv_buff[RECV_SIZE] = {""};
    int *displs;
    int *sendcount;
    char start_scatter = '0';

    double start, finish;

    if (myrank == 0)
    {

        /* Reading string from default file */
        char local_string[INPUT_SIZE] = {""};
        input_string = local_string;
        char default_textfile[] = "input.txt";
        read_input_string(input_string, INPUT_SIZE, default_textfile);

        /* Calculating substing per process */
        int input_size = strlen(input_string);
        int padding = input_size % world_size;
        int size_per_process = floor(input_size / world_size);
        int k = 0, i = 0;
        /* Initializing scatter params */
        if (input_size < world_size || world_size == 1)
        {
            calculate_frequencies(alphabeth, input_string, reduce_buff);
        }
        else
        {
            start_scatter = '1';
            displs = malloc(world_size * sizeof(int));
            sendcount = malloc(world_size * sizeof(int));
            for (i = 0; i < world_size; i++)
            {
                /* Last process will receive also the padding */
                
                if (i == (world_size - 1))
                    sendcount[i] = size_per_process + padding;
                else
                    sendcount[i] = size_per_process;

                displs[i] = k;
                k += size_per_process;
            }
        }
    }

    start = MPI_Wtime();
    MPI_Bcast(&start_scatter, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    if (start_scatter == '1')
    {
        printf("Scatter process %d\n", myrank);
        MPI_Scatterv(input_string, sendcount, displs, MPI_CHAR, recv_buff, RECV_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
        calculate_frequencies(alphabeth, recv_buff, frequencies);
        printf("before reduce process: %d char %c: %d\n", myrank, alphabeth[23], frequencies[23]);
        MPI_Reduce(frequencies, reduce_buff, sizeof(frequencies) / sizeof(int), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /* Process 0 takes care of preparing alphabeth and frequencies output arrays */
    if (myrank == 0)
    {
        int len = strlen(alphabeth);
        out_alphabet = (char *)malloc(len * sizeof(char));
        out_freq = (int *)calloc(len, sizeof(int));
        int i, count = 0;
        for (i = 0; i < len; i++)
        {
            if (reduce_buff[i] != 0)
            {
                out_freq[count] = reduce_buff[i];
                out_alphabet[count] = alphabeth[i];
                count++;
            }
        }
        out_alphabet = realloc(out_alphabet, count * sizeof(char));
        out_freq = realloc(out_freq, count * sizeof(int));
        finish = MPI_Wtime();

        /* Debug output */
        for (i = 0; i < count; i++)
        {
            printf("char: %c freq: %d\n", out_alphabet[i], out_freq[i]);
        }
        printf("Total execution time: %e\n", finish - start);
       
        size = count;
        /* Build huff tree */
        HuffmanCodes(out_alphabet, out_freq, count);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}