#include <lib.h>

int main(int argc, char **argv)
{
    int fd;
    if (argc != 2)
    {
        printf("Usage: unset file_name\n");
        return;
    }
    syscall_envar(argv[1], "", VAR_Unset, 0);
    return 0;
}