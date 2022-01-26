/**
 * @file frequencies-mpi.c
 * @author Nicola Arpino & Alessandra Morellini
 * @brief Program that calculates fequencies of strings using MPI
 * @version 0.1
 * @date 2022-01-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "frequencies.h"
#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

/**
 * @param alphabeth string containing the alpabhet e.g "abc..z"
 * @param input_string input string of max lenght INPUT_SIZE
 * @param out_buffer location in which save frequency vector
 */
void frequencies_builder(char *alphabeth, char *input_string, int *out_buffer)
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

void calculate_frequencies(char* alphabeth, char* input_string, int* out_frequencies, char* out_alphabeth, MPI_Comm comm)
{
    int *frequencies;
    int *reduce_buff;
    int *displs;
    int *sendcount;
    char start_scatter = '0';
    int my_rank, world_size;

    frequencies = calloc(strlen(alphabeth), sizeof(int));
    reduce_buff = calloc(strlen(alphabeth), sizeof(int));

    double start, finish;
    MPI_Comm_size(comm, &world_size);
    MPI_Comm_rank(comm, &my_rank);
    int input_size = strlen(input_string);
    int padding = input_size % world_size;
    int size_per_process = floor(input_size / world_size);
    char *recv_buff = calloc(size_per_process + padding, sizeof(char));

    if (my_rank == 0)
    {
        /* Calculating substing per process */
        int k = 0, i;
        /* Initializing scatter params */
        if (input_size < world_size || world_size == 1)
        {
            frequencies_builder(alphabeth, input_string, reduce_buff);
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
        printf("Scatter process %d\n", my_rank);
        MPI_Scatterv(input_string, sendcount, displs, MPI_CHAR, recv_buff, size_per_process + padding, MPI_CHAR, 0, MPI_COMM_WORLD);
        frequencies_builder(alphabeth, recv_buff, frequencies);
        MPI_Reduce(frequencies, reduce_buff, strlen(alphabeth), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }


    /* Process 0 takes care of preparing alphabeth and frequencies output arrays */
    if (my_rank == 0)
    {

        int i, count = 0, len = strlen(alphabeth);
        for (i = 0; i < len; i++)
        {
            if (reduce_buff[i] != 0)
            {
                out_frequencies[count] = reduce_buff[i];
                out_alphabeth[count] = alphabeth[i];
                count++;
            }
        }
        out_alphabeth = realloc(out_frequencies, count * sizeof(char));
        out_frequencies = realloc(out_frequencies, count * sizeof(int));
        finish = MPI_Wtime();

        /* Debug output */
        for (i = 0; i < count; i++)
        {
            printf("char: %c freq: %d\n", out_alphabeth[i], out_frequencies[i]);
        }
        printf("Total execution time: %e\n", finish - start);
    }

}