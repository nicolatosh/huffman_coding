#include <mpi.h>

#ifndef FREQENCIES_H
# define FREQENCIES_H

void calculate_frequencies(char* alphabeth, char* input_string, int* out_frequencies, char* out_alphabeth, MPI_Comm comm);

#endif