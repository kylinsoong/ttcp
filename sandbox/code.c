/*
 * gcc -O2 -S code.c
 */

int accum =0;

int sum(int x, int y)
{
  int t = x + y;
  accum += t;
  return t;
}
