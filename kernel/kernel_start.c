#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "clock.h"
#include "start.h"
#include "task.h"
#include "shm.h"
#include "ktests.h"
#include "pf_allocator.h"

#define START_TEST(n) do {\
    printf("Starting test: " #n ".\n"); \
    start(test##n, 512, 128, "test" #n, NULL); \
    printf("Test " #n " successfull.\n"); \
} while(0)

//void test_CR3(void);
void kernel_start(void)
{
    //test_CR3();
    printf("\f");
    printf("Hello world!!!\n");
    preempt_disable();
    init_clock();
    shm_init();
    start_idle();
    sti();
    preempt_enable();
    

    START_TEST(1);
    START_TEST(2);
    START_TEST(3);
    START_TEST(5);

    //start(test6, 512, 128, "", NULL);

    while(1)
        hlt();
}