#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "structure.h"
#include "ntfs.h"
#include "ntfs_logic.h"
#include "shell.h"
#include "path_logic.h"

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
}

int test_folder(int argc, char *argv[])
{
    // Deklarace
    int *uids = NULL;
    int uid_count = -1;

    if(argc < 2)
    {
        printf("ERROR: Program was started without arguments!");
        return -1;
    }

    printf("NTFS START!\n");
    printf("\n");

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

    // Vytvoreni SHELLU
    printf("CREATING SHELL!\n");
    shell *sh = create_root_shell(record, argv[1]);

    // IDCKA
    get_folder_members(sh, sh->cwd, &uids, &uid_count);
    // VYPIS IDECEK
    printf("VYPIS IDECEK ROOTU:\n");
    for(int i = 0; i < uid_count; i++)
    {
        printf("UID: %d\n", uids[i]);
    }

    // CREATE MULTIPLE FOLDER
    create_folder(sh, "folder1");
    create_folder(sh, "folder2");
    create_folder(sh, "folder3");

    // IDCKA
    *uids = NULL;
    uid_count = -1;
    get_folder_members(sh, sh->cwd, &uids, &uid_count);
    // VYPIS IDECEK
    printf("VYPIS IDECEK ROOTU:\n");
    for(int i = 0; i < uid_count; i++)
    {
        printf("(%d) -> %s\n", uids[i], find_mft_item_by_uid(sh,uids[i])->item_name);
    }

    printf("\n\n");

    //  VYTVORENI SLOZEK V JINYCH SLOZKACH
    //  prikaz: cd /folder1
    sh->cwd = 2;
    *uids = NULL;
    uid_count = -1;
    get_folder_members(sh, sh->cwd, &uids, &uid_count);
    // VYPIS IDECEK
    printf("VYPIS IDECEK FOLDER1:\n");
    for(int i = 0; i < uid_count; i++)
    {
        printf("(%d) -> %s\n", uids[i], find_mft_item_by_uid(sh,uids[i])->item_name);
    }

    create_folder(sh,"test1");
    create_folder(sh,"test2");
    create_folder(sh,"test3");

    uids = NULL;
    uid_count = -1;
    get_folder_members(sh, sh->cwd, &uids, &uid_count);
    // VYPIS IDECEK
    printf("VYPIS IDECEK FOLDER1:\n");
    for(int i = 0; i < uid_count; i++)
    {
        printf("(%d) -> %s\n", uids[i], find_mft_item_by_uid(sh,uids[i])->item_name);
    }

    bitmap = read_bitmap(sh->filename, sh->boot);
    print_bitmap(bitmap, sh->boot);

    printf("\n");

    // TEST PATH
    printf("/ -> %d\n",path_exist(sh, "/"));
    printf("/folder1 -> %d\n",path_exist(sh, "/folder1"));
    printf("/folder1/ -> %d\n",path_exist(sh, "/folder1/"));
    printf("/folder1/test3 -> %d\n",path_exist(sh, "/folder1/test3"));
    printf("/folder1/test3/ -> %d\n",path_exist(sh, "/folder1/test3/"));
    printf("/folder1/test4 -> %d\n",path_exist(sh, "/folder1/test4"));

    return 0;
}

// Obecny test systemu - netrideny
int test(int argc, char *argv[]) {
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

    printf("\n\n");

    // Prvni volny cluster
    int32_t *first_empty_addr = malloc(sizeof(int32_t));
    int32_t *first_empty_iter = malloc(sizeof(int32_t));
    find_empty_cluster(sh, first_empty_addr, first_empty_iter);
    printf("FIRST EMPTY CLUSTER ADDR: %d\n", *first_empty_addr);
    printf("FIRST EMPTY CLUSTER BITMAP INDEX: %d\n", *first_empty_iter);

    printf("\n\n");

    // Nalezeni prvniho volneho UID
    int32_t next_uid = get_next_uid(sh);
    printf("NEXT FREE UID: %d\n", next_uid);

    printf("\n\n");


    // Vytvoreni slozky
    printf("CREATING FOLDER NAMED TEST\n");
    int operation_code = create_folder(sh, "test");
    printf("CREATE OPERATION CODE: %d\n", operation_code);

    printf("\n\n");

    // Precteni souboru shell2
    shell *shell2 = create_root_shell(record, argv[1]);
    printf("READING BITMAP AFTER FOLDER CREATE: \n");
    int *bitmap2 = read_bitmap(argv[1], record);
    print_bitmap(bitmap2, record);
    printf("\n");

    printf("READING MFT AFTER FOLDER CREATE: \n");
    mft_item *mft_array2 = NULL;
    int mft_array_size2 = -1;
    read_mft_items(argv[1], record, &mft_array2, &mft_array_size2);
    print_mft_items(mft_array2, mft_array_size2);
    printf("\n\n\n\n\n");

    int *uids2 = NULL;
    int uid_count2 = -1;
    get_folder_members(sh, 2, &uids2, &uid_count2);
    // VYPIS IDECEK
    printf("VYPIS IDECEK TESTU:\n");
    for(int i = 0; i < uid_count2; i++)
    {
        printf("UID: %d\n", uids2[i]);
    }


    printf("\n\n");
    printf("\n\n");

    // Test cesty
    shell *shell3 = create_root_shell(record, argv[1]);
    printf("VYPISUJI CESTU ROOTU: \n");
    printf("%s\n", get_current_path(shell3));

    printf("VYPISUJI CESTU TEST: \n");
    shell3->cwd = 2;
    printf("%s\n", get_current_path(shell3));

    printf("\n\n");
    printf("\n\n");

    // Vytvarim slozku ahoj ve slozce TEST
    create_folder(shell3, "ahoj");

    printf("READING MFT AFTER FOLDER CREATE: \n");
    mft_item *mft_array3 = NULL;
    int mft_array_size3 = -1;
    read_mft_items(argv[1], record, &mft_array3, &mft_array_size3);
    print_mft_items(mft_array3, mft_array_size3);

    printf("VYPISUJI CESTU AHOJ: \n");
    shell3->cwd = 3;
    printf("%s\n", get_current_path(shell3));


    printf("\n\n");

    // EXISTENCE CESTY
    printf("EXISTUJE CESTA K ROOTU: %d\n", path_exist(shell3, "/"));
    printf("\n");
    printf("EXISTUJE CESTA K TEST: %d\n", path_exist(shell3, "/test/"));
    printf("\n");
    printf("EXISTUJE CESTA K TEST2: %d\n", path_exist(shell3, "/test"));
    printf("\n");
    printf("EXISTUJE CESTA K AHOJ: %d\n", path_exist(shell3, "/test/ahoj"));
    printf("\n");
    printf("EXISTUJE CESTA K SVETE: %d\n", path_exist(shell3, "/svete/"));
    printf("\n");
    printf("EXISTUJE CESTA K CURRENT WORKING DIRECTORY (%s): %d\n", get_current_path(shell3), path_exist(shell3, get_current_path(shell3)));
    printf("\n");

    printf("\n\n");

    printf("NTFS END!\n");

    return 0;
}