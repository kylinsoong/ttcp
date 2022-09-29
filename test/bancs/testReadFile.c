#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const char* filename = "test.stat";

int main(int argc, char *argv[])
{
    FILE *in_file = fopen(filename, "r");

    struct stat sb;
    stat(filename, &sb);

    char *file_contents = malloc(sb.st_size);

    while (fscanf(in_file, "%[^\n] ", file_contents) != EOF) {
        printf("len: %d, message: %s\n", strlen(file_contents), file_contents);
    }

    fclose(in_file);
    exit(EXIT_SUCCESS);
}
