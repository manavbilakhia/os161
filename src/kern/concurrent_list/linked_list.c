#include <linked_list.h>
#include <lib.h>
#include <thread.h>

Linked_List *linked_list_create(void)
{
  Linked_List * ptr = kmalloc(sizeof(Linked_List));
  ptr -> length = 0;
  ptr -> first = NULL;
  ptr -> last = NULL;

  return ptr;
}

Linked_List_Node *linked_list_create_node(int key, void *data)
{
  Linked_List_Node *newnode = kmalloc(sizeof(Linked_List_Node));
  newnode -> prev = NULL;
  newnode -> next = NULL;
  newnode -> key = key;
  newnode -> data = data;

  return newnode;
}

void linked_list_prepend(Linked_List *list, void *data)
{
  Linked_List_Node * newnode;
  Linked_List_Node * f = list -> first;

  if (list -> first == NULL) {
    newnode = linked_list_create_node(0, data);
    list -> first = newnode;
    list -> last = newnode;
  } else {
    newnode = linked_list_create_node(f -> key - 1, data);

    if (testnum == 1)
    {
      thread_yield();
    }

    newnode -> next = list -> first;
    f -> prev = newnode;
    list -> first = newnode;
  }

  list -> length ++;

}

void linked_list_printlist(Linked_List *list, int which)
{
  Linked_List_Node *runner = list -> first;

  kprintf("%d: ", which);

  while (runner != NULL) {
    kprintf("%d[%c] ", runner -> key, *((int *)runner -> data));
    runner = runner -> next;
  }

  kprintf("\n");

}
void linked_list_insert(Linked_List *list, int key, void *data)
{
  Linked_List_Node *newnode = linked_list_create_node(key,data);
  Linked_List_Node *runner;

  if (list->first == NULL)
  {
    list->first = newnode;
    list->last = newnode;
  }
  else
  {
    runner = list->first;
    while (runner != NULL && runner -> key < key)
    {
      runner = runner->next;
      if (testnum == 2)
      {
        thread_yield();
      }
    }
    if (runner == NULL) // inserting it at the end
    {
      newnode->prev = list->last;
      list ->last->next = newnode;
      list->last = newnode;
    }
    else // inserts before the runner
    {
      newnode -> prev = runner->prev;
      newnode ->next = runner;

      if (runner->prev ==NULL)
      {
        list->first = newnode;

      }
      else
      {
        runner->prev ->next = newnode;
      }
      runner -> prev = newnode;
      if (testnum == 3)
      {
        thread_yield();
      }
    }
  }
    list->length++;
}
void *linked_list_remove_head(Linked_List *list, int *key)
{
  if (list->length == 0)
  {
    return NULL;
  }
 if(list->first == NULL)
 {
  *key = -1;
  return NULL;
 } 
 Linked_List_Node *oldhead = list->first;
 void *data = oldhead->data;
 *key = oldhead->key;

 if (list ->first == list->last) //only one node in the list
 {
  list->first = NULL;
  list->last = NULL;
 }
 else
 {
  list->first = oldhead->next;
  list->first->prev = NULL;

  if (testnum == 4)
  {
    thread_yield();
  }

 }


 kfree (oldhead); //frees the previously allocated memory; takes in the pointer returned by kmalloc


 list->length--;

 return data; 
}
