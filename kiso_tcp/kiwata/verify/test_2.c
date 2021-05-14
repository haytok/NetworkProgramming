#include <stdio.h>

#define var(i) printf("var" #i " = %d\n" , var ## i)
#define PRINT(str) printf(#str "\n")

int main() {
    int var1 = 10 , var2 = 20;
    var(1); // var1 = 10 が出力される
    var(2); // var2 = 20 が出力される

    PRINT(Hello);

    return 0;
}
 