#include "ntfs_logic.h"
#include "ntfs.h"
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <structure.h>

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
 * Backend pro mkdir
 *
 * TODO: vytvorit implementaci pro path misto currect working directory
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

    // TODO: Existuje soubor/slozka/symlink s danym nazvem = pokud ano nelze jej vytvorit

    // Deklarace hodnot pro adresu clusteru a index bitmapy
    int32_t free_cluster_addr = -1;
    int32_t free_cluster_index = -1;

    // Nalezeni clusteru
    int operation_result = find_empty_cluster(shell, &free_cluster_addr, &free_cluster_index);

    // Overeni na chybu < 0 => error
    if(operation_result < 0)
    {
        // Find cluster operation error
        return -3;
    }

    // Overeni na notice => navrat notice
    if(operation_result > 0)
    {
        // 1 = No operation - nelze pokracovat ale neni to chyba
        // 2 = Zadny cluster neni volny
        return operation_result;
    }

    // Overeni adresy a inedexu
    if(free_cluster_addr < 0 || free_cluster_index < 0)
    {
        // -4 = spatne hodnoty pro vytvoreni slozky
        return -4;
    }

    // Nyni muzeme vyrobit dany MFT zaznam a zapsat ho na danou adresu
    int32_t uid = get_next_uid(shell);
    bool isDirectory = 1;
    int8_t item_order = 1;
    int8_t item_order_total = 1;
    int32_t item_size = 8;              // 4bytes = current folder uid, 4 bytes = parent folder uid
    mft_fragment *frag = create_mft_fragment(free_cluster_addr, 1);

    // Vytvoreni mft itemu
    mft_item *created_mft_item = create_mft_item(uid, isDirectory, item_order, item_order_total, folder_name, item_size);

    // Aktualizace mft fragmentu
    created_mft_item->fragments[0] = *frag;

    // Aktualizace ostatnich mft fragmentu
    for(int j = 1; j < MFT_FRAGMENTS_COUNT; j++)
    {
        created_mft_item->fragments[j].fragment_start_address = -1;
        created_mft_item->fragments[j].fragment_count = -1;
    }

    // Vypocet adresy volneho mft itemu
    int index = get_free_mft_item_index(shell);

    // Overeni existence volneho mft itemu
    if(index < 0)
    {
        // -5 = Neni volny MFT item
        return -5;
    }


    // Na tuto adresu zapiseme mft item
    int32_t mft_addr = shell->boot->mft_start_address + (sizeof(mft_item) * index);

    // Pridat UID do parent slozky
    parrent_add_uid(shell, shell->cwd, uid);

    // Precteni bitmapy - zapis #1
    int *bitmap = read_bitmap(shell->filename, shell->boot);
    bitmap[index] = 1;

    // Zapis slozky do current working directory
    FILE *file = fopen(shell->filename, "r+b");
    fseek(file, mft_addr, SEEK_SET);
    fwrite(created_mft_item, sizeof(mft_item), 1, file);

    // Zapis aktualizovane bitmapy do souboru
    fseek(file, shell->boot->bitmap_start_address, SEEK_SET);
    fwrite(bitmap, sizeof(int32_t), shell->boot->cluster_count, file);

    // Aktualizace boot recordu
    shell->mft_array[index] = *created_mft_item;

    // ZAPIS obsahu složky
    // AKTUAlNI UID, PARENT UID
    fseek(file, free_cluster_addr, SEEK_SET);
    int32_t odkaz[2];
    // Prvni dve polozky jsou (1) aktualni slozka (2) nadrazena slozka
    odkaz[0] = uid;
    odkaz[1] = shell->cwd;
    fwrite(odkaz, sizeof(int32_t), 2, file);

    // TODO: mozna jeste neco dunno

    // Uklid
    fclose(file);
    free(frag);
    free(created_mft_item);

    // TODO: implementace
    return 0;




}

/**
 * Prida fyzicky zaznam o souboru/slozce/symlinku do nadrazene slozky
 *
 * @param shell kontext
 * @param parent_uid uid nadrazene slozky
 * @param add_uid pridavane uid
 * @return chyba nebo uspech ( = 0 )
 *
 */
