#include <types.h>
#include <lib.h>
#include <proc_table.h>
#include <test.h>
#include <proc.h>


int proc_table_test_run(int nargs, char **args){
    (void) nargs;
    (void) args;
    kprintf("Num of active procs at first should be 1. -actual: %d\n", global_proc_table -> active_procs);
    int d = get_available_pid(global_proc_table);
    kprintf("First available pid should be 3. -actual: %d\n", d);

    struct proc *dummy_proc = proc_create("dummy proc");
    d = dummy_proc -> process_id;
    kprintf("Dummy proc should have pid of 3. -actual: %d\n", d);

    d = get_proc(3, global_proc_table) -> process_id;
    kprintf("Dummy proc should have pid of 3 (uses getter). -actual: %d\n", d);

    struct proc *dummy_proc_two = proc_create("dummy proc two");
    d = dummy_proc_two -> process_id;
    kprintf("Dumy proc two should have pid of 4. -actual: %d\n", d);

    d = get_proc(4, global_proc_table) -> process_id;
    kprintf("Dummy proc should have pid of 4. (uses getter). -actual: %d\n", d);

    remove_process(global_proc_table, 3);
    d = get_available_pid(global_proc_table);
    kprintf("Getting an available pid when there are processes in slots after the first available pid (should be 3). -actual: %d\n", d);
    return 0;
}
