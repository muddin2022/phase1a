#include <stdio.h>
#include "phase1.h" // will this error go away when running in docker ?

struct PCB {
    int pid;
    USLOSS_Context *context;
    void *stack;
    int stackSize;
}

void phase1_init(void) {
    // Initialize data structures
    phase2_start_service_processes();
    phase3_start_service_processes();
    phase4_start_service_processes();
    phase5_start_service_processes();
    phase5_mmu_pageTable_alloc(int pid);
    phase5_mmu_pageTable_free (int pid, USLOSS_PTE*);

    // Create process table entry for init
    USLOSS_ContextInit();

    // Create testcase_main process
    // Make a wrapper function around the function pointer to detect 
    // if the function returns
    testcase_main();

    // Context switch to testcase_main process
    USLOSS_ContextSwitch();
}

