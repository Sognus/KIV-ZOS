#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>

#include "structure.h"
#include "ntfs.h"

int main(int argc, char *argv[]) {
    if(argc < 2)
    {
        printf("ERROR: Program was started without arguments!");
        return -1;
    }

    printf("NTFS START!\n");

    create_file(argv[1], MAX_CLUSTER_COUNT, MAX_CLUSTER_SIZE);

    printf("NTFS END!\n");
    return 0;
}