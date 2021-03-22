#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "stdint.h"
#include "list.h"

typedef void thread_func(void*);

#define PCB_STACK_MAGIC 0x20000125

// status of process or thread
enum task_status{
	TASK_RUNNING,
	TASK_READY,
	TASK_BLOCKED,
	TASK_WAITING,
	TASK_HANGING,
	TASK_DIED
};


// protect context when interrupt occur
struct intr_stack{
	// saved by interrupt entry function(interrupt.S)
	uint32_t intr_vec;     		
	uint32_t edi;		// pushad
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp_dummy;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;		// end pushad
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;

	// saved by CPU automatically when interrupt occur
	uint32_t err_code;
	void (*eip) (void);
	uint32_t cs;
	uint32_t eflags;
	void* esp;
	uint32_t ss;
};


struct thread_stack{
	uint32_t ebp;
	uint32_t ebx;
	uint32_t edi;
	uint32_t esi;
	void (*eip)(thread_func* function, void* arg);

	// key to implement thread machanism, arrange stack in a skillfull way(liken pwn), allow
	// control flow goes to handler function by ret(from switch_to) when first time being scheduled,
	// then control flow ret to schedule from switch_to normally(in all later schedule), and restore 
	// context as each thread_stack in every PCB is in same form
	void (*dummy_ret_addr);
	thread_func* function;
	void* func_arg;
};


struct pcb_struct{
	uint32_t* self_kstack;
	enum task_status status;
	uint8_t priority;
	char name[16];
	uint8_t ticks;					// CPU ticks to run every time this thread is active
	uint32_t ticks_total;			// CPU ticks total this thread have run
	struct list_elem general_tag;	// tag in general list(thread_ready_list)
	struct list_elem all_list_tag;	// tag in thread_all_list
	uint32_t* pgdir;				// NULL for thread

	uint32_t stack_magic;
};

struct pcb_struct* thread_start(char* name, int prio, thread_func function_ptr, void* arg);
struct pcb_struct* get_pcb_ptr();
void thread_enviroment_init();
void switch_to(struct pcb_struct*, struct pcb_struct*);
void schedule();
void thread_block(enum task_status status);
void thread_unblock(struct pcb_struct* thread_pcb_ptr);
struct pcb_struct* elem2pcb(struct list_elem* tag);
#endif