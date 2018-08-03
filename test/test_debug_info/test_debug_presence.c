#include<stdio.h>
#include<stdlib.h>

typedef struct person_t {
    int age;
    char name[10];
} Person;

int f(Person *p1) {
    return 10;
}

int main() {
    Person p = {10, "Jack"}; 
    Person *p1 = &p; 
    f(p1);
}
