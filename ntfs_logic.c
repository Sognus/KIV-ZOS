#include "ntfs_logic.h"
#include "ntfs.h"
#include <stdlib.h>

/**
 * Vrati jestli se jedna o slozku
 *
 * @param shell kontext
 * @param uid identifikator mft itemu
 * @return 0 = ne, 1 = ano, -1 = chyba
 */
int is_folder(shell *shell, int32_t uid)
{
    // TODO: jedna se o symlink na directory?

    // Chybny vstup
    if(shell == NULL || uid < 0)
    {
        return  -1;
    }

    mft_item *item = find_mft_item_by_uid(shell,uid);
    int rtm = item->isDirectory == 1;

    // TODO: uvolneni pameti


    return  rtm;
}

/**
 * Vrati jestli se jedna o soubor
 *
 * @param shell kontext
 * @param uid identifikator mft itemu
 * @return 0 = ne, 1 = ano, -1 = chyba
 */
int is_file(shell *shell, int32_t uid)
{
    // Chybny vstup
    if(shell == NULL || uid < 0)
    {
        return  -1;
    }

    // TODO: uvolneni pameti
    mft_item *item = find_mft_item_by_uid(shell,uid);
    int rtm = item->isDirectory == 0;

    return  rtm;

}

/**
 * Vrati zda se jedna o symlink
 *
 * @param shell kontext
 * @param uid identifikator mft itemu
 * @return 0 = ne, 1 = ano, -1 = chyba
 */
int is_symlink(shell *shell, int32_t uid)
{
    // Chybny vstup
    if(shell == NULL || uid < 0)
    {
        return  -1;
    }

    mft_item *item = find_mft_item_by_uid(shell,uid);
    int rtm = item->isDirectory == 2;

    // TODO: uvolneni pameti

    return  rtm;

}

/**
 * Vrati mft item na zaklade uid
 *
 * @param shell kontext
 * @param uid identifikator mft itemu
 * @return null když neexistuje, struct když existuje
 */
mft_item *find_mft_item_by_uid(shell *shell, int32_t uid)
{
    if(shell == NULL)
    {
        return  NULL;
    }

    // Iterace danymi mft itemy pro ziskani struktury
    for(int i = 0; i < shell->mft_array_size; i++)
    {
        mft_item item = shell->mft_array[i];

        // ID je stejne
        if(item.uid == uid) {
            return &shell->mft_array[i];
        }

    }

    return NULL;




}

/**
 * Vytvori novou slozku ve FS na zaklade current working directory
 *
 * @param shell
 * @param folder_name
 * @return
 */
int create_folder(shell *shell, char folder_name[12])
{
    // Overeni shellu
    if(shell == NULL)
    {
        return -1;
    }

    // TODO: implementace
    return -1;




}

/**
 * Vrati 1 pokud ma current working directory soubor/slozku/symlink s danym jmenem
 * Vrati 0 pokud ^ ne
 *
 * Vrati -1 pokud nastane chyba
 *
 *
 * @return viz popis
 */
int cwd_has_item_name(shell *shell, char item_name[13])
{
    //TODO: Implementace
    return -1;
}

/**
 * Vrati identifikatory souboru/slozek/symlinku ktere jsou ve slozce current directory
 *
 * @param shell
 * @param uid
 */
