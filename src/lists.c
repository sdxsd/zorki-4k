#include "../includes/lists.h"

// Removes an element from the front of the list.
void list_remove_front(t_list **list) {
  t_list *list_ptr = *list;
  if (!list_ptr->next) {
    free(list_ptr);
    *list = NULL;
  }
  if (list_ptr->next && !list_ptr->next->next) { // Is end of list?
    free(list_ptr->next);
    list_ptr->next = NULL;
  }
  else
    list_remove_front(&list_ptr->next);
}

// Adds an element to the front of the list.
void list_add_front(t_list **list, void *data) {
  t_list *list_ptr = *list;
  if (!list_ptr->next) {
    list_ptr->next = new_list(data);
    if (!list_ptr->next)
      return;
  }
  else
    list_add_front(&list_ptr->next, data);
}

// Deallocates the list.
void list_clear(t_list **list) {
  t_list *list_ptr = *list;

  if (!list || !*list)
    return;
  list_clear(&list_ptr->next);
  free(list_ptr);
  *list = NULL;
}

// Returns a pointer to the head of a new list.
t_list *new_list(void *data) {
  t_list *list = calloc(1, sizeof(t_list));
  list->data = data;
  return (list);
}

size_t list_count(t_list *list) {
  size_t count = 1;
  if (!list)
    return (0);
  if (list->next)
    return (count + list_count(list->next));
  else
    return (count);
}
