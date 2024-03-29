/**
 * Program that includes parallel encoding and decoding of Huffman algorithm
 * @file main.c
 * @author Nicola Arpino, Alessandra Morellini
 * @brief 
 * @version 1.3
 * @date 2022-02-06
 * 
 */

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

/* Configuration of constants */

/* Make sure RECV_SIZE and INPUT_SIZE are equals */
/* Their value is the max size of the string that can be read */
#define RECV_SIZE 200000
#define INPUT_SIZE 200000

/* This number should be calculated as "log2(length of alphabet)" */
/* It is an upper-bound and represents  */

#define SYMBOL_MAX_BITS 5

/* Length of a single-code. Must be at least as SYMBOL_MAX_BITS */
#define CODES_LEN 15


/* Those constants comes from original Min-heap algorithm. */
#define MAX_TREE_HT 100
#define HASHSIZE 100

/* This is the alphabet. If input-string contains additional characters, put them here */
char alphabeth[] = "!#$&'()*+-.,/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVZ[]^_abcdefghijklmnopqrstuvwxyz{|} ";

/* A Huffman tree node */
struct MinHeapNode
{
    char data;                        /* defined char */
    unsigned freq;                    /* frequency */
    struct MinHeapNode *left, *right; /* pointers to left and right nodes */
};

struct nlist
{                         /* table entry: */
    char name;            /* defined char */
    char code[CODES_LEN]; /* code */
};
int size;
struct nlist codes_list[HASHSIZE];