int parrent_add_uid(shell *shell, int32_t parent_uid, int32_t add_uid)
{
    // Overeni shellu
    if(shell == NULL)
    {
        // -1 = Kontext neni nastaven
        return -1;
    }

    // Overeni existence rodice
    mft_item *parent = find_mft_item_by_uid(shell, parent_uid);

    if(parent == NULL){
        // -2 = Neexistuje cilova slozka
        return -2;
    }

    int32_t cluster_size = MAX_CLUSTER_SIZE;
    int32_t size_needed = sizeof(int32_t) * (get_folder_members_count(shell, parent_uid) + 1) ;

    double clusters_needed_help = (double)size_needed / (double)cluster_size;
    int32_t clusters_needed = (int32_t)(ceil(clusters_needed_help));

    int32_t allocated_clusters = get_allocated_cluster_count(parent);
    int32_t free_space_in_cluster = size_needed % cluster_size;         // kolik by melo byt mista v poslednim clusteru

    // Priprava na vypocet adresy
    int32_t uid_write_addr = -1;


    // Clusteru je dost
    if(clusters_needed <= allocated_clusters || free_space_in_cluster < sizeof(int32_t))
    {
        uid_write_addr = get_folder_next_member_adress(shell, parent_uid);

    }

    // Alokace dalšího clusteru pro složku? Please no
    if(uid_write_addr < 0)
    {
        // -3 = Not implemented and never will be
        return -3;
    }

    FILE *file = fopen(shell->filename, "r+b");

    if(file == NULL)
    {
        // -4 = Soubor je derpnutej nebo neni
        return -4;
    }



    // Nastaveni ukazatele
    fseek(file, uid_write_addr, SEEK_SET);

    // Zapis uid na adresu
    fwrite((const void*)&add_uid, (size_t)sizeof(int32_t), 1, file);

    // Uzavreni souboru
    fclose(file);


}

/**
 * Nalezne index volneho mft itemu, pokud neexistuje, hodi -2
 *
 * @param shell
 * @return index volneho mft itemu nebo chyba
 */
int get_free_mft_item_index(shell *shell)
{
    // Overeni shellu
    if(shell == NULL)
    {
        return -1;
    }

    for(int i = 0; i < shell->mft_array_size; i++)
    {
        mft_item item = shell->mft_array[i];

        if(item.uid == 0)
        {
            // Nalezen volny mft item na indexu i
            return i;
        }
    }

    // Volny mft item nenalezen
    return -2;

}

/**
 * Projde MFT tabulku, najde dalsi volne uid
 *
 * Nejprve inkrementuje nejvyssi nalezene cislo o 1, v pripade, ze je dosahnuto maximalniho
 * cisla 32bit, pak prochazi cisla od 1 do max cisla
 *
 * @param shell
 * @return
 */
int32_t get_next_uid(shell *shell)
{
    // Overeni shellu
    if(shell == NULL)
    {
        return -1;
    }

    // Precteni mft tabulky
    mft_item *mft_array = NULL;
    int mft_array_size = -1;
    read_mft_items(shell->filename, shell->boot, &mft_array, &mft_array_size);

    // Overeni na chybu ve cteni MFT
    if(mft_array == NULL || mft_array_size < 1)
    {
        // -2 = Nepodarilo se precist MFT zaznam
        return -2;
    }

    // Linearni pruchod MFT - prvni zpusob nalezeni
    int32_t largest = 0;
    for(int i = 0; i < mft_array_size; i++)
    {
        mft_item item = mft_array[i];

        if(item.uid > largest)
        {
            largest = item.uid;
        }
    }

    // Otestovani, jestli iterace pretece
    int32_t a = largest;
    int32_t x = 1;
    if ((x > 0 && x > INT_MAX - x) ||
        (x < 0 && x < INT_MIN - x))
    {
        // Hodnota pretece, musime nalezt jinak volne UID
        // Jinak = sort struktur podle UID, pokud dostaneme sekvenci 1 2 3 4 6 tak vime ze 5 je volne
        // tj. nacteme dve hodnoty a pokud lisi o vice jak 1 tak je zde volne misto
        qsort(mft_array, mft_array_size, sizeof(mft_item) ,compare_mft_items);


        for(int z = 0; z < mft_array_size-1; z++)
        {
            mft_item item1 = mft_array[z];
            mft_item item2 = mft_array[z+1];

            // Nalezena mezera mezi mft itemy - ukoncuji vyhledavani
            if(abs(item1.uid - item2.uid) > 1)
            {
                largest = item1.uid + 1;
                break;
            }
        }


    }
    else
    {
        // Hodnota nepretece, muzeme ji vratit
        largest = largest + 1;
    }

    // Uvolneni pameti
    free(mft_array);

    // Vraceni hodnoty dalsiho indexu
    return largest;

}


