/**
 * @file frequencies-mpi.c
 * @author your name Nicola Arpino & Alessandra Morellini
 * @brief Program that calculates fequencies of strings using MPI
 * @version 0.1
 * @date 2022-01-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#define RECV_SIZE 200000
#define INPUT_SIZE 200000

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
        idx = strlen(alphabeth) - strlen(ret);
        out_buffer[idx] += 1;
    }
}

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    char alphabeth[] = "ABCDEFGHIJKLMNOPQRSTUVZabcdefghijklmnopqrstuvwxyz ,";
    char input_string[INPUT_SIZE] = {""};
    int frequencies[sizeof(alphabeth) / sizeof(char)] = {0};
    int reduce_buff[sizeof(alphabeth) / sizeof(char)] = {0};
    int *final_freq;
    char *final_alphabeth;
    char recv_buff[RECV_SIZE] = {""};
    int *displs;
    int *sendcount;
    char start_scatter = '0';

    double start, finish;

    if (world_rank == 0)
    {
        /* Reading string from file */
        char *filename = "input.txt";
        FILE *fp = fopen(filename, "r");

        if (fp == NULL)
        {
            fprintf(stderr, "%s", "Error reading input textfile.\n");
            exit(-1);
        }

        char ch;
        int i = 0;
        while ((ch = fgetc(fp)) != EOF && ch != '\0' && i < INPUT_SIZE)
        {
            if(ch == '\n'){
                input_string[i] = ' ';
                i++;
                continue;
            }
            input_string[i] = ch;
            i++;
        }
        input_string[i] = '\0';
        fclose(fp);

        /* Calculating substing per process */
        int input_size = strlen(input_string);
        int padding = input_size % world_size;
        int size_per_process = floor(input_size / world_size);
        int k = 0;
        i = 0;
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
        printf("Scatter process %d\n", world_rank);
        MPI_Scatterv(input_string, sendcount, displs, MPI_CHAR, recv_buff, RECV_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
        calculate_frequencies(alphabeth, recv_buff, frequencies);
        MPI_Reduce(frequencies, reduce_buff, sizeof(frequencies) / sizeof(int), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /* Process 0 takes care of preparing alphabeth and frequencies output arrays */
    if (world_rank == 0)
    {

        int len = strlen(alphabeth);
        final_alphabeth = (char *)malloc(len * sizeof(char));
        final_freq = (int *)calloc(len, sizeof(int));
        int i, count = 0;
        for (i = 0; i < len; i++)
        {
            if (reduce_buff[i] != 0)
            {
                final_freq[count] = reduce_buff[i];
                final_alphabeth[count] = alphabeth[i];
                count++;
            }
        }
        final_alphabeth = realloc(final_alphabeth, count * sizeof(char));
        final_freq = realloc(final_freq, count * sizeof(int));
        finish = MPI_Wtime();

        /* Debug output */
        for (i = 0; i < count; i++)
        {
            printf("char: %c freq: %d\n", final_alphabeth[i], final_freq[i]);
        }
        printf("Total execution time: %e\n", finish - start);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}