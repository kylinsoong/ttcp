#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/socket.h>

#define MAX_CONF_LINE 100

int buflen = 8 * 1024;
int af =  AF_UNSPEC; 

FILE * fp;
char bufr[MAX_CONF_LINE];

int main(int argc, char ** argv) 
{
    if((fp = fopen("/etc/ttcp.conf","r")) != NULL) {
        while(fgets(bufr, MAX_CONF_LINE, fp) != NULL) {
            if (strncmp("#", bufr, strlen("#")) == 0 || strlen(bufr) < 3) {
                continue;
            }

            char *key = strtok(bufr, "=");
            char *val = strtok(NULL, "=");
            key = strtok(key, "\r\t\n ");
            val = strtok(val, "\r\t\n ");
            
            if(val ==  NULL) {
                continue;
            }

            if (strcmp(key, "ip_version") == 0 ) {
                if(val[0] == '4') {
                    af = AF_INET;
                } else if(val[0] == '6') {
                    af = AF_INET6;
                }
            } else if (strcmp(key, "buffer_length") == 0) {
                buflen = atoi(val);
            } else if (strcmp(key, "protocol") == 0) {
                printf("%s=%s\n", key, val);
            }
        }
    } 
}
