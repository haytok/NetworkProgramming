nasm -f elf64 -o syscall.o syscall.asm
gcc -c test_3.c
gcc test_3.o syscall.o
./a.out