/**
 * Porovna dve struktury na zaklade UID pro Qsort
 *
 * @param s1 struktura 1
 * @param s2 struktura 2
 * @return porovnani
 */
int compare_mft_items(const void *s1, const void *s2)
{
    mft_item *e1 = (mft_item *)s1;
    mft_item *e2 = (mft_item *)s2;
    return e1->uid - e2->uid;
}

/**
 *  * Na zaklade aktualniho kontextu shell, najde PRVNI PRAZDNOU adresu a poradi daneho clusteru
 *
 * @param shell kontext funkce
 * @param cluster_addr ukazatel na output int jaka je adresa clusteru volneho
 * @param cluster_order ukazatel na output int kolikaty je volny cluster v bitmape
 * @return chybova hlaska, 0 = bez chyby, <-max, 0) = error, (0, max) = notice
 */
int32_t find_empty_cluster(shell *shell, int32_t *cluster_addr, int *cluster_order)
{
    // return
    int rtn = 0;

    // Overeni shellu
    if(shell == NULL)
    {
        // -1 = neexistuje shell
        return -1;
    }

    // Funkce muze menit jednu hodnotu na out adresach nebo dve. Pokud jsou oba ukazatele null, neudela nic
    if(cluster_addr == NULL && cluster_order == NULL)
    {
        // 1 = nebyla provedena zadna operace
        return 1;
    }

    // Precteni bitmapy
    int *bitmap = read_bitmap(shell->filename, shell->boot);
    int bitmap_size = shell->boot->cluster_count;

    // Iterace pro ziskani poradi
    int iter = 0;
    for(iter = 0; iter < bitmap_size; iter++)
    {
        // Nalezen prvni nulovy cluster
        if(bitmap[iter] == 0)
        {
            break;
        }

    }

    int32_t data_start_addr;
    int32_t first_cluster_empty_addr;

    if(iter == bitmap_size - 1 && bitmap[iter] == 1)
    {
        // Vsechny clustery jsou obsazene
        data_start_addr = -1;
        first_cluster_empty_addr = -1;
        // Return 2 = neexistuje prazdny cluster
        rtn = 2;
    }
    else {
        // vypocet adresy
        data_start_addr = shell->boot->data_start_address;
        first_cluster_empty_addr = data_start_addr + iter * shell->boot->cluster_size;
    }

    // Zajisteni vystupu adresy
    if(cluster_addr != NULL)
    {
        *cluster_addr = first_cluster_empty_addr;
    }

    if(cluster_order != NULL)
    {
        *cluster_order = iter;
    }

    // Navrat infa
    return rtn;
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
    if(!is_folder(shell, uid))
    {
        return -2;
    }

    // Ziskani mft itemu
    mft_item *current_folder = find_mft_item_by_uid(shell, uid);

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

            //printf("DEBUG IDko %d\n", *read_uid);

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
 *  Vrati adresu dalsiho prvku kam zapsat UID
 *
 * @param shell
 * @param uid
 */
int get_folder_next_member_adress(shell *shell, int32_t uid)
{
    // Overeni shellu
    if(shell == NULL)
    {
        return -1;
    }

    // TODO: or is symlink to folder
    if(!is_folder(shell, uid))
    {
        return -2;
    }

    // Ziskani mft itemu
    mft_item *current_folder = find_mft_item_by_uid(shell, uid);

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

            // pokud mame 0, nasli jsme volne misto
            if(*read_uid == 0)
            {
                fclose(file);
                return  current_addr;
            }

            // Posun po 4 bytes
            current_addr = current_addr + sizeof(int32_t);
        }



    }

    // Uzavreni souboru
    fclose(file);

    // -5 = Unknown
    return -5;
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
    if(!is_folder(shell, uid))
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
    mft_item *current_folder = find_mft_item_by_uid(shell, uid);

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

/**
 * Vrati pocet alokovanych clusteru k jednomu mft itemu
 *
 * @param shell
 * @param mft_item
 * @return
 */
int get_allocated_cluster_count(mft_item *mft_item)
{
    if(mft_item == NULL)
    {
        // -1 = Mft item neni nastaveno
        return -1;
    }

    int clusters_so_far = 0;

    // Projiti vsech fragmentu, scitani clusteru
    for(int i = 0; i < MFT_FRAGMENTS_COUNT; i++)
    {
        mft_fragment frag = mft_item->fragments[i];

        if(frag.fragment_count > 0) {
            clusters_so_far += frag.fragment_count;
        }
    }

    return clusters_so_far;


}



