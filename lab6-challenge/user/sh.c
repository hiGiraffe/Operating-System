#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2)
{
	*p1 = 0;
	*p2 = 0;
	if (s == 0)
	{
		return 0;
	}
	// 清除s开头的空白字符
	while (strchr(WHITESPACE, *s))
	{
		*s++ = 0;
	}
	// 如果s没了
	if (*s == 0)
	{
		return 0;
	}
	// 实现“”
	if (*s == '"')
	{
		*s = 0;
		*p1 = ++s;
		while (s != 0 && *s != '"')
		{
			s++;
		}
		*s = 0;
		*p2 = s;
		return 'w';
	}
	// 如果s是特殊字符，则返回特殊字符，并将*p1指向该位置，然后将该字符替换为零字符，并将*p2指向下一个位置。
	if (strchr(SYMBOLS, *s))
	{
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}
	// 如果s指向的字符是普通字符，将*p1指向该位置，并将s向后移动
	// 直到遇到空白字符或特殊符号
	// 将*p2指向最后一个非空白字符的下一个位置。
	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s))
	{
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1)
{
	static int c, nc;		// c 上一个标记类型 nc这次标记类型（留给下次用）
	static char *np1, *np2; // np1标志的起始位置，np2标志结束的下一个位置
	// 首次解析新的字符串
	if (s)
	{
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe)
{
	int argc = 0;
	while (1)
	{
		char *t;
		int fd, r;
		// 调用gettoken函数获取下一个标记（token），并将标记的类型保存在变量c中，将标记的起始位置保存在指针t中。
		int c = gettoken(0, &t);
		switch (c)
		{
		case 0: // 如果标记类型为0，表示命令字符串已经解析完毕，此时返回参数个数argc。
			return argc;
		case 'w': // 如果标记类型为字符w，表示是一个单词（命令、参数或文件名），将该单词添加到参数列表argv中，并增加argc计数。
			if (argc >= MAXARGS)
			{
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case ';':
			if ((*rightpipe = fork()) == 0)
			{
				return argc;
			}
			else
			{
				wait(*rightpipe);
				do
				{
					close(0);
					if ((r = opencons()) < 0)
					{
						user_panic("1");
					}
				} while (r != 0);
				dup(0, 1);
				return parsecmd(argv, rightpipe);
			}
			break;
		case '&':
			if ((*rightpipe = fork()) == 0)
			{
				return argc;
			}
			else
			{
				return parsecmd(argv, rightpipe);
			}
			break;
		case '<': // 如果标记类型为字符<，表示是输入重定向符号，即后面跟着输入重定向的文件名。
			if (gettoken(0, &t) != 'w')
			{
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			// 打开该文件并将其作为标准输入（文件描述符0）。
			fd = open(t, O_RDONLY);
			dup(fd, 0);
			close(fd);
			break;
		case '>': // 如果标记类型为字符>，表示是输出重定向符号，即后面跟着输出重定向的文件名。
			if (gettoken(0, &t) != 'w')
			{
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			// 打开该文件并将其作为标准输出（文件描述符1）。
			fd = open(t, O_WRONLY);
			dup(fd, 1);
			close(fd);
			break;
		case '|':; // 如果标记类型为字符|，表示是管道符号，即后面跟着管道命令的右侧部分。
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			if ((r = pipe(p)) < 0)
			{
				debugf("failed to create pipe\n");
				exit();
			}
			if ((*rightpipe = fork()) == 0)
			{
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe);
			}
			else
			{
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			break;
		}
	}
	return argc;
}

void runcmd(char *s)
{
	hist_store(s);
	// 调用gettoken函数，开始解析命令字符串s并获取下一个标记（token）。
	gettoken(s, 0);

	char *argv[MAXARGS]; // 存储命令的参数列表。
	int rightpipe = 0;	 // 存储右侧管道命令的子进程ID。
	// 调用parsecmd函数解析命令字符串，并将解析得到的参数存储在argv数组中。
	// 如果命令中存在管道符号|，则将右侧管道命令的子进程ID存储在rightpipe变量中。
	int argc = parsecmd(argv, &rightpipe);
	// 如果参数个数为0，即没有解析到任何命令参数，则直接返回，不执行后续操作。
	if (argc == 0)
	{
		return;
	}
	// argv[argc] = 0;：将参数数组的最后一个元素设置为NULL，以便在后续的执行中可以使用NULL结尾的参数列表。
	argv[argc] = 0;
	// 调用spawn函数创建一个新的子进程，并执行命令。argv[0]是要执行的命令的名称，argv是参数列表。
	int child = spawn(argv[0], argv);
	// 关闭当前进程的所有文件描述符，以防止子进程继承不必要的文件描述符。
	close_all();
	// 根据spawn函数的返回值判断子进程的创建是否成功。
	if (child >= 0)
	{
		wait(child);
	}
	else
	{
		debugf("spawn %s: %d\n", argv[0], child);
	}
	// 如果存在右侧管道命令，等待右侧管道命令的子进程执行结束。
	if (rightpipe)
	{
		wait(rightpipe);
	}
	// 当前进程退出。
	exit();
}
void readline(char *buf, u_int n)
{
	int len = 0;
	int r;
	char temp;
	char history[1024][1024];
	memset(history, 0, 1024 * 1024);
	int row = get_hist(history);
	int offset = 0;
	// for (int i = 0; i < row; i++)
	// {
	// 	printf("history %d = %s\n", i, history[i]);
	// }
	for (int i = 0; i < n; i++)
	{
		if ((r = read(0, &temp, 1)) != 1)
		{
			if (r < 0)
			{
				debugf("read error: %d\n", r);
			}
			exit();
		}
		if (temp == '\b' || temp == 0x7f)
		{
			if (i > 0)
			{
				if (i == len) //假如在最右侧
				{
					buf[i - 1] = 0;
					printf("\b \b");
				}
				else //假如不在最右侧
				{
					for (int j = i - 1; j < len - 1; j++)
					{
						buf[j] = buf[j + 1];
					}
					buf[len - 1] = 0;
					MOVELEFT(i);
					printf("%s ", buf);
					MOVELEFT(len - i + 1);
				}
				len--;
				i -= 2;
			}
			else
			{
				i = -1;
			}
		}
		else if (temp == '\033')
		{
			char temp1, temp2;
			if ((r = read(0, &temp1, 1)) != 1)
			{
				if (r < 0)
				{
					printf("read error: %d\n", r);
				}
				exit();
			}
			if ((r = read(0, &temp2, 1)) != 1)
			{
				if (r < 0)
				{
					printf("read error: %d\n", r);
				}
				exit();
			}

			if (temp1 == '[')
			{
				switch (temp2)
				{
				case 'A': // 上键
					MOVEDOWN(1);
					flush(strlen(buf) + 2);
					printf("$ ");
					if (offset < row)
					{
						offset++;
					}
					strcpy(buf, history[row - offset]);
					len = strlen(buf);
					debugf("%s", buf);
					i = len - 1;
					break;
				case 'B': // 下键]
					flush(strlen(buf) + 2);
					printf("$ ");
					if (offset > 0)
					{
						offset--;
					}
					if (offset != 0)
					{
						strcpy(buf, history[row - offset]);
						len = strlen(buf);
						debugf("%s", buf);
					}
					else
					{
						memset(buf, 0, sizeof(buf));
						len = strlen(buf);
					}
					i = len - 1;
					break;
				case 'C': // 右键
					if (i < len)
					{
						i = i;
					}
					else
					{
						i = len - 1;
						MOVELEFT(1);
					}
					break;
				case 'D': // 左键
					if (i > 0)
					{
						i -= 2;
					}
					else
					{
						i = -1;
						MOVERIGHT(1);
					}
					break;
				default:
					i--;
					break;
				}
			}
		}
		else if (temp == '\n' || temp == '\r')
		{
			buf[len] = 0;
			return;
		}
		else
		{
			if (i == len)
			{
				buf[i] = temp;
			}
			else
			{
				for (int j = len; j > i; j--)
				{
					buf[j] = buf[j - 1];
				}
				buf[i] = temp;
				buf[len + 1] = 0;
				printf("%s", buf + i + 1);
				MOVELEFT(len - i);
			}
			len += 1;
		}
		if (len >= n)
		{
			break;
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n')
	{
		;
	}
	buf[0] = 0;
}

void MOVELEFT(int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("\033[D");
	}
}

void MOVERIGHT(int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("\033[C");
	}
}
void MOVEDOWN(int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("\033[B");
	}
}
void MOVEUP(int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("\033[A");
	}
}
void flush(int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("\b \b");
	}
}

char buf[1024];

void usage(void)
{
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int shell_id;
int main(int argc, char **argv)
{
	int r;
	int interactive = iscons(0); // 是否处于交互模式，默认为标准输入是终端时为1，否则为0。
	int echocmds = 0;			 // 是否打印命令行
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	shell_id = syscall_create_shell_id();
	debugf("shell_id is %d\n", shell_id);
	ARGBEGIN
	{		  // 解析命令行参数
	case 'i': // 设置交互模式
		interactive = 1;
		break;
	case 'x': // 打印命令行
		echocmds = 1;
		break;
	default: // 命令行参数超过1个，则调用usage函数打印帮助信息。
		usage();
	}
	ARGEND
	// 如果参数使用错误，则调用usage函数打印帮助信息。
	if (argc > 1)
	{
		usage();
	}
	// 如果指定了命令文件参数（即argc == 1）
	if (argc == 1)
	{
		// 关闭标准输入（文件描述符0）
		close(0);
		// 以只读模式打开该文件
		if ((r = open(argv[1], O_RDONLY)) < 0)
		{
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}

	hist_init();

	// 进入无限循环，等待用户输入并执行命令
	for (;;)
	{
		// 如果处于交互模式，打印命令行提示符$
		if (interactive)
		{
			printf("\n$ ");
		}

		// 调用readline函数从标准输入读取一行用户输入，并将其存储在缓冲区buf中。
		readline(buf, sizeof buf);
		// 如果输入的是注释或空行（以#开头），则忽略该行，继续下一次循环。
		if (buf[0] == '#')
		{
			continue;
		}
		// 如果设置了echocmds，则打印命令行。
		if (echocmds)
		{
			printf("# %s\n", buf);
		}
		// 创建子进程
		if ((r = fork()) < 0)
		{
			user_panic("fork: %d", r);
		}
		if (r == 0)
		{
			// 在子进程中调用runcmd函数来解析并执行命令。
			runcmd(buf);
			// 子进程执行完命令后调用exit函数退出。
			exit();
		}
		else
		{
			// 父进程等待子进程执行完毕。
			wait(r);
		}
	}
	return 0;
}
