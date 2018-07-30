int a = 4;

void __attribute__((noinline)) f(int* ptr){
    *ptr = *ptr + 1;
}

int main(){
    int* b = &a;
    f(b);
    return *b;
}
