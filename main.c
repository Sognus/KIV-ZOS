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
    printf("\n");

    //create_file(argv[1], MAX_CLUSTER_COUNT, MAX_CLUSTER_SIZE);

    // Vytvoreni FS
    printf("CREATING FORMATTED FILE!\n");
    format_file(argv[1],  128000);


    // Precteci BR z FS
    printf("READING BOOT RECORD!\n");
    boot_record *record = read_boot_record(argv[1]);
    print_boot_record(record);
    printf("\n");

    // Precteni bitmapy z FS
    printf("READING BITMAP!\n");
    int *bitmap = read_bitmap(argv[1], record);
    print_bitmap(bitmap, record);
    printf("\n");

    // Precteni MFT z fS
    printf("READING MFT!\n");
    mft_item *mft_array = NULL;
    int mft_array_size = -1;
    read_mft_items(argv[1], record, &mft_array, &mft_array_size);
    print_mft_items(mft_array, mft_array_size);
    printf("\n");


    printf("NTFS END!\n");
    return 0;
}