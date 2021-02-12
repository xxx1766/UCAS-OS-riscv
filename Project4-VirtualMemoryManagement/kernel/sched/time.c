#include <os/time.h>
#include <os/mm.h>
#include <os/irq.h>
#include <type.h>

LIST_HEAD(sleep_queue);

pcb_t *gethead_timer(list_head* queue);

uint64_t time_elapsed = 0;
uint32_t time_base = 0;

void timer_create(TimerCallback func, void* parameter, uint64_t tick)
{
    disable_preempt();
    current_running = (get_current_cpu_id() == 0) ? &current_running_core0 : &current_running_core1;
    ((pcb_t *)(*current_running))->timer.timeout_tick = get_ticks() + tick;  
    ((pcb_t *)(*current_running))->timer.callback_func = func;
    ((pcb_t *)(*current_running))->timer.parameter = parameter;
    enqueue_timer(&sleep_queue, *current_running);
    enable_preempt();
}

void enqueue_timer(list_head* queue, pcb_t* item)
{
     list_add_tail(&item->list, queue);
}

pcb_t *gethead_timer(list_head* queue)
{
    pcb_t *tmp = list_entry(queue, pcb_t, list);
    return tmp;
}

void timer_check()
{
    disable_preempt();
    pcb_t *tmp;
    uint8_t id = get_current_cpu_id();
    list_node_t *ptr = sleep_queue.next, *q;
    while(ptr != &sleep_queue){
        tmp = list_entry(ptr, pcb_t, list);
        q = ptr->next;
        if((tmp->sleep_add_core == id) && (get_ticks() >= tmp->timer.timeout_tick)){
            do_unblock(ptr);
          }
        ptr = q;
     } 
    enable_preempt();
} 

uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

uint64_t get_timer()
{
    return get_ticks() / time_base;
}

uint64_t get_time_base()
{
    return time_base;
}

void latency(uint64_t time)
{
    uint64_t begin_time = get_timer();

    while (get_timer() - begin_time < time);
    return;
}
