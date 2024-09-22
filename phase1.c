#include <stdio.h>
#include "phase1.h"
#include <string.h>

struct PCB {
    int pid;
    char *name[MAXNAME];
    int status;
    int priority;
    int stackSize;
    USLOSS_Context context;
    void (*funcPtr) (void);
    char *stack;
}pcb;

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

    memset(procTable, 0, sizeof(procTable));

    PCB initProc;
    initProc.pid = getNextPid();
    initProc.name = "init";
    initProc.priority = 6;
    initProc.stackSize = USLOSS_MIN_STACK;
    initProc.funcPtr = &init;

    int index = initProc.pid % MAXPROC;
    procTable[index] = initProc;

    USLOSS_ContextInit(&procTable[index].context, initStackPtr, USLOSS_MIN_STACK, NULL, initProc.funcPtr);

    currProc = &initProc;
    restoreInterrupts(oldPsr);   
}

void init(void) {
    unsigned int oldPsr = disableInterrupts();

    // start services
    phase2_start_service_processes();
    phase3_start_service_processes();
    phase4_start_service_processes();
    phase5_start_service_processes();
    phase5_mmu_pageTable_alloc(currProc->pid);
    phase5_mmu_pageTable_free (currProc->pid, NULL);

    restoreInterrupts(oldPsr);

    // create testcase_main proc
    spork("testcaseMain", &testcase_main, NULL, USLOSS_MIN_STACK, 3);

    // call join to clean up procTable
    int joinRetVal = 1;
    int status;
    while (joinRetVal > 0) {
        join(&status);
    }

    if (joinRetVal == -2) {
        USLOSS_Console("ERROR: init has no more children, terminating simulation\n");
        return; 

    } else if (joinRetVal == -3) {
        USLOSS_Console("ERROR: status pointer is null");
    }
}


/* --- Helper functions, not defined in spec --- */ 
/*
 * Checks the nextPid to see if it maps to a position in the procTable 
 * that is alredy filled. It keeps checking for a blank spot in the procTable and
 * updates the global variable.
 */
int getNextPid(void) {
    // check if nextPid already in use
    while (procTable[nextPid % MAXPROC]->pid != 0) {
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

