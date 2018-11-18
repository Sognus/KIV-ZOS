#include <string.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "structure.h"


/**
 * Na zaklade vstupnich hodnot prepocita hodnoty v boot zaznamu a zmeni je
 *
 * @param record zaznam ke zmeneni
 * @param cluster_count novy pocet clusteru
 * @param cluster_size velikost jednoho clusteru
 */
void boot_record_resize(boot_record *record, int32_t cluster_count, int32_t cluster_size)
{
    /* VYPOCET VELIKOSTI JEDNOTLIVYCH CASTI NTFS */

    // Velikost datove casti: pocet clusteru * velikost clusteru v B
    int32_t data_size = cluster_count * cluster_size;
    // Velikost bitmapy: pocet clusteru * velikost bool
    int32_t bitmap_size = cluster_count * sizeof(int);
    // MFT = 10% datoveho bloku
    int32_t mft_size = (int32_t)(ceil(0.1 * data_size));
    // boot_record size
    int32_t boot_record_size = sizeof(boot_record);

    int32_t ntfs_size = data_size + bitmap_size + mft_size + boot_record_size;

    /* ------------------------------------------- */
    /* VYPOCET POCATECNICH ADRES JEDNOTLIVYCH CASTI NTFS  */
    int32_t mft_start = boot_record_size;
    int32_t bitmap_start = boot_record_size + mft_size;
    int32_t data_start = boot_record_size + mft_size + bitmap_size;

    /* -------------------------------------------- */
    /* UPRAVA BOOT RECORDU */
    record->cluster_count = cluster_count;
    record->cluster_size = cluster_size;
    record->data_start_address = data_start;
    record->bitmap_start_address = bitmap_start;
    record->mft_start_address = mft_start;
    // NOTE: Disk size je hodnota bez alignment bytes je nutno ji pripocitat
    record->disk_size = ntfs_size;


}

/**
 * Na zaklade vstupnich parametru vytvori boot_record pro filesystem
 *
 * @param signature ID FS
 * @param volume_descriptor popis FS
 * @param disk_size celkova velikost FS
 * @param cluster_size velikost clusteru
 * @param cluster_count pocet clusteru
 * @param mft_start_adress adresa pocatku MFT
 * @param bitmap_start_adress adresa pocatku bitmapy
 * @param data_start_adress adresa pocatku dat
 * @param mft_max_fragment_count maximalni pocet fragmentu v jednom MFT zaznamu
 * @return pointer na vytvoreny boot_record
 */
boot_record *create_boot_record(char signature[9], char volume_descriptor[251], int32_t disk_size,
        int32_t cluster_size, int32_t cluster_count, int32_t mft_start_adress, int32_t bitmap_start_adress,
        int32_t data_start_adress, int32_t mft_max_fragment_count)
{
    // TODO: Ověření vstupů

    // Variable for boot_record pointer declared
    boot_record *pointer = NULL;

    // Memory for struct allocated
    pointer = malloc(sizeof(boot_record));

    // Filling assigned memory
    strcpy(pointer->signature, signature);
    pointer->signature[8] = '\0';
    strcpy(pointer->volume_descriptor, volume_descriptor);
    pointer->volume_descriptor[250] = '\0';

    pointer->disk_size = disk_size;
    pointer->cluster_size = cluster_size;
    pointer->cluster_count = cluster_count;
    pointer->mft_start_address = mft_start_adress;
    pointer->bitmap_start_address = bitmap_start_adress;
    pointer->data_start_address = data_start_adress;
    pointer->mft_max_fragment_count = mft_max_fragment_count;
}

/**
 * Vytvori standardni boot_record pro KIV/ZOS
 *
 * @return pointer na vytvoreny boot_record
 */
boot_record *create_standard_boot_record()
{
    /* VYPOCET VELIKOSTI JEDNOTLIVYCH CASTI NTFS */

    // Velikost datove casti: pocet clusteru * velikost clusteru v B
    int32_t data_size = MAX_CLUSTER_COUNT * MAX_CLUSTER_SIZE;
    // Velikost bitmapy: pocet clusteru * velikost bool
    int32_t bitmap_size = MAX_CLUSTER_COUNT * sizeof(int);
    // MFT = 10% datoveho bloku
    int32_t mft_size = (int32_t)(ceil(0.1 * data_size));
    // boot_record size
    int32_t boot_record_size = sizeof(boot_record);

    int32_t ntfs_size = data_size + bitmap_size + mft_size + boot_record_size;

    /* ------------------------------------------- */
    /* VYPOCET POCATECNICH ADRES JEDNOTLIVYCH CASTI NTFS  */
    int32_t mft_start = boot_record_size;
    int32_t bitmap_start = boot_record_size + mft_size;
    int32_t data_start = boot_record_size + mft_size + bitmap_size;

    return create_boot_record("viteja", "KIV/ZOS - PSEUDO NTFS - JAKUB VITEK - ZS 2018", ntfs_size,
            MAX_CLUSTER_SIZE, MAX_CLUSTER_COUNT, mft_start, bitmap_start, data_start,  MFT_FRAGMENTS_COUNT);

}

/**
 * Na zaklade vstupnich pararametru vytvori strukturu mft_item a vrati na ni ukazatel
 *
 * @param uid unikatni ID zaznamu
 * @param isDirectory zda se jedna o slozku ci ne (pripadne slink)
 * @param item_order poradi zaznamu pri vice zaznamech pro jeden soubor
 * @param item_order_total celkovy pocet zaznamu pro jeden soubor
 * @param item_name nazev souboru
 * @param item_size velikost souboru
 * @return vytvorena struktura mft_item
 */
