#include <lib.h>
int num_dir;
int num_file;
void dfs(char *path, int depth, int finalFile[])
{
    struct Fd *fd;
    struct Filefd *fileFd;
    int r;
    // debugf("open %s", path);
    if ((r = open(path, O_RDONLY)) < 0)
    {
        debugf("%s [error opening dir]\n", path);
        return;
    }

    for (int i = 0; i < depth; i++)
    {
        if (i == depth - 1)
        { // 到文件那层了
            if (finalFile[i])
            { // 是最后一个
                printf("└── ");
            }
            else
            {
                printf("├── ");
            }
        }
        else
        { // 仍未到
            if (finalFile[i])
            {
                printf("    ");
            }
            else
            {
                printf("│   ");
            }
        }
    }

    fd = num2fd(r);
    fileFd = (struct FileFd *)fd;
    if (fileFd->f_file.f_type == FTYPE_REG)
    { // 是文件
        printf("%s\n", fileFd->f_file.f_name);
        num_file++;
        close(r);
        return;
    }
    else
    { // 是目录
        printf("\033[34m%s\033[m\n", fileFd->f_file.f_name);
        num_dir++;
    }

    u_int size = fileFd->f_file.f_size;
    u_int num = ROUND(size, sizeof(struct File)) / sizeof(struct File);
    struct File *file = (struct File *)fd2data(fd);
    u_int file_num = 0; // 记录目前文件块数量
    for (int i = 0; i < num; i++)
    {
        // 如果是一个文件
        if (file[i].f_name[0] != '\0')
        {
            file_num++;
        }
    }

    u_int j = 0; // 已处理的有效子文件
    for (int i = 0; i < num; i++)
    {
        if (file[i].f_name[0] == '\0')
        {
            continue;
        }

        // 制作新的path
        char nextPath[1024];
        memset(nextPath, 0, 1024);
        strcpy(nextPath, path);
        int len = strlen(nextPath);
        if (path[len - 1] == '/' && len != 1)
        {
            return;
        }
        // debugf("path = %s depth = %d len = %d file_name = %s\n\n", path, depth, len, file[i].f_name);
        nextPath[len] = '/';
        len++;
        strcpy(nextPath + len, file[i].f_name);

        if (j == file_num - 1)
        {
            finalFile[depth] = 1;
        }

        dfs(nextPath, depth + 1, finalFile);
        j++;
    }
    finalFile[depth] = 0;
    close(r);
}
int main(int argc, char **argv)
{
    int finalFile[1024] = {0};
    num_dir = 0;
    num_file = 0;
    if (argc == 1)
    {
        dfs("/", 0, finalFile);
    }
    else
    {
        for (int i = 1; i < argc; i++)
        {
            // printf("dfs %s\n", argv[i]);
            dfs(argv[i], 0, finalFile);
            memset(finalFile, 0, 1024);
        }
    }
    debugf("%d directories, %d files\n", num_dir - argc, num_file);
}