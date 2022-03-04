#include "debugger.h"
#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "cga.h"
#include "mem.h"
#include "clock.h"
#include "task.h"
#include "test-kernel/test0.h"
#include "test-kernel/test1.h"
#include "test-kernel/test2.h"
#include "test-kernel/test3.h"
#include "test-kernel/test4.h"
#include "test-kernel/test5.h"
#include "test-kernel/test6.h"
#include "test-kernel/test7.h"
#include "test-kernel/test8.h"
#include "test-kernel/test9.h"
#include "test-kernel/test_start_with_args.h"

int proc1(void *arg __attribute__((unused)))
{
    printf("proc1: pid %d, prio %d\n", getpid(), getprio(getpid()));
    assert(start(proc1, 512, MAX_PRIO, "proc1", NULL) == 0);
    printf("%d kill()ing itself\n", getpid());
    assert(kill(123) != 0);
    exit(2);
    assert(kill(2) == 0);
    return 0;
}

int sleep_proc()
{
    wait_clock(2 * CLOCK_FREQ);
    return 0;
}

int proc2(void *arg __attribute__((unused)))
{
    for (;;) {
	printf("Proc2: Creation of a task\n");
	assert(start(sleep_proc, 512, MAX_PRIO, "sleep_proc", NULL) == 0);
	printf("Proc2: Wait until the end of sleep_proc\n");
	waitpid(-1, NULL);
	printf("Proc2: sleep_proc is finished\n");
    }
}

//void kernel_start(void)
//{
//    preempt_disable();
//    printf("\f");
//
//    // Call interrupt handler builders
//    init_clock();
//    sti();
//
//    create_idle_task();
//
//    start(proc1, 512, MAX_PRIO, "proc1", NULL);
//    start(proc2, 512, MIN_PRIO, "proc2", NULL);
//
//    preempt_enable();
//
//    while (1)
//	hlt();
//}

// Kernel start for tests
void kernel_start(void)
{
    preempt_disable();
    printf("\f");

    // Call interrupt handler builders
    init_clock();
    sti();

    create_idle_task();

    struct point *p = (struct point *) mem_alloc(sizeof(struct point));
    p->x = 1;
    p->y = 1;

    start(test_start_with_args_main, 512, MAX_PRIO, "args", (void *) p);

    preempt_enable();

    while (1)
	hlt();
}

// void kernel_start(void){
//     preempt_disable();
//     printf("\f");

//     // Call interrupt handler builders
//     init_clock();
//     sti();

//     create_idle_task();

//     test0_main();

//     preempt_enable();

//     while (1)
// 	hlt();
// }