/* hash: returns the hash value for char s */
unsigned hash(char s)
{
    char *ret;
    int idx;
    ret = strchr(alphabeth, s);
    idx = strlen(alphabeth) - strlen(ret);
    return idx;
    
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
        fprintf(stderr, "ERROR: Bad lookup. %c literal hash is already used by %c!\n", s, np.name);
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

/* This node is used into decoding phase. See 'decoded_node()' function */ 
struct decoded_node
{
    char *string;     /* decoded string */
    int padding_bits; /* bits needed to decode another valid literal */
};

/* Parallel function task based to create the code-word table
 * This is another possible solution w.r.t the one used. More details
 * can be found in the project report
 */
// void FillCodesList(struct MinHeapNode *root, char arr[], int top)
// {
//     // Assign 0 to left edge and recur
//     if (root->left)
//     {
//         #pragma omp task firstprivate(root, arr, top) depend(out: top)
//         {
//             #pragma omp critical
//                 arr[top] = '0';

//             FillCodesList(root->left, arr, top + 1);
//         }
//     }

//     // Assign 1 to right edge and recur
//     if (root->right)
//     {
//         #pragma omp task firstprivate(root, arr, top) depend(in: top)
//         {
//             #pragma omp critical
//                 arr[top] = '1';

//             FillCodesList(root->right, arr, top + 1);
//         }
//     }

//     // If this is a leaf node, then
//     // it contains one of the input
//     // characters, print the character
//     // and its code from arr[]
//     #pragma omp taskwait
//     if (isLeaf(root))
//     {
//         arr[top] = '\0';
//         bool res = install(root->data, arr);
//         if (!res)
//         {
//             fprintf(stderr, "Bad install!\n");
//             exit(-1);
//         }
//     }
// }

/* Serial function to create the code-word table */
void FillCodesList(struct MinHeapNode *root, char arr[], int top)
{
    // Assign 0 to left edge and recur
    if (root->left)
    {
        arr[top] = '0';
        FillCodesList(root->left, arr, top + 1);
    }

    // Assign 1 to right edge and recur
    if (root->right)
    {
        arr[top] = '1';
        FillCodesList(root->right, arr, top + 1); 
    }

    // If this is a leaf node, then
    // it contains one of the input
    // characters, print the character
    // and its code from arr[]
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
    int i = 0, code_len = 0, buff_len = 10, string_len = strlen(in_str);
    char *code;
    char *out_string = (char *)calloc(buff_len, sizeof(char));
    for (i = 0; i < string_len; i++)
    {
        code = codes_list[hash(in_str[i])].code;
        code_len = strlen(code);
        if ((buff_len - strlen(out_string)) <= code_len)
        {
            buff_len += (code_len * SYMBOL_MAX_BITS);
            out_string = (char *)realloc(out_string, buff_len);
        }
        strcat(out_string, code);
    }
    return out_string;
}

/**
 * @brief Function to decode a piece of string. Called within parallel region.
 * 
 * @param root root of Huff tree
 * @param in_string input string
 * @param size_per_thread how much of the string to manage
 * @param padding extra padding
 * @param total_threads how many threads are available in total
 * @param offset how much overlap between decoded strings of different threads
 * @return struct decoded_node* 
 */
struct decoded_node *decode_string(struct MinHeapNode *root, char *in_string, int size_per_thread, int padding, int total_threads, int offset)
{
    /* Myrank */
    int thread_rank = omp_get_thread_num();
    struct MinHeapNode *node = root;
    int len = strlen(in_string);
    /* Pointer to different nodes. There are up to 'offset' nodes */
    /* Each thread will have 'offset' number of different decoded strings */
    struct decoded_node *d_node = (struct decoded_node *)malloc(sizeof(d_node) * offset);
    char *local_string;
    int initial_offset, end_offset, i, k, local_initial_offset;
    initial_offset = thread_rank * size_per_thread;
    end_offset = initial_offset + size_per_thread;

    /* Last thread will manage also padding */
    if (total_threads == thread_rank && padding > 0)
        end_offset += padding;

    /* Each thread will decode many candidate decode-strings. How many? up to 'offset' */
    for (k = 0; k < offset; k++)
    {
        /* Those indices makes the ovelapping strategy for each thread */
        local_initial_offset = initial_offset - k;
        local_string = calloc(len, sizeof(char));
        int last_literal_index = 0;
        node = root;
        for (i = local_initial_offset; i < end_offset; i++)
        {
            if (in_string[i] == '0' && node->left != NULL)
            {
                node = node->left;
            }
            else if (node->right != NULL)
            {
                node = node->right;
            }

            if (isLeaf(node))
            {
                strncat(local_string, &node->data, 1);
                node = root;
                last_literal_index = i;
            }
        }
        d_node[k].string = (char *)calloc(sizeof(char), len);
        strncpy(d_node[k].string, local_string, len);
        d_node[k].padding_bits = (i - last_literal_index - 1);

        /* Thread 0, since started from beginning produces only one string */
        if (thread_rank == 0)
        {
            break;
        }
    }

    return d_node;
}

/* Main code */
int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    // Reading number of threads    
    int thread_count = atoi(argv[1]);
    char *input_string, *out_alphabet;
    int frequencies[sizeof(alphabeth) / sizeof(char)] = {0};
    int reduce_buff[sizeof(alphabeth) / sizeof(char)] = {0};
    char recv_buff[RECV_SIZE] = {""};
    int *out_freq, *displs, *sendcount;
    char start_scatter = '0';
    size = strlen(alphabeth);

    /* Timing data */
    double start, finish;
    /* Derived datatype for struct 'mpi_codeblock' */
    /* This is needed in order to be able to send 
    * decoded candidate nodes to a collector (Process 0)
    */
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
    
    /* Here actual program starts. The MPI process 0 will:
    *   1) Read the input from file.
    *   2) Calculate how much substring should each other process manage.
    *   3) Send a piece of string to each other process (ScatterV).
    *   4) Takes time for the whole encoding operation (tree building + encoding)
    */
    printf("Process rank %d\n", myrank);
    if (myrank == 0)
    {
        /* Reading string from default file */
        input_string = (char*)calloc(sizeof(char), INPUT_SIZE);
        char default_textfile[] = "input.txt";
        read_input_string(input_string, INPUT_SIZE, default_textfile);

        /* Calculating substing per process */
        int input_size = strlen(input_string);
        int padding = input_size % world_size;
        int size_per_process = floor(input_size / world_size);
        int k = 0, i = 0;
        /* Initializing scatter params in case the size can be divided into the processes */
        if (input_size > world_size)
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

    /* Process 0 starts timer for measuring encoding time */
    if (myrank == 0)
        start = MPI_Wtime();

    MPI_Bcast(&start_scatter, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    /*MPI_Scatterv and MPI_Reduce are done only if the input can be divided into processes */
    if (start_scatter == '1')
    {
        
	MPI_Scatterv(input_string, sendcount, displs, MPI_CHAR, recv_buff, RECV_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
	calculate_frequencies(alphabeth, recv_buff, frequencies);
	MPI_Reduce(frequencies, reduce_buff, sizeof(frequencies) / sizeof(int), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    }else if (myrank == 0){
        /* Otherwise only process 0 calculates the frequences for the entire string */
        /* This situation may happen when the input string is very short */
        calculate_frequencies(alphabeth, input_string, reduce_buff);
        strncpy(recv_buff, input_string, strlen(input_string));
    }


    /* Waiting every process to complete frequencies calculation */
    MPI_Barrier(MPI_COMM_WORLD);

    struct MinHeapNode *root;
    /* Process 0 takes care of preparing alphabet and frequencies output arrays */
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

        /* Build Huff-tree */
        root = HuffmanCodes(out_alphabet, out_freq, count);

        char arr[MAX_TREE_HT];
        int top = 0;

        if (thread_count == 1)
        {
            FillCodesList(root, arr, top);
        }
        else
        {
            #pragma omp parallel firstprivate(arr, top) num_threads(2)
            {
                if (omp_get_thread_num() == 0)
                {
                    arr[top] = '1';
                    FillCodesList(root->right, arr, top + 1);
                }
                else if (omp_get_thread_num() == 1)
                {
                    arr[top] = '0';
                    FillCodesList(root->left, arr, top + 1);
                }
            }
        }
        // for (i = 0; i < count; i++)
        // {
        //     /* Only for debug, comment in production */
        //     printf("char %c code %s\n", codes_list[hash(out_alphabet[i])].name, codes_list[hash(out_alphabet[i])].code);
        // }
        free(out_freq);
        free(out_alphabet);
    }

    /* Sending code-word table to all processes */
    /* In this way each process can encode a piece of the intial input-string */
    MPI_Bcast(codes_list, 1, mpi_codelist, 0, MPI_COMM_WORLD);
    

    char *out, *final_string;
    out = calculate_huff_code(recv_buff);

    /* When scatter equals to 1 process 0 collect with a MPO_Gatherv all the encoded string from the other processes. */
    if (start_scatter == '1')
    {
        int counts[world_size], gather_disps[world_size], i;
        int nelem = strlen(out);
        MPI_Gather(&nelem, 1, MPI_INT, counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
        for (i = 0; i < world_size; i++)
            gather_disps[i] = (i > 0) ? (gather_disps[i - 1] + counts[i - 1]) : 0;

        if (myrank == 0)
            final_string = (char *)calloc(gather_disps[world_size - 1] + counts[world_size - 1], sizeof(char));

        MPI_Gatherv(out, nelem, MPI_CHAR, final_string, counts, gather_disps, MPI_CHAR, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    }else if(myrank == 0){
        /*Otherwise the process 0 don't collect anything from other process and set final_string variable with
         the content of out */
        final_string = (char *)calloc(strlen(out), sizeof(char));
        strncpy(final_string, out, strlen(out));
    }
    
    
    /*In any case process 0 print the actual encoded final_string value */
    if (myrank == 0)
    {
        finish = MPI_Wtime();
        printf("Encoding execution time: %e\n", finish - start);
    }


    /* Parallel decoding part */
    if(myrank == 0){
        int len, i; 
	    len = strlen(final_string);
        int padding = len % thread_count;
        int size_per_process = floor(len / thread_count);
        double tstart, tstop;
        int offset = 5;
        if (size_per_process < offset)
        {
            thread_count = 1;
            omp_set_num_threads(1);
            size_per_process = len;
            padding = 0;
        }
        struct decoded_node **decoded_list = (struct decoded_node **)malloc(sizeof(decoded_list) * thread_count);
        char *final_decoded_string = (char *)calloc(len, sizeof(char));

        /* Parallel decoding */
        tstart = omp_get_wtime();
        #pragma omp parallel 
        {
	    printf("Thread num %d\n", omp_get_thread_num());
            decoded_list[omp_get_thread_num()] = decode_string(root, final_string, size_per_process, padding, thread_count - 1, offset);
        }
        int bits;
        tstop = omp_get_wtime();
        char *temp_string;
        printf("Merging of decoded contributions\n");

        /* Thread 0 token is special, getting bits */
        temp_string = decoded_list[0][0].string;
        bits = decoded_list[0][0].padding_bits;
        strncat(final_decoded_string, temp_string, strlen(temp_string));
    
        /* Other threads tokens */
        for (i = 1; i < thread_count; i++)
        {
            temp_string = decoded_list[i][bits].string;
            bits = decoded_list[i][bits].padding_bits;
            strncat(final_decoded_string, temp_string, strlen(temp_string));
        }

        tstop = omp_get_wtime();
        printf("Decoding execution time: %f\n", tstop - tstart);
	    /* Verify of correctness */
        int res = strcmp(input_string, final_decoded_string);
        printf("res: [%d]\n", res);
        free(decoded_list);
        free(final_string);
        free(input_string);
    }

    // Finalize the MPI environment.
    MPI_Type_free(&mpi_codelist);
    MPI_Type_free(&mpi_codeblock);
    MPI_Finalize();
    return 0;
}
