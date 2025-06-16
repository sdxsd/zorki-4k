#ifndef LISTS_H
#define LISTS_H

#include <stdlib.h>

typedef struct s_list {
  void *data;
  struct s_list *next;
} t_list;

void list_remove_front(t_list **list);
void list_add_front(t_list **list, void *data);
void list_clear(t_list **list);
t_list *new_list(void *data);
size_t list_count(t_list *list);

#endif // LISTS_H
