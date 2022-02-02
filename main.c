// C program for Huffman Coding

#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "tree_utils.h"
#include "frequencies_utils.h"

#define RECV_SIZE 2000
#define INPUT_SIZE 2000
#define REALLOC_OFFSET 5

// This constant can be avoided by explicitly
// calculating height of Huffman Tree
#define MAX_TREE_HT 100
#define HASHSIZE 100
#define CODES_LEN 10

// A Huffman tree node
struct MinHeapNode
{
    char data;                          /* defined char */
    unsigned freq;                      /* frequency */
    struct MinHeapNode *left, *right;   /* pointers to left and right nodes */
};

struct nlist
{                         /* table entry: */
    char name;            /* defined char */
    char code[CODES_LEN]; /* code */
};
int size;
struct nlist codes_list[HASHSIZE];

/* hash: form hash value for char s */
unsigned hash(char s)
{
    unsigned hashval;
    hashval = s;
    return hashval % size;
}

/* lookup: look for s in codes_list */
bool lookup(char s)
{
    struct nlist np;
    np = codes_list[hash(s)];
    if (np.name == s)
    {
        return true; /* found */
    }
    else if (np.name != 0)
    {
        fprintf(stderr, "Bad lookup: %c literal hash is already used!\n", s);
        exit(-1);
    }
    return false; /* not found */
}

/* install: put (name, code) in codes_list */
bool install(char name, char *code)
{
    unsigned hashval;
    if (!lookup(name))
    { /* not found */
        hashval = hash(name);
        codes_list[hashval].name = name;
        memcpy(codes_list[hashval].code, code, strlen(code));
        return true;
    }
    return false;
}


// Traverse the huffman tree and fill the codes_list array.
void FillCodesList(struct MinHeapNode *root, char arr[],
                   int top)

{
    // Assign 0 to left edge and recur
    if (root->left)
    {
#pragma omp task shared(arr, top) firstprivate(root) depend(out \
                                                            : top)
        {
#pragma omp critical
            arr[top] = '0';

            FillCodesList(root->left, arr, top + 1);
        }
    }

    // Assign 1 to right edge and recur
    if (root->right)
    {
#pragma omp task shared(arr, top) firstprivate(root) depend(in \
                                                            : top)
        {
#pragma omp critical
            arr[top] = '1';

            FillCodesList(root->right, arr, top + 1);
        }
    }

// If this is a leaf node, then
// it contains one of the input
// characters, print the character
// and its code from arr[]
#pragma omp taskwait
    if (isLeaf(root))
    {
        arr[top] = '\0';
        bool res = install(root->data, arr);
        if (!res)
        {
            fprintf(stderr, "Bad install!\n");
            exit(-1);
        }
    }
}

/**
 * @brief Walks the code-table and returns the huffman code for a given string
 *
 * @param in_str 
 * @return char* huff code
 */
