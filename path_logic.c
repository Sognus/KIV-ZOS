#include "path_logic.h"

/**
 * Na zaklade aktualniho kontextu shell vytvori retezec
 * obsahujici textovou reprezentaci absolutni cesty pro
 * current working directory
 *
 * @param shell aktualni kontext
 * @return ukazatel na retezec
 */
char *get_current_path(shell *shell)
{
    /*
     * 1) Zacneme v cwd - ziskame jeji nazev a pridame velikost nazvu do int strlen,
     * 2) precteme prvni dva int32_t datove casti cwd (2 * 4bytes)
     * 3) pokud jsou obe cisla stejna, jsme v rootu, ukoncujeme prohledavani
     * 4) pokud jsou  rozdilne, vyhledame mft_item dle druheho cisla
     * 5) pridame do cesty oddělovač složek a jeho velikost do int strlen
     * 6) z mft_itemu ziskame nazev slozky a pridame velikost do strlen
     * 7) Dle itemu precteme z datove casti prvni dva int32_t (pokračujeme krokem 2)
     *
     * Cesta je omezena na urcity pocet casti (256) id defaultne
    */

    if(shell == NULL)
    {
        return NULL;
    }

    // Vypocet velikosti cesty a zjisteni vsech UID v ceste
    int path_ids[PATH_PARTS_LIMITATION] = { 0 };
    int path_parts = 0;
    int path_strlen = 0;

    // Current operation directory - urcuje slozku se kterou se nyni pracuje
    int32_t cod = shell->cwd;

    // Priprava souboru
    FILE *file = fopen(shell->filename, "rb");

    // Otevreni souboru
    if(file == NULL)
    {
        // Nepodarilo se otevrit soubor
        return NULL;
    }

    // Delam tak dlouho dokud nenarazim na ukoncovaci podminku
    while(1)
    {
        // Najdu MFT zaznam current operation directory
        mft_item *item = find_mft_item_by_uid(shell, cod);

        // Zvetseni velikosti cesty
        path_strlen = path_strlen + strlen(item->item_name);
        // IDcka se ukladaji inverzne - 0 se preskoci a vypisi se jen cisla vetsi nebo rovna 1
        path_ids[(PATH_PARTS_LIMITATION - 1 - path_parts)] = cod;

        // Pocet pouzitych casti pole
        path_parts = path_parts + 1;

        // Overeni preteceni delky cesty (dle casti)
        if(path_parts > PATH_PARTS_LIMITATION)
        {
            // Cesta ma prilis mnoho casti
            return NULL;
        }

        // Paranoia = overim existenci zaznamu
        if(item == NULL || item->uid < 1)
        {
            // Operace nemohla byt vykonana - neexistuje zaznam pro slozku nebo se to proste nejak rozbilo
            return NULL;
        }

        // Ziskani prvniho fragmentu current operation directory
        mft_fragment fragment = item->fragments[0];

        // Deklarace mista na ulozeni
        int32_t *folder_meta_uids = malloc(2 * sizeof(int32_t));

        // 0 = Current UID
        folder_meta_uids[0] = -1;
        // 1 = Parrent UID
        folder_meta_uids[1] = -1;

        // Precteni souboru
        fseek(file, fragment.fragment_start_address, SEEK_SET);
        fread(folder_meta_uids, sizeof(int32_t), 2, file);

        // Overeni precteni
        if(folder_meta_uids[0] == -1 || folder_meta_uids[1] == -1)
        {
            // Nelze precist ID, ktere rozhodne neexistuje
            return NULL;
        }

        // Zastaveni algoritmu v pripade, ze se dosahlo root slozky
        if(cod == NTFS_ROOT_UID)
        {
            // Dosahli jsme root slozky
            break;
        }

        // Overeni existence rodicovske slozky
        mft_item *parrent = find_mft_item_by_uid(shell, folder_meta_uids[1]);

        if(parrent == NULL || parrent->uid < 1)
        {
            // Zaznam neexistuje nebo je prazdny
            return NULL;
        }

        // Jsme mimo root, musime prohledavat dal v nadrazene slozce
        cod = folder_meta_uids[1];

        // Pridame delku delimiteru do velikosti cesty
        path_strlen = path_strlen + PATH_SEPARATOR_LENGTH;


        // Uvolneni pameti
        free(folder_meta_uids);
    }


    int32_t path_t = (sizeof(char) * path_strlen);
    char *path = malloc(path_t);
    memset(path, '\0', path_t);

    // Prozatim debug vypis
    for(int i = 0; i < PATH_PARTS_LIMITATION; i++)
    {
        if(path_ids[i] < 1)
        {
            continue;
        }

        mft_item *contains_name = find_mft_item_by_uid(shell, path_ids[i]);

        //printf("%s", contains_name->item_name);
        strcat(path, contains_name->item_name);

        if(contains_name->uid > 1) {
            strcat(path, PATH_SEPARATOR);
            //printf("%s", PATH_SEPARATOR);
        }
    }
    //printf("\n");

    return path;


}