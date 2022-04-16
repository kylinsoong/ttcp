#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *x = malloc(10 * sizeof(int));

    free(x);
}