mft_item *create_mft_item(int32_t uid, bool isDirectory, int8_t item_order, int8_t item_order_total,
                          char item_name[12], int32_t item_size)
{
    // Deklarace ukazatele
    mft_item *pointer = NULL;

    // Alokace pameti pro mft_item
    pointer = malloc(sizeof(struct mft_item));

    // Vyplneni struktury daty
    pointer->uid = uid;
    pointer->isDirectory = isDirectory;
    pointer->item_order = item_order;
    pointer->item_order_total = item_order_total;
    pointer->item_size = item_size;

    strcpy(pointer->item_name, item_name);
    pointer->item_name[12] = '\0';

    // Navrat ukazatele
    return pointer;
}

/**
 * Vytvori strukturu mft_fragment a vrati na ni ukazatel
 *
 * @param fragment_start_adress pocatecni adresa fragmentu (casti souboru)
 * @param fragment_count souvisly pocet clusteru, ve kterych je fragment
 * @return ukazatel na strukturu mft_fragment
 */
mft_fragment *create_mft_fragment(int32_t fragment_start_adress, int32_t fragment_count)
{
    // Deklarace ukazatele
    mft_fragment *pointer = NULL;

    // Alokace pameti
    pointer = malloc(sizeof(mft_fragment));

    // Vyplneni struktury daty
    pointer->fragment_count = fragment_count;
    pointer->fragment_start_address = fragment_start_adress;

    // Navrat ukazatele
    return pointer;
}


/**
 *  Vypise formatovany stav struktury boot_record
 *
 * @param record ukazatel na strukturu boot_record
 */
void print_boot_record(boot_record *pointer)
{
    printf("Vypisuji boot record:\n");

    if(!pointer)
    {
        printf("BOOT RECORD je prazdny\n");
        return;
    }

    printf("Signature: %s\n", pointer->signature);
    printf("Volume descriptor: %s\n", pointer->volume_descriptor);
    printf("Disk size: %dB\n", pointer->disk_size);
    printf("Cluster size: %dB\n", pointer->cluster_size);
    printf("Cluster count: %d\n", pointer->cluster_count);
    printf("MFT start adress: %d\n", pointer->mft_start_address);
    printf("Bitmap start adress: %d\n", pointer->bitmap_start_address);
    printf("Data start adress: %d\n", pointer->data_start_address);
    printf("MFT fragment count: %d\n", pointer->mft_max_fragment_count);

}


/**
 *  Na zaklade ukazatele vztazeneho k boot recordu vypise bitmapu
 *
 * @param bitmap ukazatel na bitmapu v pameti
 * @param record ukazatel na pouzity boot
 */
void print_bitmap(int *bitmap, boot_record *record) {
    if (record == NULL || bitmap == NULL) {
        // Nelze vypsat bitmapu
        printf("ERROR: Nelze vypsat bitmapu!\n");
        return;
    }

    for (int i = 0; i < record->cluster_count; i++) {
        printf("%d ", bitmap[i]);
    }
    printf("\n");
}

/**
 * Vypise pole struktur mft_item
 *
 * @param array ukazatel na pole
 * @param size velikost pole
 */
void print_mft_items(mft_item *array, int size)
{
    int zero_uid = 0;

    for(int i = 0; i < size; i++)
    {
        mft_item itm = array[i];

        // Vypocet poctu prazdnych mft itemu
        if(itm.uid == 0)
        {
            zero_uid = zero_uid + 1;
            continue;
        }

        print_mft_item(array[i]);
        printf("***\n");
    }
    // vypis poctu prazdnych mft itemu
    printf("EMPTY MFT ITEMS COUNT: %d\n", zero_uid);
    printf("***\n");
}

/**
 * Vypise jeden mft_item
 *
 * @param item ukazatel na mft item
 */
void print_mft_item(mft_item item)
{
    int empty_fragments = 0;

    printf("UID: %d\n", item.uid);
    printf("DIRECTORY: %d\n", item.isDirectory);
    printf("ITEM ORDER: %d\n", item.item_order);
    printf("ITEM ORDER TOTAL: %d\n", item.item_order_total);
    printf("ITEM NAME: %s\n", item.item_name);
    printf("ITEM SIZE %d\n", item.item_size);

    printf("**\n");
    for(int i = 0; i < MFT_FRAGMENTS_COUNT; i++)
    {
        mft_fragment frag = item.fragments[i];

        // Vypocet prazdnych fragmentu pro redukci textu
        if(frag.fragment_count == -1 && frag.fragment_start_address == -1 )
        {
            empty_fragments = empty_fragments + 1;
            continue;
        }

        print_mft_fragment(item.fragments[i]);
        printf("*\n");
    }
    // Vypis poctu prazdnych fragmentu
    printf("EMPTY FRAGMENTS COUNT: %d\n", empty_fragments);
    printf("*\n");
}

/**
 * Vypise jeden mft fragment
 *
 * @param fragment
 */
void print_mft_fragment(mft_fragment fragment)
{
    printf("FRAGMENT START ADRESS: %d\n", fragment.fragment_start_address);
    printf("FRAGMENT COUNT: %d\n", fragment.fragment_count);

}