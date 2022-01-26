#include <mpi.h>
#include <stdbool.h>

#ifndef FREQENCIES_H
# define FREQENCIES_H

/**
 * @brief Reads string from textfile.
 * 
 * @param out_string string read. Caller must allocate memory
 * @param maxlen how much charachters to be read
 * @param in_filename optional filename e.g NULL or "somefile.txt"
 * @return true if read did not fail
 */
bool read_input_string(char *out_string, int maxlen, char *in_filename);

/**
 * @param alphabeth string containing the alpabhet e.g "abc..z"
 * @param input_string input string of max lenght INPUT_SIZE
 * @param out_buffer location in which save frequency vector
 */
void calculate_frequencies(char *alphabeth, char *input_string, int *out_buffer);

#endif