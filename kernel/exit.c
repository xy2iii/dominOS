#include "task.h"
#include "errno.h"

static void unlock_interrupted_child_parent(struct task * parent)
{
    if (is_task_interrupted_child(parent))
        set_task_ready(parent);
}

int __exit_task(struct task * task_ptr, int retval)
{
    if (!task_ptr)
        return -ESRCH;

    if (is_idle(task_ptr))
        return -EINVAL;

    if (is_task_zombie(task_ptr))
        return -ESRCH;

    remove_from_global_list(task_ptr);
    set_task_return_value(task_ptr, retval);
    set_task_zombie(task_ptr);
    unlock_interrupted_child_parent(task_ptr->parent);
    return 0;
}

void __unexplicit_exit(void)
{
    int ret;
    __asm__("mov %%eax, %0" : "=r"(ret));
    __exit_task(current(), ret);
    schedule_no_ready();
}

void __explicit_exit(int retval)
{
    __exit_task(current(), retval);
    schedule_no_ready();
}

void __attribute__((noreturn)) exit(int retval)
{
    __explicit_exit(retval);
    for (;;);
}