#include "list.h"
#include "thread.h"
#include "memory.h"
#include "string.h"
#include "stdint.h"
#include "interrupt.h"
#include "debug.h"
#include "print.h"


#define PG_SIZE 4096

struct pcb_struct* main_thread;		//main thread pcb pointer

struct list thread_ready_list;
struct list thread_all_list;
static struct list_elem* thread_tag;


// the setting of param is corresponding to stack contex prepared by thread_create, after
// ret it will call real thread function and pass arguments
static void kernel_thread(thread_func* function, void* func_arg){
	// when first called, control flow goes to handler function by ret, so won't execute iret
	// need manually turn on interrupt
	intr_enable();
	function(func_arg);
}


// write pcb_struct, set self_kstack to the top of PCB page
void init_thread(struct pcb_struct* thread_pcb_ptr, char* name, int prio){
	memset(thread_pcb_ptr, 0, sizeof(*thread_pcb_ptr));
	thread_pcb_ptr->self_kstack = (uint32_t*)((uint32_t)thread_pcb_ptr + PG_SIZE);
	// thread_pcb_ptr->status = TASK_RUNNING;
	if(thread_pcb_ptr == main_thread){
		thread_pcb_ptr->status = TASK_RUNNING;
	}else{
		thread_pcb_ptr->status = TASK_READY;
	}
	thread_pcb_ptr->priority = prio;
	strcpy(thread_pcb_ptr->name, name);
	thread_pcb_ptr->ticks = prio;
	thread_pcb_ptr->ticks_total = 0;
	thread_pcb_ptr->pgdir = NULL;
	thread_pcb_ptr->stack_magic = PCB_STACK_MAGIC;
}


// reserve intr_stack and write thread_stack, prepare for calling thread function by ret
void thread_create(struct pcb_struct* thread_pcb_ptr, thread_func function, void* arg){
	// thread_pcb_ptr->self_kstack -= sizeof(struct intr_stack);
	thread_pcb_ptr->self_kstack -= sizeof(struct thread_stack);

	struct thread_stack* thread_stack_ptr = (struct thread_stack*) thread_pcb_ptr->self_kstack;
	thread_stack_ptr->ebp = thread_stack_ptr->ebx = thread_stack_ptr->esi = thread_stack_ptr->edi = 0;
	thread_stack_ptr->eip = kernel_thread;
	// following is the new stack for kernel_thread, which calls thread function
	// thread_stack_ptr->dummy_ret_addr = 0;
	thread_stack_ptr->function = function;
	thread_stack_ptr->func_arg = arg;
}


struct pcb_struct* thread_start(char* name, int prio, thread_func function_ptr, void* arg){
	// allocate one physical page for PCB
	struct pcb_struct* thread_pcb_ptr = apply_kernel_pages(1);
	init_thread(thread_pcb_ptr, name, prio);
	thread_create(thread_pcb_ptr, function_ptr, arg);

	ASSERT(!elem_find(&thread_ready_list, &thread_pcb_ptr->general_tag));
	list_append(&thread_ready_list, &thread_pcb_ptr->general_tag);

	ASSERT(!elem_find(&thread_all_list, &thread_pcb_ptr->all_list_tag));
	list_append(&thread_all_list, &thread_pcb_ptr->all_list_tag);
	// asm volatile ("movl %0, %%esp; \
	// 			   pop %%ebp;	\
	// 			   pop %%ebx; \
	// 			   pop %%edi; \
	// 			   pop %%esi; \
	// 			   ret"::"g"(thread_pcb_ptr->self_kstack):"memory");
	return thread_pcb_ptr;
}


// get high 22 bit of esp as pointer, which points to the start of current physical page, the start
// address of current PCB too
struct pcb_struct* get_pcb_ptr(){
	uint32_t esp;
	asm volatile ("mov %%esp, %0":"=g"(esp));
	return (struct pcb_struct*)(esp & 0xfffff000);
}


static void make_main_thread(){
	main_thread = get_pcb_ptr();
	init_thread(main_thread, "main", 31);

	ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
	list_append(&thread_all_list, &main_thread->all_list_tag);
}


// the very connection between thread list(the data structure) and what's going on in real world
struct pcb_struct* elem2pcb(struct list_elem* tag){
	return (struct pcb_struct*)((uint32_t)tag & 0xfffff000);
}


// call by clock interrupt handler
void schedule(){
	ASSERT(intr_get_status() == INTR_OFF);		// should be turned off auto by CPU when enter an intr
	struct pcb_struct* current_thread = get_pcb_ptr();
	if(current_thread->status == TASK_RUNNING){
		// thread CPU time use up, add to ready list
		ASSERT(!elem_find(&thread_ready_list, &current_thread->general_tag));
		list_append(&thread_ready_list, &current_thread->general_tag);
		current_thread->ticks = current_thread->priority;
		current_thread->status = TASK_READY;
	}else{
		// block
	}

	ASSERT(!list_empty(&thread_ready_list));
	thread_tag = NULL;
	thread_tag = list_pop(&thread_ready_list);
	struct pcb_struct* next_thread = elem2pcb(thread_tag);
	next_thread->status = TASK_RUNNING;
	switch_to(current_thread, next_thread);
}


// set current thread to status(block/wait/hanging), then call schedule
void thread_block(enum task_status status){
	ASSERT((status==TASK_BLOCKED) || (status==TASK_WAITING) || (status==TASK_HANGING));
	enum intr_status intr_status = intr_disable();
	struct pcb_struct* current_thread = get_pcb_ptr();
	current_thread->status = status;
	schedule();		// goes to another PCB, return from another schedule call when being scheduled
	intr_set_status(intr_status);
}


// add thread tag to ready_list, set the thread status to TASK_READY
void thread_unblock(struct pcb_struct* thread_pcb_ptr){
	enum intr_status intr_status = intr_disable();
	ASSERT((thread_pcb_ptr->status==TASK_BLOCKED) || (thread_pcb_ptr->status==TASK_WAITING) || (thread_pcb_ptr->status==TASK_HANGING));
	ASSERT(thread_pcb_ptr->status != TASK_READY);
	ASSERT(!elem_find(&thread_ready_list, &thread_pcb_ptr->general_tag));
	list_push(&thread_ready_list, &thread_pcb_ptr->general_tag);
	thread_pcb_ptr->status = TASK_READY;
	intr_set_status(intr_status);
}


// init thread enviroment
void thread_enviroment_init(){
	list_init(&thread_ready_list);
	list_init(&thread_all_list);
	make_main_thread();
	put_string("thread enviroment init done\n");

}
