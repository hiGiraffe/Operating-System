#include <elf.h>
#include <env.h>
#include <lib.h>
#include <mmu.h>

#define debug 0

int init_stack(u_int child, char **argv, u_int *init_sp)
{
	int argc, i, r, tot;
	char *strings;
	u_int *args;

	// Count the number of arguments (argc)
	// and the total amount of space needed for strings (tot)
	tot = 0;
	for (argc = 0; argv[argc]; argc++)
	{
		tot += strlen(argv[argc]) + 1;
	}

	// Make sure everything will fit in the initial stack page
	if (ROUND(tot, 4) + 4 * (argc + 3) > BY2PG)
	{
		return -E_NO_MEM;
	}

	// Determine where to place the strings and the args array
	strings = (char *)(UTEMP + BY2PG) - tot;
	args = (u_int *)(UTEMP + BY2PG - ROUND(tot, 4) - 4 * (argc + 1));

	if ((r = syscall_mem_alloc(0, (void *)UTEMP, PTE_D)) < 0)
	{
		return r;
	}

	// Copy the argument strings into the stack page at 'strings'
	char *ctemp, *argv_temp;
	u_int j;
	ctemp = strings;
	for (i = 0; i < argc; i++)
	{
		argv_temp = argv[i];
		for (j = 0; j < strlen(argv[i]); j++)
		{
			*ctemp = *argv_temp;
			ctemp++;
			argv_temp++;
		}
		*ctemp = 0;
		ctemp++;
	}

	// Initialize args[0..argc-1] to be pointers to these strings
	// that will be valid addresses for the child environment
	// (for whom this page will be at USTACKTOP-BY2PG!).
	ctemp = (char *)(USTACKTOP - UTEMP - BY2PG + (u_int)strings);
	for (i = 0; i < argc; i++)
	{
		args[i] = (u_int)ctemp;
		ctemp += strlen(argv[i]) + 1;
	}

	// Set args[argc] to 0 to null-terminate the args array.
	ctemp--;
	args[argc] = (u_int)ctemp;

	// Push two more words onto the child's stack below 'args',
	// containing the argc and argv parameters to be passed
	// to the child's main() function.
	u_int *pargv_ptr;
	pargv_ptr = args - 1;
	*pargv_ptr = USTACKTOP - UTEMP - BY2PG + (u_int)args;
	pargv_ptr--;
	*pargv_ptr = argc;

	// Set *init_sp to the initial stack pointer for the child
	*init_sp = USTACKTOP - UTEMP - BY2PG + (u_int)pargv_ptr;

	if ((r = syscall_mem_map(0, (void *)UTEMP, child, (void *)(USTACKTOP - BY2PG), PTE_D)) <
		0)
	{
		goto error;
	}
	if ((r = syscall_mem_unmap(0, (void *)UTEMP)) < 0)
	{
		goto error;
	}

	return 0;

error:
	syscall_mem_unmap(0, (void *)UTEMP);
	return r;
}

static int spawn_mapper(void *data, u_long va, size_t offset, u_int perm, const void *src,
						size_t len)
{
	u_int child_id = *(u_int *)data;
	try(syscall_mem_alloc(child_id, (void *)va, perm));
	if (src != NULL)
	{
		int r = syscall_mem_map(child_id, (void *)va, 0, (void *)UTEMP, perm | PTE_D);
		if (r)
		{
			syscall_mem_unmap(child_id, (void *)va);
			return r;
		}
		memcpy((void *)(UTEMP + offset), src, len);
		return syscall_mem_unmap(0, (void *)UTEMP);
	}
	return 0;
}

