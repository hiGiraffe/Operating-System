#include <lib.h>

char buf[8192];

void history(int f)
{
    long n;
    int r;

    while ((n = read(f, buf, (long)sizeof buf)) > 0)
    {
        if ((r = write(1, buf, n)) != n)
        {
            user_panic("write error copying %d", r);
        }
    }
    if (n < 0)
    {
        user_panic("error reading %d", n);
    }
}

int main(int argc, char **argv)
{
    int f, i;

    if (argc != 1)
    {
        printf("Usage: history\n");
        return;
    }

    f = open("/.history", O_RDONLY);
    if (f < 0)
    {
        user_panic("can't open .history: %d", f);
    }
    else
    {
        history(f);
        close(f);
    }

    return 0;
}
