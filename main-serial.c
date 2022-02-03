// C program for Huffman Coding

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "tree_utils.h"
#include "frequencies_utils.h"

#define INPUT_SIZE 2000
#define REALLOC_OFFSET 5

// This constant can be avoided by explicitly
// calculating height of Huffman Tree
#define MAX_TREE_HT 100
#define HASHSIZE 100
#define CODES_LEN 15


// A Huffman tree node
struct MinHeapNode
{
    char data;                          /* defined char */
    unsigned freq;                      /* frequency */
    struct MinHeapNode *left, *right;   /* pointers to left and right nodes */
};

struct nlist
{                         
    char name;            /* defined char */
    char code[CODES_LEN]; /* code */
};
char alphabeth[] = "!#$&'()*+-.,/0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVZ[]^_abcdefghijklmnopqrstuvwxyzìèéòàù{|} ";
int size;
struct nlist codes_list[HASHSIZE];

/* hash: form hash value for char s */
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



// Traverse the huffman tree and fill the codes_list array.
void FillCodesList(struct MinHeapNode *root, char arr[],
                   int top)

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

    // If this is a leaf node, then it contains one of the input
    // characters, install a new node into code_list
    if (isLeaf(root))
    {
        arr[top] = '\0';
        bool res = install(root->data, arr);
        if (!res)
        {
            fprintf(stderr, "ERROR: Bad install!\n");
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


// Driver code
int main()
{
    size = strlen(alphabeth);
    char *input_string;
    int frequencies[sizeof(alphabeth) / sizeof(char)] = {0};
    int *out_freq;
    char *out_alphabet;
   
    /* Reading string from default file */
    input_string = (char*)calloc(sizeof(char), INPUT_SIZE);
    char default_textfile[] = "myText.txt";
    read_input_string(input_string, INPUT_SIZE, default_textfile);

    /* Calculate frequences of chars */
    calculate_frequencies(alphabeth, input_string, frequencies);

    int len = strlen(alphabeth);
    out_alphabet = (char *)calloc(len, sizeof(char));
    out_freq = (int *)calloc(len, sizeof(int));
    int i, count = 0;
    for (i = 0; i < len; i++)
    {
        if (frequencies[i] != 0)
        {
            out_freq[count] = frequencies[i];
            out_alphabet[count] = alphabeth[i];
            count++;
        }
    }
    out_alphabet = realloc(out_alphabet, count * sizeof(char));
    out_freq = realloc(out_freq, count * sizeof(int));

    /* Build huff tree */
    struct MinHeapNode *root;
    root = HuffmanCodes(out_alphabet, out_freq, count);

    // Fill the codes list using
    // the Huffman tree built 
    char arr[CODES_LEN], top = 0;
    
    FillCodesList(root, arr, top);

    // for (i = 0; i< count; i++){
    //     printf("char %c code %s\n", codes_list[hash(out_alphabet[i])].name, codes_list[hash(out_alphabet[i])].code);
    // }

    char *final_string;
    final_string = calculate_huff_code(input_string);
    //printf("coded string %s\n", final_string);
    
    /* decoding */
    struct MinHeapNode *node = root;
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
    printf("decoded string %s\n", decoded_string);
    return 0;
}