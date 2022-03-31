#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "clock.h"
#include "start.h"
#include "task.h"
#include "shm.h"
#include "ktests.h"

#define START_TEST(n) do {\
    printf("Starting test: " #n ".\n"); \
    start(test##n, 512, 128, "test" #n, NULL); \
    printf("Test " #n " successfull.\n"); \
} while(0)

int test_page_fault(void *arg __attribute__((unused)))
{
    unsigned int * p = (unsigned int *)0xdeadbeef;
    *p = 0xcafebabe;
    printf("THIS MESSAGE SHOULDN'T BE PRINTED!!!\n");
    return 0;
}

void kernel_start(void)
{
    printf("\f");

    preempt_disable();
    init_clock();
    init_page_fault_handler();
    shm_init();
    start_idle();
    sti();
    preempt_enable();
    

    start(test_page_fault, 512, 128, "page_fault", NULL);
    printf("Hello world!\n");

    while(1)
        hlt();
}