#include <lib.h>
#include <error.h>

void setValue(char *name, char *value, u_int isGlobalEnVar)
{ // 如果
    int r;
    if ((r = syscall_envar(name, value, VAR_Create, isGlobalEnVar)) == -E_FILE_EXISTS)
    {
        syscall_envar(name, value, VAR_SetValue, isGlobalEnVar);
    }
    else if (r < 0)
    {
        user_panic("can't set Value: %d\n", r);
    }
}

int main(int argc, char **argv)
{
    // printf("argc = %d ", argc);
    // for (int i = 0; i < argc; i++)
    // {
    //     printf("argv %d = %s \n", i, argv[i]);
    // }
    if (argc > 4)
    {
        printf("Usage: declare [-xr] [NAME [=VALUE]]\n");
        return;
    }
    else if (argc == 1)
    { // 输出当前 shell 的所有变量
        syscall_envar("", "", VAR_GetAll, 0);
    }
    else if (argc == 2)
    { // declare NAME
      // char *str = strchr(argv[1], '=');
      // if (str == NULL)
      // {
        setValue(argv[1], "", 0);
        // }else{
        //     setValue
        // }
    }
    else if (argc == 3)
    { // declare -xr NAME 或 declare NAME =VALUE
        if (argv[1][0] == '-')
        {
            if (strchr(argv[1], 'x') != NULL)
            {
                setValue(argv[2], "", 1);
            }
            else
            {
                setValue(argv[2], "", 0);
            }
            if (strchr(argv[1], 'r') != NULL)
            {
                syscall_envar(argv[2], "", VAR_SetRead, 0);
            }
        }
        else
        {
            setValue(argv[1], &argv[2][1], 0);
        }
    }
    else
    { // declare -xr NAME =VALUE
        if (strchr(argv[1], 'x') != NULL)
        {
            setValue(argv[2], &argv[3][1], 1);
        }
        else
        {
            setValue(argv[2], &argv[3][1], 0);
        }
        if (strchr(argv[1], 'r') != NULL)
        {
            syscall_envar(argv[2], &argv[3][1], VAR_SetRead, 0);
        }
    }
}