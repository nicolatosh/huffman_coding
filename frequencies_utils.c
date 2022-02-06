/**
 * @file frequencies_utils.c
 * @author Nicola Arpino, Alessandra Morellini
 * @brief Implementation of frequencies utilities
 * @version 0.2
 * @date 2022-02-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "frequencies_utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Reads string from textfile
 * 
 * @param out_string string read. Caller must allocate memory
 * @param maxlen how much charachters to be read
 * @param in_filename optional filename e.g NULL or "somefile.txt"
 * @return true if read did not fail
 */
bool read_input_string(char *out_string, int maxlen, char *in_filename)
{
    /* Reading string from default file */
    char *filename = "input.txt";

    /* Check on optional param filename */
    if (in_filename != NULL)
    {
        filename = in_filename;
    }

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error reading textfile [%s].\n", filename);
        return false;
    }

    char ch;
    int i = 0;
    while ((ch = fgetc(fp)) != EOF && ch != '\0' && i < maxlen)
    {
        if (ch == '\n')
        {
            out_string[i] = ' ';
            i++;
            continue;
        }
        out_string[i] = ch;
        i++;
    }
    out_string[i] = '\0';
    fclose(fp);
    return true;
}

/**
 * @param alphabeth string containing the alpabhet e.g "abc..z"
 * @param input_string input string of max lenght INPUT_SIZE
 * @param out_buffer location in which save frequency vector
 */
void calculate_frequencies(char *alphabeth, char *input_string, int *out_buffer)
{
    int i, idx = 0;
    char *ret;

    /* Finding occurencies */
    for (i = 0; i < strlen(input_string); i++)
    {
        ret = strchr(alphabeth, input_string[i]);
        if (ret == NULL){
            fprintf(stderr, "ERROR: character not found in the alphabet!\n");
            exit(-1);
        }
        idx = strlen(alphabeth) - strlen(ret);
        out_buffer[idx] += 1;
    }
}