void hello(char *string, int len);
// int hello(int value);

int main (){
  char *string = "ABCD\n";
  hello(string, 5);
//   return hello(10);
    // hello("A", 1);
    return 0;
}

// bits 32

// global hello

// hello:
//   mov edx, [esp+4]
//   add edx, 0x1
//   mov eax, edx
//   ret
