#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

#include "lists.h"

#define BLKSIZE 1024
#define true 1
#define false 0

// Structures:
typedef struct s_shellcode {
  char *buf;
  size_t length;
  char chunk_number;
  char chunk_total;
} t_shellcode;

typedef struct s_file {
  char *absolute_path;
  char *filename;
} t_file;

// Functions:
void DEBUG_print_shellcode(t_shellcode *shellcode);
long size_of_file(FILE *fp);
t_shellcode *read_shellcode(FILE *fp);
t_file *new_file(char *absolute_path, char *filename);
void *free_file(t_file **file);
void free_file_list(t_list **list);
int check_extension(t_file *file);
int handle_regular_file(t_list *file_list, struct dirent *dir, char *root_path);
t_list *find_files(char *root_path);
int is_valid_target_file(t_file *file);

#endif // UTILS_H
