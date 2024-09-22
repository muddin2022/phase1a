#include <stdio.h>
#include "phase1.h" 

struct PCB {
    int pid;
    int status;
    int priority;
    int stackSize;
    USLOSS_Context context;
    void stack[USLOSS_MIN_STACK]; // not sure if this is right...
    void (*funcPtr) (void);
}

/* --- Global variables --- */
PCB *currProc; // Current process pointer
PCB procTable[MAXPROC];// Process table
// Init stack
// Init stack pointer ?
int nextPid = 1; // next pid to try

/* --- Function prototypes --- */
int getNextPid(void);
unsigned int disableInterrupts(void);
void restoreInterrupts(unsigned int);

/* --- Functions from spec --- */
void phase1_init(void) {
    unsigned int oldPsr = disableInterrupts();

    // Initialize data structures
    phase2_start_service_processes();
    phase3_start_service_processes();
    phase4_start_service_processes();
    phase5_start_service_processes();
    phase5_mmu_pageTable_alloc(int pid);
    phase5_mmu_pageTable_free (int pid, USLOSS_PTE*);

    memset(procTable, 0, sizeof(procTable));
    PCB initProc;
    initProc.pid = getNextPid();
    initProc.priority = 6;

    USLOSS_ContextInit();

    restoreInterrupts(oldPsr); 
    
    USLOSS_ContextSwitch();   
}

void init() {
    
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

