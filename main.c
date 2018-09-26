#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>

#include "bool.h"
#include "boot_record.h"


const int32_t UID_ITEM_FREE = 0;
const int32_t MFT_FRAGMENTS_COUNT = 32;



int main(int argc, char *argv[]) {
    printf("NTFS START!\n");

    struct boot_record *boot = malloc(sizeof(struct boot_record));

    printf("BOOT RECORD CREATED\n");

    strncpy(boot->signature, "test", sizeof("test"));

    printf("BOOT SIGNATURE: %s\n", boot->signature);

    bool test = true;

    printf("Bool: %d\n", test);


    free(boot);
    printf("NTFS END!\n");
    return 0;
}