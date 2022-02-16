#include <cpu.h>
#include <clock.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"
#include "task.h"
#include "../shared/debug.h"
#include "cpu.h"
#include "../shared/string.h"
#include "swtch.h"
#include "queue.h"
#include "mem.h"



/**************
* READY TASKS *
***************/

struct list_link tasks_ready_queue = LIST_HEAD_INIT(tasks_ready_queue);

void set_task_ready(struct task * task_ptr)
{
    task_ptr->state = TASK_READY;
    queue_add(task_ptr, &tasks_ready_queue, struct task, tasks, priority);
}



/**************
* DYING TASKS *
***************/

struct list_link tasks_dying_queue = LIST_HEAD_INIT(tasks_dying_queue);



/*****************
* SLEEPING TASKS *
******************/ 

struct list_link tasks_sleeping_queue = LIST_HEAD_INIT(tasks_sleeping_queue);

void set_task_sleeping(struct task * task_ptr)
{
    task_ptr->state = TASK_SLEEPING;
    queue_add(task_ptr, &tasks_ready_queue, struct task, tasks, wake_time);
}

void try_wakeup_tasks(void)
{
    struct task * task_cur;
    struct task * task_tmp;

    queue_for_each_safe (task_cur, task_tmp, &tasks_sleeping_queue, struct task, tasks) {
        if (task_cur->wake_time <= current_clock()) {
            task_cur->wake_time = 0;
            queue_del(task_cur, tasks);
            set_task_ready(task_cur);
        }
    }
}

void wait_clock(unsigned long clock)
{
    current()->wake_time = current_clock() + clock;
    set_task_sleeping(current());
    schedule();
}



/***************
* RUNNING TASK *
****************/

static struct task *__running_task = NULL; // Currently running task. Can be NULL in interrupt context or on startup.

struct task * current(void)
{
    return __running_task;
}

static void set_task_running(struct task * task_ptr)
{
    task_ptr->state = TASK_RUNNING;
    __running_task = task_ptr;
}



/*************
* SCHEDULING *
**************/

static bool __preempt_enabled = false;

void preempt_enable(void)
{
    __preempt_enabled = true;
}

void preempt_disable(void)
{
    __preempt_enabled = false;
}

bool is_preempt_enabled(void)
{
    return __preempt_enabled;
}

void switch_task(struct task * new, struct task * old)
{
    set_task_running(new);
    swtch(&old->context, new->context);
}

void schedule()
{
    try_wakeup_tasks();

    struct task * old_task = current();
    struct task * new_task = queue_out(&tasks_ready_queue, struct task, tasks);

    if (new_task != NULL /* MIGHT BE CHANGED */ && new_task != old_task) {
        set_task_ready(old_task);
        switch_task(new_task, old_task);
    } else {
        // Keeps running old_task
    }
}



/**********************
 * Process management *
 **********************/

static struct task * alloc_empty_task()
{
    struct task *task_ptr = mem_alloc(sizeof(struct task));
    task_ptr->kstack = mem_alloc(KERNEL_STACK_SIZE * sizeof(uint32_t));
    return task_ptr;
}

static void set_task_startup_context(struct task * task_ptr, int (*func_ptr)(void*), void * arg)
{
    task_ptr->kstack[KERNEL_STACK_SIZE - 2] = (uint32_t)func_ptr;
    task_ptr->kstack[KERNEL_STACK_SIZE - 1] = (uint32_t) arg;
    task_ptr->context = (struct cpu_context *)&task_ptr->kstack[KERNEL_STACK_SIZE - 6];
}

static void set_task_name(struct task * task_ptr, const char * name)
{
    strncpy(task_ptr->comm, name, COMM_LEN);
}

static void set_task_priority(struct task * task_ptr, int priority)
{
    if (priority < MIN_PRIO || priority > MAX_PRIO)
        panic("Cannot create a kernel task with priority: %d", priority);
    task_ptr->priority = priority;
}

static void create_kernel_task(int (*func_ptr)(void*), unsigned long ssize __attribute__((unused)), int prio, const char *name, void *arg)
{
    struct task * task_ptr = alloc_empty_task();
    task_ptr->pid = alloc_pid();
    set_task_name(task_ptr, name);
    set_task_startup_context(task_ptr, func_ptr, arg);
    set_task_priority(task_ptr, prio);
    set_task_ready(task_ptr);
}


int start(int (*func_ptr)(void*), unsigned long ssize, int prio, const char *name, void *arg)
{
    create_kernel_task(func_ptr, ssize, prio, name, arg);
    return 0;
}



/*************
 * IDLE task *
 *************/
static int __attribute__((noreturn)) __idle(void * arg __attribute__((unused))) {
    for (;;) {
        sti();
        hlt();
        cli();
    }
}

void create_idle_task(void)
{
    struct task * idle_ptr = alloc_empty_task();
    idle_ptr->pid = alloc_pid();
    set_task_name(idle_ptr, "idle");
    set_task_startup_context(idle_ptr, __idle, NULL);
    set_task_priority(idle_ptr, MIN_PRIO);
    set_task_running(idle_ptr);
}