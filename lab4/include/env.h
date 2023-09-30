#ifndef _ENV_H_
#define _ENV_H_

#include <mmu.h>
#include <queue.h>
#include <trap.h>
#include <types.h>

#define LOG2NENV 10
#define NENV (1 << LOG2NENV)
#define ENVX(envid) ((envid) & (NENV - 1))

// Values of env_status in struct Env
#define ENV_FREE 0
#define ENV_RUNNABLE 1
#define ENV_NOT_RUNNABLE 2

struct Env {
	struct Trapframe env_tf;  // Saved registers 进程上下文环境
	LIST_ENTRY(Env) env_link; // Free list 构造空闲链表
	u_int env_id;		  // Unique environment identifier 标识符
	u_int env_asid;		  // ASID 
	u_int env_parent_id;	  // env_id of this env's parent 父进程id
	u_int env_status;	  // Status of the environment 表示状态 free、not runnable、runnable
	Pde *env_pgdir;		  // Kernel virtual address of page dir 页目录的内核虚拟地址
	TAILQ_ENTRY(Env) env_sched_link;	//构造调度队列 env_sched_list（TALIQ开头的宏，双端队列）
	u_int env_pri;			//进程优先级
	// Lab 4 IPC
	u_int env_ipc_value;   // data value sent to us 进程传递的具体数值
	u_int env_ipc_from;    // envid of the sender  发送方的进程 ID
	u_int env_ipc_recving; // env is blocked receiving 1：等待接受数据中；0：不可接受数据
	u_int env_ipc_dstva;   // va at which to map received page 接收到的页面需要与自身的哪个虚拟页面完成映射
	u_int env_ipc_perm;    // perm of page mapping received 传递的页面的权限位设置

	// Lab 4 fault handling
	u_int env_user_tlb_mod_entry; // user tlb mod handler

	// Lab 6 scheduler counts
	u_int env_runs; // number of times been env_run'ed
	int  env_ov_cnt;
};

LIST_HEAD(Env_list, Env);
TAILQ_HEAD(Env_sched_list, Env);
extern struct Env *curenv;		     // the current env
extern struct Env_sched_list env_sched_list; // runnable env list

void env_init(void);
int env_alloc(struct Env **e, u_int parent_id);
void env_free(struct Env *);
struct Env *env_create(const void *binary, size_t size, int priority);
void env_destroy(struct Env *e);

int envid2env(u_int envid, struct Env **penv, int checkperm);
void env_run(struct Env *e) __attribute__((noreturn));
void enable_irq(void);

void env_check(void);
void envid2env_check(void);

#define ENV_CREATE_PRIORITY(x, y)                                                                  \
	({                                                                                         \
		extern u_char binary_##x##_start[];                                                \
		extern u_int binary_##x##_size;                                                    \
		env_create(binary_##x##_start, (u_int)binary_##x##_size, y);                       \
	})

#define ENV_CREATE(x)                                                                              \
	({                                                                                         \
		extern u_char binary_##x##_start[];                                                \
		extern u_int binary_##x##_size;                                                    \
		env_create(binary_##x##_start, (u_int)binary_##x##_size, 1);                       \
	})

#endif // !_ENV_H_
