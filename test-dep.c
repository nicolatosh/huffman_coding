
/* Not runnable code. Used for dependence analysis */
for (i = 1; i < thread_count; i++)
{
    temp_string = decoded_list[i][bits].string;
    bits = decoded_list[i][bits].padding_bits;
    strncat(final_decoded_string, temp_string, strlen(temp_string));
}