char *calculate_huff_code(char *in_str)
{
    int i = 0, code_len = 0, buff_len = 10;
    char *code;
    char *out_string = (char *)calloc(buff_len, sizeof(char));
    for (i = 0; i < strlen(in_str); i++)
    {
        code = codes_list[hash(in_str[i])].code;
        code_len = strlen(code);
        if ((buff_len - strlen(out_string)) <= code_len)
        {
            buff_len += (code_len * REALLOC_OFFSET);
            out_string = (char *)realloc(out_string, buff_len);
        }
        strcat(out_string, code);
    }
    return out_string;
}


    /**
     * @brief 
     * 
     * @param root 
     * @param in_string 
     * @param size_per_thread 
     * @param padding 
     */
    char *decode_string(struct MinHeapNode* root, char* in_string, int size_per_thread, int padding, int total_threads)
    {
        int thread_rank = omp_get_thread_num();
        struct MinHeapNode *node = root;
        int len = strlen(in_string);
        char *decoded_string = (char*)calloc(len, sizeof(char));
        int initial_offset, end_offset, i;

        initial_offset = thread_rank * size_per_thread;
        end_offset = initial_offset + size_per_thread;
        if(total_threads == thread_rank && padding > 0)
            end_offset += padding;
        
        printf("Thread %d size_per_thread %d initial_off %d final_off %d\n", thread_rank, size_per_thread, initial_offset, end_offset);
        for(i=initial_offset; i<end_offset; i++)
        {
            if(in_string[i] == '0' && node->left != NULL){
                node = node->left;
            }else if(node->right != NULL){
                node = node->right;
            }

            if(isLeaf(node)){
                strncat(decoded_string, &node->data, 1);
                node = root;
            }
        }
        printf("Thread [%d] string %s\n", thread_rank, decoded_string);
        return decoded_string;
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

    /* Derived datatype for struct */
    const int nitems = 2;
    int blocklengths[2] = {1, CODES_LEN};
    MPI_Datatype types[2] = {MPI_CHAR, MPI_CHAR};
    MPI_Aint offsets[2];
    MPI_Datatype mpi_codeblock;
    MPI_Datatype mpi_codelist;
    offsets[0] = offsetof(struct nlist, name);
    offsets[1] = offsetof(struct nlist, code);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_codeblock);
    MPI_Type_commit(&mpi_codeblock);
    MPI_Type_contiguous(HASHSIZE, mpi_codeblock, &mpi_codelist);
    MPI_Type_commit(&mpi_codelist);

    size = strlen(alphabeth);
    if (myrank == 0)
    {

        /* Reading string from default file */
        char local_string[INPUT_SIZE] = {""};
        input_string = local_string;
        char default_textfile[] = "myText.txt";
        read_input_string(input_string, INPUT_SIZE, default_textfile);

        /* Calculating substing per process */
        int input_size = strlen(input_string);
        int padding = input_size % world_size;
        int size_per_process = floor(input_size / world_size);
        int k = 0, i = 0;
        /* Initializing scatter params */
        if (input_size < world_size)
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
        MPI_Scatterv(input_string, sendcount, displs, MPI_CHAR, recv_buff, RECV_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
        calculate_frequencies(alphabeth, recv_buff, frequencies);
        MPI_Reduce(frequencies, reduce_buff, sizeof(frequencies) / sizeof(int), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    printf("process %d string %s\n", myrank, recv_buff);
    
    MPI_Barrier(MPI_COMM_WORLD);

    struct MinHeapNode *root;
    /* Process 0 takes care of preparing alphabeth and frequencies output arrays */
    if (myrank == 0)
    {
        int len = strlen(alphabeth);
        out_alphabet = (char *)calloc(len, sizeof(char));
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
        printf("Total execution time: %e\n", finish - start);

        /* Build huff tree */
        root = HuffmanCodes(out_alphabet, out_freq, count);
        printf("-------\n");
        
        // Print Huffman codes using
        // the Huffman tree built above
        char arr[MAX_TREE_HT], top = 0;

    #pragma omp parallel
        {
    /* TODO check correcrness with and without nowait*/
    #pragma omp single
            FillCodesList(root, arr, top);
        }

        for (i = 0; i < count; i++)
        {
            printf("char %c code %s\n", codes_list[hash(out_alphabet[i])].name, codes_list[hash(out_alphabet[i])].code);
        }
    }



    /* Sending code-table to all processes */
    MPI_Bcast(codes_list, 1, mpi_codelist, 0, MPI_COMM_WORLD);

    char *out, *final_string;
    out = calculate_huff_code(recv_buff);

    /* Encoding with huff codes */
    if (start_scatter == '1'){
        int counts[world_size], gather_disps[world_size], i;
        int nelem = strlen(out);
        MPI_Gather(&nelem, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
        for (i = 0; i < world_size; i++)
            gather_disps[i] = (i > 0) ? (gather_disps[i - 1] + counts[i - 1]) : 0;
        
        
        if (myrank == 0)
            final_string = (char *)calloc(gather_disps[world_size - 1] + counts[world_size - 1], sizeof(char));

        MPI_Gatherv(out, nelem, MPI_CHAR, final_string, counts, gather_disps, MPI_CHAR, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        if(myrank == 0){
            printf("FINAL: %s\n", final_string);
        }
    }
    
    

    /* Serial decoding */
    if(myrank == 0){
        printf("\n---- SERIAL DECODING ----\n");
        struct MinHeapNode *node = root;
        int i, len;
        len = strlen(final_string);
        char *decoded_string = (char*)calloc(len, sizeof(char));
        for(i=0; i<len; i++)
        {
            if(final_string[i] == '0' && node->left != NULL){
                node = node->left;
            }else if(node->right != NULL){
                node = node->right;
            }

            if(isLeaf(node)){
                strncat(decoded_string, &node->data, 1);
                node = root;
            }
        }
    }

    if(myrank == 0){
        printf("\n---- PARALLEL DECODING ----\n");
        omp_set_num_threads(2);
        double tstart, tstop;
        int len;
        len = strlen(final_string);
        int thread_count = 2;
        int padding = len % thread_count;
        int size_per_process = floor(len / thread_count);
        char *decoded_str;

        printf("len %d\n", omp_get_num_threads());

        tstart = omp_get_wtime();
        #pragma omp parallel
        decoded_str = decode_string(root, final_string, size_per_process, padding, thread_count);
        tstop = omp_get_wtime();
        printf("Elapsed time: %f\n", tstop - tstart);
    }

    // Finalize the MPI environment.
    MPI_Type_free(&mpi_codelist);
    MPI_Type_free(&mpi_codeblock);
    MPI_Finalize();
    return 0;
}