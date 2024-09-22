#include <stdio.h>
#include "phase1.h"
#include <string.h>

struct PCB {
    int pid;
    int status;
    int priority;
    int stackSize;
    USLOSS_Context context;
    void (*funcPtr) (void);
    char *stack;
}

/* --- Global variables --- */
struct PCB *currProc;
struct PCB procTable[MAXPROC];             
char initStack[USLOSS_MIN_STACK];
char *initStackPtr; 
int nextPid = 1; 

/* --- Function prototypes --- */
int getNextPid(void);
unsigned int disableInterrupts(void);
void restoreInterrupts(unsigned int);
void init(void);

/* --- Functions from spec --- */
void phase1_init(void) {
    unsigned int oldPsr = disableInterrupts();

    // Initialize data structures
    phase2_start_service_processes();
    phase3_start_service_processes();
    phase4_start_service_processes();
    phase5_start_service_processes();
    phase5_mmu_pageTable_alloc(currProc->pid);
    phase5_mmu_pageTable_free (currProc->pid, NULL);

    memset(procTable, 0, sizeof(procTable));
    struct PCB initProc;
    initProc.pid = getNextPid();
    initProc.priority = 6;
    initProc.funcPtr = &init;

    //USLOSS_ContextInit();

    restoreInterrupts(oldPsr); 
    
    //USLOSS_ContextSwitch();   
}

void init(void) {

}

/* --- Helper functions, not defined in spec --- */ 
/*
 * Checks the nextPid to see if it maps to a position in the procTable 
 * that is alredy filled. It keeps checking for a blank spot in the procTable and
 * updates the global variable.
 */
int getNextPid(void) {
    // check if nextPid already in use
    while (procTable[nextPid % MAXPROC].pid != 0) {
        nextPid++;
    }
}

/*
 * Disables interrupts and return the old PSR value, so that interrupts
 * can be restored in the future.
 */
unsigned int disableInterrupts(void) {
    unsigned int oldPsr = USLOSS_PsrGet();
    USLOSS_PsrSet(oldPsr & ~USLOSS_PSR_CURRENT_INT);
    return oldPsr;
}

/*
 * Restores interrupts to the value they were before they got disabled.
 */
void restoreInterrupts(unsigned int oldPsr) {
    USLOSS_PsrSet(oldPsr);
}

