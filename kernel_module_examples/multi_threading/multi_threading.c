// Multi-threading example using completions

#include <linux/completion.h>
 
#include <linux/err.h> // for IS_ERR()
#include <linux/init.h>  
#include <linux/kthread.h>
 
#include <linux/module.h>  
#include <linux/printk.h>  
#include <linux/spinlock.h>
#include <linux/version.h>  

 
static struct completion crank_comp;  
static struct completion flywheel_comp;  

static int machine_counter;

static DEFINE_SPINLOCK(machine_lock);
 
static int machine_crank_thread(void *arg)  
{  
    pr_info("Turn the crank\n");  

    spin_lock(&machine_lock);
    machine_counter++;
    spin_unlock(&machine_lock);


    complete_all(&crank_comp);  
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
 
    kthread_complete_and_exit(&crank_comp, 0);  
#else  
    complete_and_exit(&crank_comp, 0);
 
#endif  
}  
  
static int machine_flywheel_spinup_thread(void *arg)  
{
 
    // completion is basically just a counter, and 
    // wakes once for completion
    // It does not provide exactly the same semantics 
    // as conditional variables as it does not take 
    // mutex to repeatedly check a shared variable
    // pthread_mutex_lock(&m) like as follows:
    // while (!ready)
    //     pthread_cond_wait(&cv, &m);
    // ...
    // pthread_mutex_unlock(&m);

    wait_for_completion(&crank_comp);  
  
    spin_lock(&machine_lock);
    if (machine_counter == 1)
        machine_counter++;
    else
        pr_warn("machine_counter expected 1, got %d\n", machine_counter);
    spin_unlock(&machine_lock);

    pr_info("Flywheel spins up\n");  

 
    complete_all(&flywheel_comp);  
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
 
    kthread_complete_and_exit(&flywheel_comp, 0);  
#else  
    complete_and_exit(&flywheel_comp, 0);
 
#endif  
}  
  
static int __init completions_init(void)  
{  
    struct task_struct *crank_thread;
 
    struct task_struct *flywheel_thread;  
  
    pr_info("completions example\n");
 
  
    init_completion(&crank_comp);  
    init_completion(&flywheel_comp);  
    spin_lock_init(&machine_lock);
    machine_counter = 0;

 
    // kthread_create just creates
    // it puts the thread into a TASK_UNINTERRUPTIBLE state
    // and you need to wake it up with wake_up_process
    // kthread_bind to a specific CPU
    // kthread_run creates and runs a thread
    crank_thread = kthread_create(machine_crank_thread, NULL, "KThread Crank");
 
    if (IS_ERR(crank_thread))  
        goto ERROR_THREAD_1;  

 
    flywheel_thread = kthread_create(machine_flywheel_spinup_thread, NULL,
 
                                     "KThread Flywheel");  
    if (IS_ERR(flywheel_thread))
 
        goto ERROR_THREAD_2;  
  
    wake_up_process(flywheel_thread);
 
    wake_up_process(crank_thread);  
  
    return 0;  
  
ERROR_THREAD_2:  
    kthread_stop(crank_thread);
 
ERROR_THREAD_1:  
  
    return -1;  
}  
  
static void __exit completions_exit(void)  
{
 
    wait_for_completion(&crank_comp);  
    wait_for_completion(&flywheel_comp);  

 
    pr_info("completions exit\n");  
}  
  
module_init(completions_init);
module_exit(completions_exit);  
  
MODULE_LICENSE("GPL");
MODULE_AUTHOR("EPFL CS477");
MODULE_DESCRIPTION("A simple example that ueses completions for multi-threading");
MODULE_VERSION("1.0");
