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
    memset(path, 0, path_t);

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

        // Zmena znaku separatoru na string zakonceny null terminatorem
        char separator_string[2] = {PATH_SEPARATOR, '\0'};


        if(contains_name->uid > 1) {
            strcat(path, separator_string);
            //printf("%s", PATH_SEPARATOR);
        }
    }
    //printf("\n");

    return path;


}

/**
 * Na zaklade kontextu shell overi zda cesta urcena vstupem
 * path existuje v ramci VFS
 *
 * @param shell
 * @param path
 * @return err (-inf,-1>, {0, 1}, ostatni wtf
 */
int path_exist(shell *shell, char *path)
{
    // Detekce validity kontextu
    if(shell == NULL)
    {
        // -1 = Kontext nesmi byt null
        return -1;

    }

    // Overeni delky cesty
    if(path == NULL || strlen(path) < 1)
    {
        // -2 = Ukazatel je NULL nebo je retezec moc kratky
        return -2;
    }

    int path_len = strlen(path);
    printf("PATH: %s \n", path);

    /*
     * Implementovany syntax cesty
     * absolutni
     *  /dir1/dir2/file
     *  /dir1/dir2/file/
     *  /dir1/dir2/dir3/
     *  /dir1/dir2/dir3
     * relativni
     *  dir1/dir2/dir3
     *  .././dir
     *  ../../
     *
     */

    // Pokud cesta zacina / je vzdy absolutni
    if(path[0] == '/')
    {
        // Nacitam znaky tak dlouho dokud nenarazim na dalsi / ci konec retezce
        char buffer[PATH_PART_MAX_LENGTH];
        int buffer_used = 0;
        memset(buffer, 0, PATH_PART_MAX_LENGTH);

        // Current operation directory (1 = ROOT)
        int cod = NTFS_ROOT_UID;
        // Urcuje zda byl znak separatoru nalezen ci ne
        int first_separator = 0;

        for(int i = 0 ; i < strlen(path)+1; i++) {
            // Aktualni znak
            char current_char = i == strlen(path) ? path[i-1] : path[i];

            // Pridani prvniho lomitka do bufferu - pro overeni rootu
            if(i == 0 || (i == path_len-1 && current_char != '/'))
            {
                buffer[buffer_used] = current_char;
                buffer_used++;
                first_separator = 1;
            }

            // Jsme na poslednim znaku nebo jsme nalezli jsme dalsi cestu
            if (i == path_len-1 || current_char == PATH_SEPARATOR) {
                // Zpracovani retezce - nalezneme vsechny MFT itemy v COD
                int *uids = NULL;
                int uid_count = -1;

                get_folder_members(shell, cod, &uids, &uid_count);

                // Overeni zpracovani podslozek/souboru ve slozce
                if(uids == NULL || uid_count < 1)
                {
                    // Nebyla nalezena podslozka/soubor -> tj. cesta neexistuje
                    return 0;
                }

                // Prochazime vsechny idcka - hledame jmeno
                int name_found = 0;
                for(int a = 0; a < uid_count; a++)
                {
                    // Nalezeni MFT zaznamu pro UID
                    mft_item *item = find_mft_item_by_uid(shell, uids[a]);

                    if(item == NULL)
                    {
                        // Nebyl nalezen MFT zaznam pro podslozku
                        return -3;
                    }

                    //printf("ITEM_NAME: %s VS ", item->item_name);
                    //printf("BUFFER: %s ", buffer);
                    //printf("(CMP: %d)\n", strcmp(item->item_name, buffer));

                    // Overeni jmena slozky/souboru
                    if(strcmp(item->item_name, buffer) == 0)
                    {
                        name_found = 1;
                        cod = item->uid;

                        // Vymazani pouziteho bufferu po nalezeni
                        memset(buffer, 0, PATH_PART_MAX_LENGTH);
                        buffer_used = 0;

                        break;
                    }
                }

                // Overeni zda byl item nalezen
                if(name_found == 0)
                {
                    // Nebyl -> tj. cesta neexistuje
                    return 0;
                }

                // Vymazani pouziteho bufferu
                memset(buffer, 0, PATH_PART_MAX_LENGTH);
                buffer_used = 0;
            }

            // Osetreni posledniho znaku
            if(i == path_len - 1)
            {
                break;
            }

            // Pridani znaku do bufferu - krome separatoru
            if(current_char != PATH_SEPARATOR) {
                buffer[buffer_used] = current_char;
                buffer_used++;
            }

        }
        // Vsechno v ceste bylo nalezeno -> tj. cesta existuje
        return 1;

    }
    else // Jinak je cesta relativni
    {
        // TODO: Overeni relativni cesty
        /*
         * TODO: nacitat pocet ..
         * TODO: ignorovat ./
         * TODO: prochazet FS od CWD->COD misto od rootu
         * TODO: .. prepne COD na nadrazenou slozku (pokud se narazi na roota, zustava v rootu - parent rootu je root)
         *
         */
        return 20;
    }

}