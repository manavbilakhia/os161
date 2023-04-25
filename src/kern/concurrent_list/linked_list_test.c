#include <linked_list.h>
#include <lib.h>
#include <thread.h>
#include <spl.h>
#include <test.h>

static struct semaphore *test_sem;

static void linked_list_test_adder(void *list, unsigned long which)
{
  //splhigh();

  int i;
  int *c;

  for (i = 0; i < 10; i++) {
    c = kmalloc(sizeof(int));
    *c = 'A' + i;
    linked_list_prepend(list, c);
    //linked_list_printlist(list, which);
  }
  kprintf("final list from adder");
  linked_list_printlist(list, which);

  V(test_sem);
}

static void linked_list_test_insert(void *list, unsigned long which)
{
  //splhigh();
  int i;
  int *c;

  int key[] = {2,0,6,3,1,5,4}; //array of keys

  for (i = 0; i < 7; i++) 
  {
    c = kmalloc(sizeof(int));
    *c = 'A' + i;
    linked_list_insert(list, key[i], c);
    //linked_list_printlist(list, which);
  }
  kprintf("final list from insert");
  linked_list_printlist(list, which);
    V(test_sem);
}

static void linked_list_test_remove_head(void *list, unsigned long which)
{
  //splhigh();
  int key;
  linked_list_remove_head(list, &key);
  //linked_list_printlist(list, which);

  kprintf("final list from remove head");
  linked_list_printlist(list, which);
    V(test_sem);
}

int linked_list_test_run(int nargs, char **args)
{ 
  test_sem = sem_create("test_sem", 0);
  if (nargs == 2) {
    testnum = args[1][0] - '0'; // XXX - Hack - only works for testnum 0 -- 9
  }

  kprintf("testnum: %d\n", testnum);

  Linked_List * list = linked_list_create();

  thread_fork("adder 1",
	      NULL,
	      linked_list_test_adder,
	      list,
	      1);

  thread_fork("adder 2",
	      NULL,
	      linked_list_test_adder,
	      list,
	      2);
  
  Linked_List *list2 = linked_list_create();

  thread_fork("insert1",
        NULL,
        linked_list_test_insert,
        list2,
        3);
  
  Linked_List *list3 = linked_list_create();

  thread_fork("remove1",
        NULL,
        linked_list_test_remove_head,
        list2,
        4);
 
  thread_fork("remove2",
        NULL,
        linked_list_test_remove_head,
        list3,
        5);

  // XXX - Bug - We're returning from this function without waiting
  // for these two threads to finish.  The execution of these
  // threads may interleave with the kernel's main menu thread and
  // cause interleaving of console output.  We going to accept this
  // problem for the moment until we learn how to fix in Project 2.
  // An enterprising student might investigate why this is not a
  // problem with other tests suites the kernel uses.
  for (int i = 0; i<5; i++)
  {
    P(test_sem);
  }
  sem_destroy(test_sem);
  return 0;
}
