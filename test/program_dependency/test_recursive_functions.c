#include <stdio.h>
void recursive_print(int a) {
    if (a < 15) {
        printf("%d\n", a);  
        recursive_print(a + 1);
    }
}

int main() {
    recursive_print(0);
    return 0;
}