int get_folder_members_count(shell *shell, int32_t uid)
{
    // Overeni shellu
    if(shell == NULL)
    {
        return -1;
    }

    // TODO: or is symlink to folder
    if(!is_folder(shell, shell->cwd))
    {
        return -2;
    }

    // Ziskani mft itemu
    mft_item *current_folder = find_mft_item_by_uid(shell, shell->cwd);

    // Overeni itemu
    if(current_folder == NULL)
    {
        return -3;
    }

    // Otevreni souboru pro precteni
    FILE *file = fopen(shell->filename, "rb");

    // Overeni otevreni
    if(file == NULL)
    {
        return -4;
    }

    // Count of items
    int estimated_count = 0;

    // Projiti mft fragmentu pro cteni dat
    for(int i = 0; i < MFT_FRAGMENTS_COUNT; i++)
    {
        mft_fragment frag = current_folder->fragments[i];

        // Zbytecny cist, kdyz je adresa zaporna
        if(frag.fragment_count == -1 && frag.fragment_start_address == -1)
        {
            continue;
        }

        int32_t start_addr = frag.fragment_start_address;
        int32_t end_addr = frag.fragment_start_address + MAX_CLUSTER_SIZE * frag.fragment_count;
        int32_t current_addr = start_addr;

        // Zjisteni poctu itemu
        while(current_addr < end_addr)
        {
            // Priprava pro fread
            int *read_uid = malloc(sizeof(int32_t));

            // Seek set
            fseek(file, current_addr, SEEK_SET);

            // Precteni data
            fread(read_uid, sizeof(int32_t), 1, file);

            //printf("DEBUG IDčko %d\n", *read_uid);

            // Jestlize mame uid vetsi jak 0 mame uid clenu current slozky
            if(*read_uid > 0)
            {
                // Inkrementace o jeden
                estimated_count = estimated_count + 1;
            }

            // Uklid po fread
            free(read_uid);

            // Posun po 4 bytes
            current_addr = current_addr + sizeof(int32_t);
        }



    }

    // Uzavreni souboru
    fclose(file);

    return estimated_count;



}

/**
 * Upraví ukazetele out proměnný tak, aby jejich obsah bylo pole vsech uid v dane slozce
 * out_count pak predstavuje pocet polozek v danem poli
 *
 * @param shell kontext
 * @param uid uid prohledavane slozky
 * @param out_array ukazatel na ukazatel pole kam se budou ukladat polozky pole
 * @param out_count
 */
void get_folder_members(shell *shell, int32_t uid, int **out_array, int *out_count)
{
    // Overeni shellu
    if(shell == NULL)
    {
        return;
    }

    // TODO: or is symlink to folder
    if(!is_folder(shell, shell->cwd))
    {
        return;
    }

    // Zjisteni poctu UID
    int array_size = get_folder_members_count(shell, uid);

    // Overeni zda se neco nerozbilo
    if(array_size < 1)
    {
        // Rozbilo se neco - podle navratove hodnoty - 0 = blbost, slozka nemuze mit 0 UID v sobe
        return;
    }

    // Vytvoreni pole o zjistene velikosti
    int *array = malloc(array_size * sizeof(int32_t));

    // Ziskani mft itemu
    mft_item *current_folder = find_mft_item_by_uid(shell, shell->cwd);

    // Overeni itemu
    if(current_folder == NULL)
    {
        return;
    }

    // Otevreni souboru pro precteni
    FILE *file = fopen(shell->filename, "rb");

    // Overeni otevreni
    if(file == NULL)
    {
        return;
    }

    // Count of items
    int estimated_count = 0;

    // Projiti mft fragmentu pro cteni dat
    for(int i = 0; i < MFT_FRAGMENTS_COUNT; i++)
    {
        mft_fragment frag = current_folder->fragments[i];

        // Zbytecny cist, kdyz je adresa zaporna
        if(frag.fragment_count == -1 && frag.fragment_start_address == -1)
        {
            continue;
        }

        int32_t start_addr = frag.fragment_start_address;
        int32_t end_addr = frag.fragment_start_address + MAX_CLUSTER_SIZE * frag.fragment_count;
        int32_t current_addr = start_addr;

        // Zjisteni poctu itemu
        while(current_addr < end_addr)
        {
            // Priprava pro fread
            int *read_uid = malloc(sizeof(int32_t));

            // Seek set
            fseek(file, current_addr, SEEK_SET);

            // Precteni data
            fread(read_uid, sizeof(int32_t), 1, file);

            // Ulozeni hodnoty do pole
            array[estimated_count] = *read_uid;

            // Jestlize mame uid vetsi jak 0 mame uid clenu current slozky
            if(*read_uid > 0)
            {
                // Inkrementace o jeden
                estimated_count = estimated_count + 1;
            }

            // Uklid po fread
            free(read_uid);

            // Posun po 4 bytes
            current_addr = current_addr + sizeof(int32_t);
        }



    }

    // Uprava out promennych
    *out_array = array;
    *out_count = estimated_count;

    // Uzavreni souboru
    fclose(file);


}



