#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_DATA_LINE 4096

FILE * fp;
const char* filename = "test.stat";

char bufr[MAX_DATA_LINE];

int main(int argc, char *argv[])
{

    if((fp = fopen("test.stat1","r")) != NULL) {
        while(fgets(bufr, MAX_DATA_LINE, fp) != NULL) {
            printf("len: %d, msg: %s/n", strlen(bufr), bufr);
        }
    } else {
        printf("file not exists\n");
    }

/*
    FILE *in_file = fopen(filename, "r");

    struct stat sb;
    stat(filename, &sb);

    char *file_contents = malloc(sb.st_size);

    while (fscanf(in_file, "%[^\n] ", file_contents) != EOF) {
        printf("len: %d, message: %s\n", strlen(file_contents), file_contents);
    }

    fclose(in_file);
    exit(EXIT_SUCCESS);
*/


}
