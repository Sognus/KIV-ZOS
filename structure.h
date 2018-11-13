#ifndef KIV_ZOS_STRUCTURE_H
#define KIV_ZOS_STRUCTURE_H

#include <stdbool.h>
#include <stdint.h>

// Znak, ktery bude vyuzit jako vypln prazdneho souboru
#define NTFS_NEUTRAL_CHAR '\0'

// ID MFT_ITEM v priopade, ze je item prazdny
#define UID_ITEM_FREE 0

// Pomer velikosti MFT ku velikosti datove slozky FS
#define MFT_RATIO 0.1

// Maximalni pocet fragmentu v jednom zaznamu MFT
#define MFT_FRAGMENTS_COUNT 32

// Celkovy pocet datovych clusteru ve filesystemu
#define MAX_CLUSTER_COUNT 4096

// Velikost jednoho clusteru v B
#define MAX_CLUSTER_SIZE 4096

typedef struct boot_record {
    char signature[9];              //login autora FS
    char volume_descriptor[251];    //popis vygenerovaného FS
    int32_t disk_size;              //celkova velikost VFS
    int32_t cluster_size;           //velikost clusteru
    int32_t cluster_count;          //pocet clusteru
    int32_t mft_start_address;      //adresa pocatku mft
    int32_t bitmap_start_address;   //adresa pocatku bitmapy
    int32_t data_start_address;     //adresa pocatku datovych bloku
    int32_t mft_max_fragment_count; //maximalni pocet fragmentu v jednom zaznamu v mft (pozor, ne souboru)
    // stejne jako   MFT_FRAGMENTS_COUNT
} boot_record;


typedef struct mft_fragment {
    int32_t fragment_start_address;     //start adresa
    int32_t fragment_count;             //pocet clusteru ve fragmentu
} mft_fragment;


typedef struct mft_item {
    int32_t uid;                                        //UID polozky, pokud UID = UID_ITEM_FREE, je polozka volna
    bool isDirectory;                                   //soubor, nebo adresar (0 = soubor, 1 = složka, 2 = symbolic link)
    int8_t item_order;                                  //poradi v MFT pri vice souborech, jinak 1
    int8_t item_order_total;                            //celkovy pocet polozek v MFT
    char item_name[12];                                 //8+3 + /0 C/C++ ukoncovaci string znak
    int32_t item_size;                                  //velikost souboru v bytech
    mft_fragment fragments[MFT_FRAGMENTS_COUNT];        //fragmenty souboru
} mft_item;



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
                                int32_t data_start_adress, int32_t mft_max_fragment_count);

/**
 * Vytvori standardni boot_record pro KIV/ZOS
 *
 * @return pointer na vytvoreny boot_record
 */
boot_record *create_standard_boot_record();

/**
 *  Vypise formatovany stav struktury boot_record
 *
 * @param record ukazatel na strukturu boot_record
 */
void print_boot_record(boot_record *pointer);

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
        char item_name[12], int32_t item_size);

/**
 * Vytvori strukturu mft_fragment a vrati na ni ukazatel
 *
 * @param fragment_start_adress pocatecni adresa fragmentu (casti souboru)
 * @param fragment_count souvisly pocet clusteru, ve kterych je fragment
 * @return ukazatel na strukturu mft_fragment
 */
mft_fragment *create_mft_fragment(int32_t fragment_start_adress, int32_t fragment_count);

/**
 * Na zaklade vstupnich hodnot prepocita hodnoty v boot zaznamu a zmeni je
 *
 * @param record zaznam ke zmeneni
 * @param cluster_count novy pocet clusteru
 * @param cluster_size velikost jednoho clusteru
 */
void boot_record_resize(boot_record *record, int32_t cluster_count, int32_t cluster_size);

/**
 *  Na zaklade ukazatele vztazeneho k boot recordu vypise bitmapu
 *
 * @param bitmap ukazatel na bitmapu v pameti
 * @param record ukazatel na pouzity boot
 */
void print_bitmap(int *bitmap, boot_record *record);

#endif //KIV_ZOS_STRUCTURE_H