int spawn(char *prog, char **argv)
{
	// Step 1: Open the file 'prog' (the path of the program).
	// Return the error if 'open' fails.
	// 打开程序文件（prog）
	int fd;
	if ((fd = open(prog, O_RDONLY)) < 0)
	{
		char tmp[1024];
		int i;

		for (i = 0; *(prog + i) != '\0'; i++)
		{
			*(tmp + i) = *(prog + i);
		}
		*(tmp + i) = '.';
		*(tmp + i + 1) = 'b';
		*(tmp + i + 2) = '\0';
		if ((fd = open(tmp, O_RDONLY)) <= 0)
		{
			return fd;
		}
	}

	// Step 2: Read the ELF header (of type 'Elf32_Ehdr') from the file into 'elfbuf' using
	// 'readn()'.
	// If that fails (where 'readn' returns a different size than expected),
	// set 'r' and 'goto err' to close the file and return the error.
	// 读取ELF头部：通过调用readn()系统调用读取ELF文件的头部信息（ELF header），并将其存储在elfbuf缓冲区中。如果读取失败（返回的字节数与期望的ELF头部大小不一致），返回错误码。
	int r;
	u_char elfbuf[512];
	/* Exercise 6.4: Your code here. (1/6) */
	if ((r = readn(fd, elfbuf, sizeof(Elf32_Ehdr))) != sizeof(Elf32_Ehdr))
	{
		r = -E_NOT_EXEC;
		goto err;
	}

	const Elf32_Ehdr *ehdr = elf_from(elfbuf, sizeof(Elf32_Ehdr));
	if (!ehdr)
	{
		r = -E_NOT_EXEC;
		goto err;
	}
	u_long entrypoint = ehdr->e_entry;

	// Step 3: Create a child using 'syscall_exofork()' and store its envid in 'child'.
	// If the syscall fails, set 'r' and 'goto err'.
	// 创建子进程：通过调用syscall_exofork()系统调用创建一个子进程，并将子进程的envid存储在变量child中。如果创建子进程失败，返回错误码。如果是子进程，直接返回0。
	u_int child;
	/* Exercise 6.4: Your code here. (2/6) */
	if ((r = syscall_exofork()) < 0)
	{
		goto err;
	}
	if (r == 0)
		return 0;
	child = r;

	// Step 4: Use 'init_stack(child, argv, &sp)' to initialize the stack of the child.
	// 'goto err1' if that fails.
	// 初始化子进程的堆栈：调用init_stack()函数为子进程初始化堆栈，并将堆栈指针存储在变量sp中。如果初始化堆栈失败，跳转到err1标签处进行错误处理。
	u_int sp;
	/* Exercise 6.4: Your code here. (3/6) */
	if ((r = init_stack(child, argv, &sp)) < 0)
	{
		goto err1;
	}

	// Step 5: Load the ELF segments in the file into the child's memory.
	// This is similar to 'load_icode()' in the kernel.
	// 加载ELF文件段：使用ELF_FOREACH_PHDR_OFF宏遍历ELF文件的每个程序头表项。
	// 对于每个类型为PT_LOAD的段，首先使用seek()和readn()读取段的内容到elfbuf中，然后使用read_map()函数将该段的数据映射到内存中。
	// 接下来，使用elf_load_seg()函数将段加载到子进程的内存空间中。如果读取或映射失败，跳转到err1标签处进行错误处理。
	size_t ph_off;
	ELF_FOREACH_PHDR_OFF(ph_off, ehdr)
	{
		// Read the program header in the file with offset 'ph_off' and length
		// 'ehdr->e_phentsize' into 'elfbuf'.
		// 'goto err1' on failure.
		// You may want to use 'seek' and 'readn'.
		/* Exercise 6.4: Your code here. (4/6) */
		if ((r = seek(fd, ph_off)) < 0)
		{
			goto err1;
		}
		if ((r = readn(fd, elfbuf, ehdr->e_phentsize)) != ehdr->e_phentsize)
		{
			r = -E_NOT_EXEC;
			goto err1;
		}

		Elf32_Phdr *ph = (Elf32_Phdr *)elfbuf;
		if (ph->p_type == PT_LOAD)
		{
			void *bin;
			// Read and map the ELF data in the file at 'ph->p_offset' into our memory
			// using 'read_map()'.
			// 'goto err1' if that fails.
			/* Exercise 6.4: Your code here. (5/6) */
			if ((r = read_map(fd, ph->p_offset, &bin)) < 0)
			{
				goto err1;
			}
			// Load the segment 'ph' into the child's memory using 'elf_load_seg()'.
			// Use 'spawn_mapper' as the callback, and '&child' as its data.
			// 'goto err1' if that fails.
			/* Exercise 6.4: Your code here. (6/6) */
			if ((r = elf_load_seg(ph, bin, spawn_mapper, &child)) < 0)
			{
				goto err1;
			}
		}
	}
	close(fd);
	// 设置子进程的入口点和堆栈指针：将子进程的入口地址设置为ELF头部中的入口地址，将子进程的堆栈指针设置为步骤4中初始化的堆栈指针。
	struct Trapframe tf = envs[ENVX(child)].env_tf;
	tf.cp0_epc = entrypoint;
	tf.regs[29] = sp;
	if ((r = syscall_set_trapframe(child, &tf)) != 0)
	{
		goto err2;
	}

	// Pages with 'PTE_LIBRARY' set are shared between the parent and the child.
	// 共享内存页面：将具有PTE_LIBRARY标志的页面在父进程和子进程之间进行共享，通过调用syscall_mem_map()系统调用实现。
	// 遍历虚拟页目录和页表，找到标记为PTE_V且PTE_LIBRARY的页面，并将其映射到子进程的相应地址空间中。
	for (u_int pdeno = 0; pdeno <= PDX(USTACKTOP); pdeno++)
	{
		if (!(vpd[pdeno] & PTE_V))
		{
			continue;
		}
		for (u_int pteno = 0; pteno <= PTX(~0); pteno++)
		{
			u_int pn = (pdeno << 10) + pteno;
			u_int perm = vpt[pn] & ((1 << PGSHIFT) - 1);
			if ((perm & PTE_V) && (perm & PTE_LIBRARY))
			{
				void *va = (void *)(pn << PGSHIFT);

				if ((r = syscall_mem_map(0, va, child, va, perm)) < 0)
				{
					debugf("spawn: syscall_mem_map %x %x: %d\n", va, child, r);
					goto err2;
				}
			}
		}
	}
	// 设置子进程为可运行状态：调用syscall_set_env_status()系统调用将子进程的状态设置为可运行。如果设置失败，跳转到err2标签处进行错误处理。
	if ((r = syscall_set_env_status(child, ENV_RUNNABLE)) < 0)
	{
		debugf("spawn: syscall_set_env_status %x: %d\n", child, r);
		goto err2;
	}
	return child;

err2:
	syscall_env_destroy(child);
	return r;
err1:
	syscall_env_destroy(child);
err:
	close(fd);
	return r;
}

int spawnl(char *prog, char *args, ...)
{
	// Thanks to MIPS calling convention, the layout of arguments on the stack
	// are straightforward.
	return spawn(prog, &args);
}
