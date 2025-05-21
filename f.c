// f.c
#include <stdio.h>
#include <stdlib.h>

int f(int x) {
    if (x == 1) return 0;
    if (x == 2) while (1);
    return 1;
}

int main(int argc, char* argv[]) {
    int x;
    scanf("%d", &x);
    printf("%d\n", f(x));
    return 0;
}