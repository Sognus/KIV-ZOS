#include "ntfs.h"
#include "structure.h"

// POZN: MFT je 10% data size

/*
 * Vytvori soubor s danym pseudo NTFS v pripade, ze neexistuje
 *
 * filename         - nazev souboru
 * cluster_count    - maximalni pocet clusteru pro data
 * cluster_size     - maximalni velikost jednoho clusteru v Byte
 */
void create_file(char filename[], int32_t cluster_count, int32_t cluster_size)
{
    // Deklarace promennych
    FILE *file;
    int bitmap[cluster_count];

    if(file_exists(filename))
    {
        // Nebudeme vytvaret soubor, ktery existuje
        printf("ERROR: Nelze prepsat soubor, ktery jiz existuje!\n");
        return;
    }

    file = fopen(filename, "wb");

    // Paranoia je mocna - radsi zkontrolovat otevreni souboru
    if(file == NULL)
    {
        printf("ERROR: Soubor %s nelze otevrit/vytvorit!\n", filename);
        return;
    }

    /* VYTVORENI A ZAPIS BOOT RECORDU */

    // Vytvoreni standardniho boot recordu
    boot_record *record = create_standard_boot_record();
    // Zapis boot_record do souboru
    fwrite(record, sizeof(boot_record), 1, file);

    /* ----------------------------------------------------------- */
    /* ZAPIS BITMAPY  */

    // Posun na zacatek oblasti pro bitmapu
    fseek(file, record->bitmap_start_address, SEEK_SET);
    // Prvni cluster je využit rootem
    bitmap[0] = true;
    // Vytvoreni prazdne bitmapy
    for(int i = 1; i < cluster_count; i++)
    {
        bitmap[i] = false;
    }
    // Zapis bitmapy
    fwrite(bitmap, 4, cluster_count, file);

    /* --------------------------------------------------------- */
    /* VYTVORENI MFT PRO ROOT */

    // Posun na misto v souboru, kde zacina MFT
    fseek(file, record->mft_start_address, SEEK_SET);

    // Vytvoreni mft itemu
    mft_item *mfti = create_mft_item(1, 1, 1, 1, "/\0", 1);

    // Vytvoreni prvniho fragmentu
    mfti->fragments[0].fragment_start_address = record->data_start_address;
    mfti->fragments[0].fragment_count = 1;

    for(int i = 1; i < MFT_FRAGMENTS_COUNT; i++)
    {
        mfti->fragments[i].fragment_start_address = -1;
        mfti->fragments[i].fragment_count = -1;
    }

    // Zapis mft_item do soubooru
    fwrite(mfti, sizeof(mft_item), 1, file);

    // Zapis mft fragmentu do souboru
    // TODO: Zapisuje se struktura rekurzivne do souboru nebo je nutne prilepit fragmenty za mft_item?
    /*
    for(int i = 0 ; i < MFT_FRAGMENTS_COUNT; i++)
    {
        mft_fragment *fragment = &mfti->fragments[i];
        fwrite(fragment, sizeof(mft_fragment), 1, file);
    }
    */

    /* ---------------------------------------------------------------------------------------------------- */
    /* ZAPIS ROOTU A PRAZDNEHO MISTA  */
    fseek(file, record->data_start_address, SEEK_SET);
    char odkaz[2];
    odkaz[0] = '0';
    odkaz[1] = '\0';
    fwrite(odkaz, 1, 2, file);

    int32_t used_free = 2;
    int32_t data_size = MAX_CLUSTER_COUNT * MAX_CLUSTER_SIZE;

    while(used_free < data_size)
    {
        fputc('\0', file);
        used_free = used_free + 1;
    }


    // Uvolneni pameti
    free(record);
    free(mfti);
    fclose(file);
}

/**
 * Na zaklade vstupniho parametru overi, zda soubor s danym jmenem existuje
 *
 * @param file_name nazev/cesta souboru k overeni
 * @return existence souboru (0 - neexistuje, 1 existuje)
 */
bool file_exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

