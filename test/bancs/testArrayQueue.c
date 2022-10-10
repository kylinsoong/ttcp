#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#define size 5

int main()
{
    int arr[size],R=-1,F=0,ch,n,i;

    for(;;)		// An infinite loop
    {
        system("cls");		// for clearing the screen
        printf("1. Add\n");
        printf("2. Delete\n");
        printf("3. Display\n");
        printf("4. Exit\n");
        printf("Enter Choice: ");
        scanf("%d",&ch);

        switch(ch)
        {
            case 1:
                if(R==size-1)
                {
                    printf("Queue is full");
                    getch();	// pause the loop to see the message
                }
                else
                {
                    printf("Enter a number ");
                    scanf("%d",&n);
                    R++;
                    arr[R]=n;
                }
                break;

            case 2:
                if(F>R)
                {
                    printf("Queue is empty");
                    getch();	// pause the loop to see the message
                }
                else
                {
                    printf("Number Deleted = %d",arr[F]);
                    F++;
                    getch();	// pause the loop to see the number
                }
                break;

            case 3:
                if(F>R)
                {
                    printf("Queue is empty");
                    getch();	// pause the loop to see the message
                }
                else
                {
                    for(i=F; i<=R; i++)
                    {
                        printf("%d ",arr[i]);
                    }
                    getch();	// pause the loop to see the numbers
                }
                break;

            case 4:
                exit(0);
                break;

            default:
                printf("Wrong Choice");
                getch();	// pause the loop to see the message
        }
    }
    return 0;
}

int getch_echo(bool echo=true){
    struct termios oldt, newt;
    int ch;
    tcgetattr( STDIN_FILENO, &oldt );
    newt = oldt;
    newt.c_lflag &= ~ICANON;
    if(echo)
	newt.c_lflag &=  ECHO;
    else
        newt.c_lflag &= ~ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
    return ch;
}
      
int getch(){
    getch_echo(false);
}
