#include <stdio.h>

int main()
{
    int i = 3, j = 6;
    int * l, * m, * n;
    int k = i + j;
    char a = '0', b = '1';
    char * c, * d;
    l = &i;
    m = &j;
    n = &k;
    c = &a;
    d = &b;
    printf("&i: %d, i: %d\n", &i, i);
    printf("&j: %d, j: %d\n", &j, j);
    printf("&k: %d, k: %d\n", &k, k);
    printf("&l: %d, l: %d, *l: %d\n", &l, l, *l);
    printf("&m: %d, m: %d, *m: %d\n", &m, m, *m);
    printf("&n: %d, n: %d, *n: %d\n", &n, n, *n);
    printf("&a: %d, a: %c\n", &a, a);
    printf("&b: %d, b: %c\n", &b, b);
    printf("&c: %d, c: %d, *c: %c\n", &c, c, *c);
    printf("&d: %d, d: %d, *d: %c\n", &d, d, *d);
    return 0;
}
