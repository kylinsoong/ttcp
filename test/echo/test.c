#include <stdio.h>
#include <time.h>


void test_time();
void test_chargen();

int main ()
{
    test_chargen();
}

void test_chargen()
{
    char randomletter = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*"[random () % 45];
    printf("%c\n", randomletter);        
}

void test_time()
{
    time_t now = time(0);
    char buffer [50];
    sprintf (buffer, "%lu" , now);
    printf("%s\n", buffer);
}
