#ifndef SYSCALL_H
#define SYSCALL_H
#ifndef __ASSEMBLER__

enum
{
	SYS_putchar,
	SYS_print_cons,
	SYS_getenvid,
	SYS_yield,
	SYS_env_destroy,
	SYS_set_tlb_mod_entry,
	SYS_mem_alloc,
	SYS_mem_map,
	SYS_mem_unmap,
	SYS_exofork,
	SYS_set_env_status,
	SYS_set_trapframe,
	SYS_panic,
	SYS_ipc_try_send,
	SYS_ipc_recv,
	SYS_cgetc,
	SYS_write_dev,
	SYS_create_shell_id,
	SYS_read_dev,
	SYS_envar,
	MAX_SYSNO,
};

#endif

// envar
#define VAR_Create 0
#define VAR_GetOne 1
#define VAR_SetValue 2
#define VAR_Unset 3
#define VAR_GetAll 4
#define VAR_SetRead 5
#endif
