/**
 * @file frequencies-serial.c
 * @author Nicola Arpino, Alessandra Morellini
 * @brief First version of a serial frequencies calculation.
 * @version 0.2
 * @date 2022-02-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{

    typedef struct
    {
        char literal;
        int frequency;
    } Freqnode;

    Freqnode **freqlist = (Freqnode **)malloc(sizeof(Freqnode *) * 26);

    /* Modify this string, this is the input */
    char *string = "ciao";
    char alphabeth[] = "abcdefghijklmnopqrstuvwxyz";

    int frequencies[26];
    int i, k, idx = 0;
    char *ret;

    memset(frequencies, 0, 25 * sizeof(int));
    /* Finding occurencies */
    for (i = 0; i <= strlen(string); i++)
    {
        ret = strchr(alphabeth, string[i]);
        idx = strlen(alphabeth) - strlen(ret);
        frequencies[idx]++;
    }

    i = 0;
    for (k = 0; k <= 25; k++)
    {
        if (frequencies[k] != 0)
        {
            Freqnode *node = (Freqnode *)malloc(sizeof(Freqnode));
            node->frequency = frequencies[k];
            node->literal = alphabeth[k];
            freqlist[i] = node;
            i++;
        }
    }

    freqlist = realloc(freqlist, i*sizeof(Freqnode));
    printf("Frequencies: ");
    for (k = 0; k < i - 1; k++)
        printf("%c,%d ", freqlist[k]->literal, freqlist[k]->frequency);
}