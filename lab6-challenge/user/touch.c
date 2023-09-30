#include <lib.h>

int main(int argc, char **argv)
{
    int fd;
    if (argc != 2)
    {
        printf("Usage: touch file_name\n");
        return;
    }
    if ((fd = open(argv[1], O_RDONLY)) >= 0)
    {
        printf("File %s touch success\n", argv[1]);
        return;
    }
    if ((fd = create(argv[1], FTYPE_REG)) < 0)
    {
        printf("Fail to create directory %s\n", argv[1]);
    }
    return 0;
}