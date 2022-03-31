/**
 *  * Author: Kylin Soong(kylinsoong.1214@gmail.com)
 *   * Date  : 2019-12-31
 *    */

#include "neth.h"

void err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    printf("%s\n", fmt);
    va_end(ap);

    exit(1);
}

void out_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    printf("%s\n", fmt);
    va_end(ap);
}

char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char *randstring(int size)
{
    int i;
    char *res = malloc(size + 1);
    for(i = 0; i < size; i++) {
        res[i] = (char) (rand() % (ASCII_END - ASCII_START)) + ASCII_START;
    }
    res[i] = '\0';
    return res;
}
