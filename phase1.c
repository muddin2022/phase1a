#include <stdlib.h>
#include <stdio.h>
#include "phase1.h"
#include <string.h>

struct SporkTramData
{
    int (*func)(void *);
    void *arg;
} sporkTData;

struct PCB
{
    int pid;
    char *name[MAXNAME];
    int status;
    int priority;
    int stackSize;
    USLOSS_Context context;
    void (*funcPtr)(void);
    char *stack;

    struct PCB *parent;
    struct PCB *prevSibling;
    struct PCB *nextSibling;
};

/* --- Global variables --- */
struct PCB *currProc;
struct PCB procTable[MAXPROC];
char initStack[USLOSS_MIN_STACK];
char *initStackPtr;
int nextPid = 1;

/* --- Function prototypes --- */
void getNextPid(void);
unsigned int disableInterrupts(void);
void restoreInterrupts(unsigned int);
void init(void);
int testcaseMainWrapper(void *args);
void sporkTrampoline(void);

/* --- Functions from spec --- */
void phase1_init(void)
{
    unsigned int oldPsr = disableInterrupts();

    memset(procTable, 0, sizeof(procTable));

    struct PCB initProc;
    getNextPid();
    initProc.pid = nextPid;
    strcpy(initProc.name, "init");
    initProc.priority = 6;
    initProc.stackSize = USLOSS_MIN_STACK;
    initProc.funcPtr = &init;

    int index = initProc.pid % MAXPROC;
    procTable[index] = initProc;

    USLOSS_ContextInit(&procTable[index].context, initStackPtr, USLOSS_MIN_STACK, NULL, initProc.funcPtr);

    currProc = &initProc;
    restoreInterrupts(oldPsr);
}

void init(void)
{
    unsigned int oldPsr = disableInterrupts();

    // start services
    phase2_start_service_processes();
    phase3_start_service_processes();
    phase4_start_service_processes();
    phase5_start_service_processes();
    phase5_mmu_pageTable_alloc(currProc->pid);
    phase5_mmu_pageTable_free(currProc->pid, NULL);

    restoreInterrupts(oldPsr);

    // create testcase_main proc
    spork("testcaseMain", &testcaseMainWrapper, NULL, USLOSS_MIN_STACK, 3);

    // call join to clean up procTable
    int deadPid = 1;
    int status, index;
    while (deadPid > 0)
    {
        deadPid = join(&status);
        index = deadPid % MAXPROC;
        memset(&procTable[index], 0, sizeof(struct PCB));
    }

    if (deadPid == -2)
    {
        USLOSS_Console("ERROR: init has no more children, terminating simulation\n");
        USLOSS_Halt(1);
        return;
    }
    else if (deadPid == -3)
    {
        USLOSS_Console("ERROR: status pointer is null");
    }
}

void sporkTrampoline()
{
    sporkTData.func(sporkTData.arg);
}

/*
 * Create child proc of current proc
 * func takes a void arg and returns an int
 */
int spork(char *name, int (*func)(void *), void *arg, int stacksize, int priority)
{
    // disable interrupts for new process creation
    unsigned int oldPsr = disableInterrupts();

    getNextPid();
    int pid = nextPid;
    int slot = pid % MAXPROC;
    if (procTable[slot].pid != 0 || strlen(name) > MAXNAME || priority < 1 || priority > 5)
        return -1;
    if (stacksize < USLOSS_MIN_STACK)
        return -2;

    struct PCB *newProc = &procTable[slot];

    newProc->pid = pid;
    newProc->priority = priority;
    newProc->funcPtr = func(&arg);
    newProc->stack = malloc(stacksize);

    sporkTData.func = func;
    sporkTData.arg = arg;

    USLOSS_ContextInit(&newProc->context, newProc->stack, stacksize, NULL, &sporkTrampoline);

    // dont need to call dispatcher in phase1a?

    // reenable interrupts then call startFunc(func parameter)
    restoreInterrupts(oldPsr);
    func(&arg);

    return pid;
}

void TEMP_switchTo(int pid)
{
}

int join(int *status)
{
}

void quit_phase_1a(int status, int switchToPid)
{
}

void dumpProcesses(void)
{
}

/* --- Helper functions, not defined in spec --- */
/*
 * Checks the nextPid to see if it maps to a position in the procTable
 * that is alredy filled. It keeps checking for a blank spot in the procTable and
 * updates the global variable.
 */
void getNextPid(void)
{
    // check if nextPid already in use
    while (procTable[nextPid % MAXPROC].pid != 0)
    {
        nextPid++;
    }
}

/*
 * Disables interrupts and return the old PSR value, so that interrupts
 * can be restored in the future.
 */
unsigned int disableInterrupts(void)
{
    unsigned int oldPsr = USLOSS_PsrGet();
    return USLOSS_PsrSet(oldPsr & ~USLOSS_PSR_CURRENT_INT);
}

/*
 * Restores interrupts to the value they were before they got disabled.
 */
void restoreInterrupts(unsigned int oldPsr)
{
    USLOSS_PsrSet(oldPsr);
}

/*
 * Creates a wrapper around testcase_main so that spork can be called. Gets
 * called by init().
 */
int testcaseMainWrapper(void *args)
{
    return testcase_main();
}