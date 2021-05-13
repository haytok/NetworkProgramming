#include <stdio.h>

int main()
{
    puts("--start--");

    #ifdef __unix
    puts("unix");
    #endif

    #ifdef __linux
    puts("linux");
    #endif

    #ifndef __linux
    puts("not linux");
    #endif

    #ifdef FORK_SERVER
    puts("FORK_SERVER");
    #endif

    #ifdef __GNUC__
    puts("GNUC");
    #endif

    #ifdef __sun
    puts("sun");
    #endif

    #ifdef __sparc
    puts("sparc");
    #endif

    #ifdef __solaris
    puts("solaris");
    #endif

    puts("--end--");
    return 0;
}
