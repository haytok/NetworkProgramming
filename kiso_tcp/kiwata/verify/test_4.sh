nasm -f elf32 -o syscall_test_4.o test_4.s
gcc -m32 -o test_4.o -c test_4.c
gcc -m32 -o test_4.out test_4.o syscall_test_4.o
./test_4.out
