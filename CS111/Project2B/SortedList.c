#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>
#include <pthread.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t* list, SortedListElement_t* element)
{
  SortedListElement_t *curr = list;

  if (curr == NULL || element == NULL) 
    return;

  while (curr->next != list)
  {
    if (strcmp(curr->next->key, element->key) < 0) 
      break; 
    curr = curr->next;
  }

  if (opt_yield && INSERT_YIELD)
    pthread_yield();

  curr->next->prev = element;
  element->next = curr->next;
  element->prev = curr;
  curr->next = element;
}

int SortedList_delete(SortedListElement_t* element)
{
  if (element == NULL)
    return 1;
  if (element->next == NULL || element->prev == NULL)
    return 1;
  if (element->next->prev != element || element->prev->next != element)
    return 1;

  if (element->next != NULL) // check if it's the tail
    element->next->prev = element->prev;

  if (opt_yield && DELETE_YIELD)
    pthread_yield();

  if (element->prev != NULL) // check if it's the head
    element->prev->next = element->next;

  return 0;
}

SortedListElement_t* SortedList_lookup(SortedList_t* list, const char* key)
{
  SortedListElement_t *curr = list;

  if (curr == NULL || key == NULL)
    return NULL;

  while (curr->next != list)
  {
    if (opt_yield && LOOKUP_YIELD) 
      pthread_yield();
    if (curr->next == NULL)
      break;
    if (strcmp(curr->next->key, key) == 0)
      return curr->next;
    curr = curr->next;
  }
  return NULL;
}

int SortedList_length(SortedList_t* list)
{
  int length = 0;
  SortedListElement_t *curr = list;

  while (curr->next != list)
  {
    length++;
    if (opt_yield && LOOKUP_YIELD)
      pthread_yield();
    if (curr->next->prev != curr || curr->prev->next != curr)
      return -1;
    curr = curr->next;
  }

  return length;
}