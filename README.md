## Huffman coding

This is an UNi project and its goal is to produce an *Hybrid* parallelized version of the famous algorithm using both OpenMPI and OpenMP.

---

The problem was approached identifying a series of steps:
1.  Frequencies calculation
2.  Creation of the binary tree
3.  Tree traversing to retrieve the codewords  
4.  Encoding  
5.  Decoding

Only the second step is left as **Serial** code menawhile the other steps are parallelized. For any detail, check the report of the project.

---
### How to run

The project has been tested on the HPC cluster ***@UniTN*** using *PBS* scheduler available.

The `main.c` file contains the whole parallel application and runs both the encoding and decoding phase. 

The input string is read from `input.txt` file. The default one is almost *800.000* long. In case you would like to modify the file, you can create a custom one with a custom string.

> ***Note***: Remember to add *'\0'* to the end of the string

*TIP: `CTRL+D` if your are editing the txt file within an enditor/terminal*


Default problem size e.g the input_string must be tuned in the *main.c*. Check it for comments.

#### Run script

In the project, the script `runhuffman.sh` is provided. Check *PBS* manual in order to modify the number of resouces allocated.

**Default configuration is:**
*select=4:ncpus=8:mpiprocs=4:ompthreads=16:mem=1gb*
- 4 chunks
- each chunk with 8 cpus
- 4 MPI processes per chunk => 16 processes in total
- 16 threads in total


  
