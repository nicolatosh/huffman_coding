#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#define RECV_SIZE 20

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

    char alphabeth[] = "abcdefghijklmnopqrstuvwxyz";
    char *input_string;
    int frequencies[sizeof(alphabeth) / sizeof(char)] = {0};
    int reduce_buff[sizeof(alphabeth) / sizeof(char)] = {0};
    int *final_freq;
    char *final_alphabeth;
    char recv_buff[RECV_SIZE] = {""};
    int *displs;
    int *sendcount;
    char start_scatter = '0';

    if (world_rank == 0)
    {
        input_string = argv[1];

        /* Checking input correctness */
        if (input_string == NULL)
            input_string = "examplestring";

        /* Calculating substing per process */
        int input_size = strlen(input_string);
        int padding = input_size % world_size;
        int size_per_process = floor(input_size / world_size);
        int i = 0, k = 0;
        /* Initializing scatter params */
        if (input_size < world_size)
        {
            calculate_frequencies(alphabeth, input_string, frequencies);
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

    MPI_Bcast(&start_scatter, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    if (start_scatter == '1')
    {
        MPI_Scatterv(input_string, sendcount, displs, MPI_CHAR, recv_buff, RECV_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
        calculate_frequencies(alphabeth, recv_buff, frequencies);
        MPI_Reduce(frequencies, reduce_buff, sizeof(frequencies) / sizeof(int), MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }


    if(world_rank == 0){
        int len = strlen(alphabeth);
        final_alphabeth = (char*) malloc(len * sizeof(char));
        final_freq = (int*) calloc(len, sizeof(int));
        int i, count = 0;
        for(i = 0; i < len; i++){
            if(reduce_buff[i] != 0){
                final_freq[count] = reduce_buff[i];
                final_alphabeth[count] = alphabeth[i];
                count++;
            }
        }
        final_alphabeth = realloc(final_alphabeth, count * sizeof(char));
        final_freq = realloc(final_freq, count * sizeof(int));
        for(i = 0; i < count; i++){
            printf("char: %c freq: %d\n", final_alphabeth[i], final_freq[i]);
        }
    }
    
    // Finalize the MPI environment.
    MPI_Finalize();
}