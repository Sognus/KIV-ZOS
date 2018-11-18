#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>

#include "structure.h"
#include "ntfs.h"
#include "ntfs_logic.h"
#include "shell.h"

/*
 * TODO: Implementace commandu ve virtual shellu
 *
 *      TODO: Ukladani slozek.
 *          na miste dat budou UID jednotlivych MFT itemu
 *          prvni hodnota bude vzdy UID aktualni slozky
 *          druha hodnota bude UID slozky nadrazene (pro root bude nadrazena slozka root)
 *
 *      TODO: command ls
 *          Z MFT fragmentu se prectou ciselna data
 *          na zaklade ciselnych dat (UID) se prectou nazvy jednotlivych souboru a slozek
 *          nazvy jsou ulozene v MFT_ITEM pod polozkou item_name
 *      TODO: Path
 *          vzdy se cesta bude buildit odzadu rekurzivne
 *          najde se posledni slozka/soubor v ceste (tzn. pro cestu /slozka1/slozka2/file1.txt se bude hledat mft item s nazvem file1.txt)
 *          pokud je posledni cast cesty soubor odrizne se z cesty a precte se nazev slozky (tzn. z  /slozka1/slozka2/file.txt se stane /slozka1/slozka2)
 *          nasledne se dle uid rodice slozky rekurzivne projde az k rootu / (root ma stejneho rodice jako svoje id)
 *          v ramci rekurzivniho prochazeni se bude ukladat nazvy slozek (+ nazev souboru) - do linked list (nebo stack) a na zaklade ulozeneho textu se slozi cesta
 *
 *          je nutne overovat zdali cesta existuje
 *          pri incp neni nutne aby soubor ve FS existoval, musi existovat jen slozky
 *      TODO: mkdir
 *          v dane slozce vytvori podslozku
 *          cely fs jsou podslozky
 *          jedina slozka je root
 *
 *          overi se cesta (zda existuje rodic)
 *          u overeni cesty nemusi existovat vytvarena slozka
 *
 *          zapise se mft item, vytvori se mft fragment
 *          do fragmentu se zapise uid aktualniho souboru a uid rodice (dle cesty
 *
 *
 *
 *
 */

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
    printf("\n\n\n\n\n");

    // SIMULACE SHELLU

    shell *sh = create_root_shell(record, argv[1]);
    print_shell(sh);
    printf("\n");
    mft_item *root = find_mft_item_by_uid(sh, 1);
    print_mft_item(*root);
    printf("IS ROOT FOLDER: %d\n", is_folder(sh,1));
    printf("IS ROOT FILE: %d\n", is_file(sh,1));
    printf("IS ROOT SYMLINK: %d\n", is_symlink(sh,1));

    printf("\n\n");

    // Pocet polozek ve slozce
    printf("ROOT NALEZENA UID: %d\n", get_folder_members_count(sh, 1));

    // IDCKA
    int *uids = NULL;
    int uid_count = -1;
    get_folder_members(sh, 1, &uids, &uid_count);
    // VYPIS IDECEK
    printf("VYPIS IDECEK ROOTU:\n");
    for(int i = 0; i < uid_count; i++)
    {
        printf("UID: %d\n", uids[i]);
    }

    printf("NTFS END!\n");
    return 0;
}