#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <sys/socket.h>

#define MAX_CONF_LINE 150

int buflen = 8 * 1024;
int af =  AF_UNSPEC; 
int udp = 0;
char *sport = "";             
char *port = "5001";  
int trans;
int sinkmode = 0; 
int bufoffset = 0;              /* align buffer to this */
int bufalign = 16*1024; 
int verbose = 0;   
int options = 0; 
int sockbufsize = 0;  
char fmt = 'K';  
int nbuf = 2 * 1024; 
int nodelay = 0; 
static long wait = 0; 
int b_flag = 0;  
int touchdata = 0;  
char *device = NULL;

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

            if (strcmp(key, "ip_version") == 0 && strlen(val) == 1 && val[0] == '4') {
                af = AF_INET;
            } else if (strcmp(key, "ip_version") == 0 && strlen(val) == 1 && val[0] == '6') {
                af = AF_INET6;
            } else if (strcmp(key, "buffer_length") == 0 && atoi(val) > 0) {
                buflen = atoi(val);
            } else if (strcmp(key, "protocol") == 0 && strcmp(val, "udp") == 0) {
                udp = 1;
            } else if (strcmp(key, "port") == 0 && atoi(val) > 1023 && atoi(val) < 65535) {
                char result[5];
                strcpy(result, val);
                port = result;
            } else if (strcmp(key, "silent") == 0 && strlen(val) == 1 && val[0] == '1') {
                sinkmode = !sinkmode; 
            } else if (strcmp(key, "bufalign") == 0 && atoi(val) > 0) {
                bufalign = atoi(val);
            } else if (strcmp(key, "bufoffset") == 0 && atoi(val) > 0) {
                bufoffset = atoi(val);
            } else if (strcmp(key, "verbose") == 0 && strlen(val) == 1 && val[0] == '1') {
                verbose = 1;
            } else if (strcmp(key, "ttcp.sock.debug") == 0 && strlen(val) == 1 && val[0] == '1') {
                options |= SO_DEBUG;
            } else if (strcmp(key, "ttcp.sock.buf.size") == 0 && atoi(val) > 0) {
                sockbufsize = atoi(val);
            } else if (strcmp(key, "format") == 0 && strlen(val) == 1) {
                fmt = val[0] ;
            } else if (strcmp(key, "nbuf") == 0 && atoi(val) > 0) {
                nbuf = atoi(val);
            } else if (strcmp(key, "ttcp.tcp.nodelay") == 0 && strlen(val) == 1 && val[0] == '1') {
                nodelay = 1;
            } else if (strcmp(key, "write_interval") == 0 && strlen(val) > 0) {
                wait = strtol(val, (char **)NULL, 10);
            } else if (strcmp(key, "source_port") == 0 && atoi(val) > 1023 && atoi(val) < 65535) {
                char result[5];
                strcpy(result, val);
                sport = result;
            } else if (strcmp(key, "blocks_output") == 0 && strlen(val) == 1 && val[0] == '1') {
                b_flag = 1;
            } else if (strcmp(key, "touch") == 0 && strlen(val) == 1 && val[0] == '1') {
                touchdata = 1;
            } else if (strcmp(key, "device") == 0 && strlen(val) > 0) {
                char result[12];
                strcpy(result, val);
                device = result;
            }
        }
    }

    printf("buflen: %d, af: %d, udp: %d, port: %s, sinkmode: %d, bufalign: %d, bufoffset: %d, verbose: %d, options: %d, sockbufsize: %d, ", buflen, af, udp, port, sinkmode, bufalign, bufoffset, verbose, options, sockbufsize);
    printf("fmt: %c, nbuf: %d, nodelay: %d, wait: %ld, sport: %s, b_flag: %d, touchdata: %d, device: %s", fmt, nbuf, nodelay, wait, sport, b_flag, touchdata, device); 
}
