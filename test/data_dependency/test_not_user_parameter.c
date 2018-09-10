#include <stdio.h>

void emptyFuncPtr(int *X) {
    *X += 1;
    return;
}

void emptyFunc(int X) {
    return;
}

int main() {
    int x = 5;
    emptyFunc(x);
    emptyFuncPtr(&x);
}
